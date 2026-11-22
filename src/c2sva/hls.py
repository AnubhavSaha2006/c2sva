"""HLS back end (Vitis HLS; Bambu kept for comparison) and the interface model ('signal map').

The interface model relates every C-level object of the top function to RTL ports:
  control  : block-level protocol (ap_ctrl_hs / ap_ctrl_chain), its ports, reset polarity,
             and whether calls may overlap (top-level pipelining: interval < latency)
  ret      : return-value port
  scalars  : by-value parameters
  memories : array parameters. A memory is a set of storage elements e[0..size-1] and one
             function  locate(i) = (bank, word address, slot)  that covers every layout:
               plain ap_memory          (0, i, 0)
               partition cyclic B       (i % B, i // B, 0)
               partition block B        (i // C, i % C, 0)            C = ceil(size/B)
               reshape cyclic k         (0, i // k, i % k)            word holds k elements
               reshape block k          (0, i % C, i // C)            C = ceil(size/k)
               bram mode                as plain, but byte addresses and byte enables
               partition complete       one 'cell' (plain port triple) per element
  objects  : single-object pointers realized as plain ports (ap_none / ap_vld / ap_ovld)
For Vitis everything is read from the tool's own report (InterfaceSummary: port -> C object,
protocol, direction, width); layouts come from the directives the framework itself applied.
"""
import json
import math
import re
import subprocess
import xml.etree.ElementTree as ET
from dataclasses import asdict, dataclass, field
from pathlib import Path

VITIS_PART = "xcau15p-ffvb676-2-e"     # Artix UltraScale+ (free tier)

# Named, reproducible Vitis configurations (robustness study). Array directives apply to every
# array parameter of the top function with more than one element.
VITIS_CONFIGS = {
    "base":     {},
    "nopipe":   {"tcl": ["config_compile -pipeline_loops 0"]},
    "unroll":   {"tcl": ["config_unroll -tripcount_threshold 64"]},
    "fastclk":  {"clock": 2.5},
    "slowclk":  {"clock": 20},
    "chain":    {"tcl": ["set_directive_interface -mode ap_ctrl_chain {top}"]},
    "cyclic2":  {"partition": ("cyclic", 2)},
    "block2":   {"partition": ("block", 2)},
    "complete": {"partition": ("complete", 0)},
    "reshape2": {"reshape": ("cyclic", 2)},
    "reshapeb2": {"reshape": ("block", 2)},
    "bram":     {"array_interface": "bram"},
    "toppipe":  {"tcl": ["set_directive_pipeline -II 1 {top}"], "clock": 2.5},
    # fault injection: a false "no inter-iteration dependence" pragma on every array in every
    # loop of the top function (Vitis UG1399 warns this can produce incorrect hardware)
    "unsafe_dep": {"inject_dep": True},
}
BAMBU_CONFIGS = {"base": ["-O2"], "O0": ["-O0"], "O3": ["-O3"], "clk5": ["-O2", "--clock-period=5"]}


