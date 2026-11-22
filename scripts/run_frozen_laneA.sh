#!/usr/bin/env bash
# Frozen re-run, lane A: main benchmarks and their proofs (solver budget 600 s).
set -u; cd "$(dirname "$0")/.."; J=${1:-5}; LOG=runs/frozen_A.log; C=bin/c2sva
step() { echo "=== $(date '+%F %T') $*" | tee -a "$LOG"; }
NSCALE=$(cat runs/nscale_designs.txt)
step "F1 conformance";  $C run --bench conformance --mutants 3 --jobs "$J" --name F1_conformance --resume >> "$LOG" 2>&1
step "F3 ABE";          $C run --bench abe --mutants 2 --jobs "$J" --name F3_abe --resume >> "$LOG" 2>&1
step "F4 ACSLBench";    $C run --bench acslbench --proven-only --mutants 2 --jobs "$J" --name F4_acslbench --resume >> "$LOG" 2>&1
step "F4L lifted";      $C run --bench acslbench --proven-only --select 'task_363,task_564,task_586' --mutants 2 --jobs "$J" --name F4L_acslbench_lifted --lifted runs/E4_acslbench/lift/lift.json --resume >> "$LOG" 2>&1
step "survivors";       for r in F1_conformance F3_abe F4_acslbench; do $C survivors runs/$r --jobs 3 >> "$LOG" 2>&1; done
step "QED-FC";          $C qed runs/F1_conformance --jobs 3 --resume >> "$LOG" 2>&1
step "A1 conformance";  $C prove runs/F1_conformance --originals --jobs 2 --strategies kind,pdr,houdini,kind+inv,ablation,latency,kmin,kmin-progress --resume >> "$LOG" 2>&1
step "A1 ABE";          $C prove runs/F3_abe --originals --jobs 3 --strategies kind,pdr,houdini,kind+inv,ablation,latency --resume >> "$LOG" 2>&1
                        $C prove runs/F3_abe --originals --select "$NSCALE" --jobs 3 --strategies houdini,kind+inv,kmin,kmin-progress >> "$LOG" 2>&1
step "lane A done"
