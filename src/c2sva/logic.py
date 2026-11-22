"""ACSL logic layer: global predicate/logic definitions and their inlining.

* Overloads are resolved like ACSL does: by arity *and* parameter kinds (pointer vs. integer),
  e.g. Equal(a, m, n, b) [b pointer] vs Equal(a, m, n, p) [p integer].
* Pointer arguments are pointer *terms* (base array, offset): passing `\\old(a) + k` for a
  pointer parameter `x` turns `x[i]` / `*(x + i)` in the body into `a[k + i]`.
* Memory-state labels ({L}, {L1,L2}) are mapped from the call site, so `P{Pre}(a)` reads the
  pre-state and `\\at(e, L1)` resolves to the right state.
* Recursive definitions are unrolled. Arrays are bounded (N cells) so meaningful recursion
  depth is bounded too; at the cutoff the call becomes a *free* value chosen by the solver.
  Free values are sound in both directions: in an assertion the solver may pick any value
  (stronger check), in an assumption it picks a satisfying one (weaker assumption).
* A node budget stops pathological blow-ups (reported as 'too large', never silently cut).
* Axiomatic/inductive definitions are recorded; they are rejected here (see c2sva.llm).
"""
import re
from dataclasses import dataclass, field

from .acsl import Parser, Unsupported, tokenize

BUDGET = 400_000          # max tree nodes produced while expanding one clause


@dataclass
class Def:
    name: str
    kind: str          # predicate | logic | axiomatic | abstract | inductive
    labels: list
    params: list
    ptrs: list = field(default_factory=list)     # True where the parameter is a pointer
    body: tuple = None
    ptypes: list = field(default_factory=list)   # declared type words per parameter
    cut_value: int = None    # value at the recursion cut-off instead of a free value (prelude only)


# Bounded definitions for Frama-C libc logic functions (axiomatic in the libc headers).
# strlen recursion is unrolled up to the RAM size; cells beyond the RAM read as 0.
PRELUDE = """
logic integer strlen{L}(char *s) = *(s + 0) == 0 ? 0 : 1 + strlen(s + 1);
predicate valid_string{L}(char *s) = 0 <= strlen(s) && \\valid(s + (0 .. strlen(s)));
predicate valid_read_string{L}(char *s) = 0 <= strlen(s) && \\valid_read(s + (0 .. strlen(s)));
"""


import threading
_OV = threading.local()   # per thread (cases run in parallel): name -> validated lifted definition


def set_overrides(d):
    """Validated LLM-lifted definitions of the design being processed (c2sva.llm); they replace
    only axiomatic/inductive/abstract definitions of that name."""
    _OV.d = dict(d or {})


def _overrides():
    return getattr(_OV, "d", {})


def logic_definitions(src, typenames=()):
    """{(name, arity): [Def, ...]} (several entries = overloads by parameter kind)."""
    defs = {}
    _parse_defs(Parser(tokenize(PRELUDE), typenames), defs)
    # strlen: a string that does not terminate within the unrolled cells cannot satisfy
    # \valid(s + (0 .. strlen(s))) in the bounded memory, so 'infinitely long' is exact there
    for d in defs.get(("strlen", 1), []):
        d.cut_value = 1 << 24
    prelude = {k: v for k, v in defs.items()}
    for block in re.findall(r"/\*@(.*?)\*/", src, re.S):
        if not re.search(r"\b(predicate|logic|lemma|axiomatic|inductive)\b", block):
            continue
        body = re.sub(r"^\s*@", "", block, flags=re.M)
        try:
            _parse_defs(Parser(tokenize(body), typenames), defs)
        except Unsupported:
            for m in re.finditer(r"\b(?:predicate|logic\s+[\w\s\*]+?)\s+(\w+)\s*(?:\{[^}]*\})?\s*\(", body):
                defs.setdefault((m.group(1), None), []).append(Def(m.group(1), "abstract", [], []))
    for k, v in prelude.items():               # libc axiomatics are replaced by the prelude
        defs[k] = [d for d in defs[k] if d.kind not in ("axiomatic", "abstract")] or v
    for name, ov in _overrides().items():      # validated lifted definitions (c2sva.llm)
        if any(d.kind in ("predicate", "logic") and d.body is not None
               for k, dl in defs.items() if k[0] == name for d in dl):
            continue                           # a concrete definition of that name: keep it
        text, dom = (ov["definition"], ov.get("domain")) if isinstance(ov, dict) else (ov, None)
        lifted = {}
        try:
            _parse_defs(Parser(tokenize(re.sub(r"^\s*/\*@|\*/\s*$", "", text.strip())), typenames), lifted)
            dom_t = Parser(tokenize(dom), typenames).expr() if dom and dom.strip() != "\\true" else None
        except (Unsupported, IndexError, AssertionError):
            continue
        for k, v in lifted.items():
            if k[0] != name:
                continue
            for d in v:            # outside the validated domain: solver-chosen value
                if dom_t is not None and d.body is not None:
                    d.body = ("ite", dom_t, d.body, ("free", fresh("dom_" + name), "bool" if d.kind == "predicate" else "int"))
            defs[k] = v
            defs.pop((name, None), None)
    return defs


