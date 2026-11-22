"""Translation-only check (no HLS, no formal): front end + rules against a synthetic interface.

Used by `c2sva translate` to measure contract-translation coverage quickly over a whole suite
(every memory is a plain single-bank RAM, every scalar a port, structs use the packed layout).
"""
import json
import multiprocessing as mp
import resource
from pathlib import Path

from . import acsl, frontend, translate
from .hls import Bank, Interface, Memory, ObjPort, struct_layouts


def synthetic_interface(fn):
    itf = Interface("synthetic", "m", dict(clk="c", rst="r", start="s", done="d", rst_active_low=False))
    if fn.ret_width:
        itf.ret, itf.ret_width = "ret", fn.ret_width
        if fn.ret_fields:
            fn.ret_layout, fn.ret_width = struct_layouts(fn.ret_fields)["aligned"]
    for p in fn.params:
        if p.fields:
            p.layout, p.width = struct_layouts(p.fields)["packed"]
        if not p.is_array:
            itf.scalars[p.name] = p.name
        elif p.size == 1:
            itf.objects[p.name] = ObjPort(p.name, p.width, p.signed, "ap_ovld", f"{p.name}_i", f"{p.name}_o",
                                          f"{p.name}_o_ap_vld")
        else:
            ports = {"address0": "a0", "ce0": "c0", "we0": "w0", "d0": "d0", "q0": "q0"}
            itf.memories[p.name] = Memory(p.name, p.size, p.width, p.signed,
                                          banks=[Bank(ports, max(1, (p.size - 1).bit_length()), p.width)])
    return itf


def check(spec, workdir, array_size=4):
    out = {"id": spec.id}
    try:
        src = Path(spec.files[0]).read_text(errors="replace")
        prep = frontend.prepare(spec, src, workdir, array_size)
        fn = prep.fn
        itf = synthetic_interface(fn)
        props, skipped, tr = translate.build_properties(fn, itf, None, 72, array_size + 2)
        out.update(status="ok", top=fn.name,
                   translated=len([p for p in props if p.kind != "term"]),
                   skipped=[s["reason"][:90] for s in skipped],
                   notes=sorted({n.split(":")[0][:60] for p in props for n in p.notes}))
    except acsl.Unsupported as e:
        out.update(status="unsupported", reason=str(e)[:120])
    except Exception as e:                                     # front-end crash = a bug to fix
        out.update(status="crash", reason=f"{type(e).__name__}: {str(e)[:120]}")
    return out


def _worker(args):
    spec, workdir, n = args
    resource.setrlimit(resource.RLIMIT_AS, (3 << 30, 3 << 30))
    return check(spec, workdir, n)


def run(specs, outdir: Path, jobs=4, array_size=4):
    outdir.mkdir(parents=True, exist_ok=True)
    tasks = [(s, outdir / s.id.replace("/", "__"), array_size) for s in specs]
    results = []
    with mp.Pool(jobs, maxtasksperchild=1) as pool:
        for r in pool.imap_unordered(_worker, tasks):
            results.append(r)
    (outdir / "translate.jsonl").write_text("\n".join(json.dumps(r) for r in results) + "\n")
    return results
