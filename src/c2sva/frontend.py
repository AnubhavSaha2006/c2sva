"""Front end: from C files (+ include dirs) to a canonical contract and HLS-ready sources.

* Contracts come from Frama-C's own normalized print (`frama-c -print`): headers merged,
  macros expanded, predicate overloads and implicit \\old made explicit. One syntax for
  every benchmark, no regex guessing over raw sources.
* HLS sources are the original files (so HLS sees the code as written), with the top
  function's pointer parameters rewritten to fixed-size arrays and any called functions
  that live in other files of the benchmark corpus linked in.
* The top function is given, or chosen automatically as the root of the call graph among
  functions that carry a contract.
"""
import re
import shutil
import subprocess
from dataclasses import dataclass, field
from pathlib import Path

from . import acsl


@dataclass
class DesignSpec:
    """A design to process. Produced by benchmark adapters; nothing design-specific elsewhere."""
    id: str
    files: list                       # C files; files[0] holds the top function
    include_dirs: list = field(default_factory=list)
    top: str = None
    corpus: list = field(default_factory=list)   # dirs searched for callee definitions
    meta: dict = field(default_factory=dict)      # e.g. {"reference_proven": True}


def _cpp_args(spec):
    # -iquote first: benchmark headers must win over same-named system/libc headers (e.g. search.h)
    return " ".join(f"-iquote{d}" for d in spec.include_dirs) + " " + " ".join(f"-I{d}" for d in spec.include_dirs)


def framac_print(spec: DesignSpec, files, out: Path, timeout=120):
    cmd = ["frama-c", "-no-unicode", "-print", "-ocode", str(out)]
    if spec.include_dirs:
        cmd.append(f"-cpp-extra-args={_cpp_args(spec)}")
    cmd += [str(f) for f in files]
    p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
    if not out.exists():
        err = [l for l in (p.stdout + p.stderr).splitlines() if "rror" in l][:2]
        raise acsl.Unsupported("Frama-C could not parse the input: " + " | ".join(err)[:200])
    return out.read_text()


def framac_wp(spec: DesignSpec, files, top, logfile: Path, timeout=300, goal_timeout=10,
              provers="alt-ergo,z3,cvc5"):
    cmd = ["frama-c", "-wp", "-wp-rte", "-warn-unsigned-overflow", "-warn-unsigned-downcast",
           "-wp-prover", provers, "-wp-timeout", str(goal_timeout), "-wp-fct", top]
    if spec.include_dirs:
        cmd.append(f"-cpp-extra-args={_cpp_args(spec)}")
    cmd += [str(f) for f in files]
    try:
        import resource
        lim = 3 << 30
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout,
                           preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_AS, (lim, lim)))
        out = p.stdout + p.stderr
    except subprocess.TimeoutExpired:
        logfile.write_text("timeout")
        return None
    logfile.write_text(" ".join(cmd) + "\n\n" + out)
    m = re.search(r"Proved goals:\s+(\d+)\s*/\s*(\d+)", out)
    return (int(m.group(1)), int(m.group(2))) if m else None


# ------------------------------------------------------------------ call graph / top selection

def _strip(src):
    src = re.sub(r"/\*.*?\*/|//[^\n]*", lambda m: " " * len(m.group(0)), src, flags=re.S)
    return re.sub(r'"(?:\\.|[^"\\])*"', lambda m: '"' + " " * (len(m.group(0)) - 2) + '"', src)


