r"""Counterexample replay: validate every RTL failure at the C level with an independent oracle.

From the solver's trace we take the call that violated a property:
  inputs  : scalar arguments and the pre-state of every memory/object at call start
  outputs : the RTL result and the post-state of every memory/object after ap_done
We then run the *original C function* on the same inputs, compiled with E-ACSL (Frama-C's
runtime assertion checker, exact ACSL semantics with arbitrary-precision integers), and print
its result and post-state. Classification:
  c-contract-violation   E-ACSL reports a failed postcondition: the C code itself breaks its
                         contract on this input (a genuine bug, e.g. a mutant)
  c-runtime-error        E-ACSL reports undefined behaviour (out-of-bounds access, overflow,
                         division by zero) on this input: a genuine bug in the C code
Loop annotations are removed before replay (they are proof artefacts, not the contract).
  invalid-witness        E-ACSL reports a failed precondition: the input should have been
                         excluded by the harness assumptions (translation gap)
  rtl-c-divergence       C satisfies the contract but RTL outputs differ from C outputs on the
                         same input: the hardware does not implement the C semantics (checked
                         first: if the C output depends on uninitialised locals, i.e. differs
                         between -ftrivial-auto-var-init=zero and =pattern, it is a
                         c-runtime-error instead)
  translation-error      C satisfies the contract (E-ACSL and the concrete evaluator agree) and
                         RTL outputs equal C outputs, yet the RTL check failed: the generated
                         property is wrong (framework bug)
  undetermined           C and RTL agree, E-ACSL found nothing, and the concrete evaluator could
                         not decide the violated clause
E-ACSL does not support every construct (it ignores `assigns` and some \old forms), so every
clause is also checked by the concrete evaluator (c2sva.oracle) on the C pre/post states.
  c-nontermination       the C function does not terminate on this input (genuine bug)
  replay-error           the replay could not be built or run
"""
import re
import subprocess
from pathlib import Path

from .acsl import resolve_type


def parse_vcd(path: Path):
    """-> list of {signal: int-or-None} per time point, for signals of the top scope 'harness'."""
    ids, widths, scope = {}, {}, []
    snaps, cur, t = [], {}, None
    with open(path) as fh:
        for line in fh:
            line = line.strip()
            if line.startswith("$scope"):
                scope.append(line.split()[2])
                continue
            if line.startswith("$upscope"):
                scope.pop()
                continue
            if line.startswith("$var"):
                parts = line.split()
                if scope == ["harness"]:
                    ids.setdefault(parts[3], parts[4])
                    widths[parts[4]] = int(parts[2])
                continue
            if line.startswith("#"):
                if t is not None:
                    snaps.append(dict(cur))
                t = int(line[1:])
                continue
            if line.startswith("b"):
                val, i = line[1:].split()
                if i in ids:
                    cur[ids[i]] = int(val, 2) if set(val) <= {"0", "1"} else None
            elif line[:1] in "01xz" and line[1:] in ids:
                cur[ids[line[1:]]] = int(line[0]) if line[0] in "01" else None
    if t is not None:
        snaps.append(dict(cur))
    # one snapshot per clock cycle (the trace has a clock-high and a clock-low point per step)
    clk = next((n for n in ("ap_clk", "clock") if n in widths), None)
    if clk:
        snaps = [s for s in snaps if s.get(clk) == 1] or snaps
    return snaps, widths


def signed(v, w):
    if v is None:
        return 0
    return v - (1 << w) if v >> (w - 1) & 1 else v


