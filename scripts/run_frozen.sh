#!/usr/bin/env bash
# Frozen end-to-end re-run with the final code (after the 2026-11-21 review fixes, see
# docs/review/FINDINGS.md). Every stage writes a fresh runs/F* directory; earlier runs/E*
# directories are kept for comparison. Resumable: re-running skips finished cases (--resume).
#   scripts/run_frozen.sh [jobs]
set -u
cd "$(dirname "$0")/.."
J=${1:-6}
LOG=runs/frozen.log
step() { echo "=== $(date '+%F %T') $*" | tee -a "$LOG"; }
C="bin/c2sva"
NSCALE=$(cat runs/nscale_designs.txt)

step "F1 conformance (mutants 3)";      $C run --bench conformance --mutants 3 --jobs "$J" --name F1_conformance --resume >> "$LOG" 2>&1
step "F3 ABE (mutants 2)";              $C run --bench abe --mutants 2 --jobs "$J" --name F3_abe --resume >> "$LOG" 2>&1
step "F8 unsafe pragmas (ABE subset)";  $C run --bench abe --select 'Numeric/partial_sum,Numeric/partial_sum_inv,Numeric/adjacent_difference,Numeric/adjacent_difference_inv,Mutating/remove,Mutating/remove_copy,Numeric/accumulate,Numeric/iota,Heap/push_heap,Heap/pop_heap,Sorting/bubble_sort,Sorting/selection_sort,Mutating/rotate,Mutating/reverse,Mutating/copy_backward' --config unsafe_dep --mutants 0 --jobs "$J" --name F8_unsafe_dep --resume >> "$LOG" 2>&1
step "F4 ACSLBench (proven, mutants 2)"; $C run --bench acslbench --proven-only --mutants 2 --jobs "$J" --name F4_acslbench --resume >> "$LOG" 2>&1
step "F4L ACSLBench with LLM-lifted definitions"; $C lift runs/F4_acslbench --backend files >> "$LOG" 2>&1 || true
[ -f runs/F4_acslbench/lift/lift.json ] || mkdir -p runs/F4_acslbench/lift
$C run --bench acslbench --proven-only --select 'task_363,task_564,task_586' --mutants 2 --jobs "$J" --name F4L_acslbench_lifted --lifted runs/E4_acslbench/lift/lift.json --resume >> "$LOG" 2>&1
step "F7 N-scaling N=8/16";             for N in 8 16; do $C run --bench abe --select "$NSCALE" --array-size $N --mutants 0 --jobs "$J" --name F7_abe_N$N --resume >> "$LOG" 2>&1; done
step "F6 conformance N=2/8";            for N in 2 8; do $C run --bench conformance --array-size $N --mutants 0 --jobs "$J" --name F6_conformance_N$N --resume >> "$LOG" 2>&1; done
step "F2 conformance x 11 configs";     $C run --bench conformance --config base,cyclic2,block2,complete,reshape2,reshapeb2,bram,nopipe,unroll,fastclk,chain --mutants 0 --jobs "$J" --name F2_conformance_cfg --resume >> "$LOG" 2>&1
step "F5 ABE x 6 configs";              $C run --bench abe --config toppipe,cyclic2,reshape2,bram,nopipe,unroll --mutants 0 --jobs "$J" --name F5_abe_cfg --resume >> "$LOG" 2>&1

step "survivors";                       for r in F1_conformance F3_abe F4_acslbench; do $C survivors runs/$r --jobs 3 >> "$LOG" 2>&1; done
step "QED-FC baseline (conformance)";   $C qed runs/F1_conformance --jobs 3 --resume >> "$LOG" 2>&1
P=$((J / 3 > 0 ? J / 3 : 1))
step "A1 conformance";                  $C prove runs/F1_conformance --originals --jobs "$P" --strategies kind,pdr,houdini,kind+inv,ablation,latency,kmin,kmin-progress --resume >> "$LOG" 2>&1
for N in 2 8; do $C prove runs/F6_conformance_N$N --originals --jobs "$P" --strategies houdini,kind+inv,kmin,kmin-progress --resume >> "$LOG" 2>&1; done
step "A1 ABE";                          $C prove runs/F3_abe --originals --jobs "$P" --strategies kind,pdr,houdini,kind+inv,ablation,latency --resume >> "$LOG" 2>&1
$C prove runs/F3_abe --originals --select "$NSCALE" --jobs "$P" --strategies houdini,kind+inv,kmin,kmin-progress >> "$LOG" 2>&1
for N in 8 16; do $C prove runs/F7_abe_N$N --originals --jobs "$P" --strategies kind,houdini,kind+inv,kmin,kmin-progress --resume >> "$LOG" 2>&1; done
step "reports"
for r in runs/F*/; do $C report "$r" > /dev/null 2>&1; [ -n "$(ls $r/cases/*/*/*/*/prove.json 2>/dev/null | head -1)" ] && $C prove-report "$r" > /dev/null 2>&1; done
PYTHONPATH=src python scripts/paper_tables.py --prefix F >> "$LOG" 2>&1
step "done"
