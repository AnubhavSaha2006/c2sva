"""c2sva: ACSL contracts -> RTL assertions for HLS designs, checked formally.

  c2sva run    --bench {demo|abe|acslbench|PATH} [selection] [--tool vitis,bambu] [--config base,...]
  c2sva list   --bench ...                      list the designs a selection resolves to
  c2sva report RUN_DIR                          rebuild summary tables from a run directory
  c2sva configs                                 list the named HLS configurations

Examples
  c2sva run --bench demo --tool vitis,bambu --mutants 3
  c2sva run --bench abe --select 'MinMax/*,Nonmutating/*' --tool vitis
  c2sva run --bench acslbench --proven-only --sample 50 --seed 1 --tool vitis --mutants 2
  c2sva run --bench my_designs/ -I my_designs/include --tool vitis --config base,cyclic2,nopipe
"""
import argparse
import fnmatch
import json
import re
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

from rich.console import Console
from rich.table import Table

from . import acsl, bench, frontend, hls, mutate, pipeline, report

ROOT = Path(__file__).resolve().parents[2]
con = Console(width=160)


def _safe(s):
    return re.sub(r"[^\w.-]+", "__", s)


def plan_cases(specs, args, st):
    """Expand designs into cases: original + automatic mutants, x tools x configs."""
    tools = args.tool.split(",")
    configs = args.config.split(",")
    cases, problems = [], {}

    def expand(spec):
        src = Path(spec.files[0]).read_text(errors="replace")
        variants = [("original", src)]
        if args.mutants:
            top = spec.top
            if not top:
                tmp = Path(args.out) / "_probe" / _safe(spec.id)
                try:
                    top = frontend.prepare(spec, src, tmp, st.array_size).fn.name
                except (acsl.Unsupported, Exception) as e:
                    return spec, variants, str(e)
            for i, (desc, msrc) in enumerate(mutate.mutants(src, top, args.mutants), 1):
                variants.append((f"mutant-{i}", msrc))
                spec.meta.setdefault("mutants", {})[f"mutant-{i}"] = desc
        return spec, variants, None

    with ThreadPoolExecutor(args.jobs) as ex:
        for spec, variants, err in ex.map(expand, specs):
            if err:
                problems[spec.id] = err
            for tool in tools:
                for cfg in configs:
                    if cfg not in hls.TOOLS[tool][2]:
                        continue
                    for vname, vsrc in variants:
                        cases.append((spec, vname, vsrc, tool, cfg))
    return cases, problems


def _apply_lifted(path):
    """Validated LLM-lifted definitions (c2sva lift), per design (pipeline.run_case installs the
    design's own overrides in its thread)."""
    if not path:
        return
    pipeline.LIFTED.clear()
    pipeline.LIFTED.update(json.loads(Path(path).read_text()).get("lifted", {}))


def _power_on(args):
    from . import formal
    formal.POWER_ON = getattr(args, "power_on", "fpga")


