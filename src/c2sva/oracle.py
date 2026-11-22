"""Concrete contract evaluator (C-level oracle used by counterexample replay).

Evaluates the function contract on concrete values obtained by *running the C code*:
scalars, the pre-state and post-state of every memory, and the result. Integers are Python
integers, i.e. exact ACSL `integer` semantics. It is independent of the SVA translation,
the harness and the HLS tool; it shares only the parser and logic inlining.

Evaluation of unbounded constructs:
  \\forall / \\exists   enumerate the bound variable over [-2, N+2] (N = largest memory); the
                       contract's own guards restrict meaningful values to indices, and a
                       result that depends on values outside this window is reported unknown
  memory access        outside the modelled cells -> unknown (never guessed)
  recursion cut-off    unknown
Every requires / ensures / assigns clause (default and behaviors) is checked; the frame
condition is evaluated exactly over all cells.
"""
from .acsl import Unsupported
from .logic import expand

UNKNOWN = object()


class Unknown(Exception):
    pass


def _cmp(op, a, b):
    return {"<": a < b, "<=": a <= b, ">": a > b, ">=": a >= b, "==": a == b, "!=": a != b}[op]


def _cdiv(a, b):
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b >= 0) else -q


class Evaluator:
    def __init__(self, fn, scalars, pre, post, result):
        self.fn, self.sc, self.pre, self.post, self.res = fn, scalars, pre, post, result
        self.nmax = max([p.size for p in fn.params if p.is_array], default=0)

    def ev(self, e, env, old=False, post_view=True):
        k = e[0]
        E = lambda x, o=old: self.ev(x, env, o, post_view)
        if k == "int":
            return e[1]
        if k == "bool":
            return e[1]
        if k in ("true_by_hw", "valid"):
            return True
        if k == "var":
            if e[1] in env:
                return env[e[1]]
            if e[1] in self.sc:
                return self.sc[e[1]]
            raise Unknown(f"name {e[1]}")
        if k == "result":
            if self.res is None:
                raise Unknown("result")
            return self.res
        if k == "old":
            return E(e[1], True)
        if k == "idx":
            i = E(e[2])
            mem = (self.pre if (old or not post_view) else self.post).get(e[1])
            if mem is None or not (0 <= i < len(mem)):
                raise Unknown(f"{e[1]}[{i}] outside the modelled memory")
            return mem[i]
        if k == "field":
            base = e[1]
            if base[0] == "idx":
                el = E(base)
            elif base[0] == "old":
                el = self.ev(base[1], env, True, post_view)
            else:
                el = E(base)
            if not isinstance(el, dict) or e[2] not in el:
                raise Unknown(f"field {e[2]}")
            return el[e[2]]
        if k == "un":
            v = E(e[2])
            return (not v) if e[1] == "!" else (-v if e[1] == "-" else ~v)
        if k == "bin":
            op = e[1]
            if op == "==>":
                return (not E(e[2])) or bool(E(e[3]))
            if op == "&&":
                return bool(E(e[2])) and bool(E(e[3]))
            if op == "||":
                return bool(E(e[2])) or bool(E(e[3]))
            a, b = E(e[2]), E(e[3])
            if op in ("/", "%"):
                if b == 0:
                    raise Unknown("division by zero")
                q = _cdiv(a, b)
                return q if op == "/" else a - b * q
            return {"+": a + b, "-": a - b, "*": a * b, "&": a & b, "|": a | b, "^": a ^ b,
                    "<<": a << b if b >= 0 else 0, ">>": a >> b if b >= 0 else 0}[op]
        if k == "cmp":
            vs = [E(x) for x in e[2]]
            return all(_cmp(op, a, b) for op, a, b in zip(e[1], vs, vs[1:]))
        if k == "ite":
            return E(e[2]) if E(e[1]) else E(e[3])
        if k == "cast":
            w, s, v = e[1], e[2], E(e[3])
            v &= (1 << w) - 1
            return v - (1 << w) if s and v >> (w - 1) & 1 else v
        if k == "call":
            a = [E(x) for x in e[2]]
            if e[1] == "\\abs":
                return abs(a[0])
            if e[1] == "\\min":
                return min(a)
            if e[1] == "\\max":
                return max(a)
            raise Unknown(f"call {e[1]}")
        if k == "free":
            raise Unknown("recursion cut off")
        if k == "agg":
            lo, hi = E(e[2]), E(e[3])
            total = 0
            for i in range(lo, hi + 1):
                v = self.ev(e[5], dict(env, **{e[4]: i}), old, post_view)
                total += (1 if v else 0) if e[1] == "numof" else v
            return total
        if k in ("forall", "exists"):
            rng = range(-2, self.nmax + 3)
            envs = [dict(env)]
            for v in e[1]:
                envs = [dict(x, **{v: i}) for x in envs for i in rng]
            unknown = False
            for x in envs:
                try:
                    r = bool(self.ev(e[2], x, old, post_view))
                except Unknown:
                    unknown = True
                    continue
                if k == "forall" and not r:
                    return False
                if k == "exists" and r:
                    return True
            if unknown:
                raise Unknown("quantifier over cells outside the model")
            return k == "forall"
        raise Unknown(f"construct {k}")


