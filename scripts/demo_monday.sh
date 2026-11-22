#!/usr/bin/env bash
# End-to-end live demo of c2sva on one design (ACSL by Example `find`), about 5 minutes.
#   scripts/demo_monday.sh            (fresh run into runs/DEMO_find)
# Stages: C + ACSL contract -> Frama-C WP -> Vitis HLS -> interface model -> RTL properties ->
# BMC (SymbiYosys) -> mutants killed + C-level replay -> unbounded proof from the C loop
# invariants (A1) -> LLM-lifting guardrail (precomputed study) -> summary.
set -u
cd "$(dirname "$0")/.."
source setup/env.sh > /dev/null 2>&1
OUT=runs/DEMO_find
C=bin/c2sva
pause() { [ -t 0 ] && read -r -p $'\n[enter] ' _ || echo; }
hdr() { printf '\n\033[1;36m=== %s ===\033[0m\n' "$*"; }

hdr "1. The C function and its ACSL contract (ACSL by Example: find)"
sed -n '/\/\*@/,/^}/p' benchmarks/raw/acsl-by-example/StandardAlgorithms/Nonmutating/find.c 2>/dev/null | head -40 || true
grep -v '^#' "$(find benchmarks/raw/acsl-by-example -path '*Nonmutating*' -name 'find.h' | head -1)" | sed -n '/\/\*@/,/\*\//p' | head -40
pause

hdr "2. Full pipeline: Frama-C WP -> Vitis HLS -> properties -> BMC -> 2 mutants -> C replay"
rm -rf "$OUT"
time $C run --bench abe --select 'Nonmutating/find' --mutants 2 --jobs 3 --name DEMO_find 2>&1 | grep -E '^\[|designs ->|failures|kills'
D=$OUT/cases/Nonmutating__find/vitis/base
pause

hdr "3. Interface model read from the Vitis report (C object -> RTL ports)"
python3 -c "import json; d=json.load(open('$D/original/interface.json')); print(json.dumps({k: d[k] for k in ('module','scalars','memories','control','latency') if k in d}, indent=1)[:1500])"
pause

hdr "4. Generated properties (SVA view of the contract)"
sed -n 1,30p "$D/original/props.sva"
pause

hdr "5. Mutants: what changed, and the C-level confirmation of each kill"
for m in "$D"/mutant-*; do
  echo "--- $(basename "$m")"; diff <(grep -v '^#' "$D/original/src/hls_src/find.c") <(grep -v '^#' "$m/src/hls_src/find.c") | head -4
  python3 -c "import json; r=json.load(open('$m/result.json')); print('RTL:', r['outcome'], '| C replay:', (r.get('replay') or {}).get('class'), '| failed:', r.get('failed'))"
done
pause

hdr "6. Unbounded proof (A1): C loop invariants -> RTL invariants -> k-induction"
time $C prove "$OUT" --originals --jobs 1 -k 4,8 --strategies kind,houdini,kind+inv 2>&1 | tail -3
python3 - <<PY
import json
o = json.load(open("$D/original/prove.json"))
h = o.get("houdini") or {}
print("plain k-induction  :", (o.get("kind") or {}).get("status"))
print("candidates          :", (o.get("candidates") or {}).get("total"), (o.get("candidates") or {}).get("by_origin"))
print("certified invariants:", h.get("invariants"), "  C loop invariants kept:", h.get("c_invariants_survived"))
print("contract, all runs  :", o.get("kind+inv"))
PY
pause

hdr "7. RQ6: LLM-lifted definitions are admitted only after a Frama-C proof"
python3 - <<'PY'
import json, collections
d = json.load(open("runs/E4_acslbench/lift/lift.json"))
print(collections.Counter(r["status"] for r in d["rows"]))
for r in d["rows"]:
    print(f"  {r['design']:9s} {r['name']:15s} {r['status']}")
PY
echo "seeded wrong definitions: 11/11 rejected (docs/methodology.md section 6.2)"
pause

hdr "8. Campaign results (frozen re-run tables)"
for f in runs/paper_tables_F.md runs/paper_tables.md; do [ -f "$f" ] && { sed -n 1,25p "$f"; break; }; done