def defined_functions(src):
    """{name: (start, body_end)} for function definitions in C source."""
    code = _strip(src)
    out = {}
    for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\(([^;{}()]*)\)\s*\{", code):
        name = m.group(1)
        if name in ("if", "for", "while", "switch", "return", "sizeof"):
            continue
        depth, i = 0, m.end() - 1
        while i < len(code):
            depth += code[i] == "{"
            depth -= code[i] == "}"
            if depth == 0:
                break
            i += 1
        out[name] = (m.start(), i)
    return out


def calls_in(src, span):
    code = _strip(src)[span[0]:span[1]]
    body = code[code.find("{"):]
    return set(re.findall(r"\b([A-Za-z_]\w*)\s*\(", body)) - {"if", "for", "while", "switch", "return",
                                                              "sizeof"}


def choose_top(norm_src):
    """Root of the call graph among defined functions that carry a contract."""
    with_contract = [f[0] for f in acsl.find_functions(norm_src)]
    if not with_contract:
        raise acsl.Unsupported("no function with an ACSL contract")
    defs = defined_functions(norm_src)
    called = set()
    for name, span in defs.items():
        called |= calls_in(norm_src, span) - {name}
    roots = [f for f in with_contract if f not in called]
    return (roots or with_contract)[-1]


# ------------------------------------------------------------------ linking

_DEF_INDEX = {}


def definition_index(corpus):
    key = tuple(str(c) for c in corpus)
    if key not in _DEF_INDEX:
        idx = {}
        for d in corpus:
            for f in sorted(Path(d).rglob("*.c")):
                try:
                    for name in defined_functions(f.read_text(errors="replace")):
                        idx.setdefault(name, f)
                except OSError:
                    pass
        _DEF_INDEX[key] = idx
    return _DEF_INDEX[key]


def link_files(spec: DesignSpec, top_src: str, top: str):
    """Other corpus files that define functions transitively called from `top`."""
    if not spec.corpus:
        return []
    idx = definition_index(spec.corpus)
    have = set(defined_functions(top_src))
    files, todo, seen = [], [top_src], set()
    while todo:
        src = todo.pop()
        for name, span in defined_functions(src).items():
            for callee in calls_in(src, span):
                if callee in have or callee in seen or callee not in idx:
                    continue
                seen.add(callee)
                f = idx[callee]
                if f not in files and f != Path(spec.files[0]):
                    files.append(f)
                    s = f.read_text(errors="replace")
                    have |= set(defined_functions(s))
                    todo.append(s)
    return files


# ------------------------------------------------------------------ HLS source

def hls_ready_source(src: str, fn: "acsl.Function"):
    """Rewrite the top definition's pointer parameters as fixed-size arrays (RAM interfaces).
    `T *p` and `T p[]` become `T p[N]`; C semantics are unchanged (array params decay)."""
    defs = defined_functions(src)
    if fn.name not in defs:
        raise acsl.Unsupported(f"definition of {fn.name} not found in its source file")
    start = defs[fn.name][0]
    code = _strip(src)
    m = re.compile(r"\b%s\s*\(([^;{}()]*)\)\s*\{" % re.escape(fn.name)).match(code, start)
    a, b = m.start(1), m.end(1)
    plist = src[a:b]
    new = plist
    for p in fn.params:
        if p.is_array and p.from_pointer:
            new = re.sub(r"\s*\*\s*(const\s+)?\b%s\b" % p.name, " %s[%d]" % (p.name, p.size), new)
            new = re.sub(r"\b%s\s*\[\s*\]" % p.name, "%s[%d]" % (p.name, p.size), new)
    return src[:a] + new + src[b:]


@dataclass
class Prepared:
    spec: DesignSpec
    fn: "acsl.Function"
    norm_src: str
    hls_files: list          # [top file (rewritten), linked files...]
    include_dirs: list


def prepare(spec: DesignSpec, src_override: str, workdir: Path, array_size: int, int_ptr_size: int = 1):
    """`src_override`: text to use for files[0] (e.g. a mutant). Returns Prepared."""
    workdir.mkdir(parents=True, exist_ok=True)
    top_file = workdir / Path(spec.files[0]).name
    top_file.write_text(src_override)
    incs = list(dict.fromkeys([str(Path(spec.files[0]).parent)] + [str(d) for d in spec.include_dirs]))
    spec2 = DesignSpec(spec.id, [top_file] + list(spec.files[1:]), incs, spec.top, spec.corpus, spec.meta)
    norm = framac_print(spec2, [top_file], workdir / "framac_norm.c")
    top = spec.top or choose_top(norm)
    fn = acsl.parse_function(norm, top, array_size=array_size, single_size=int_ptr_size)
    hls_dir = workdir / "hls_src"
    hls_dir.mkdir(exist_ok=True)
    hls_top = hls_dir / top_file.name
    hls_top.write_text(hls_ready_source(src_override, fn))
    linked = link_files(spec2, src_override, top)
    files = [hls_top]
    for f in linked:
        dst = hls_dir / f.name
        shutil.copy(f, dst)
        files.append(dst)
        incs.append(str(f.parent))
    return Prepared(spec2, fn, norm, files, list(dict.fromkeys(incs)))
