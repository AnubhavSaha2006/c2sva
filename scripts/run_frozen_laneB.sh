#!/usr/bin/env bash
# Frozen re-run, lane B: fault injection, N-scaling, configuration matrices (solver budget 300 s).
set -u; cd "$(dirname "$0")/.."; J=${1:-5}; LOG=runs/frozen_B.log; C=bin/c2sva; T="--sby-timeout 300"
step() { echo "=== $(date '+%F %T') $*" | tee -a "$LOG"; }
NSCALE=$(cat runs/nscale_designs.txt)
step "F8 unsafe pragmas"; $C run --bench abe --select 'Numeric/partial_sum,Numeric/partial_sum_inv,Numeric/adjacent_difference,Numeric/adjacent_difference_inv,Mutating/remove,Mutating/remove_copy,Numeric/accumulate,Numeric/iota,Heap/push_heap,Heap/pop_heap,Sorting/bubble_sort,Sorting/selection_sort,Mutating/rotate,Mutating/reverse,Mutating/copy_backward' --config unsafe_dep --mutants 0 --jobs "$J" --name F8_unsafe_dep --resume >> "$LOG" 2>&1
step "F6 N=2/8";          for N in 2 8; do $C run --bench conformance --array-size $N --mutants 0 --jobs "$J" --name F6_conformance_N$N --resume >> "$LOG" 2>&1; done
step "F7 N=8/16";         for N in 8 16; do $C run --bench abe --select "$NSCALE" --array-size $N --mutants 0 --jobs "$J" $T --name F7_abe_N$N --resume >> "$LOG" 2>&1; done
step "F2 configs";        $C run --bench conformance --config base,cyclic2,block2,complete,reshape2,reshapeb2,bram,nopipe,unroll,fastclk,chain --mutants 0 --jobs "$J" $T --name F2_conformance_cfg --resume >> "$LOG" 2>&1
step "A1 N-scaling";      for N in 2 8; do $C prove runs/F6_conformance_N$N --originals --jobs 2 --strategies houdini,kind+inv,kmin,kmin-progress --resume >> "$LOG" 2>&1; done
                          for N in 8 16; do $C prove runs/F7_abe_N$N --originals --jobs 2 --strategies kind,houdini,kind+inv,kmin,kmin-progress --resume >> "$LOG" 2>&1; done
step "F5 ABE configs";    $C run --bench abe --config toppipe,cyclic2,reshape2,bram,nopipe,unroll --mutants 0 --jobs "$J" $T --name F5_abe_cfg --resume >> "$LOG" 2>&1
step "lane B done"
