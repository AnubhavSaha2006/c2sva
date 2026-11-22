"""Analysis of surviving mutants (RTL check passed, Frama-C/WP fails on the mutant).

A survivor is either (a) a bug the RTL properties missed (false negative), (b) a memory-safety
bug that does not change any contract-relevant output (the functional properties cannot see it;
the access check is shared with benign HLS speculation), or (c) a mutant that is equivalent
w.r.t. the contract (WP fails only because the loop annotations no longer hold).

We decide by bounded random testing of the mutant C code: input vectors are drawn at random
(biased to small and extreme values), kept only if they satisfy every precondition (concrete
evaluator) and fit the N-cell memories, and the C code is run natively with AddressSanitizer +
UndefinedBehaviorSanitizer; the whole contract is evaluated on each run.
  missed-bug            some vector violates a translated postcondition / frame condition
  untranslated-clause   the only violated clauses are ones the translation reported as not
                        translated (expected: the RTL check is partial for this design)
  memory-safety-only    no contract violation, but a sanitizer reports undefined behaviour
  no-violation-found    neither, over all vectors (likely equivalent)
"""
import random
import re
import subprocess
from pathlib import Path

from .acsl import walk
from .logic import expand
from .oracle import Evaluator, Unknown, evaluate


def _stdin_driver(fn):
    L = ["", "#undef main", "int printf(const char *, ...);", "int scanf(const char *, ...);",
         "int main(void) {"]
    args = []

    def rd(target):
        L.append(f"  {{ long long t; if (scanf(\"%lld\", &t) != 1) return 3; {target} = t; }}")

    for p in fn.params:
        t = " ".join(p.ctype.replace("*", " ").replace("const", " ").split())
        if p.is_array:
            L.append(f"  {t} {p.name}_m[{p.size}];")
            for i in range(p.size):
                if p.layout:
                    for f, *_ in p.layout:
                        rd(f"{p.name}_m[{i}].{f}")
                else:
                    rd(f"{p.name}_m[{i}]")
            args.append(f"{p.name}_m")
        else:
            L.append(f"  {t} {p.name}_v;")
            if p.layout:
                for f, *_ in p.layout:
                    rd(f"{p.name}_v.{f}")
            else:
                rd(f"{p.name}_v")
            args.append(f"{p.name}_v")
    call = f"{fn.name}({', '.join(args)})"
    if fn.ret_width and fn.ret_layout:
        L.append(f"  __typeof__({call}) r = {call};")
        for f, *_ in fn.ret_layout:
            L.append(f'  printf("RESF {f} %lld\\n", (long long)r.{f});')
    elif fn.ret_width:
        L.append(f'  printf("RES %lld\\n", (long long)({call}));')
    else:
        L.append(f"  {call};")
    for p in fn.params:
        if not p.is_array:
            continue
        for i in range(p.size):
            if p.layout:
                for f, *_ in p.layout:
                    L.append(f'  printf("MEMF {p.name} {i} {f} %lld\\n", (long long){p.name}_m[{i}].{f});')
            else:
                L.append(f'  printf("MEM {p.name} {i} %lld\\n", (long long){p.name}_m[{i}]);')
    L.append("  return 0;\n}")
    return "\n".join(L) + "\n"


def _val(rng, width, signed, low_entropy=False):
    lo, hi = (-(1 << (width - 1)), (1 << (width - 1)) - 1) if signed else (0, (1 << width) - 1)
    if low_entropy:                       # repeated values reach 'all equal' / 'none found' paths
        return rng.choice([0, 1])
    r = rng.random()
    if r < 0.7:
        v = rng.randint(-3 if signed else 0, 6)
    elif r < 0.85:
        v = rng.choice([lo, hi, lo + 1, hi - 1, 0])
    else:
        v = rng.randint(lo, hi)
    return max(lo, min(hi, v))


def _vector(rng, fn):
    ins, pre = {}, {}
    low = rng.random() < 0.35
    for p in fn.params:
        le = low and (p.is_array or p.signed)
        gen = (lambda: {f: _val(rng, w, s, le) for f, _, w, s in p.layout}) if p.layout else \
            (lambda p=p: _val(rng, p.width, p.signed, le))
        if p.is_array:
            pre[p.name] = [gen() for _ in range(p.size)]
        else:
            ins[p.name] = gen()
    return ins, pre


