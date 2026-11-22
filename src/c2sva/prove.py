"""Unbounded proof stage (A1): run on a finished case directory, after the bounded check.

  1. candidates   invariants.Generator: C loop invariants / loop assigns / variants mapped to
                  RTL registers, contract frame during the call, HLS-structural facts
  2. Houdini      drop candidates violated within the BMC depth (base; capped at 36 steps, a
                  pure filter: the final k-induction checks its own base case), then iterate
                  k-induction on the candidate set, dropping the candidates a
                  counterexample-to-induction violates, until the set is inductive
                  (certified invariants I) or empty
  3. contract     k-induction and IC3/PDR (abc pdr) on the contract, each without and
                  with I assumed. Assuming I is sound: I was proved inductive from reset,
                  so it only removes unreachable states.
The harness is the sequential-call harness (harness_acc.sv; access checks are not
compiled in). An unbounded PASS covers every call sequence from reset, every latency and
any number of calls, for the modelled memory size N.
"""
import json
import os
import re
import signal
import subprocess
import time
from pathlib import Path

from . import acsl, formal, hls, invariants

ENG = {"kind": "smtbmc yices", "kind-bw": "smtbmc bitwuzla", "pdr": "abc pdr", "bmc-all": "smtbmc yices"}


def _sby(rundir, rtl, harness_text, connects, mode, depth, engine, timeout):
    """-> (status PASS|FAIL|UNKNOWN|TIMEOUT|ERROR, failed harness lines, secs, phase of failure)"""
    rundir.mkdir(parents=True, exist_ok=True)
    (rundir / "h.sv").write_text(harness_text)
    reads = "\n".join(f"read -formal {Path(f).name}" for f in rtl)
    flat = "hierarchy -top harness\nproc\nflatten\n" + "\n".join(connects) + "\n" if connects else ""
    files = "\n".join(str(f) for f in rtl) + f"\n{rundir / 'h.sv'}"
    (rundir / "p.sby").write_text(f"""[options]
mode {mode}
depth {depth}

[engines]
{ENG[engine]}

[script]
{reads}
read -formal h.sv
{flat}prep -top harness
{formal.power_on_cmd(rtl)}
[files]
{files}
""")
    t0 = time.time()
    p = subprocess.Popen(["sby", "-f", "p.sby"], cwd=rundir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                         text=True, start_new_session=True,
                         preexec_fn=formal._limit_memory(formal.MEM_LIMIT))
    try:
        out, _ = p.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        os.killpg(p.pid, signal.SIGKILL)
        p.communicate()
        return "TIMEOUT", [], round(time.time() - t0, 1), None
    (rundir / "sby.log").write_text(out)
    m = re.search(r"DONE \((\w+)", out)
    st = m.group(1) if m else "ERROR"
    if st == "ERROR" and re.search(r"bad_alloc|MemoryError|[Oo]ut of memory|Cannot allocate|Killed", out):
        st = "TIMEOUT"
    failed = [(ph, int(n)) for ph, n in
              re.findall(r"engine_\d+(?:\.(\w+))?: ##.*?Assert failed in harness: h\.sv:(\d+)\.", out)]
    if st == "ERROR":
        err = [l for l in out.splitlines() if "ERROR" in l][:3]
        return "ERROR", err, round(time.time() - t0, 1), None
    return st, [n for _, n in failed], round(time.time() - t0, 1), (failed[0][0] if failed else None)


KMIN_LADDER = [2, 4, 6, 8, 12, 16, 24, 32]     # overridable (fine ladders for the flush-prefix study)


