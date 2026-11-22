"""Deprecated entry point: use `bin/c2sva run --bench demo --mutants 3`."""
import subprocess, sys
from pathlib import Path
sys.exit(subprocess.call([str(Path(__file__).resolve().parents[1] / "bin" / "c2sva"), "run", "--bench", "demo",
                          "--mutants", "3"] + sys.argv[1:]))