def _add(defs, d):
    d.ptypes = list(getattr(_decl, "last_ptypes", [])) if len(getattr(_decl, "last_ptypes", [])) == len(d.params) else []
    defs.setdefault((d.name, len(d.params)), []).append(d)


def _decl(p, kind):
    """After 'predicate' / 'logic': [ret-type] name [{labels}] [(params)]."""
    if kind == "logic":
        while p.peek(1) not in ("(", "{", "=", ";"):
            p.eat()
    name = p.eat()
    labels = p.labels()
    params, ptrs, ptypes = [], [], []
    if p.peek() == "(":
        p.eat("(")
        while p.peek() != ")":
            words, depth = [], 0
            while not (p.peek() in (",", ")") and depth == 0):
                tok = p.eat()
                depth += tok == "(" and 1 or 0
                depth -= tok == ")" and 1 or 0
                words.append(tok)
            params.append(words[-1])
            ptrs.append("*" in words or "[" in words)
            ptypes.append(" ".join(w for w in words[:-1] if w not in ("*", "const")))
            if p.peek() == ",":
                p.eat()
        p.eat(")")
    _decl.last_ptypes = ptypes
    return name, labels, params, ptrs


def _parse_defs(p, defs):
    while p.peek() is not None:
        kw = p.eat()
        if kw == "axiomatic":
            p.eat()
            p.eat("{")
            depth = 1
            while depth:
                tok = p.eat()
                if tok == "{":
                    depth += 1
                elif tok == "}":
                    depth -= 1
                elif tok in ("predicate", "logic"):
                    name, labels, params, ptrs = _decl(p, tok)
                    _add(defs, Def(name, "axiomatic", labels, params, ptrs))
            continue
        if kw == "lemma":
            while p.eat() != ":":
                pass
            p.expr()
            p.eat(";")
            continue
        if kw == "inductive":
            name, labels, params, ptrs = _decl(p, "predicate")
            _add(defs, Def(name, "inductive", labels, params, ptrs))
            depth = 0
            while True:
                tok = p.eat()
                depth += tok == "{" and 1 or 0
                depth -= tok == "}" and 1 or 0
                if depth == 0 and tok == "}":
                    break
            continue
        if kw not in ("predicate", "logic"):
            raise Unsupported(f"global annotation {kw!r}")
        name, labels, params, ptrs = _decl(p, kw)
        if p.peek() in (";", "reads"):
            while p.eat() != ";":
                pass
            _add(defs, Def(name, "abstract", labels, params, ptrs))
            continue
        p.eat("=")
        _add(defs, Def(name, kw, labels, params, ptrs, p.expr()))
        p.eat(";")


# ------------------------------------------------------------------ pointer terms

def ptr_term(e, ptrs):
    """Normalize a pointer-valued expression to ('ptr', base, offset) or None."""
    if e[0] == "ptr":
        return e
    if e[0] == "old":
        return ptr_term(e[1], ptrs)
    if e[0] == "var" and e[1] in ptrs:
        return ("ptr", e[1], ("int", 0))
    if e[0] == "bin" and e[1] in ("+", "-"):
        base = ptr_term(e[2], ptrs)
        if base:
            off = e[3] if e[1] == "+" else ("un", "-", e[3])
            return ("ptr", base[1], _plus(base[2], off))
        if e[1] == "+":
            base = ptr_term(e[3], ptrs)
            if base:
                return ("ptr", base[1], _plus(base[2], e[2]))
    return None


def _plus(a, b):
    if a == ("int", 0):
        return b
    if b == ("int", 0):
        return a
    return ("bin", "+", a, b)


# ------------------------------------------------------------------ substitution & expansion

def _map(e, f):
    return tuple(f(x) if isinstance(x, tuple)
                 else [f(y) if isinstance(y, tuple) else y for y in x] if isinstance(x, list)
                 else x for x in e)


def substitute(e, env):
    if not isinstance(e, tuple):
        return e
    k = e[0]
    if k == "var" and e[1] in env:
        return env[e[1]]
    if k in ("idx", "valid") and e[1] in env:
        tgt = env[e[1]]
        if tgt[0] != "ptr":
            raise Unsupported("array argument is not a pointer term")
        rest = [substitute(x, env) for x in e[2:]]
        if k == "idx":
            return ("idx", tgt[1], _plus(tgt[2], rest[0]))
        return ("valid", tgt[1], _plus(tgt[2], rest[0]), _plus(tgt[2], rest[1]))
    if k in ("forall", "exists"):
        return (k, e[1], substitute(e[2], {a: b for a, b in env.items() if a not in e[1]}))
    if k == "agg":
        inner = {a: b for a, b in env.items() if a != e[4]}
        return ("agg", e[1], substitute(e[2], env), substitute(e[3], env), e[4], substitute(e[5], inner))
    if k == "let":
        inner = {a: b for a, b in env.items() if a != e[1]}
        return ("let", e[1], substitute(e[2], env), substitute(e[3], inner))
    return _map(e, lambda x: substitute(x, env))


