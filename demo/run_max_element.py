"""max_element only: original + two hand-picked bugs, step-by-step output (presentation demo).

Usage:  python demo/run_max_element.py
Uses the same pipeline as `bin/c2sva run` (Vitis HLS, generated properties, BMC, C replay).
"""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))
from c2sva import frontend, pipeline        # noqa: E402
from rich.console import Console            # noqa: E402
from rich.markup import escape              # noqa: E402
from rich.table import Table                # noqa: E402

con = Console(width=150)
DESIGN = ROOT / "demo" / "designs" / "max_element.c"
SRC = DESIGN.read_text()
OUT = ROOT / "demo" / "out" / "max_element_demo"
VARIANTS = [("original", None, None, ""),
            ("bug-1", "i < n;", "i < n - 1;", "loop skips the last element"),
            ("bug-2", "i < n;", "i <= n;", "reads one past the end")]
spec = frontend.DesignSpec("max_element", [DESIGN])
rows = []
for name, frm, to, bug in VARIANTS:
    src = SRC if frm is None else SRC.replace(frm, to)
    con.print(f"[bold]max_element / {name}[/]" + (f"  ({bug})" if bug else ""))
    r = pipeline.run_case(spec, name, src, "vitis", "base", pipeline.Settings(), OUT / name)
    con.print(f"  1. Frama-C WP ........... {r['c_proof']}")
    con.print(f"  2. Vitis HLS ............ {r['times'].get('hls')} s, mode: {r['mode']}, latency bound {r['latency']}")
    con.print(f"  3. Clauses translated ... {r['clauses']}")
    con.print(f"  4. Functional check ..... {r['outcome']}  (cover: {r['cover']})")
    for f in r["failed"]:
        con.print(f"       violated: {escape(f)}")
    con.print(f"  5. Memory-access check .. {r['access']}")
    if r.get("replay"):
        con.print(f"  6. C-level replay ....... {r['replay']['class']}  inputs {r['replay'].get('witness', {}).get('inputs')}")
    con.print()
    rows.append((name, bug, r))
t = Table(title="max_element: C contract -> RTL properties (Vitis)")
for c in ("Variant", "C proof", "RTL check", "Violated clause", "Memory access", "C replay"):
    t.add_column(c)
for name, bug, r in rows:
    t.add_row(name + (f": {bug}" if bug else ""), str(r["c_proof"]), r["outcome"],
              escape("\n".join(r["failed"])) or "-", str(r["access"]), (r.get("replay") or {}).get("class", "-"))
con.print(t)