def cmd_run(args):
    _power_on(args)
    _apply_lifted(getattr(args, "lifted", None))
    st = pipeline.Settings(array_size=args.array_size, lat_cap=args.lat_cap, engines=args.engines.split(","),
                           sby_timeout=args.sby_timeout, wp=not args.no_wp, access=not args.no_access,
                           replay=not args.no_replay)
    specs = bench.load(args.bench, args.include, args.select, args.ids, args.sample, args.seed, args.proven_only)
    if not specs:
        con.print("[red]selection is empty[/]")
        return 1
    name = args.name or f"{_safe(Path(args.bench).name)}_{time.strftime('%Y%m%d-%H%M%S')}"
    out = Path(args.out) / name
    args.out = str(out)
    out.mkdir(parents=True, exist_ok=True)
    meta = {s.id: s.meta for s in specs}
    cases, problems = plan_cases(specs, args, st)
    (out / "run.json").write_text(json.dumps({"args": vars(args), "settings": vars(st),
                                              "designs": [s.id for s in specs]}, indent=2, default=str))
    con.print(f"[bold]{len(specs)} designs -> {len(cases)} cases[/] "
              f"(tools {args.tool}, configs {args.config}, mutants {args.mutants}) -> {out}")

    def job(c):
        spec, vname, vsrc, tool, cfg = c
        d = out / "cases" / _safe(spec.id) / tool / cfg / vname
        if args.resume and (d / "result.json").exists():          # finished in an earlier invocation
            try:
                return json.loads((d / "result.json").read_text())
            except ValueError:
                pass
        return pipeline.run_case(spec, vname, vsrc, tool, cfg, st, d)

    done = 0
    with ThreadPoolExecutor(args.jobs) as ex:
        futs = [ex.submit(job, c) for c in cases]
        for f in as_completed(futs):
            r = f.result()
            done += 1
            tag = r["rtl"] or r["status"]
            rp = (r.get("replay") or {}).get("class", "")
            con.print(f"[{done}/{len(cases)}] {r['design']:32s} {r['variant']:9s} {r['config']:8s} "
                      f"C {str(r['c_proof']):7s} {r.get('outcome', tag):16s} clauses {str(r['clauses']):6s} "
                      f"access {str(r['access']):9s} lat {r['latency']} {rp}")
    (out / "meta.json").write_text(json.dumps(meta, indent=2, default=str))
    results = report.load_results(out / "cases")
    S = report.write(out, results, meta)
    show(S, out)
    return 0


def show(S, out):
    t = Table(title="Summary per tool / HLS configuration")
    cols = ["tool", "config", "designs", "checked", "pass_full", "pass_partial", "fail", "bounded", "vacuous",
            "agree", "disagree", "clause_rate", "access_speculative", "access_out_of_memory", "overlap_mode", "mutants_checked", "killed",
            "mutation_score", "equivalent_pass", "kills_confirmed_in_C"]
    for c in cols:
        t.add_column(c.replace("_", " "))
    for row in S["by_tool_config"]:
        t.add_row(*[str(row[c]) for c in cols])
    con.print(t)
    for k, d in S["details"].items():
        con.print(f"[bold]{k}[/] failures on originals (C replay): {dict(d['failure_replay'])}; "
                  f"mutant kills (C replay): {dict(d['kill_replay'])}")
    if S["not_checked_reasons"]:
        con.print("[bold]Originals not checked:[/]")
        for k, n in S["not_checked_reasons"][:12]:
            con.print(f"  {n:4d}  {k}")
    con.print(f"Details: {out}/summary.md, {out}/results.csv, {out}/table.tex")


def cmd_list(args):
    for s in bench.load(args.bench, args.include, args.select, args.ids, args.sample, args.seed, args.proven_only):
        con.print(f"{s.id:40s} top={s.top or '(auto)'}  ref={s.meta.get('reference_proven')}")
    return 0


def cmd_report(args):
    out = Path(args.run_dir)
    meta = json.loads((out / "meta.json").read_text()) if (out / "meta.json").exists() else {}
    S = report.write(out, report.load_results(out / "cases"), meta)
    show(S, out)
    return 0


def cmd_translate(args):
    """Translation coverage without HLS (front end + rules on a synthetic interface)."""
    import collections
    from . import dryrun
    specs = bench.load(args.bench, args.include, args.select, args.ids, args.sample, args.seed, args.proven_only)
    out = Path(args.out) / (args.name or f"translate_{_safe(Path(args.bench).name)}")
    res = dryrun.run(specs, out, args.jobs, args.array_size)
    ok = [r for r in res if r["status"] == "ok"]
    tr = sum(r["translated"] for r in ok)
    sk = sum(len(r["skipped"]) for r in ok)
    con.print(f"{len(res)} designs: {dict(collections.Counter(r['status'] for r in res))}; "
              f"clauses translated {tr}/{tr + sk} ({100 * tr / max(1, tr + sk):.1f}%)")
    for k, n in collections.Counter(r.get("reason", "")[:80] for r in res if r["status"] != "ok").most_common(12):
        con.print(f"  design {n:4d}  {k}")
    for k, n in collections.Counter(x for r in ok for x in r["skipped"]).most_common(12):
        con.print(f"  clause {n:4d}  {k}")
    con.print(f"Details: {out}/translate.jsonl")
    return 0


