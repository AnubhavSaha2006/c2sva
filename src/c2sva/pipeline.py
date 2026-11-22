"""One case = (design, variant source, HLS tool, HLS configuration) through every stage:

  frontend (Frama-C normalize, top, pointer->RAM)  ->  Frama-C WP on the variant
  ->  HLS  ->  interface model  ->  property IR  ->  harness + SVA
  ->  latency search (BMC depth)  ->  BMC functional  ->  BMC access  ->  cover
  ->  counterexample replay on C with an independent oracle (E-ACSL) for every RTL failure
Every stage's artefacts are kept in the case directory; the outcome is result.json.
"""
import json
import time
import traceback
from dataclasses import dataclass, field
from pathlib import Path

from . import acsl, formal, frontend, harness, hls, logic, translate

LIFTED = {}       # design id -> {logic name: validated lifted definition} (c2sva lift / run --lifted)


@dataclass
class Settings:
    array_size: int = 4          # memory cells for pointer/array parameters given as ranges
    lat_start: int = 8           # first latency bound tried when the report has none
    lat_cap: int = 128           # termination not established beyond this
    engines: list = field(default_factory=lambda: ["yices", "bitwuzla"])
    sby_timeout: int = 600
    wp: bool = True
    wp_timeout: int = 300
    access: bool = True
    replay: bool = True
    fifo_depth: int = 8          # overlapping calls tracked at once


def base_width(fn):
    """Integer width for ACSL 'integer' terms: enough for sums of products of two operands.
    Raised further when the contract nests more multiplications (see run_case)."""
    pw = max([p.width for p in fn.params] + [8])
    return max(64, fn.ret_width + 8, 2 * pw + 8)