_fresh = [0]


def fresh(base):
    _fresh[0] += 1
    return f"{base}_{_fresh[0]}"


def rename_bound(e):
    if not isinstance(e, tuple):
        return e
    if e[0] in ("forall", "exists"):
        new = [fresh(v) for v in e[1]]
        body = substitute(e[2], {v: ("var", n) for v, n in zip(e[1], new)})
        return (e[0], new, rename_bound(body))
    if e[0] == "agg":
        n = fresh(e[4])
        return ("agg", e[1], rename_bound(e[2]), rename_bound(e[3]), n,
                rename_bound(substitute(e[5], {e[4]: ("var", n)})))
    return _map(e, rename_bound)


def relabel(e, lmap):
    if not isinstance(e, tuple):
        return e
    if e[0] == "at":
        return ("at", relabel(e[1], lmap), lmap.get(e[2], e[2]))
    if e[0] == "call":
        return ("call", e[1], [relabel(a, lmap) for a in e[2]], [lmap.get(l, l) for l in e[3]])
    return _map(e, lambda x: relabel(x, lmap))


PRE = ("Pre", "Old")
POST = ("Here", "Post")


def arg_type(a, types):
    """Declared C type of a logic argument when it is obvious (variables, cells, \\result)."""
    if a[0] == "old":
        return arg_type(a[1], types)
    if a[0] == "var":
        return types.get(a[1])
    if a[0] == "idx":
        return types.get(("elem", a[1]))
    if a[0] == "result":
        return types.get("\\result")
    return None


def resolve_overload(name, args, defs, ptrs, types=None):
    cands = defs.get((name, len(args))) or []
    if not cands:
        if (name, None) in defs:
            raise Unsupported(f"{defs[(name, None)][0].kind} logic {name}")
        raise Unsupported(f"unknown logic function/predicate {name}")
    if len(cands) == 1:
        return cands[0]
    kinds = [ptr_term(a, ptrs) is not None for a in args]
    match = [d for d in cands if d.ptrs == kinds]
    if len(match) > 1 and types:
        # ACSL resolution: a parameter of a C type matches an argument of exactly that type
        # better than a logic 'integer' parameter does.
        norm = types.get("__norm__", lambda t: t)
        def score(d):
            sc = 0
            for pt, a, isptr in zip(d.ptypes, args, d.ptrs):
                if isptr or pt in ("integer", "boolean", "real"):
                    continue
                at = arg_type(a, types)
                if at is None:
                    continue
                sc += 1 if norm(pt) == norm(at) else -100
            return sc
        best = max(score(d) for d in match)
        match = [d for d in match if score(d) == best]
    if len(match) != 1:
        raise Unsupported(f"ambiguous overload of {name}")
    return match[0]


class _Budget:
    def __init__(self):
        self.n = 0

    def tick(self, k=1):
        self.n += k
        if self.n > BUDGET:
            raise Unsupported("clause too large after inlining (node budget exceeded)")


def expand(e, defs, cutoff=8, ptrs=frozenset(), stack=(), budget=None, types=None):
    """Inline definitions; resolve \\let, \\at and pointer terms; unroll recursion up to `cutoff`."""
    budget = budget or _Budget()
    budget.tick()
    if not isinstance(e, tuple):
        return e
    X = lambda x, st=stack: expand(x, defs, cutoff, ptrs, st, budget, types)
    k = e[0]
    if k == "let":
        return X(substitute(e[3], {e[1]: e[2]}))
    if k == "at":
        if e[2] in PRE:
            return ("old", X(e[1]))
        if e[2] in POST:
            return X(e[1])
        raise Unsupported(f"memory label {e[2]}")
    if k == "call" and not e[1].startswith("\\"):
        d = resolve_overload(e[1], e[2], defs, ptrs, types)
        if d.kind not in ("predicate", "logic"):
            raise Unsupported(f"{d.kind} logic {e[1]}")
        key = (d.name, len(d.params), tuple(d.ptrs))
        if stack.count(key) >= cutoff:
            if d.cut_value is not None:
                return ("int", d.cut_value)
            return ("free", fresh("cut_" + e[1]), "bool" if d.kind == "predicate" else "int")
        call_labels = e[3] or (["Here"] * len(d.labels))
        lmap = dict(zip(d.labels, call_labels))
        env = {}
        for name, isptr, arg in zip(d.params, d.ptrs, e[2]):
            if isptr:
                t = ptr_term(arg, ptrs)
                if t is None:
                    raise Unsupported(f"argument for pointer parameter {name} of {e[1]} is not a pointer term")
                env[name] = t
            else:
                env[name] = arg
        body = relabel(rename_bound(d.body), lmap)
        body = substitute(body, env)
        if len(d.labels) == 1 and lmap[d.labels[0]] in PRE:
            body = ("old", body)
        budget.tick(50)
        return expand(body, defs, cutoff, ptrs, stack + (key,), budget, types)
    if k == "ptr":
        raise Unsupported("pointer value used as a value (pointer comparison/arithmetic result)")
    return _map(e, X)