def extract(snaps, widths, fn, itf, fail_step=None):
    """The trace ends at the violation, so the violating call is the last one: its inputs at its
    start (last h_start_evt before the last h_done_d) and its outputs at that h_done_d."""
    # Calls complete in order, so the k-th completion belongs to the k-th accepted call
    # (this also holds when calls overlap).
    last = fail_step if fail_step is not None and fail_step < len(snaps) else len(snaps) - 1
    dones = [i for i, s in enumerate(snaps[:last + 1]) if s.get("h_done_d") == 1]
    starts = [i for i, s in enumerate(snaps[:last + 1]) if s.get("h_start_evt") == 1]
    if not starts:
        return None
    if dones and len(starts) >= len(dones):
        i1, i0 = dones[-1], starts[len(dones) - 1]
    else:
        i1, i0 = None, starts[-1]
    def raw(s, name, width, sgn):
        if name not in widths:
            raise KeyError(name)
        v = s.get(name) or 0
        v &= (1 << width) - 1
        return signed(v, width) if sgn else v

    def split(v, layout):
        """raw struct bits -> {field: value}"""
        out = {}
        for f, off, fw, fs in layout:
            x = (v >> off) & ((1 << fw) - 1)
            out[f] = signed(x, fw) if fs else x
        return out

    def cell(s, a, i, post_state):
        p = pmap[a]
        if a in itf.objects or (itf.memories.get(a) and itf.memories[a].protocol == "cells"):
            ob = itf.objects.get(a) or itf.memories[a].cells[i]
            name = f"m_{a}_{i}" if (post_state or not ob.in_port) else ob.in_port
        else:
            name = f"m_{a}_{i}"
        if p.layout:
            return split(raw(s, name, p.width, False), p.layout)
        return raw(s, name, p.width, p.signed)

    pmap = {p.name: p for p in fn.params}
    ins, pre, post = {}, {}, {}
    s0 = snaps[i0]
    for c, port in itf.scalars.items():
        v = raw(s0, port, pmap[c].width, pmap[c].signed and not pmap[c].layout)   # the harness input port
        ins[c] = split(v, pmap[c].layout) if pmap[c].layout else v
    for p in fn.params:
        if p.is_array:
            pre[p.name] = [cell(s0, p.name, i, False) for i in range(p.size)]
            if i1 is not None:
                post[p.name] = [cell(snaps[i1], p.name, i, True) for i in range(p.size)]
    res = None
    if i1 is not None and itf.ret:
        res = raw(snaps[i1], "h_res_cap", itf.ret_width, fn.ret_signed and not fn.ret_layout)
        if fn.ret_layout:
            res = split(res, fn.ret_layout)
    return {"inputs": ins, "pre": pre, "rtl_post": post, "rtl_result": res, "completed": i1 is not None}


def _elem_type(p):
    t = p.ctype.replace("*", " ").replace("const", " ")
    return " ".join(t.split())


def _init(v):
    if isinstance(v, dict):
        return "{" + ", ".join(f".{f} = {x}LL" for f, x in v.items()) + "}"
    return f"{v}LL"


def driver(fn, w):
    """main() appended to the design's own translation unit (so typedefs/structs are in scope):
    calls the top function on the witness and prints result and post-state."""
    L = ["", "#undef main", "int printf(const char *, ...);   /* no <stdio.h>: it may clash with design names */",
         "int main(void) {"]
    args = []
    for p in fn.params:
        t = _elem_type(p)
        if p.is_array:
            vals = ", ".join(_init(v) for v in w["pre"][p.name])
            L.append(f"  {t} {p.name}_m[{p.size}] = {{{vals}}};")
            args.append(f"{p.name}_m")
        else:
            v = w["inputs"][p.name]
            L.append(f"  {t} {p.name}_v = {_init(v) if isinstance(v, dict) else f'({t}){v}LL'};")
            args.append(f"{p.name}_v")
    call = f"{fn.name}({', '.join(args)})"
    if fn.ret_width and fn.ret_layout:
        L.append(f"  __typeof__({call}) r = {call};")
        for f, *_ in fn.ret_layout:
            L.append(f'  printf("RESF {f} %lld\\n", (long long)r.{f});')
    elif fn.ret_width:
        L.append(f"  long long r = (long long)({call});")
        L.append('  printf("RES %lld\\n", r);')
    else:
        L.append(f"  {call};")
    for p in fn.params:
        if not p.is_array:
            continue
        if p.layout:
            for f, *_ in p.layout:
                L.append(f'  for (int i = 0; i < {p.size}; i++) printf("MEMF {p.name} %d {f} %lld\\n", i, (long long){p.name}_m[i].{f});')
        else:
            L.append(f'  for (int i = 0; i < {p.size}; i++) printf("MEM {p.name} %d %lld\\n", i, (long long){p.name}_m[i]);')
    L.append("  return 0;\n}")
    return "\n".join(L) + "\n"