def cmd_revalidate(args):
    """Re-run the C-level replay of every RTL failure of a run from its saved artefacts."""
    from . import hls as H, replay as R
    out = Path(args.run_dir).resolve()
    run = json.loads((out / "run.json").read_text())
    a, st = run["args"], run["settings"]
    specs = {s.id: s for s in bench.load(a["bench"], a.get("include") or [], None, None, None, 0, False)}
    todo = []
    for f in sorted((out / "cases").rglob("result.json")):
        r = json.loads(f.read_text())
        if r.get("rtl") == "FAIL" and r.get("design") in specs:
            todo.append((f, r))

    def job(item):
        f, r = item
        d = f.parent
        try:
            spec = specs[r["design"]]
            src = (d / "src" / Path(spec.files[0]).name).read_text(errors="replace")
            prep = frontend.prepare(spec, src, d / "src_revalidate", st.get("array_size", 4))
            itf = H.TOOLS[r["tool"]][1](d / "hls", prep.fn, r["config"])
            r["replay"] = R.replay(prep, prep.fn, itf, d / "formal" / "check_bmc", d / "replay",
                                   fail_step=r.get("cex_step"))
        except Exception as e:
            r["replay"] = {"class": "replay-error", "detail": f"revalidate: {str(e)[:150]}"}
        f.write_text(json.dumps(r, indent=2, default=str))
        return r["replay"].get("class")

    import collections
    with ThreadPoolExecutor(args.jobs) as ex:
        classes = collections.Counter(ex.map(job, todo))
    con.print(f"revalidated {len(todo)} failing cases: {dict(classes)}")
    meta = json.loads((out / "meta.json").read_text()) if (out / "meta.json").exists() else {}
    show(report.write(out, report.load_results(out / "cases"), meta), out)
    return 0


def cmd_survivors(args):
    """Bounded random testing of surviving mutants (RTL pass, WP fails) -> classify survivors."""
    from . import hls as H, survivors as SV
    out = Path(args.run_dir).resolve()
    run = json.loads((out / "run.json").read_text())
    a, st = run["args"], run["settings"]
    specs = {s.id: s for s in bench.load(a["bench"], a.get("include") or [], None, None, None, 0, False)}
    todo = []
    for f in sorted((out / "cases").rglob("result.json")):
        r = json.loads(f.read_text())
        if r["variant"] != "original" and r.get("c_ok") is False and (r.get("outcome") or "").startswith("pass") \
                and r["design"] in specs and (args.redo or not r.get("survivor")):
            todo.append((f, r))

    def job(item):
        f, r = item
        d = f.parent
        try:
            spec = specs[r["design"]]
            src = (d / "src" / Path(spec.files[0]).name).read_text(errors="replace")
            prep = frontend.prepare(spec, src, d / "src_survivor", st.get("array_size", 4))
            H.TOOLS[r["tool"]][1](d / "hls", prep.fn, r["config"])          # struct layouts
            r["survivor"] = SV.analyse(prep, prep.fn, d / "survivor", args.vectors,
                                       untranslated=[x["source"] for x in r.get("skipped") or []])
        except Exception as e:
            r["survivor"] = {"class": "analysis-error", "detail": str(e)[:150]}
        f.write_text(json.dumps(r, indent=2, default=str))
        return r["survivor"]["class"]

    import collections
    with ThreadPoolExecutor(args.jobs) as ex:
        classes = collections.Counter(ex.map(job, todo))
    con.print(f"analysed {len(todo)} surviving mutants: {dict(classes)}")
    meta = json.loads((out / "meta.json").read_text()) if (out / "meta.json").exists() else {}
    show(report.write(out, report.load_results(out / "cases"), meta), out)
    return 0


