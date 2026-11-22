"""Baselines for the evaluation.

QED-FC (after A-QED, Singh et al., DAC 2020; G-QED, DAC 2023): specification-free *functional
consistency*. In one trace of the harness (arbitrary call sequences from reset), a reference
call is chosen nondeterministically; at every later completion whose inputs (scalar arguments
and memory pre-state) equal the reference call's, the outputs (result and memory post-state)
must equal the reference call's outputs. It finds hardware that depends on stale state or is
nondeterministic, and nothing that is deterministic but wrong, which is the point of comparing
it with the contract check.

AutoINV-style structural invariants are the `hls` candidate pool of c2sva prove (kmin/ablation).
"""
import re
import json
from pathlib import Path

from . import formal, prove


def qed_block(itf, fn):
    """Verilog appended to the sequential harness (contract assertions disabled)."""
    clk = itf.control["clk"]
    pw = {p.name: p.width for p in fn.params}
    ins, outs = [], []
    for c in itf.scalars:
        ins.append((f"{c}_cap", pw[c]))
    for a, mem in itf.memories.items():
        for i in range(mem.size):
            ins.append((f"o_{a}_{i}", mem.width))
            outs.append((f"m_{a}_{i}", mem.width))
    for a, ob in itf.objects.items():
        ins.append((f"o_{a}_0", ob.width))
        outs.append((f"m_{a}_0", ob.width))
    if itf.ret:
        outs.append(("h_res_cap", itf.ret_width))
    L = ["  // ---- QED-FC baseline: a nondeterministically chosen reference call",
         "  (* anyseq *) reg h_pick;",
         "  reg h_ref = 0;"]
    for k, (s, w) in enumerate(ins + outs):
        L.append(f"  reg [{w - 1}:0] h_q{k} = 0;")
    L.append(f"  always @(posedge {clk}) if (!h_init && h_done_d && !h_ref && h_pick) begin")
    L.append("    h_ref <= 1;")
    for k, (s, w) in enumerate(ins + outs):
        L.append(f"    h_q{k} <= {s};")
    L.append("  end")
    same_in = " && ".join(f"({s} == h_q{k})" for k, (s, w) in enumerate(ins)) or "1'b1"
    same_out = " && ".join(f"({s} == h_q{k + len(ins)})" for k, (s, w) in enumerate(outs)) or "1'b1"
    L.append(f"  always @(*) if (!h_init && h_done_d && h_ref && ({same_in})) assert({same_out});")
    return L


def qed_check(d: Path, depth_calls=2, timeout=600):
    """BMC of QED-FC on a finished case directory. -> dict(status, depth, secs)."""
    fn, itf, rtl = prove.load_case(d)
    r = json.loads((d / "result.json").read_text())
    base = (d / "formal" / "harness_acc.sv").read_text().rstrip().split("\n")
    assert base[-1].strip() == "endmodule"
    lines = [("    // (contract assertion disabled for the QED-FC baseline)"
              if re.match(r"\s*if \(h_done_d\) assert\(", l) else l) for l in base[:-1]]
    text = "\n".join(lines + qed_block(itf, fn) + ["endmodule"]) + "\n"
    L = r.get("latency") or 8
    depth = depth_calls * (L + 2) + 4
    st, failed, secs, _ = prove._sby(d / "formal" / "qed", rtl, text, [], "bmc", depth, "kind", timeout)
    out = {"status": st, "depth": depth, "secs": secs}
    (d / "qed.json").write_text(json.dumps(out, indent=2))
    return out
