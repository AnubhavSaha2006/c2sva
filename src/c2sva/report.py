"""Aggregate case results into the tables of the evaluation (Markdown, CSV, JSON, LaTeX).

Per (tool, HLS configuration):
  funnel        designs -> front end -> HLS -> checked -> outcome
  originals     pass / pass-partial / fail / bounded / vacuous; agreement with the C reference;
                replay class of every RTL failure
  mutants       killed / survived split by the C-level verdict (Frama-C WP on the mutant);
                replay confirmation of kills; mutation score on non-equivalent mutants
  clauses       translated / total; reasons for the rest; translation strategies used
  access        memory-access (\\valid) violations on originals (e.g. speculative reads)
  latency       established bound vs. HLS report; call mode (sequential / overlapping)
  time          median seconds per stage
"""
import collections
import csv
import json
import statistics
from pathlib import Path

from .pipeline import outcome, verdict


def load_results(run_dir: Path):
    out = []
    for f in sorted(run_dir.rglob("result.json")):
        try:
            r = json.loads(f.read_text())
            r["_dir"] = str(f.parent.relative_to(run_dir))
            r["outcome"] = outcome(r)                 # always recomputed with the current rules
            out.append(r)
        except ValueError:
            pass
    return out


_ORIG_WP = {}


def reference(r, meta):
    """C-level ground truth. Originals: the benchmark's proof status (else our WP run).
    Mutants: our WP run on the mutant, but only if the original proves with the same WP flags
    (otherwise a failed proof says nothing about the mutation)."""
    if r["variant"] == "original":
        ref = meta.get(r["design"], {}).get("reference_proven")
        return ref if ref is not None else r.get("c_ok")
    if _ORIG_WP.get((r["design"], r["tool"], r["config"])) is False:
        return None
    return r.get("c_ok")


def _median(xs):
    xs = [x for x in xs if isinstance(x, (int, float))]
    return round(statistics.median(xs), 1) if xs else None


def summarize(results, meta):
    _ORIG_WP.clear()
    _ORIG_WP.update({(r["design"], r["tool"], r["config"]): r.get("c_ok") for r in results if r["variant"] == "original"})
    groups = collections.defaultdict(list)
    for r in results:
        groups[(r["tool"], r["config"])].append(r)
    rows, details = [], {}
    for (tool, cfg), rs in sorted(groups.items()):
        orig = [r for r in rs if r["variant"] == "original"]
        muts = [r for r in rs if r["variant"] != "original"]
        oc = collections.Counter(r["outcome"] if not r["outcome"].startswith("bounded") else "bounded" for r in orig)
        ver = collections.Counter(verdict(r, reference(r, meta)) for r in orig)
        cl_ok = sum(r.get("translated") or 0 for r in orig)
        cl_all = sum(r.get("total_clauses") or 0 for r in orig)
        m_checked = [r for r in muts if r["outcome"] in ("pass", "pass-partial", "fail", "fail-inconclusive")]
        killed = [r for r in m_checked if r["outcome"].startswith("fail")]
        surv = [r for r in m_checked if reference(r, meta) is False and r["outcome"].startswith("pass")]
        sc = collections.Counter((r.get("survivor") or {}).get("class", "not analysed") for r in surv)
        nonequiv = [r for r in m_checked if reference(r, meta) is False]
        killed_ne = [r for r in nonequiv if r["outcome"].startswith("fail")]
        equiv_pass = [r for r in m_checked if reference(r, meta) and r["outcome"].startswith("pass")]
        confirmed = [r for r in killed if (r.get("replay") or {}).get("class") in
                     ("c-contract-violation", "c-runtime-error", "c-nontermination")]
        rows.append(dict(
            tool=tool, config=cfg, designs=len(orig),
            frontend_ok=sum(1 for r in orig if r["status"] != "unsupported" and r["stage"] != "frontend"),
            hls_ok=sum(1 for r in orig if r["stage"] not in ("frontend", "wp", "hls")),
            checked=sum(oc[k] for k in ("pass", "pass-partial", "fail", "fail-inconclusive", "vacuous")),
            pass_full=oc["pass"], pass_partial=oc["pass-partial"], fail=oc["fail"] + oc["fail-inconclusive"],
            bounded=oc["bounded"], vacuous=oc["vacuous"],
            agree=ver["agree"], disagree=ver["rtl-fail-on-proven-c"] + ver["c-unproven-rtl-pass"],
            clauses=f"{cl_ok}/{cl_all}", clause_rate=round(100 * cl_ok / cl_all, 1) if cl_all else None,
            access_speculative=sum(1 for r in orig if (r.get("access") or "").startswith("speculative")),
            access_out_of_memory=sum(1 for r in orig if r.get("access") == "out-of-memory"),
            overlap_mode=sum(1 for r in orig if (r.get("mode") or "").startswith("overlapping")),
            mutants=len(muts), mutants_checked=len(m_checked), killed=len(killed),
            mutation_score=round(100 * len(killed_ne) / len(nonequiv), 1) if nonequiv else None,
            survivors=len(surv), survivors_missed_bug=sc["missed-bug"], survivors_safety_only=sc["memory-safety-only"],
            survivors_untranslated=sc["untranslated-clause"],
            survivors_no_violation=sc["no-violation-found"], survivors_not_analysed=sc["not analysed"],
            equivalent_pass=len(equiv_pass), kills_confirmed_in_C=len(confirmed),
            median_total_s=_median([(r.get("times") or {}).get("total") for r in orig]),
        ))
        details[f"{tool}/{cfg}"] = dict(
            original_outcomes=dict(oc),
            failure_replay=collections.Counter((r.get("replay") or {}).get("class", "none")
                                               for r in orig if r["outcome"].startswith("fail")),
            kill_replay=collections.Counter((r.get("replay") or {}).get("class", "none") for r in killed),
            survived_c_unproven=[r["_dir"] for r in m_checked if reference(r, meta) is False and r["outcome"].startswith("pass")],
            stage_median_s={k: _median([(r.get("times") or {}).get(k) for r in orig])
                            for k in ("frontend", "wp", "hls", "latency", "bmc", "access", "cover", "total")},
        )
    S = {"by_tool_config": rows, "details": details}
    S["not_checked_reasons"] = collections.Counter(
        f"{r['status']}: {(r.get('error') or '')[:100]}" for r in results
        if r["variant"] == "original" and r["status"] != "ok").most_common(30)
    S["untranslated_clause_reasons"] = collections.Counter(
        s["reason"][:90] for r in results if r["variant"] == "original" for s in r.get("skipped", [])).most_common(25)
    S["translation_strategies"] = collections.Counter(
        n.split(":")[0][:70] for r in results if r["variant"] == "original" for n in r.get("notes", [])).most_common(25)
    S["disagreements"] = [dict(case=r["_dir"], outcome=r["outcome"], ref=reference(r, meta), failed=r["failed"][:3],
                               replay=(r.get("replay") or {}).get("class"))
                          for r in results if r["variant"] == "original" and
                          verdict(r, reference(r, meta)) in ("rtl-fail-on-proven-c", "vacuous")]
    S["access_violations"] = [dict(case=r["_dir"], detail=r["access_failed"][:2])
                              for r in results if r["variant"] == "original" and r.get("access") not in (None, "ok")]
    est = [r for r in results if r["variant"] == "original" and r.get("latency_status") == "established"]
    S["latency"] = dict(established=len(est),
                        report_undefined=sum(1 for r in est if not r.get("latency_report")),
                        not_established=sum(1 for r in results if r["variant"] == "original"
                                            and r.get("latency_status") not in (None, "established")))
    return S