def houdini(gen, cands, base_text, rtl, wdir, bmc_depth, k, timeout, log, refuted=None, assumed=()):
    """-> (invariants, stats). `refuted`: ids of candidates a BMC trace already falsified (false
    for every k); updated in place, so later depths skip them."""
    refuted = set() if refuted is None else refuted
    cur = [c for c in cands if c.id not in refuted]
    stats = {"candidates": len(cands), "base_dropped": len(cands) - len(cur), "ind_dropped": 0, "rounds": 0,
             "status": None}
    for r in range(60):                                         # base: within the BMC depth
        if not cur:
            break
        text, lmap = invariants.emit(base_text, gen, cur, "assert", keep_contract=False, assumed=assumed)
        st, failed, secs, _ = _sby(wdir / f"base{r}", rtl, text, gen.connects(), "bmc", bmc_depth, "bmc-all",
                                   timeout)
        stats["rounds"] += 1
        log(f"    houdini base {r}: {st} {len(cur)} cands depth {bmc_depth}, {len(failed)} violated ({secs}s)")
        if st == "PASS":
            break
        if st == "TIMEOUT" and bmc_depth > max(k, 8):
            bmc_depth = max(k, 8, bmc_depth // 2)               # the base filter is only a filter
            continue
        bad = {lmap[n] for n in failed if n in lmap}
        if st != "FAIL" or not bad:
            stats["status"] = f"base {st.lower()}"
            return [], stats
        stats["base_dropped"] += len(bad)
        refuted |= bad
        cur = [c for c in cur if c.id not in bad]
    stats["base_depth"] = bmc_depth
    for r in range(200):                                        # induction: drop CTI-violated
        if not cur:
            stats["status"] = "empty"
            return [], stats
        text, lmap = invariants.emit(base_text, gen, cur, "assert", keep_contract=False, assumed=assumed)
        st, failed, secs, phase = _sby(wdir / f"ind{r}", rtl, text, gen.connects(), "prove", k, "kind", timeout)
        stats["rounds"] += 1
        log(f"    houdini ind {r}: {st} {len(cur)} cands, {len(failed)} violated ({secs}s)")
        if st == "PASS":
            stats["status"] = "inductive"
            return cur, stats
        bad = {lmap[n] for n in failed if n in lmap}
        if not bad:
            stats["status"] = f"induction {st.lower()}"
            return [], stats
        stats["ind_dropped"] += len(bad)
        cur = [c for c in cur if c.id not in bad]
    stats["status"] = "round limit"
    return [], stats


def prove_case(d: Path, fn, itf, rtl, ks=(4, 8, 12, 16), timeout=300, pdr_timeout=600, log=print, strategies=None):
    """d: case directory with result.json and formal/harness_acc.sv.
    ks: k-induction depths tried in order (a k-inductive set stays inductive for larger k, so
    the first depth at which the contract is proved is reported). Results are merged into an
    existing prove.json (strategies not run keep their earlier result)."""
    r = json.loads((d / "result.json").read_text())
    wdir = d / "formal" / "prove"
    wdir.mkdir(parents=True, exist_ok=True)
    base_text = (d / "formal" / "harness_acc.sv").read_text()
    W, L = r.get("width") or 64, r.get("latency") or 8
    old = json.loads((d / "prove.json").read_text()) if (d / "prove.json").exists() else {}
    out = {k: v for k, v in old.items() if k in ("kind", "pdr", "pdr+inv", "candidates", "houdini", "kind+inv",
                                                 "ablation")}
    out.update(latency=L, ks=list(ks))
    strategies = strategies or ["kind", "pdr", "houdini", "kind+inv", "pdr+inv"]
    t0 = time.time()
    if "kind" in strategies:
        for k in ks:
            st, _, secs, ph = _sby(wdir / f"kind_k{k}", rtl, base_text, [], "prove", k, "kind", timeout)
            out["kind"] = {"status": st, "secs": secs, "k": k}
            log(f"    kind k={k}: {st} ({secs}s)")
            if st in ("PASS", "FAIL", "TIMEOUT", "ERROR"):
                break
    if "pdr" in strategies:
        st, _, secs, _ = _sby(wdir / "pdr", rtl, base_text, [], "prove", ks[0], "pdr", pdr_timeout)
        out["pdr"] = {"status": st, "secs": secs}
        log(f"    pdr: {st} ({secs}s)")
    if "houdini" in strategies:
        sigs = invariants.rtl_signals(rtl, itf.module, wdir)
        norm = (d / "src" / "framac_norm.c").read_text(errors="replace")
        cutoff = max([p.size for p in fn.params if p.is_array], default=4) + 2
        extra = [f.read_text(errors="replace") for f in sorted((d / "src" / "hls_src").glob("*.c"))]
        aliases = invariants.schedule_aliases(d / "hls")
        gen = invariants.Generator(fn, itf, W, norm, sigs, L, cutoff, extra_sources=extra, aliases=aliases)
        cands = gen.generate()
        out["candidates"] = {"total": len(cands), "by_origin": _count(cands), "loops": len(gen.loops),
                             "loop_invariants": sum(len(lp.invariants) for lp in gen.loops),
                             "skipped": gen.skipped[:30]}
        log(f"    candidates: {len(cands)} {out['candidates']['by_origin']}; {len(gen.skipped)} clauses not mapped")
        t = time.time()
        tried, refuted = [], set()
        for k in ks:
            inv, hs = houdini(gen, cands, base_text, rtl, wdir / f"houdini_k{k}", min(L + 4, 36), k, timeout, log,
                              refuted)
            hs.update(k=k, secs=round(time.time() - t, 1), invariants=len(inv), by_origin=_count(inv),
                      c_invariants_survived=sorted({c.source.split("  [")[0] for c in inv
                                                    if c.origin in ("loop-invariant", "loop-invariant-lag")}),
                      c_invariants_lagged=sorted({c.source.split("  [")[0] for c in inv
                                                  if c.origin == "loop-invariant-lag"}))
            out["houdini"] = hs
            (wdir / "invariants.json").write_text(json.dumps([c.__dict__ for c in inv], indent=1))
            if not inv:
                out["kind+inv"] = {"status": "no invariants", "k": k}
                tried.append(k)
                if str(hs.get("status")).startswith("base"):
                    break                  # the base filter does not depend on k: do not retry
                continue
            text, _ = invariants.emit(base_text, gen, inv, "assume", keep_contract=True)
            (wdir / "harness_inv.sv").write_text(text)
            st, _, secs, _ = _sby(wdir / f"kind_inv_k{k}", rtl, text, gen.connects(), "prove", k, "kind", timeout)
            out["kind+inv"] = {"status": st, "secs": secs, "k": k}
            log(f"    k={k}: houdini {hs['status']} {len(inv)}/{len(cands)}; kind+inv {st} ({secs}s)")
            tried.append(k)
            if st in ("PASS", "FAIL"):
                break
        out["houdini"]["k_tried"] = tried
        k = out["houdini"]["k"]
        if "pdr+inv" in strategies and inv and (out["kind+inv"]["status"] != "PASS" or "pdr+inv!" in strategies):
            st, _, secs, _ = _sby(wdir / "pdr_inv", rtl, text, gen.connects(), "prove", k, "pdr", pdr_timeout)
            out["pdr+inv"] = {"status": st, "secs": secs}
            log(f"    pdr+inv: {st} ({secs}s)")
        # ablation: which candidate families the k-induction proof needs
        if "ablation" in strategies and out["kind+inv"]["status"] == "PASS":
            out["ablation"] = {}
            for name, fams in (("hls", {"structural", "pipeline-copy"}),
                               ("hls+contract", {"structural", "pipeline-copy", "call-requires", "call-frame", "contract-persist"})):
                sub = [c for c in cands if c.origin in fams]
                inv2, hs2 = houdini(gen, sub, base_text, rtl, wdir / f"abl_{name}", min(L + 4, 36), k, timeout, log,
                                    set(refuted))
                res = {"candidates": len(sub), "invariants": len(inv2), "houdini": hs2.get("status"), "k": k}
                if inv2:
                    t2, _ = invariants.emit(base_text, gen, inv2, "assume", keep_contract=True)
                    st, _, secs, _ = _sby(wdir / f"abl_{name}_kind", rtl, t2, gen.connects(), "prove", k, "kind",
                                          timeout)
                    res.update(status=st, secs=secs)
                else:
                    res["status"] = "no invariants"
                out["ablation"][name] = res
                log(f"    ablation {name}: {res}")
    # minimal induction depth per candidate pool (fair ablation: the C artefacts may not be
    # needed at a large k, but shorten the proof; k_min as a function of N is the measure)
    if "kmin" in strategies and "houdini" in strategies:
        ladder = KMIN_LADDER
        pools = {"none": set(), "hls": {"structural", "pipeline-copy"},
                 "hls+contract": {"structural", "pipeline-copy", "call-requires", "call-frame", "contract-persist"},
                 "all": None}
        if "kmin-progress" in strategies:          # + call-age facts (h_cyc bounded by the loop counter)
            pools["all+progress"] = "progress"
        res = {}
        refuted_all = set()
        for name, fams in pools.items():
            res[name] = None
            if fams == "progress":
                sub = list(cands) + gen.progress(L)
            else:
                sub = [c for c in cands if fams is None or c.origin in fams]
            for k in ladder:
                if name == "none":
                    st, _, secs, _ = _sby(wdir / f"kmin_none_k{k}", rtl, base_text, [], "prove", k, "kind", timeout)
                else:
                    inv_p, hs_p = houdini(gen, sub, base_text, rtl, wdir / f"kmin_{name}_k{k}", min(L + 4, 36), k,
                                          timeout, log, refuted_all)
                    if not inv_p:
                        if "timeout" in str(hs_p.get("status")):
                            res[name] = f"houdini timeout at k={k}"
                            break
                        continue
                    t2, _ = invariants.emit(base_text, gen, inv_p, "assume", keep_contract=True)
                    st, _, secs, _ = _sby(wdir / f"kmin_{name}_kind_k{k}", rtl, t2, gen.connects(), "prove", k,
                                          "kind", timeout)
                if st == "PASS":
                    res[name] = k
                    break
                if st in ("FAIL", "TIMEOUT", "ERROR"):
                    res[name] = f"{st.lower()} at k={k}"
                    break
            log(f"    k_min {name}: {res[name]}")
        out["kmin"] = res

    # RQ3: unbounded latency bound (every call completes within B cycles, for all executions)
    lat_file = d / "formal" / "harness_lat.sv"
    if "latency" in strategies and lat_file.exists() and "houdini" in strategies:
        lat_text = lat_file.read_text()
        m = re.search(r"assert\(h_cyc <= (\d+)\)", lat_text)
        B = int(m.group(1)) if m else L
        res = {"bound": B, "established_by_bmc": r.get("latency_status")}
        for k in ks:
            st, _, secs, _ = _sby(wdir / f"lat_kind_k{k}", rtl, lat_text, [], "prove", k, "kind", timeout)
            res["kind"] = {"status": st, "secs": secs, "k": k}
            if st in ("PASS", "FAIL", "TIMEOUT", "ERROR"):
                break
        prog = gen.progress(B)
        res["candidates"] = len(prog)
        t = time.time()
        refuted = set()
        for k in ks:
            P, hs = houdini(gen, prog, lat_text, rtl, wdir / f"lat_houdini_k{k}", min(B + 4, 36), k, timeout, log,
                            refuted, assumed=inv)
            text, _ = invariants.emit(lat_text, gen, P, "assume", keep_contract=True, assumed=inv)
            st, _, secs, _ = _sby(wdir / f"lat_inv_k{k}", rtl, text, gen.connects(), "prove", k, "kind", timeout)
            res["kind+inv"] = {"status": st, "secs": secs, "k": k, "progress_invariants": len(P),
                               "houdini": hs.get("status"), "houdini_secs": round(time.time() - t, 1)}
            log(f"    latency k={k}: {len(P)} progress invariants; bound {B}: {st}")
            if st in ("PASS", "FAIL"):
                break
        res["proved"] = res["kind"]["status"] == "PASS" or res["kind+inv"]["status"] == "PASS"
        out["latency_proof"] = res
    proved = [s for s in ("kind", "kind+inv", "pdr", "pdr+inv") if out.get(s, {}).get("status") == "PASS"]
    out["unbounded"] = bool(proved)
    out["proved_by"] = proved
    # a FAIL in prove mode = a counterexample from reset (beyond the BMC depth if BMC passed)
    out["deep_cex"] = [s for s in ("kind", "kind+inv", "pdr", "pdr+inv") if out.get(s, {}).get("status") == "FAIL"]
    out["secs"] = round(time.time() - t0, 1)
    (d / "prove.json").write_text(json.dumps(out, indent=2))
    return out


def _count(cands):
    by = {}
    for c in cands:
        by[c.origin] = by.get(c.origin, 0) + 1
    return by


def load_case(d: Path, array_size=4):
    """Rebuild (fn, itf, rtl files) from a case directory without re-running HLS or Frama-C."""
    r = json.loads((d / "result.json").read_text())
    norm = (d / "src" / "framac_norm.c").read_text(errors="replace")
    fn = acsl.parse_function(norm, r["top"], array_size=array_size)
    itf = hls.TOOLS[r["tool"]][1](d / "hls", fn, r["config"])
    sbys = [d / "formal" / "check.sby"] + sorted((d / "formal").glob("check_*.sby"))
    sby = next(f for f in sbys if f.exists()).read_text()     # one per engine since the parallel portfolio
    files = sby.split("[files]")[1].split()
    rtl = [Path(f) for f in files if f.endswith(".v")]
    return fn, itf, rtl
