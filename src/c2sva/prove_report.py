"""Aggregate the A1 unbounded-proof results (prove.json) of a run directory."""
import json
from pathlib import Path


def load(run_dir: Path):
    rows = []
    for f in sorted((run_dir / "cases").rglob("prove.json")):
        o = json.loads(f.read_text())
        r = json.loads((f.parent / "result.json").read_text())
        o["_case"] = str(f.parent.relative_to(run_dir / "cases"))
        o["_design"], o["_variant"], o["_config"] = r["design"], r["variant"], r["config"]
        o["_bmc"] = r.get("outcome")
        rows.append(o)
    return rows


def _st(o, key):
    x = o.get(key) or {}
    s = x.get("status", "-")
    if s == "PASS" and "secs" in x:
        return f"PASS {x['secs']}s" + (f" k={x['k']}" if "k" in x else "")
    return s.lower() if s != "-" else "-"


def summarize(run_dir: Path):
    rows = load(run_dir)
    n = len(rows)
    by = lambda m: sum(1 for o in rows if (o.get(m) or {}).get("status") == "PASS")
    unb = [o for o in rows if o.get("unbounded")]
    bmc_bounded = [o for o in rows if str(o["_bmc"]).startswith("bounded")]
    c_total = sum((o.get("candidates") or {}).get("loop_invariants", 0) for o in rows)
    c_surv = sum(len((o.get("houdini") or {}).get("c_invariants_survived") or []) for o in rows)
    abl = [o for o in rows if o.get("ablation")]
    s = {
        "cases": n,
        "unbounded": len(unb),
        "by_kind": by("kind"), "by_pdr": by("pdr"), "by_kind_inv": by("kind+inv"), "by_pdr_inv": by("pdr+inv"),
        "only_with_invariants": sum(1 for o in unb if o.get("proved_by") and set(o["proved_by"]) <= {"kind+inv", "pdr+inv"}),
        # the C proof artefacts are necessary: structural+contract candidates fail at the same k
        "c_artefacts_necessary": sum(1 for o in rows if o.get("ablation")
                                     and o["ablation"].get("hls+contract", {}).get("status") != "PASS"),
        "contract_facts_necessary": sum(1 for o in rows if o.get("ablation")
                                        and o["ablation"].get("hls", {}).get("status") != "PASS"
                                        and o["ablation"].get("hls+contract", {}).get("status") == "PASS"),
        "bmc_bounded": len(bmc_bounded), "bmc_bounded_now_unbounded": sum(1 for o in bmc_bounded if o.get("unbounded")),
        "deep_cex": sum(1 for o in rows if o.get("deep_cex")),
        "c_loop_invariants": c_total, "c_loop_invariants_survived": c_surv,
        "ablation_cases": len(abl),
        "ablation_hls_suffices": sum(1 for o in abl if o["ablation"].get("hls", {}).get("status") == "PASS"),
        "ablation_hls_contract_suffices": sum(1 for o in abl if o["ablation"].get("hls+contract", {}).get("status") == "PASS"),
        "errors": sum(1 for o in rows if o.get("error")),
        "latency_attempted": sum(1 for o in rows if o.get("latency_proof")),
        "latency_proved": sum(1 for o in rows if (o.get("latency_proof") or {}).get("proved")),
        "latency_proved_plain_kind": sum(1 for o in rows if ((o.get("latency_proof") or {}).get("kind") or {}).get("status") == "PASS"),
        "total_correctness": sum(1 for o in rows if o.get("unbounded") and (o.get("latency_proof") or {}).get("proved")),
    }
    L = [f"# A1 unbounded proofs — {run_dir.name}", "",
         f"* cases: {n}; unbounded: **{s['unbounded']}** (k-ind {s['by_kind']}, PDR {s['by_pdr']}, "
         f"k-ind+inv {s['by_kind_inv']}, PDR+inv {s['by_pdr_inv']}); proved only with Houdini-certified "
         f"invariants: {s['only_with_invariants']}",
         f"* BMC-bounded cases: {s['bmc_bounded']}, now unbounded: {s['bmc_bounded_now_unbounded']}; "
         f"deep counterexamples: {s['deep_cex']}; errors: {s['errors']}",
         f"* C loop invariants that hold on the RTL (some placement/state): {c_surv}/{c_total}",
         f"* ablation (cases proved with all candidates): {s['ablation_cases']}; HLS-structural alone suffices "
         f"{s['ablation_hls_suffices']}, structural+contract suffices {s['ablation_hls_contract_suffices']}; "
         f"C proof artefacts necessary: **{s['c_artefacts_necessary']}**, contract-derived facts necessary: "
         f"{s['contract_facts_necessary']}",
         f"* RQ3 latency bound proved for all executions: {s['latency_proved']}/{s['latency_attempted']} "
         f"(plain k-induction {s['latency_proved_plain_kind']}); contract and latency both unbounded "
         f"(total correctness, N cells): {s['total_correctness']}",
         "",
         "| case | BMC | k-ind | PDR | candidates | invariants | C inv kept | k | k-ind+inv | PDR+inv | ablation hls / +contract | latency bound | k_min none/hls/+contract/all |",
         "|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for o in rows:
        h, c = o.get("houdini") or {}, o.get("candidates") or {}
        ab = o.get("ablation") or {}
        L.append(f"| {o['_case']} | {o['_bmc']} | {_st(o, 'kind')} | {_st(o, 'pdr')} | {c.get('total', '-')} | "
                 f"{h.get('invariants', '-')} | {len(h.get('c_invariants_survived') or [])}/{c.get('loop_invariants', '-')} | "
                 f"{h.get('k', '-')} | {_st(o, 'kind+inv')} | {_st(o, 'pdr+inv')} | "
                 f"{(ab.get('hls') or {}).get('status', '-')} / {(ab.get('hls+contract') or {}).get('status', '-')} | "
                 + (lambda lp: f"{lp['bound']}: " + ("proved" if lp.get("proved") else "open")
                    + f" (k={((lp.get('kind+inv') or {}).get('k'))})" if lp else "-")(o.get("latency_proof"))
                 + " | " + ("/".join(str((o.get("kmin") or {}).get(x, "-")) for x in ("none", "hls", "hls+contract", "all"))
                            if o.get("kmin") else "-")
                 + (f" ERROR {o['error'][:60]}" if o.get("error") else "") + " |")
    text = "\n".join(L) + "\n"
    (run_dir / "prove_summary.md").write_text(text)
    (run_dir / "prove_summary.json").write_text(json.dumps(s, indent=2))
    return s, text