def replay(prep, fn, itf, task_dir: Path, outdir: Path, timeout=300, fail_step=None):
    outdir, task_dir = Path(outdir).resolve(), Path(task_dir).resolve()
    outdir.mkdir(parents=True, exist_ok=True)
    vcds = sorted(task_dir.glob("engine_*/trace*.vcd"))
    if not vcds:
        return {"class": "replay-error", "detail": "no trace"}
    snaps, widths = parse_vcd(vcds[0])
    try:
        w = extract(snaps, widths, fn, itf, fail_step)
    except KeyError as e:
        return {"class": "replay-error", "detail": f"signal {e} not in trace"}
    if not w:
        return {"class": "replay-error", "detail": "no call in the trace"}
    (outdir / "witness.txt").write_text(repr(w))
    top_src = outdir / Path(prep.spec.files[0]).name    # variant source without loop annotations
    text = Path(prep.spec.files[0]).read_text(errors="replace")
    text = re.sub(r"/\*@\s*loop\b.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//@\s*loop[^\n]*", "", text)
    top_src.write_text("#define main c2sva_original_main\n" + text + driver(fn, w))
    files = [str(top_src)] + [str(f) for f in prep.hls_files[1:]]
    incs = " ".join(f"-iquote{d} -I{d}" for d in prep.include_dirs)
    exe = outdir / "replay"
    cmd = ["e-acsl-gcc", "--rte=all", "-c", "-X", "-O", str(exe), "-o", str(outdir / "instrumented.c")] + \
          (["-E", incs, "-e", incs] if incs else []) + files
    try:
        b = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, cwd=outdir)
        blog = b.stdout + b.stderr
    except subprocess.TimeoutExpired:
        blog = "E-ACSL build timeout"
    (outdir / "build.log").write_text(" ".join(cmd) + "\n" + blog)
    binary, checker = Path(str(exe) + ".e-acsl"), "e-acsl"
    def native():
        nb = outdir / "replay.native"
        gcc = ["gcc", "-std=gnu11", "-O0", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=undefined",
               "-o", str(nb)] + [x for d in prep.include_dirs for x in (f"-iquote{d}", f"-I{d}")] + files
        g = subprocess.run(gcc, capture_output=True, text=True, timeout=timeout, cwd=outdir)
        (outdir / "build_native.log").write_text(" ".join(gcc) + "\n" + g.stdout + g.stderr)
        return nb if nb.exists() else None

    if not binary.exists():
        # E-ACSL cannot instrument this program (unsupported construct / internal error): run the
        # plain C code with AddressSanitizer + UndefinedBehaviorSanitizer; the contract itself is
        # then checked by the concrete evaluator only.
        binary, checker = native(), "native+sanitizers"
        if binary is None:
            return {"class": "replay-error", "detail": "E-ACSL and native builds failed", "witness": w}
    try:
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=60, cwd=outdir)
    except subprocess.TimeoutExpired:
        return {"class": "c-nontermination", "detail": "C execution did not terminate within 60 s", "witness": w,
                "checker": checker}
    (outdir / "run.log").write_text(run.stdout + run.stderr)
    err = run.stderr
    c_res = re.search(r"RES (-?\d+)", run.stdout)
    c_resf = {f: int(v) for f, v in re.findall(r"RESF (\w+) (-?\d+)", run.stdout)}
    c_mem = {}
    for a, i, v in re.findall(r"MEM (\w+) (\d+) (-?\d+)", run.stdout):
        c_mem.setdefault(a, {})[int(i)] = int(v)
    for a, i, f, v in re.findall(r"MEMF (\w+) (\d+) (\w+) (-?\d+)", run.stdout):
        c_mem.setdefault(a, {}).setdefault(int(i), {})[f] = int(v)
    # E-ACSL reports '<file>: In function 'f'' then 'Error: <Kind> failed:' and the predicate
    failing, func, only_unsigned = [], None, False
    lines = err.splitlines()
    for k, line in enumerate(lines):
        m = re.search(r"In function '(\w+)'", line)
        if m:
            func = m.group(1)
        m = re.search(r"Error: (\w[\w ]*?) failed:", line)
        if m:
            pred = " ".join(x.strip() for x in lines[k + 2:k + 4])[:160]
            if re.search(r"unsigned_(overflow|downcast)", " ".join(lines[k:k + 4])):
                only_unsigned = True
                continue          # unsigned wrap-around is defined C behaviour, not a runtime error
            failing.append((m.group(1), func, pred))
    if only_unsigned and not failing and checker == "e-acsl":
        # E-ACSL stopped at a defined-behaviour check: take the outputs from the plain build
        nb = native()
        if nb is not None:
            try:
                run = subprocess.run([str(nb)], capture_output=True, text=True, timeout=60, cwd=outdir)
            except subprocess.TimeoutExpired:
                return {"class": "c-nontermination", "detail": "C execution did not terminate within 60 s",
                        "witness": w, "checker": "native+sanitizers"}
            (outdir / "run_native.log").write_text(run.stdout + run.stderr)
            err, checker = run.stderr, "native+sanitizers (E-ACSL stopped at unsigned wrap-around)"
            c_res = re.search(r"RES (-?\d+)", run.stdout)
            c_resf = {f: int(v) for f, v in re.findall(r"RESF (\w+) (-?\d+)", run.stdout)}
            c_mem = {}
            for a, i, v in re.findall(r"MEM (\w+) (\d+) (-?\d+)", run.stdout):
                c_mem.setdefault(a, {})[int(i)] = int(v)
            for a, i, f, v in re.findall(r"MEMF (\w+) (\d+) (\w+) (-?\d+)", run.stdout):
                c_mem.setdefault(a, {}).setdefault(int(i), {})[f] = int(v)
    sanitizer = re.search(r"runtime error:.*|ERROR: AddressSanitizer: [\w-]+", err)
    crashed = run.returncode < 0 or run.returncode >= 128
    if not failing and not sanitizer and checker == "e-acsl":
        # second checker: AddressSanitizer red zones catch out-of-bounds accesses that land in
        # another valid object (E-ACSL only checks that the address is valid somewhere)
        nb = native()
        if nb is not None:
            try:
                nr = subprocess.run([str(nb)], capture_output=True, text=True, timeout=60, cwd=outdir)
                (outdir / "run_native.log").write_text(nr.stdout + nr.stderr)
                sanitizer = re.search(r"runtime error:.*|ERROR: AddressSanitizer: [\w-]+", nr.stderr) or sanitizer
                if sanitizer:
                    checker = "e-acsl + sanitizers"
            except subprocess.TimeoutExpired:
                pass
    out = {"witness": w, "c_result": c_resf if c_resf else (int(c_res.group(1)) if c_res else None),
           "c_post": {a: [d[i] for i in sorted(d)] for a, d in c_mem.items()},
           "eacsl": [list(x) for x in failing[:3]], "checker": checker}
    if failing:
        kind, f0, pred = failing[0]
        if kind == "Precondition" and f0 == fn.name:
            out["class"] = "invalid-witness"
        elif kind in ("Precondition", "Postcondition"):
            out["class"] = "c-contract-violation"
            if kind == "Precondition":
                out["detail"] = f"callee {f0} called outside its precondition"
        else:
            out["class"] = "c-runtime-error"
    elif sanitizer:
        out["class"] = "c-runtime-error"
        out["detail"] = sanitizer.group(0)[:160]
        out["checker"] = checker
    elif crashed:
        out["class"] = "c-runtime-error"
        out["detail"] = f"C execution crashed (exit code {run.returncode})"
    elif run.returncode != 0 and not (c_res or c_resf) and fn.ret_width:
        out["class"] = "replay-error"
        out["detail"] = f"C run exit code {run.returncode}"
    else:
        from .oracle import evaluate
        scal = {c: w["inputs"][c] for c in w["inputs"]}
        post_c = {a: v for a, v in out["c_post"].items()}
        ora = evaluate(fn, scal, w["pre"], post_c, out["c_result"])
        out["oracle"] = ora
        same = True
        if fn.ret_width and w["completed"] and w["rtl_result"] is not None and out["c_result"] is not None:
            if isinstance(out["c_result"], dict):
                same &= w["rtl_result"] == out["c_result"]
            else:
                mask = (1 << fn.ret_width) - 1
                same &= (w["rtl_result"] & mask) == (out["c_result"] & mask)
        for a, vals in w["rtl_post"].items():
            if a in out["c_post"]:
                same &= vals == out["c_post"][a]
        if not same and _reads_uninitialised(files, prep, outdir, timeout):
            # the C result depends on the initial value of uninitialised locals (undefined
            # behaviour no sanitizer here reports); the RTL simply holds a different value
            out["class"] = "c-runtime-error"
            out["detail"] = "reads uninitialised memory (output differs between zero and pattern auto-initialisation)"
            out["checker"] = checker + " + auto-init differential"
        elif not same:
            out["class"] = "rtl-c-divergence"
        elif ora["requires_violated"]:
            out["class"] = "invalid-witness"
        elif ora["violated"]:
            out["class"] = "c-contract-violation"
        elif ora["unknown"]:
            out["class"] = "undetermined"
        else:
            out["class"] = "translation-error"
    return out


def _reads_uninitialised(files, prep, outdir, timeout):
    """Build the replay twice with locals auto-initialised to zero and to a pattern; a different
    output means the C code reads uninitialised memory."""
    outs = []
    for init in ("zero", "pattern"):
        exe = outdir / f"replay.init_{init}"
        gcc = ["gcc", "-std=gnu11", "-O0", f"-ftrivial-auto-var-init={init}", "-o", str(exe)] + \
              [x for d in prep.include_dirs for x in (f"-iquote{d}", f"-I{d}")] + list(files)
        try:
            subprocess.run(gcc, capture_output=True, text=True, timeout=timeout, cwd=outdir)
            if not exe.exists():
                return False
            r = subprocess.run([str(exe)], capture_output=True, text=True, timeout=60, cwd=outdir)
        except subprocess.TimeoutExpired:
            return False
        outs.append(re.findall(r"^(?:RES|RESF|MEM|MEMF) .*$", r.stdout, flags=re.M))
    return outs[0] != outs[1]
