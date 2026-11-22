#!/usr/bin/env bash
# Full evaluation campaign (Vitis HLS). Each step writes runs/<name>/{summary.md,results.csv,table.tex}.
# Usage: scripts/run_campaign.sh [jobs]      (re-running a step overwrites its run directory)
set -u
J=${1:-8}
C="$(cd "$(dirname "$0")/.." && pwd)/bin/c2sva"
step() { name=$1; shift; echo "=== $name: $*"; rm -rf "runs/$name"; "$C" run --name "$name" --jobs "$J" "$@" > "runs/$name.log" 2>&1; tail -3 "runs/$name.log"; }
cd "$(dirname "$0")/.."
mkdir -p runs
CFGS=nopipe,unroll,fastclk,chain,toppipe,cyclic2,block2,complete,reshape2,reshapeb2,bram
# E1: rule conformance (one design per rule), originals + 3 automatic mutants
step E1_conformance      --bench conformance --mutants 3
# E2: robustness of the interface/layout model across Vitis optimisations (originals)
step E2_conformance_cfg  --bench conformance --config $CFGS
# E3: ACSL by Example (all designs), originals + 2 mutants
step E3_abe              --bench abe --mutants 2
# E4: ACSLBench (Frama-C-proven tasks), originals + 2 mutants
step E4_acslbench        --bench acslbench --proven-only --mutants 2
# E5: optimisation study on ACSL by Example (originals)
step E5_abe_cfg          --bench abe --config toppipe,cyclic2,reshape2,bram,nopipe,unroll
# E6: sensitivity to the memory bound N
step E6_conformance_N2   --bench conformance --array-size 2
step E6_conformance_N8   --bench conformance --array-size 8
echo "campaign done"