def _requires_ok(fn, ins, pre, cutoff):
    ev = Evaluator(fn, ins, pre, pre, None)
    ptrs = frozenset(p.name for p in fn.params if p.is_array)
    sizes = {p.name: p.size for p in fn.params if p.is_array}
    for beh in [fn.default]:
        for _, _, tree in beh.requires:
            for x in walk(tree):                         # every \valid range must fit the memory
                if x[0] == "valid" and x[1] in sizes:
                    try:
                        lo = ev.ev(expand(x[2], fn.defs, cutoff, ptrs), {}, False, False)
                        hi = ev.ev(expand(x[3], fn.defs, cutoff, ptrs), {}, False, False)
                    except Unknown:
                        return False
                    if hi >= lo and not (0 <= lo and hi < sizes[x[1]]):
                        return False
            try:
                if not ev.ev(expand(tree, fn.defs, cutoff, ptrs), {}, False, False):
                    return False
            except Unknown:
                return False
    return True


def analyse(prep, fn, outdir: Path, vectors=1000, seed=0, tries=100000, untranslated=()):
    outdir = Path(outdir).resolve()
    outdir.mkdir(parents=True, exist_ok=True)
    text = Path(prep.spec.files[0]).read_text(errors="replace")
    text = re.sub(r"/\*@\s*loop\b.*?\*/", "", text, flags=re.S)
    src = outdir / Path(prep.spec.files[0]).name
    src.write_text("#define main c2sva_original_main\n" + text + _stdin_driver(fn))
    exe = outdir / "survivor.native"
    gcc = ["gcc", "-std=gnu11", "-O0", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=undefined",
           "-o", str(exe)] + [x for d in prep.include_dirs for x in (f"-iquote{d}", f"-I{d}")] + \
          [str(src)] + [str(f) for f in prep.hls_files[1:]]
    g = subprocess.run(gcc, capture_output=True, text=True)
    (outdir / "build.log").write_text(" ".join(gcc) + "\n" + g.stdout + g.stderr)
    if not exe.exists():
        return {"class": "analysis-error", "detail": "build failed"}
    rng = random.Random(seed)
    cutoff = max([p.size for p in fn.params if p.is_array], default=4) + 2
    tested, safety, n_try, partial = 0, None, 0, None
    while tested < vectors and n_try < tries:
        n_try += 1
        ins, pre = _vector(rng, fn)
        if not _requires_ok(fn, ins, pre, cutoff):
            continue
        tested += 1
        feed = []
        for p in fn.params:
            vals = pre[p.name] if p.is_array else [ins[p.name]]
            for v in vals:
                feed += [str(v[f]) for f, *_ in p.layout] if p.layout else [str(v)]
        try:
            run = subprocess.run([str(exe)], input=" ".join(feed), capture_output=True, text=True, timeout=10)
        except subprocess.TimeoutExpired:
            return {"class": "missed-bug", "detail": "non-termination", "vector": [ins, pre], "tested": tested}
        san = re.search(r"runtime error:.*|ERROR: AddressSanitizer: [\w-]+", run.stderr)
        res = re.search(r"RES (-?\d+)", run.stdout)
        resf = {f: int(v) for f, v in re.findall(r"RESF (\w+) (-?\d+)", run.stdout)}
        post = {}
        for a, i, v in re.findall(r"MEM (\w+) (\d+) (-?\d+)", run.stdout):
            post.setdefault(a, [None] * len(pre[a]))[int(i)] = int(v)
        for a, i, f, v in re.findall(r"MEMF (\w+) (\d+) (\w+) (-?\d+)", run.stdout):
            post.setdefault(a, [{} for _ in pre[a]])[int(i)][f] = int(v)
        if san and safety is None:
            safety = {"detail": san.group(0)[:160], "vector": [ins, pre]}
        if (run.returncode != 0 and not san) or (san and not (res or resf or post)):
            continue
        result = resf or (int(res.group(1)) if res else None)
        ora = evaluate(fn, ins, pre, post, result)
        norm = lambda t: re.sub(r"\s+", "", t)
        skipped = {norm(u) for u in untranslated}
        missed = [v for v in ora["violated"] if not any(norm(v).startswith(u) or u.startswith(norm(v)) for u in skipped)]
        if missed:
            return {"class": "missed-bug", "detail": missed[0][:160], "vector": [ins, pre], "tested": tested}
        if ora["violated"] and partial is None:
            partial = {"detail": ora["violated"][0][:160], "vector": [ins, pre]}
    if tested == 0:
        return {"class": "analysis-error", "detail": "no input satisfies the preconditions"}
    if partial:
        return {"class": "untranslated-clause", "tested": tested, **partial}
    if safety:
        return {"class": "memory-safety-only", "tested": tested, **safety}
    return {"class": "no-violation-found", "tested": tested}
