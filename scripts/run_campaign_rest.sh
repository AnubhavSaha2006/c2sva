#!/usr/bin/env bash
# Remaining campaign steps (E4-E6). Resumable: finished cases are kept (--resume).
# Launch detached from the terminal/session:  systemd-run --user --unit c2sva-campaign scripts/run_campaign_rest.sh 8
set -u
J=${1:-8}
ROOTD="$(cd "$(dirname "$0")/.." && pwd)"
C="$ROOTD/bin/c2sva"
cd "$ROOTD"
step() { name=$1; shift; echo "=== $name: $* (resume)"; "$C" run --name "$name" --jobs "$J" --resume "$@" >> "runs/$name.log" 2>&1; tail -3 "runs/$name.log"; }
step E4_acslbench        --bench acslbench --proven-only --mutants 2
step E5_abe_cfg          --bench abe --config toppipe,cyclic2,reshape2,bram,nopipe,unroll
step E6_conformance_N2   --bench conformance --array-size 2
step E6_conformance_N8   --bench conformance --array-size 8
echo "campaign done"
