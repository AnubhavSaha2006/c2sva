"""Fold the cases of re-runs into a campaign run directory.

  PYTHONPATH=src python scripts/merge_runs.py runs/E4_acslbench runs/E4fix_valid runs/E4b_reserved ...

Each case directory cases/<design>/<tool>/<config>/<variant> of a source run replaces the one
in the target (the replaced directory is moved to <target>/replaced/<source name>/...). The
merge is logged in <target>/merges.json; rebuild the tables afterwards with `c2sva report`.
"""
import json
import shutil
import sys
import time
from pathlib import Path


def merge(target: Path, sources):
    log = json.loads((target / "merges.json").read_text()) if (target / "merges.json").exists() else []
    for src in sources:
        moved = []
        for res in sorted((src / "cases").rglob("result.json")):
            rel = res.parent.relative_to(src / "cases")
            dst = target / "cases" / rel
            if dst.exists():
                bak = target / "replaced" / src.name / rel
                bak.parent.mkdir(parents=True, exist_ok=True)
                if bak.exists():
                    shutil.rmtree(bak)
                shutil.move(str(dst), str(bak))
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copytree(res.parent, dst)
            moved.append(str(rel))
        log.append({"source": str(src), "time": time.strftime("%Y-%m-%d %H:%M"), "cases": moved})
        print(f"{src.name}: {len(moved)} cases merged into {target.name}")
    (target / "merges.json").write_text(json.dumps(log, indent=1))


if __name__ == "__main__":
    merge(Path(sys.argv[1]), [Path(s) for s in sys.argv[2:]])