def latex_table(rows):
    cols = [("config", "Config"), ("designs", "Designs"), ("checked", "Checked"), ("pass_full", "Pass"),
            ("pass_partial", "Pass$_p$"), ("fail", "Fail"), ("bounded", "Bnd"), ("agree", "Agree"),
            ("clause_rate", "Cl.\\%"), ("access_speculative", "Spec."), ("mutation_score", "MS\\%"),
            ("kills_confirmed_in_C", "Conf.")]
    out = ["\\begin{tabular}{l" + "r" * (len(cols) - 1) + "}", "\\toprule",
           " & ".join(h for _, h in cols) + " \\\\", "\\midrule"]
    for r in rows:
        out.append(" & ".join(str(r.get(k, "")) if r.get(k) is not None else "--" for k, _ in cols) + " \\\\")
    out += ["\\bottomrule", "\\end{tabular}"]
    return "\n".join(out) + "\n"


_RUN_DIR = []


def write(run_dir: Path, results, meta):
    _RUN_DIR[:] = [Path(run_dir) / "cases"]
    S = summarize(results, meta)
    (run_dir / "summary.json").write_text(json.dumps(S, indent=2, default=str))
    (run_dir / "table.tex").write_text(latex_table(S["by_tool_config"]))
    keys = ["design", "variant", "tool", "config", "top", "outcome", "status", "stage", "c_proof", "rtl", "cover",
            "access", "mode", "latency", "latency_status", "clauses", "width", "depth", "engine", "error"]
    with open(run_dir / "results.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=keys + ["verdict", "replay", "failed", "access_failed", "total_s", "case_dir"])
        w.writeheader()
        for r in results:
            row = {k: r.get(k) for k in keys}
            row.update(verdict=verdict(r, reference(r, meta)), replay=(r.get("replay") or {}).get("class"),
                       failed=" | ".join(r.get("failed", [])), access_failed=" | ".join(r.get("access_failed", [])),
                       total_s=(r.get("times") or {}).get("total"), case_dir=r["_dir"])
            w.writerow(row)
    md = ["# c2sva run summary", "", "## Per tool / HLS configuration", ""]
    if S["by_tool_config"]:
        cols = list(S["by_tool_config"][0].keys())
        md += ["| " + " | ".join(cols) + " |", "|" + "---|" * len(cols)]
        md += ["| " + " | ".join(str(row[c]) for c in cols) + " |" for row in S["by_tool_config"]]
    for k, d in S["details"].items():
        md += ["", f"### {k}", "", f"- original outcomes: {d['original_outcomes']}",
               f"- replay of RTL failures on originals: {dict(d['failure_replay'])}",
               f"- replay of mutant kills: {dict(d['kill_replay'])}",
               f"- mutants surviving although C-level proof fails: {len(d['survived_c_unproven'])}",
               f"- median seconds per stage: {d['stage_median_s']}"]
    for title, key in (("Why originals were not checked", "not_checked_reasons"),
                       ("Why clauses were not translated", "untranslated_clause_reasons"),
                       ("Translation strategies used", "translation_strategies")):
        md += ["", f"## {title}", ""] + [f"- {n} x {k}" for k, n in S[key]]
    md += ["", "## RTL failures / vacuity on C-proven originals (investigate)", ""]
    md += [f"- {d['case']}: {d['outcome']} {d['failed']} replay={d['replay']}" for d in S["disagreements"]] or ["- none"]
    md += ["", "## Memory-access violations on originals", ""]
    md += [f"- {d['case']}: {d['detail']}" for d in S["access_violations"]] or ["- none"]
    md += ["", f"Latency: {S['latency']}"]
    (run_dir / "summary.md").write_text("\n".join(md) + "\n")
    return S
