"""Deprecated entry point: use `bin/c2sva run --bench acslbench --proven-only --sample N --mutants 2`."""
import subprocess, sys
from pathlib import Path
sys.exit(subprocess.call([str(Path(__file__).resolve().parents[1] / "bin" / "c2sva"), "run", "--bench", "acslbench",
                          "--proven-only"] + sys.argv[1:]))
