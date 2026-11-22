# Demo: ACSL contracts → RTL assertions → formal check

One command runs the whole flow for each design, on the original C and on mutants (injected bugs):

1. **Frama-C WP** proves the C contract
2. **HLS** (Bambu now, Vitis once licensed) generates Verilog
3. **Signal map**: C names → RTL ports (from the generated Verilog)
4. **Assertions** generated automatically from the contract (`src/c2sva/harness.py`)
5. **SymbiYosys** checks them (BMC + cover)

```bash
source ~/cbv/setup/env.sh
python demo/run_demo.py --tool bambu                  # all designs (~1.5 min)
python demo/run_demo.py --tool bambu --design abs     # one design (~35 s)
python demo/run_demo.py --tool vitis                  # after the Vitis licence is installed
```

Expected: originals **PASS**; every mutant **FAILS** and names the violated contract clause.

| Design | Mutant | Caught by |
|---|---|---|
| `abs` | `a < -1 ? -a : a` | `ensures \result >= 0`, `ensures a < 0 ==> \result == -a` |
| `max_element` | loop `i < n - 1` | `ensures \forall i ... a[i] <= a[\result]` |
| `max_element` | loop `i <= n` | `requires \valid_read(a + (0..n-1))` (out-of-bounds RAM read) |

Outputs per case in `demo/out/<tool>/<design>/<variant>/`:
`signal_map.json`, `formal/harness.sv` (checked), `formal/props.sva` (readable SVA), `formal/sby_*.log`, `wp.log`.

Add a design: drop an ACSL-annotated `.c` in `demo/designs/` and an entry in `designs.json`
(`top`, `lat_max`, array-size `defines`, `mutants`).
