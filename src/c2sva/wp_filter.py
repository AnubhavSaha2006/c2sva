"""Run Frama-C/WP on every benchmark function and record which contracts are fully proven.

Usage:  python -m c2sva.wp_filter [--jobs 8] [--timeout 10] [--only acslbench|abe]
Output: results/wp/<suite>.csv  and  results/wp/logs/<suite>/<id>.log
"""
import argparse
import csv
import re
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RAW = ROOT / "benchmarks" / "raw"
ABE = RAW / "acsl-by-example" / "StandardAlgorithms"
OUT = ROOT / "results" / "wp"

PROVERS = "alt-ergo,z3,cvc5"
RTE_FLAGS = ["-warn-unsigned-overflow", "-warn-unsigned-downcast"]
PROVED_RE = re.compile(r"Proved goals:\s+(\d+)\s*/\s*(\d+)")


def acslbench_tasks():
    for f in sorted((RAW / "acslbench" / "ACSL_C").glob("task_*.c"),
                    key=lambda p: int(p.stem.split("_")[1])):
        yield f.stem, f, []


def abe_tasks():
    groups = [g.strip() for g in (ABE / "subdirs.list").read_text().split() if g.strip()]
    incs = [f"-I{ABE}", f"-I{ABE / 'Logic'}"] + [f"-I{ABE / g}" for g in groups]
    for g in groups:
        lst = ABE / g / "examples.list"
        if not lst.exists():
            continue
        for name in lst.read_text().split():
            c = ABE / g / f"{name}.c"
            if c.exists():
                yield f"{g}/{name}", c, ["-pp-annot", "-cpp-extra-args=" + " ".join(incs)]


def run_one(tid, cfile, extra, timeout, logdir):
    cmd = ["frama-c", *extra, *RTE_FLAGS, "-wp", "-wp-rte", "-wp-split", "-wp-model", "Typed",
           "-wp-prover", PROVERS, "-wp-timeout", str(timeout), "-wp-par", "2", str(cfile)]
    t0 = time.time()
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=max(600, timeout * 60))
        out = p.stdout + p.stderr
        rc = p.returncode
    except subprocess.TimeoutExpired as e:
        out, rc = (e.stdout or "") if isinstance(e.stdout, str) else "", "timeout"
    dt = round(time.time() - t0, 1)
    log = logdir / (tid.replace("/", "__") + ".log")
    log.write_text(" ".join(cmd) + "\n\n" + out)

    m = PROVED_RE.search(out)
    proved, total = (int(m.group(1)), int(m.group(2))) if m else (0, 0)
    if rc == "timeout":
        status = "timeout"
    elif not m:
        status = "error"           # parse/typing error, no WP run
    elif proved == total:
        status = "proven"
    else:
        status = "partial"
    err = ""
    if status == "error":
        lines = [l for l in out.splitlines() if "[kernel]" in l or "error" in l.lower()]
        err = " | ".join(lines[:3])[:300]
    return dict(id=tid, file=str(cfile.relative_to(ROOT)), status=status,
                proved=proved, total=total, seconds=dt, error=err)


def run_suite(name, tasks, jobs, timeout):
    logdir = OUT / "logs" / name
    logdir.mkdir(parents=True, exist_ok=True)
    tasks = list(tasks)
    rows = []
    with ThreadPoolExecutor(jobs) as ex:
        futs = [ex.submit(run_one, *t, timeout, logdir) for t in tasks]
        for i, f in enumerate(as_completed(futs), 1):
            r = f.result()
            rows.append(r)
            print(f"[{name} {i}/{len(tasks)}] {r['id']:40s} {r['status']:8s} "
                  f"{r['proved']}/{r['total']} {r['seconds']}s", flush=True)
    rows.sort(key=lambda r: r["id"])
    with open(OUT / f"{name}.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    counts = {}
    for r in rows:
        counts[r["status"]] = counts.get(r["status"], 0) + 1
    print(f"== {name}: {len(rows)} files -> {counts}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--jobs", type=int, default=8)
    ap.add_argument("--timeout", type=int, default=10, help="per-goal prover timeout (s)")
    ap.add_argument("--only", choices=["acslbench", "abe"])
    a = ap.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    if a.only in (None, "abe"):
        run_suite("abe", abe_tasks(), a.jobs, a.timeout)
    if a.only in (None, "acslbench"):
        run_suite("acslbench", acslbench_tasks(), a.jobs, a.timeout)


if __name__ == "__main__":
    main()