def run_case(spec, variant, src, tool, hls_cfg, st: Settings, outdir: Path):
    outdir.mkdir(parents=True, exist_ok=True)
    r = {"design": spec.id, "variant": variant, "tool": tool, "config": hls_cfg, "stage": "frontend",
         "status": None, "c_proof": None, "c_ok": None, "rtl": None, "failed": [], "access": None,
         "access_failed": [], "cover": None, "latency": None, "latency_status": None, "clauses": None,
         "translated": None, "total_clauses": None, "skipped": [], "times": {}, "error": None, "top": None,
         "mode": None, "bounded_to": None, "replay": None, "engine": None}
    t0 = time.time()
    logic.set_overrides(LIFTED.get(spec.id, {}))
    if LIFTED.get(spec.id):
        r["lifted"] = sorted(LIFTED[spec.id])
    try:
        prep = frontend.prepare(spec, src, outdir / "src", st.array_size)
        fn = prep.fn
        r["top"] = fn.name
        r["times"]["frontend"] = round(time.time() - t0, 1)
        if st.wp:
            r["stage"] = "wp"
            t = time.time()
            res = frontend.framac_wp(prep.spec, [prep.spec.files[0]], fn.name, outdir / "wp.log",
                                     timeout=st.wp_timeout)
            r["times"]["wp"] = round(time.time() - t, 1)
            r["c_ok"] = bool(res and res[0] == res[1] and res[1] > 0)
            r["c_proof"] = f"{res[0]}/{res[1]}" if res else "error"

        r["stage"] = "hls"
        t = time.time()
        run, interface, _ = hls.TOOLS[tool]
        hdir = outdir / "hls"
        rtl = run(prep.hls_files, prep.include_dirs, fn.name, hdir, hls_cfg,
                  array_params=[p.name for p in fn.params if p.is_array and p.size > 1])
        r["times"]["hls"] = round(time.time() - t, 1)
        r["stage"] = "interface"
        itf = interface(hdir, fn, hls_cfg)
        (outdir / "interface.json").write_text(itf.to_json())
        ovl = harness.overlap_mode(itf)
        r["mode"] = "overlapping calls" if ovl else "sequential calls"
        if itf.control.get("overlap") and not ovl:
            r["mode"] += " (overlap possible but memories are written: checked sequentially)"

        r["stage"] = "translate"
        cutoff = max([p.size for p in fn.params if p.is_array], default=4) + 2
        W = base_width(fn)
        props, skipped, tr = translate.build_properties(fn, itf, None, W, cutoff)
        pw = max([p.width for p in fn.params] + [8])
        need = min(256, max(fn.ret_width + 8, pw * (tr.max_mul + 1) + 16))
        if need > W:                                  # products need more bits than the default
            W = need
            props, skipped, tr = translate.build_properties(fn, itf, None, W, cutoff)
        r["width"] = W
        fdir = outdir / "formal"
        fdir.mkdir(exist_ok=True)
        # enough FIFO entries for every call that can be in flight: latency / interval + margin
        fifo = st.fifo_depth
        if itf.latency and itf.interval:
            fifo = max(fifo, min(64, itf.latency[1] // max(1, itf.interval) + 3))
        r["fifo_depth"] = fifo
        text, sva, lines = harness.generate(fn, itf, props, tr, W, fifo_depth=fifo)
        (fdir / "harness.sv").write_text(text)
        acc_text, _, acc_lines = harness.generate(fn, itf, props, tr, W, fifo_depth=fifo, force_seq=True)
        (fdir / "harness_acc.sv").write_text(acc_text)
        (outdir / "props.sva").write_text(sva)
        (outdir / "ir.json").write_text(translate.ir_json(fn, itf, props, skipped))
        n_tr = len([p for p in props if p.kind not in ("term",)])
        r.update(translated=n_tr, total_clauses=n_tr + len(skipped), clauses=f"{n_tr}/{n_tr + len(skipped)}",
                 skipped=skipped, discharged=len([p for p in props if p.kind == "discharged"]),
                 pre_dropped=len([s for s in skipped if s["kind"] == "assume"]),
                 notes=sorted({n for p in props for n in p.notes}))

        r["stage"] = "latency"
        t = time.time()

        def regen_lat(L):
            lt, _, _ = harness.generate(fn, itf, props, tr, W, lat_max=L, term_only=True)
            (fdir / "harness_lat.sv").write_text(lt)

        start = min(itf.latency[1], st.lat_cap) if itf.latency else st.lat_start
        L, lst = formal.discover_latency(fdir, rtl, regen_lat, start, st.lat_cap, st.engines, st.sby_timeout)
        r["latency"], r["latency_status"] = L, lst
        r["latency_report"] = list(itf.latency) if itf.latency else None
        r["interval_report"] = itf.interval
        r["times"]["latency"] = round(time.time() - t, 1)

        r["stage"] = "formal"
        depth = L + 4 + (fifo * max(1, itf.interval or 1) if ovl else 0)
        r["depth"] = depth
        status, failed, secs, step, eng = formal.run_portfolio(fdir, rtl, depth, L + 3, st.engines, "bmc",
                                                               lines, st.sby_timeout)
        r["rtl"], r["times"]["bmc"], r["cex_step"], r["engine"] = status, secs, step, eng
        if status == "TIMEOUT":
            r["bounded_to"] = step
        pid = {p.id: p for p in props}
        r["failed"] = [f"{x}: {pid[x].source}" if x in pid else x for x in failed]
        # a violated assertion that contains a recursion cut-off value (solver-chosen) may be
        # spurious: the property is sound but incomplete there
        r["failed_cutoff"] = [x for x in failed if x in pid and any("cut off" in n for n in pid[x].notes)]
        if st.access and any(p.kind == "access" for p in props):
            a, afailed, secs, _, _ = formal.run_portfolio(fdir, rtl, L + 4, L + 3, st.engines, "access",
                                                          acc_lines, st.sby_timeout)
            acc = [x for x in afailed if x in pid and pid[x].kind == "access"]
            r["access_failed"] = [f"{x}: {pid[x].source}" for x in acc]
            r["times"]["access"] = secs
            if a == "PASS":
                r["access"] = "ok"
            elif acc:
                # beyond the \valid range: is the address still inside the physical memory?
                ph, pfailed, psecs, _, _ = formal.run_portfolio(fdir, rtl, L + 4, L + 3, st.engines, "phys",
                                                                acc_lines, st.sby_timeout)
                r["times"]["phys"] = psecs
                phys_hit = [x for x in pfailed if x in pid and pid[x].kind == "access_phys"]
                r["access"] = ("speculative" if ph == "PASS" else "out-of-memory" if (ph == "FAIL" and phys_hit)
                               else f"speculative?({ph.lower()})")
            else:
                r["access"] = "undecided" if a == "FAIL" else a.lower()
        cv, _, secs, _, _ = formal.run_portfolio(fdir, rtl, depth, L + 3, st.engines, "cover", lines,
                                                 st.sby_timeout)
        r["cover"] = "reached" if cv == "PASS" else cv.lower()
        r["times"]["cover"] = secs
        if st.replay and status == "FAIL":
            r["stage"] = "replay"
            from . import replay
            r["replay"] = replay.replay(prep, fn, itf, fdir / "check_bmc", outdir / "replay", fail_step=step)
        r["stage"] = "done"
        r["status"] = "ok"
    except acsl.Unsupported as e:
        r["status"], r["error"] = "unsupported", str(e)
    except Exception as e:
        r["status"], r["error"] = "error", f"{r['stage']}: {str(e)[:300]}"
        (outdir / "traceback.txt").write_text(traceback.format_exc())
    r["times"]["total"] = round(time.time() - t0, 1)
    r["outcome"] = outcome(r)
    (outdir / "result.json").write_text(json.dumps(r, indent=2, default=str))
    return r


def outcome(r):
    """One label per case, used in every table."""
    if r.get("status") != "ok":
        return r.get("status") or "error"
    if r["rtl"] == "TIMEOUT":
        return f"bounded({r.get('bounded_to')})"
    if r["rtl"] == "ERROR":
        return "error"
    if r["rtl"] == "PASS":
        if r.get("cover") != "reached":
            if r.get("latency_status") not in (None, "established"):
                return "bounded(no call completes within the explored depth)"
            return "vacuous"
        return "pass" if r["translated"] == r["total_clauses"] else "pass-partial"
    if r.get("pre_dropped"):
        return "fail-inconclusive"          # a precondition could not be assumed
    if r.get("failed_cutoff") and len(r["failed_cutoff"]) == len(r.get("failed") or []) and \
            (r.get("replay") or {}).get("class") not in ("c-contract-violation", "c-runtime-error", "c-nontermination"):
        # only properties with recursion cut-off values failed, and the C replay does not
        # confirm a violation: the failure may be an artefact of the cut-off
        return "fail-inconclusive"
    return "fail"


def verdict(r, reference_ok=None):
    """Cross-level agreement between the C-level reference and the RTL check.
    reference_ok: the benchmark's own proof status (originals) or our WP result (mutants)."""
    ref = reference_ok if reference_ok is not None else r.get("c_ok")
    o = r.get("outcome") or outcome(r)
    if ref is None or o not in ("pass", "pass-partial", "fail", "vacuous"):
        return "n/a"
    if o == "vacuous":
        return "vacuous"
    if ref and o.startswith("pass"):
        return "agree"
    if not ref and o == "fail":
        return "agree"
    if ref and o == "fail":
        return "rtl-fail-on-proven-c"
    return "c-unproven-rtl-pass"