def cmd_prove(args):
    """A1: unbounded proofs (k-induction / IC3) with C loop invariants carried to the RTL."""
    _power_on(args)
    from . import prove as PV
    out = Path(args.run_dir).resolve()
    st = json.loads((out / "run.json").read_text())["settings"] if (out / "run.json").exists() else {}
    todo = []
    sel = [s.strip() for s in (args.select or "").split(",") if s.strip()]
    for f in sorted((out / "cases").rglob("result.json")):
        r = json.loads(f.read_text())
        if r.get("status") != "ok" or r.get("rtl") not in ("PASS", "TIMEOUT"):
            continue                       # only cases without a bounded counterexample
        if args.originals and r.get("variant") != "original":
            continue
        if sel and not any(fnmatch.fnmatch(r["design"], p) for p in sel):
            continue
        if args.resume and (f.parent / "prove.json").exists():
            continue
        todo.append(f.parent)
    strategies = args.strategies.split(",")
    con.print(f"prove: {len(todo)} cases, strategies {strategies}")

    def job(d):
        try:
            fn, itf, rtl = PV.load_case(d, st.get("array_size", 4))
            o = PV.prove_case(d, fn, itf, rtl, ks=tuple(int(x) for x in args.k.split(",")), timeout=args.timeout, pdr_timeout=args.pdr_timeout,
                              log=lambda m: None, strategies=strategies)
        except Exception as e:
            o = {"error": str(e)[:300], "unbounded": False}
            (d / "prove.json").write_text(json.dumps(o, indent=2))
        h = o.get("houdini") or {}
        con.print(f"{str(d.relative_to(out / 'cases')):70s} unbounded={o.get('unbounded')} by={o.get('proved_by')} "
                  f"inv={h.get('invariants')}/{h.get('candidates')} c-inv={len(h.get('c_invariants_survived') or [])} "
                  f"{('ERROR ' + o['error'][:80]) if 'error' in o else ''}")
        return o

    with ThreadPoolExecutor(args.jobs) as ex:
        res = list(ex.map(job, todo))
    con.print(f"unbounded: {sum(1 for o in res if o.get('unbounded'))}/{len(res)}")
    return 0


def cmd_prove_report(args):
    from . import prove_report
    s, text = prove_report.summarize(Path(args.run_dir).resolve())
    con.print(text)
    return 0


def cmd_lift(args):
    """RQ6: LLM lifting of axiomatic/inductive definitions, validated by Frama-C/WP."""
    from . import llm
    out = Path(args.run_dir).resolve()
    ldir = out / "lift"
    be = llm.backend(args.backend, ldir / "exchange")
    rows, lifted = [], {}
    for f in sorted((out / "cases").rglob("result.json")):
        r = json.loads(f.read_text())
        if r.get("variant") != "original":
            continue
        names = sorted({m.group(1) for s_ in r.get("skipped") or []
                        for m in [re.search(r"(?:axiomatic|inductive|abstract) logic (\w+)", s_.get("reason", ""))] if m})
        if not names:
            continue
        src = (f.parent / "src" / "framac_norm.c").read_text(errors="replace")
        blocks = llm.axiomatic_blocks(src)
        for name in names:
            key = f"{r['design']}__{name}"
            uses = [s_["source"] for s_ in r.get("skipped") or [] if name in s_.get("source", "")]
            if name not in blocks:
                rows.append({"design": r["design"], "name": name, "status": "no-definition (abstract)"})
                continue
            text = be.complete(key, llm.prompt_for(name, blocks[name], uses))
            if text is None:
                rows.append({"design": r["design"], "name": name, "status": "awaiting-response"})
                continue
            defn, lemma = llm.parse_response(text, name)
            if not defn:
                rows.append({"design": r["design"], "name": name, "status": "unparsable-response"})
                continue
            v = llm.validate(src, name, defn, lemma, ldir / "wp" / key)
            tries = 0
            while v["status"] == "error" and tries < 2:     # repair loop: Frama-C error fed back
                tries += 1
                text = be.complete(f"{key}__r{tries}", llm.repair_prompt(name, text, v.get("detail") or []))
                if text is None:
                    v = {"status": "awaiting-repair", "detail": v.get("detail")}
                    break
                defn, lemma = llm.parse_response(text, name)
                if not defn:
                    v = {"status": "unparsable-response"}
                    break
                v = llm.validate(src, name, defn, lemma, ldir / "wp" / f"{key}__r{tries}")
            v["repairs"] = tries
            rows.append({"design": r["design"], "name": name, **v})
            if v["status"].startswith("validated"):
                lifted.setdefault(r["design"], {})[name] = llm.override_text(name, defn, v.get("domain") or "\\true")
    (ldir / "lift.json").write_text(json.dumps({"rows": rows, "lifted": lifted}, indent=2))
    import collections
    con.print(f"lift: {dict(collections.Counter(x['status'] for x in rows))}  -> {ldir / 'lift.json'}")
    return 0