def evaluate(fn, scalars, pre, post, result, cutoff=8):
    """-> {'requires_violated': [...], 'violated': [...], 'unknown': [...], 'checked': n}"""
    ev = Evaluator(fn, scalars, pre, post, result)
    ptrs = frozenset(p.name for p in fn.params if p.is_array)
    out = {"requires_violated": [], "violated": [], "unknown": [], "checked": 0}

    def check(tree, text, pre_state, bucket):
        try:
            t = expand(tree, fn.defs, cutoff, ptrs)
            ok = bool(ev.ev(t, {}, False, post_view=not pre_state))
            out["checked"] += 1
            if not ok:
                out[bucket].append(text)
        except (Unknown, Unsupported) as x:
            out["unknown"].append(f"{text}  ({x})")

    for beh in [fn.default] + fn.behaviors:
        A = None
        if beh.assumes:
            A = beh.assumes[0][2]
            for x in beh.assumes[1:]:
                A = ("bin", "&&", A, x[2])
        if A is not None:
            try:
                active = bool(ev.ev(expand(A, fn.defs, cutoff, ptrs), {}, False, post_view=False))
            except (Unknown, Unsupported):
                out["unknown"].append(f"assumes of behavior {beh.name}")
                continue
            if not active:
                continue
        for _, txt, tree in beh.requires:
            check(tree, f"requires {txt}", True, "requires_violated")
        for _, txt, tree in beh.ensures:
            check(tree, f"ensures {txt}", False, "violated")
        if beh.assigns:
            from .translate import _assigns_item
            wins = {}
            unknown = False
            for item in beh.assigns:
                if item.strip() in ("\\nothing", "\\result"):
                    continue
                try:
                    w = _assigns_item(item, fn, set(fn.typedefs))
                except Unsupported:
                    w = None
                if not w:
                    unknown = True
                    continue
                try:
                    lo = ev.ev(expand(w[1], fn.defs, cutoff, ptrs), {}, True)
                    hi = ev.ev(expand(w[2], fn.defs, cutoff, ptrs), {}, True)
                except Unknown:
                    unknown = True
                    continue
                wins.setdefault(w[0], []).append((lo, hi, w[3] if len(w) > 3 else None))
            if unknown:
                out["unknown"].append("assigns (location form)")
            for a in pre:
                if a not in post:
                    continue
                for i, (u, v) in enumerate(zip(pre[a], post[a])):
                    if isinstance(u, dict):
                        bad = [f for f in u if u[f] != v.get(f) and not any(
                            lo <= i <= hi and (fl is None or fl == f) for lo, hi, fl in wins.get(a, []))]
                        if bad:
                            out["violated"].append(f"assigns (frame): {a}[{i}].{bad[0]} changed but is not assignable")
                            break
                        continue
                    if u != v and not any(lo <= i <= hi for lo, hi, _ in wins.get(a, [])):
                        out["violated"].append(f"assigns (frame): {a}[{i}] changed but is not assignable")
                        break
            out["checked"] += 1
    return out
