"""Benchmark adapters: each yields DesignSpecs (files, include dirs, optional top, reference status).

  demo         demo/designs/*.c
  conformance  tests/conformance/*.c (one design per translation rule)
  abe          ACSL by Example (contract in .h, body in .c, logic in Logic/*.acsl, callees in
               other groups); reference status from the authors' Results/*.json
  acslbench    ACSLBench ACSL_C/task_*.c; reference status from our Frama-C run (results/wp)
  <path>       any C file or directory of C files (optionally with -I include dirs)
"""
import csv
import fnmatch
import json
import random
from pathlib import Path

from .frontend import DesignSpec

ROOT = Path(__file__).resolve().parents[2]
ABE = ROOT / "benchmarks" / "raw" / "acsl-by-example" / "StandardAlgorithms"
ACSLBENCH = ROOT / "benchmarks" / "raw" / "acslbench" / "ACSL_C"


def demo():
    for f in sorted((ROOT / "demo" / "designs").glob("*.c")):
        yield DesignSpec(f.stem, [f], meta={"suite": "demo"})


def conformance():
    """Hand-written designs, one per translation rule; all proven by Frama-C WP."""
    for f in sorted((ROOT / "tests" / "conformance").glob("*.c")):
        yield DesignSpec(f.stem, [f], meta={"suite": "conformance", "reference_proven": True})


def abe():
    groups = [g for g in (ABE / "subdirs.list").read_text().split() if g.strip()]
    incs = [ABE, ABE / "Logic"] + [ABE / g for g in groups]
    for g in groups:
        lst = ABE / g / "examples.list"
        if not lst.exists():
            continue
        for name in lst.read_text().split():
            c = ABE / g / f"{name}.c"
            if not c.exists():
                continue
            ref = None
            res = ABE / "Results" / f"{name}.json"
            if res.exists():
                try:
                    ref = json.loads(res.read_text())[0].get("percent") == 100
                except (ValueError, IndexError, KeyError):
                    pass
            yield DesignSpec(f"{g}/{name}", [c], [ABE / g] + incs, top=name, corpus=[ABE],
                             meta={"suite": "abe", "reference_proven": ref})


def acslbench():
    ref = {}
    wp = ROOT / "results" / "wp" / "acslbench.csv"
    if wp.exists():
        ref = {r["id"]: r["status"] == "proven" for r in csv.DictReader(open(wp))}
    for f in sorted(ACSLBENCH.glob("task_*.c"), key=lambda p: int(p.stem.split("_")[1])):
        yield DesignSpec(f.stem, [f], meta={"suite": "acslbench", "reference_proven": ref.get(f.stem)})


def from_path(path, include_dirs=()):
    p = Path(path).resolve()
    files = [p] if p.is_file() else sorted(p.rglob("*.c"))
    for f in files:
        did = f.stem if p.is_file() else str(f.relative_to(p).with_suffix(""))
        yield DesignSpec(did, [f], [Path(d).resolve() for d in include_dirs], corpus=[f.parent],
                         meta={"suite": "path"})


SUITES = {"demo": demo, "conformance": conformance, "abe": abe, "acslbench": acslbench}


def load(bench, include_dirs=(), select=None, ids=None, sample=None, seed=0, proven_only=False):
    specs = list(SUITES[bench]()) if bench in SUITES else list(from_path(bench, include_dirs))
    if select:
        pats = select.split(",")
        specs = [s for s in specs if any(fnmatch.fnmatch(s.id, p) for p in pats)]
    if ids:
        want = set(ids.split(","))
        specs = [s for s in specs if s.id in want]
    if proven_only:
        specs = [s for s in specs if s.meta.get("reference_proven")]
    if sample and sample < len(specs):
        specs = random.Random(seed).sample(specs, sample)
        specs.sort(key=lambda s: s.id)
    return specs