def struct_layouts(fields):
    """Candidate packings of a struct: 'packed' (fields back to back, as Vitis does for pointer /
    array interfaces) and 'aligned' (C layout with padding, as for by-value ports).
    Field 0 occupies the least significant bits. -> {name: (layout, total_bits)}"""
    out, off = [], 0
    for n, w, s in fields:
        out.append((n, off, w, s))
        off += w
    packed = (out, off)
    al, off, maxa = [], 0, 8
    for n, w, s in fields:
        a = max(8, w)
        off = -(-off // a) * a
        al.append((n, off, w, s))
        off += w
        maxa = max(maxa, a)
    aligned = (al, -(-off // maxa) * maxa)
    return {"packed": packed, "aligned": aligned}


def match_layout(fields, bits, what):
    for kind, (lay, total) in struct_layouts(fields).items():
        if total == bits:
            return lay, total
    raise RuntimeError(f"{what}: struct of {bits} bits matches no known packing")


@dataclass
class Bank:
    ports: dict            # normalized keys: address0 ce0 we0 d0 q0 address1 ...
    aw: int                # address width
    word_width: int        # data width of one word
    byte_addr: bool = False
    wen_bits: int = 1      # >1: byte-enable vector (bram mode)


@dataclass
class ObjPort:
    name: str
    width: int
    signed: bool
    kind: str              # ap_none | ap_vld | ap_ovld
    in_port: str = None
    out_port: str = None
    vld_port: str = None


@dataclass
class Memory:
    name: str
    size: int
    width: int
    signed: bool
    protocol: str = "ap_memory"            # ap_memory | bram | cells
    banks: list = field(default_factory=list)
    cells: list = field(default_factory=list)   # ObjPort per element (complete partition)
    partition: tuple = None                # (kind, B)
    reshape: tuple = None                  # (kind, k)

    @property
    def slots(self):
        return self.reshape[1] if self.reshape else 1

    def locate(self, i):
        """element i -> (bank, word address, slot)"""
        if self.partition:
            kind, B = self.partition
            if kind == "cyclic":
                return i % B, i // B, 0
            c = -(-self.size // B)
            return i // c, i % c, 0
        if self.reshape:
            kind, k = self.reshape
            if kind == "cyclic":
                return 0, i // k, i % k
            c = -(-self.size // k)
            return 0, i % c, i // c
        return 0, i, 0

    def index_expr(self, bank, word, slot=0):
        """Logical element index (Verilog) for (bank, word expr, slot)."""
        if self.partition:
            kind, B = self.partition
            if kind == "cyclic":
                return f"(({word}) * {B} + {bank})"
            return f"({bank} * {-(-self.size // B)} + ({word}))"
        if self.reshape:
            kind, k = self.reshape
            if kind == "cyclic":
                return f"(({word}) * {k} + {slot})"
            return f"({slot} * {-(-self.size // k)} + ({word}))"
        return f"({word})"


@dataclass
class Interface:
    tool: str
    module: str
    control: dict
    ret: str = None
    ret_width: int = 0
    scalars: dict = field(default_factory=dict)
    memories: dict = field(default_factory=dict)
    objects: dict = field(default_factory=dict)
    ports: dict = field(default_factory=dict)
    latency: tuple = None
    interval: int = None

    def to_json(self):
        return json.dumps(asdict(self), indent=2, default=str)


# ------------------------------------------------------------------ Verilog helpers

def module_ports(vfile: Path, top: str):
    """({port: (dir, width)}, instantiation name) of module `top` (handles escaped names)."""
    src = vfile.read_text()
    m = re.search(r"\bmodule\s+(\\?%s)\s*\((.*?)\);(.*?)\bendmodule" % re.escape(top), src, re.S)
    if not m:
        raise RuntimeError(f"module {top} not found in {vfile.name}")
    ports = {}
    for d, w, names in re.findall(r"\b(input|output)\s+(?:wire|reg)?\s*(?:signed\s*)?(\[[^\]]+\])?\s*([\w\s,]+?);",
                                  m.group(0)):
        width = 1
        if w:
            hi, lo = re.match(r"\[\s*(\d+)\s*:\s*(\d+)\s*\]", w).groups()
            width = int(hi) - int(lo) + 1
        for n in names.replace("\n", " ").split(","):
            if n.strip():
                ports[n.strip()] = (d, width)
    name = m.group(1)
    return ports, (name + " ") if name.startswith("\\") else name


# ------------------------------------------------------------------ Vitis

def run_vitis(files, include_dirs, top, outdir: Path, config="base", part=VITIS_PART, timeout=900,
              array_params=()):
    outdir.mkdir(parents=True, exist_ok=True)
    cfg = VITIS_CONFIGS[config]
    cflags = " ".join(f"-iquote{d} -I{d}" for d in include_dirs)
    if cfg.get("inject_dep") and array_params:
        files = list(files)
        src = Path(files[0])
        inj = outdir / "unsafe_src"
        inj.mkdir(parents=True, exist_ok=True)
        text = src.read_text()
        prag = "".join(f"\n#pragma HLS dependence variable={a} type=inter dependent=false" for a in array_params)
        text = re.sub(r"(\b(?:for|while)\s*\([^;{]*(?:;[^;{]*){0,2}\)\s*\{)", lambda m: m.group(1) + prag, text)
        (inj / src.name).write_text(text)
        include_dirs = [str(src.parent)] + list(include_dirs)
        files[0] = inj / src.name
        cflags = " ".join(f"-iquote{d} -I{d}" for d in include_dirs)
    lines = ["open_project -reset proj", f"set_top {top}"]
    for f in files:
        lines.append(f'add_files {f} -cflags "{cflags}"')
    lines += ["open_solution -reset sol", f"set_part {part}", f"create_clock -period {cfg.get('clock', 10)}"]
    lines += [t.format(top=top) for t in cfg.get("tcl", [])]
    for a in array_params:
        if "partition" in cfg:
            kind, factor = cfg["partition"]
            lines.append(f"set_directive_array_partition -type {kind}"
                         + (f" -factor {factor}" if kind != "complete" else "") + f" {top} {a}")
        if "reshape" in cfg:
            kind, factor = cfg["reshape"]
            lines.append(f"set_directive_array_reshape -type {kind} -factor {factor} {top} {a}")
        if cfg.get("array_interface"):
            lines.append(f"set_directive_interface -mode {cfg['array_interface']} {top} {a}")
    lines += ["csynth_design", "exit"]
    (outdir / "run.tcl").write_text("\n".join(lines) + "\n")
    try:
        import resource
        lim = 6 << 30
        p = subprocess.run(["vitis-run", "--mode", "hls", "--tcl", "run.tcl"], cwd=outdir,
                           capture_output=True, text=True, timeout=timeout,
                           preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_AS, (lim, lim)))
        log = p.stdout + p.stderr
    except subprocess.TimeoutExpired:
        raise RuntimeError("Vitis timeout")
    (outdir / "hls.log").write_text(log)
    vdir = outdir / "proj" / "sol" / "syn" / "verilog"
    if not (vdir / f"{top}.v").exists():
        if "HLS 200-103" in log and not top.endswith("_c2sva"):
            # the top name is reserved in Verilog/VHDL (abs, next, loop, bins, ...): synthesise a
            # renamed copy; the interface model reads the HLS top name from top_name.txt
            new = f"{top}_c2sva"
            src = Path(files[0])
            ren = outdir / "renamed_src"
            ren.mkdir(exist_ok=True)
            (ren / src.name).write_text(re.sub(rf"\b{re.escape(top)}(\s*\()", rf"{new}\1", src.read_text()))
            out = run_vitis([ren / src.name] + list(files[1:]), [str(src.parent)] + list(include_dirs), new,
                            outdir, config, part, timeout, array_params)
            (outdir / "top_name.txt").write_text(new)
            return out
        err = [l for l in log.splitlines() if "ERROR" in l][:2]
        raise RuntimeError("Vitis synthesis failed: " + " | ".join(err)[:300])
    (outdir / "top_name.txt").write_text(top)
    return sorted(vdir.glob("*.v"))


def _obj_rows(rows, cname):
    """Report rows of C object `cname`; Vitis renames Verilog keywords (buf -> buf_r) and
    numbers partitioned elements (a_0, a_1, ...)."""
    for base in (cname, cname + "_r"):
        rs = [r for r in rows if r["Object"] == base or re.fullmatch(rf"{re.escape(base)}_\d+", r["Object"] or "")]
        if rs:
            return rs
    return []


def _objport(name, width, signed, rows):
    protos = {r["IOProtocol"] for r in rows}
    kind = "ap_ovld" if "ap_ovld" in protos else "ap_vld" if "ap_vld" in protos else "ap_none"
    ob = ObjPort(name, width, signed, kind)
    for r in rows:
        n = r["name"]
        if n.endswith("_ap_vld"):
            ob.vld_port = n
        elif r["Dir"] == "in":
            ob.in_port = n
        elif r["Dir"] == "out":
            ob.out_port = n
    return ob


BRAM_KEYS = {"Addr": "address", "EN": "ce", "WEN": "we", "Din": "d", "Dout": "q"}


def vitis_interface(outdir: Path, fn, config="base"):
    tf = outdir / "top_name.txt"
    top = tf.read_text().strip() if tf.exists() else fn.name       # renamed when fn.name is reserved
    root = ET.parse(outdir / "proj" / "sol" / "syn" / "report" / f"{top}_csynth.xml").getroot()
    rows = [{c.tag: c.text for c in p} for p in root.find(".//InterfaceSummary")]
    vports, modname = module_ports(outdir / "proj" / "sol" / "syn" / "verilog" / f"{top}.v", top)
    cfg = VITIS_CONFIGS[config]
    ctrl_rows = [r for r in rows if r["Type"] == "return value" and r.get("Attribute") == "control"]
    proto = ctrl_rows[0]["IOProtocol"] if ctrl_rows else "ap_ctrl_none"
    if proto not in ("ap_ctrl_hs", "ap_ctrl_chain"):
        raise RuntimeError(f"block protocol {proto} not supported (no start/done handshake)")
    names = {r["name"] for r in ctrl_rows}
    control = dict(protocol=proto, clk="ap_clk", rst="ap_rst" if "ap_rst" in vports else None,
                   clk_port="ap_clk" in vports, rst_active_low=False, start="ap_start", done="ap_done",
                   idle="ap_idle" if "ap_idle" in names else None,
                   ready="ap_ready" if "ap_ready" in names else None,
                   cont="ap_continue" if "ap_continue" in names else None, overlap=False)
    itf = Interface("vitis", modname, control, ports={k: list(v) for k, v in vports.items()})
    ret_rows = [r for r in rows if r["Type"] == "return value" and r.get("Attribute") != "control"]
    if ret_rows:
        itf.ret, itf.ret_width = ret_rows[0]["name"], int(ret_rows[0]["Bits"])
        if fn.ret_fields:
            fn.ret_layout, fn.ret_width = match_layout(fn.ret_fields, itf.ret_width, "return value")
    for p in fn.params:
        prow = _obj_rows(rows, p.name)
        if p.fields:                                   # struct: element width from the RTL port
            data = [int(r["Bits"]) for r in prow if r["Dir"] == "in" and not r["name"].endswith(("_ap_vld", "ce0", "ce1"))
                    or re.search(r"_[dq][01]$", r["name"])]
            if p.is_array and any(r["IOProtocol"] == "ap_memory" for r in prow):
                data = [int(r["Bits"]) for r in prow if re.search(r"_[dq][01]$", r["name"])]
            if not data:
                data = [int(r["Bits"]) for r in prow if not r["name"].endswith("_ap_vld")]
            bits = data[0]
            rs = cfg.get("reshape") if p.is_array and p.size > 1 else None
            if rs and bits % rs[1] == 0 and bits // rs[1] in [t for _, t in struct_layouts(p.fields).values()]:
                bits //= rs[1]                         # reshaped word holds k struct elements
            p.layout, p.width = match_layout(p.fields, bits, p.name)
        if not prow:
            raise RuntimeError(f"no RTL port for parameter {p.name} (optimized away or unknown mapping)")
        if not p.is_array:
            port = [r for r in prow if r["Type"] == "scalar"]
            if not port:
                raise RuntimeError(f"no scalar port for {p.name}")
            itf.scalars[p.name] = port[0]["name"]
            continue
        protos = {r["IOProtocol"] for r in prow}
        objs = sorted({r["Object"] for r in prow}, key=lambda o: int(o.rsplit("_", 1)[1]) if re.search(r"_\d+$", o) else -1)
        if p.size == 1 and protos <= {"ap_none", "ap_vld", "ap_ovld"}:
            itf.objects[p.name] = _objport(p.name, p.width, p.signed, prow)
            continue
        mem = Memory(p.name, p.size, p.width, p.signed)
        part = cfg.get("partition") if p.size > 1 else None
        if protos <= {"ap_none", "ap_vld", "ap_ovld"}:                    # complete partition
            if len(objs) != p.size:
                raise RuntimeError(f"{p.name}: {len(objs)} element ports for {p.size} elements")
            mem.protocol = "cells"
            mem.cells = [_objport(f"{p.name}[{i}]", p.width, p.signed, [r for r in prow if r["Object"] == o])
                         for i, o in enumerate(objs)]
            itf.memories[p.name] = mem
            continue
        if protos == {"bram"}:
            mem.protocol = "bram"
            ports = {}
            for r in prow:
                m = re.fullmatch(rf"{re.escape(r['Object'])}_(\w+?)_([AB])", r["name"])
                if m and m.group(1) in BRAM_KEYS:
                    ports[BRAM_KEYS[m.group(1)] + ("0" if m.group(2) == "A" else "1")] = r["name"]
            wen = next((int(r["Bits"]) for r in prow if "_WEN_" in r["name"]), 1)
            word = next((int(r["Bits"]) for r in prow if "_Din_" in r["name"] or "_Dout_" in r["name"]), p.width)
            mem.banks = [Bank(ports, 32, word, byte_addr=True, wen_bits=wen)]
            if word != p.width:
                raise RuntimeError(f"{p.name}: bram word width {word} != element width {p.width}")
            itf.memories[p.name] = mem
            continue
        if protos != {"ap_memory"}:
            raise RuntimeError(f"{p.name}: interface protocol {sorted(protos)} not modelled")
        for o in objs:
            ports, aw, word = {}, 1, p.width
            for r in prow:
                if r["Object"] != o:
                    continue
                ports[r["name"][len(o) + 1:]] = r["name"]
                if re.search(r"address[01]$", r["name"]):
                    aw = int(r["Bits"])
                if re.search(r"_[dq][01]$", r["name"]):
                    word = int(r["Bits"])
            mem.banks.append(Bank(ports, aw, word))
        if len(mem.banks) > 1:
            if not part:
                raise RuntimeError(f"{p.name}: banked without a known partition directive")
            mem.partition = (part[0], len(mem.banks))
        if mem.banks[0].word_width != p.width:
            rs = cfg.get("reshape")
            k = mem.banks[0].word_width // p.width
            if not rs or k * p.width != mem.banks[0].word_width:
                raise RuntimeError(f"{p.name}: word width {mem.banks[0].word_width} with unknown packing")
            mem.reshape = (rs[0], k)
        itf.memories[p.name] = mem
    try:
        itf.latency = (int(root.findtext(".//SummaryOfOverallLatency/Best-caseLatency")),
                       int(root.findtext(".//SummaryOfOverallLatency/Worst-caseLatency")))
    except (TypeError, ValueError):
        itf.latency = None
    try:
        itf.interval = int(root.findtext(".//SummaryOfOverallLatency/Interval-min"))
    except (TypeError, ValueError):
        itf.interval = None
    # Calls overlap when the top function is pipelined (or its loop auto-rewinds): a new call
    # may be accepted on ap_ready before the previous one has signalled ap_done.
    ptype = (root.findtext(".//PerformanceEstimates//PipelineType") or root.findtext(".//PipelineType") or "").lower()
    control["pipeline_type"] = ptype
    if ptype and ptype not in ("no", "none") and control["ready"] and control["clk_port"] and \
            (itf.interval is None or not itf.latency or itf.interval <= itf.latency[1]):
        control["overlap"] = True
    return itf


# ------------------------------------------------------------------ Bambu (kept for comparison)

def run_bambu(files, include_dirs, top, outdir: Path, config="base", timeout=900, array_params=()):
    outdir.mkdir(parents=True, exist_ok=True)
    cmd = ["bambu"] + [str(f) for f in files] + [f"--top-fname={top}", "--generate-interface=INFER"]
    cmd += BAMBU_CONFIGS[config] + [f"-I{d}" for d in include_dirs]
    try:
        p = subprocess.run(cmd, cwd=outdir, capture_output=True, text=True, timeout=timeout)
        log = p.stdout + p.stderr
    except subprocess.TimeoutExpired:
        raise RuntimeError("Bambu timeout")
    (outdir / "hls.log").write_text(" ".join(cmd) + "\n" + log)
    rtl = outdir / f"{top}.v"
    if not rtl.exists():
        err = [l for l in log.splitlines() if "rror" in l][:2]
        raise RuntimeError("Bambu synthesis failed: " + " | ".join(err)[:300])
    return [rtl]


def bambu_interface(outdir: Path, fn, config="base"):
    vports, modname = module_ports(outdir / f"{fn.name}.v", fn.name)
    control = dict(protocol="bambu", clk="clock", rst="reset" if "reset" in vports else None,
                   clk_port="clock" in vports, rst_active_low=True, start="start_port", done="done_port",
                   idle=None, ready=None, cont=None, overlap=False)
    itf = Interface("bambu", modname, control, ports={k: list(v) for k, v in vports.items()})
    if "return_port" in vports:
        itf.ret, itf.ret_width = "return_port", vports["return_port"][1]
    for p in fn.params:
        if not p.is_array:
            if p.name not in vports:
                raise RuntimeError(f"no port for scalar {p.name}")
            itf.scalars[p.name] = p.name
            continue
        ports = {k: f"{p.name}_{k}" for k in ("address0", "ce0", "we0", "d0", "q0",
                                              "address1", "ce1", "we1", "d1", "q1")
                 if f"{p.name}_{k}" in vports}
        if not ports:
            raise RuntimeError(f"no memory ports for array {p.name}")
        addr = ports.get("address0") or ports.get("address1")
        itf.memories[p.name] = Memory(p.name, p.size, p.width, p.signed,
                                      banks=[Bank(ports, vports[addr][1] if addr else 1, p.width)])
    return itf


TOOLS = {
    "vitis": (run_vitis, vitis_interface, VITIS_CONFIGS),
    "bambu": (run_bambu, bambu_interface, BAMBU_CONFIGS),
}