def cmd_qed(args):
    """Baseline: specification-free functional consistency (A-QED style) on finished cases."""
    from . import baselines
    out = Path(args.run_dir).resolve()
    todo = []
    for f in sorted((out / "cases").rglob("result.json")):
        r = json.loads(f.read_text())
        if r.get("status") == "ok" and r.get("rtl") in ("PASS", "FAIL", "TIMEOUT") and not (args.resume and (f.parent / "qed.json").exists()):
            todo.append((f.parent, r))

    def job(item):
        d, r = item
        try:
            q = baselines.qed_check(d, timeout=args.timeout)
        except Exception as e:
            q = {"status": "ERROR", "detail": str(e)[:200]}
            (d / "qed.json").write_text(json.dumps(q, indent=2))
        con.print(f"{r['design']:36s} {r['variant']:9s} contract={r.get('outcome'):22s} qed={q['status']}")
        return r, q

    with ThreadPoolExecutor(args.jobs) as ex:
        res = list(ex.map(job, todo))
    import collections
    tab = collections.Counter((r["variant"] == "original", str(r.get("outcome"))[:4], q["status"]) for r, q in res)
    con.print(dict(tab))
    return 0


def cmd_configs(args):
    for tool, (_, _, cfgs) in hls.TOOLS.items():
        for k, v in cfgs.items():
            con.print(f"{tool:6s} {k:10s} {v}")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(prog="c2sva", description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    def selection(p):
        p.add_argument("--bench", required=True, help="demo | abe | acslbench | path to .c file or directory")
        p.add_argument("-I", "--include", action="append", default=[], help="include dir (for --bench PATH)")
        p.add_argument("--select", help="comma-separated glob(s) over design ids, e.g. 'MinMax/*'")
        p.add_argument("--ids", help="comma-separated design ids")
        p.add_argument("--sample", type=int, help="random sample size")
        p.add_argument("--seed", type=int, default=0)
        p.add_argument("--proven-only", action="store_true", help="only designs whose contract is proven at C level")

    r = sub.add_parser("run")
    selection(r)
    r.add_argument("--tool", default="vitis", help="vitis,bambu")
    r.add_argument("--config", default="base", help="comma-separated HLS configurations (see `c2sva configs`)")
    r.add_argument("--mutants", type=int, default=0, help="automatic mutants per design")
    r.add_argument("--jobs", type=int, default=6)
    r.add_argument("--array-size", type=int, default=4, help="RAM cells for range pointers/arrays")
    r.add_argument("--lat-cap", type=int, default=128)
    r.add_argument("--engines", default="yices,bitwuzla", help="solver portfolio, tried in order on timeout")
    r.add_argument("--no-replay", action="store_true", help="skip C-level replay of RTL counterexamples")
    r.add_argument("--sby-timeout", type=int, default=600)
    r.add_argument("--no-wp", action="store_true", help="skip Frama-C WP on each variant")
    r.add_argument("--no-access", action="store_true", help="skip the memory-access check")
    r.add_argument("--name", help="run name (default: bench + timestamp)")
    r.add_argument("--resume", action="store_true", help="keep finished cases of an existing run directory")
    r.add_argument("--out", default=str(ROOT / "runs"))
    r.add_argument("--power-on", default="fpga", choices=["fpga", "arbitrary"],
                   help="power-on value of DUT registers without an initial value: 0 as on an FPGA (default) or arbitrary")
    r.add_argument("--lifted", help="lift.json of `c2sva lift`: use the validated LLM-lifted definitions")
    r.set_defaults(func=cmd_run)
    l = sub.add_parser("list")
    selection(l)
    l.set_defaults(func=cmd_list)
    rp = sub.add_parser("report")
    rp.add_argument("run_dir")
    rp.set_defaults(func=cmd_report)
    rv = sub.add_parser("revalidate", help="re-run the C-level replay of all RTL failures of a run")
    rv.add_argument("run_dir")
    rv.add_argument("--jobs", type=int, default=4)
    rv.set_defaults(func=cmd_revalidate)
    sv = sub.add_parser("survivors", help="classify surviving mutants by bounded random testing of the C code")
    sv.add_argument("run_dir")
    sv.add_argument("--vectors", type=int, default=1000)
    sv.add_argument("--redo", action="store_true", help="also re-analyse survivors classified before")
    sv.add_argument("--jobs", type=int, default=4)
    sv.set_defaults(func=cmd_survivors)
    t = sub.add_parser("translate", help="translation coverage only (no HLS / formal)")
    selection(t)
    t.add_argument("--jobs", type=int, default=4)
    t.add_argument("--array-size", type=int, default=4)
    t.add_argument("--name")
    t.add_argument("--out", default=str(ROOT / "runs"))
    t.set_defaults(func=cmd_translate)
    pv = sub.add_parser("prove", help="A1: unbounded proofs with C loop invariants mapped to RTL (finished runs)")
    pv.add_argument("run_dir")
    pv.add_argument("--select", help="design patterns, comma separated")
    pv.add_argument("--originals", action="store_true", help="original designs only (no mutants)")
    pv.add_argument("--strategies", default="kind,pdr,houdini,kind+inv,pdr+inv")
    pv.add_argument("-k", default="4,8,12,16", help="k-induction depths tried in order")
    pv.add_argument("--timeout", type=int, default=300)
    pv.add_argument("--pdr-timeout", type=int, default=600)
    pv.add_argument("--resume", action="store_true")
    pv.add_argument("--power-on", default="fpga", choices=["fpga", "arbitrary"])
    pv.add_argument("--jobs", type=int, default=1)
    pv.set_defaults(func=cmd_prove)
    lf = sub.add_parser("lift", help="RQ6: LLM-lift axiomatic definitions, validated by Frama-C/WP")
    lf.add_argument("run_dir")
    lf.add_argument("--backend", default="auto", choices=["auto", "files", "anthropic"])
    lf.set_defaults(func=cmd_lift)
    qd = sub.add_parser("qed", help="baseline: A-QED-style functional consistency (no specification)")
    qd.add_argument("run_dir")
    qd.add_argument("--jobs", type=int, default=2)
    qd.add_argument("--timeout", type=int, default=600)
    qd.add_argument("--resume", action="store_true")
    qd.set_defaults(func=cmd_qed)
    pr = sub.add_parser("prove-report", help="aggregate A1 results (prove.json) of a run")
    pr.add_argument("run_dir")
    pr.set_defaults(func=cmd_prove_report)
    c = sub.add_parser("configs")
    c.set_defaults(func=cmd_configs)
    args = ap.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
