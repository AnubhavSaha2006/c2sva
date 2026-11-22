"""Contract -> property IR -> Verilog conditions.

Transaction model (one C call = one HLS transaction):
  requires            -> assumption at call start (pre-state: input ports + RAM contents)
  ensures             -> assertion one cycle after done (post-state RAM, captured inputs/result)
  \\old(e)             -> scalars: value captured at start; memories: snapshot taken at start
  behavior B          -> assumes A (pre-state):  requires A ==> R,  ensures \\old(A) ==> Q
  \\valid(p+(lo..hi))  -> assumption: range fits the RAM;  access check: every access stays in range
  assigns L           -> post-state frame condition (exact ACSL semantics): every element not in L
                         has its pre-state value when the call completes (per behavior, under its
                         assumes); independent of how the memory is laid out in RTL
  termination         -> checked only by the latency search (bounded), never as a timed property
Values outside the modelled memory (index < 0 or >= N) are solver-chosen, never 0, and the same
in the pre- and post-state (the RTL cannot write them; a write would fail the access check).
\\valid(a + (lo .. hi)) inside an expression = the range is empty or within the N cells.

Soundness discipline (an assumption may only get weaker, an assertion only stronger):
  quantifier meant universally in an assertion (forall at +, exists at -) -> symbolic value
      chosen by the solver (exact)
  meant existentially in an assertion        -> enumerate indices 0..N-1 (stronger)
  meant universally in an assumption         -> enumerate indices 0..N-1 (weaker)
  meant existentially in an assumption       -> unsupported
  \\sum/\\numof over lo..hi                     -> enumerate 0..N-1, with side condition
      'lo..hi within 0..N-1' (conjoined to assertions, used as guard for assumptions)
  recursion cut-off / free values            -> solver-chosen (see logic.py)
"""
import re
from dataclasses import asdict, dataclass, field

from .acsl import Parser, Unsupported, tokenize, walk
from .logic import expand


@dataclass
class Prop:
    id: str
    kind: str          # assume | post | write | access | term
    origin: str        # requires | ensures | assigns | valid | termination
    source: str
    behavior: str = None
    cond: str = ""
    notes: list = field(default_factory=list)


def conjuncts(e):
    if e[0] == "bin" and e[1] == "&&":
        return conjuncts(e[2]) + conjuncts(e[3])
    return [e]


class Translator:
    def __init__(self, fn, itf, W):
        self.fn, self.itf, self.W = fn, itf, W
        self.pmap = {p.name: p for p in fn.params}
        self.mems = set(itf.memories) | set(itf.objects)
        self.nmax = max([p.size for p in fn.params if p.is_array], default=0)
        self.anyconst = []        # (name, width)
        self.n = 0
        self.notes = []
        self.side = []
        self.calls = 0
        self.oob_names = set()
        self.max_mul = 0
        self.ptrs = frozenset(p.name for p in fn.params if p.is_array)
        from .acsl import resolve_type
        norm = lambda t: " ".join(w for w in resolve_type(t, fn.typedefs) if w not in ("const", "*"))
        self.types = {"__norm__": norm}
        for p in fn.params:
            base = p.ctype.replace("*", " ").replace("const", " ")
            self.types[("elem", p.name) if p.is_array else p.name] = base.strip()

    def lit(self, v):
        return f"{self.W}'sd{v}" if v >= 0 else f"-{self.W}'sd{-v}"

    def tag(self, view, old):
        return "o" if (old and view == "post") else ("s" if view in ("start", "run") else "e")

    def oob(self, arr, key):
        """A solver-chosen value for a cell outside the modelled memory (sound both ways)."""
        name = f"{arr}_x{key}"
        if name not in self.oob_names:
            self.oob_names.add(name)
            self.anyconst.append((name, self.W))
        self.notes.append(f"cell of {arr} outside 0..{self.pmap[arr].size - 1}: solver-chosen value")
        return name

    def elem(self, arr, i, view, old):
        size = self.pmap[arr].size
        tag = self.tag(view, old)
        if i < 0 or i >= size:
            return self.oob(arr, f"{'m' if i < 0 else ''}{abs(i)}")
        return f"{arr}_{tag}{i}"

    def field(self, e, mode, view, old, env):
        """Struct field: element views carry one W-bit wire per field (`<view>__<field>`)."""
        base, f = e[1], e[2]
        while base[0] == "old":
            base, old = base[1], True
        if base[0] == "idx":
            a = base[1]
            p = self.pmap.get(a)
            if not p or not p.layout or f not in [x[0] for x in p.layout]:
                raise Unsupported(f"field {f} of {a}")
            ix = self.tr(base[2], mode, view, None, old, env)
            m = re.fullmatch(r"-?\d+'sd(\d+)", ix)
            if m:
                i = -int(m.group(1)) if ix.startswith("-") else int(m.group(1))
                if not 0 <= i < p.size:
                    return self.oob(a, f"{i}f{f}")
                return f"{a}_{self.tag(view, old)}{i}__{f}"
            mux = self.oob(a, f"d{_ixkey(ix)}f{f}")
            for i in reversed(range(p.size)):
                mux = f"(({ix}) == {self.lit(i)} ? {a}_{self.tag(view, old)}{i}__{f} : {mux})"
            return mux
        if base[0] == "var" and base[1] in self.itf.scalars:
            p = self.pmap[base[1]]
            if not p.layout or f not in [x[0] for x in p.layout]:
                raise Unsupported(f"field {f} of {base[1]}")
            return f"{base[1]}_{'c' if view == 'post' else 'now'}__{f}"
        if base[0] == "result":
            if view != "post" or not self.fn.ret_layout or f not in [x[0] for x in self.fn.ret_layout]:
                raise Unsupported(f"field {f} of \\result")
            return f"h_res_c__{f}"
        if base[0] == "var" and base[1] in getattr(self.fn, "globals", ()):
            raise Unsupported(f"global variable {base[1]}: global state is not modelled")
        raise Unsupported("field of an unsupported expression")

    def fresh(self, base, width):
        self.n += 1
        name = f"{base}{self.n}"
        self.anyconst.append((name, width))
        return name

    def tr(self, e, mode, view, pol=True, old=False, env=None):
        self.calls += 1
        if self.calls > 1_500_000:
            raise Unsupported("clause too large to translate (enumeration budget exceeded)")
        env = env or {}
        T = lambda x, p=pol, o=old: self.tr(x, mode, view, p, o, env)
        k = e[0]
        if k == "int":
            return self.lit(e[1])
        if k == "bool":
            return "1'b1" if e[1] else "1'b0"
        if k == "true_by_hw":
            self.notes.append(f"{e[1]} holds by construction (separate RTL memories/registers)")
            return "1'b1"
        if k == "var":
            n = e[1]
            if n in env:
                return env[n]
            if n in self.itf.scalars:
                return {"start": f"{n}_now", "run": f"{n}_now", "post": f"{n}_c"}[view]
            if n in self.pmap:
                raise Unsupported(f"array {n} used as a value")
            if n in getattr(self.fn, "globals", ()):
                raise Unsupported(f"global variable {n}: global state is not modelled")
            raise Unsupported(f"unknown name {n}")
        if k == "result":
            if view != "post" or not self.itf.ret:
                raise Unsupported("\\result outside a postcondition or without a return port")
            return "h_res_c"
        if k == "old":
            return T(e[1], pol, True)
        if k == "idx":
            if e[1] not in self.mems:
                if e[1] in getattr(self.fn, "globals", ()):
                    raise Unsupported(f"global variable {e[1]}: global state is not modelled")
                raise Unsupported(f"{e[1]} is not a memory parameter")
            ix = T(e[2], None)
            if self.pmap[e[1]].layout:
                raise Unsupported(f"whole struct element {e[1]}[...] used as a value")
            m = re.fullmatch(r"-?\d+'sd(\d+)", ix)
            if m:
                return self.elem(e[1], -int(m.group(1)) if ix.startswith("-") else int(m.group(1)), view, old)
            size = self.pmap[e[1]].size
            # one symbol per distinct index expression: different symbolic indices may denote
            # different cells (a shared symbol would make a[i] == a[j] provable for i != j)
            mux = self.oob(e[1], f"d{_ixkey(ix)}")
            for i in reversed(range(size)):
                mux = f"(({ix}) == {self.lit(i)} ? {self.elem(e[1], i, view, old)} : {mux})"
            return mux
        if k == "field":
            return self.field(e, mode, view, old, env)
        if k == "un":
            if e[1] == "!":
                return f"(!{T(e[2], None if pol is None else not pol)})"
            return f"({e[1]}{T(e[2], None)})"
        if k == "bin":
            op = e[1]
            if op == "==>":
                return f"(!({T(e[2], None if pol is None else not pol)}) || ({T(e[3])}))"
            if op in ("&&", "||"):
                return f"({T(e[2])} {op} {T(e[3])})"
            vop = ">>>" if op == ">>" else "<<<" if op == "<<" else op
            return f"({T(e[2], None)} {vop} {T(e[3], None)})"
        if k == "cmp":
            es = [T(x, None) for x in e[2]]
            return "(" + " && ".join(f"({a} {op} {b})" for op, a, b in zip(e[1], es, es[1:])) + ")"
        if k == "ite":
            return f"({T(e[1], None)} ? {T(e[2])} : {T(e[3])})"
        if k == "cast":
            w, s, inner = e[1], e[2], T(e[3], None)
            mask = f"{self.W}'sd{(1 << w) - 1}"
            if s:
                sb = f"{self.W}'sd{1 << (w - 1)}"
                return f"((({inner}) & {mask}) ^ {sb}) - {sb}"
            return f"(({inner}) & {mask})"
        if k == "call":
            a = [T(x, None) for x in e[2]]
            if e[1] == "\\abs":
                return f"(({a[0]} < {self.lit(0)}) ? -({a[0]}) : ({a[0]}))"
            if e[1] in ("\\min", "\\max"):
                c = "<" if e[1] == "\\min" else ">"
                return f"(({a[0]} {c} {a[1]}) ? ({a[0]}) : ({a[1]}))"
            raise Unsupported(f"logic call {e[1]}")
        if k == "free":
            name = self.fresh("free", 1 if e[2] == "bool" else self.W)
            self.notes.append("recursive logic cut off: free value")
            return name
        if k == "valid":
            # \valid(a + (lo .. hi)) of an N-cell array: the range is empty or fits the cells
            arr = e[1]
            size = self.pmap[arr].size if arr in self.pmap and self.pmap[arr].is_array else None
            if size is None or arr not in self.mems:
                self.notes.append("\\valid inside an expression holds for RTL memories")
                return "1'b1"
            lo, hi = T(e[2], None), T(e[3], None)
            self.notes.append(f"\\valid inside an expression: range empty or within the {size} cells")
            return f"(({hi}) < ({lo}) || (({self.lit(0)} <= ({lo})) && (({hi}) < {self.lit(size)})))"
        if k == "agg":
            if not self.nmax:
                raise Unsupported(f"\\{e[1]} without an array bound")
            lo, hi = T(e[2], None), T(e[3], None)
            self.side.append(f"(({hi}) < ({lo}) || (({lo}) >= {self.lit(0)} && ({hi}) <= {self.lit(self.nmax - 1)}))")
            terms = []
            for i in range(self.nmax):
                env2 = dict(env, **{e[4]: self.lit(i)})
                body = self.tr(e[5], mode, view, None, old, env2)
                g = f"(({lo}) <= {self.lit(i)} && {self.lit(i)} <= ({hi}))"
                terms.append(f"({g} ? ({body}) : {self.lit(0)})" if e[1] == "sum"
                             else f"(({g} && ({body})) ? {self.lit(1)} : {self.lit(0)})")
            self.notes.append(f"\\{e[1]} enumerated over 0..{self.nmax-1}")
            return "(" + " + ".join(terms) + ")"
        if k in ("forall", "exists"):
            if pol is None:
                raise Unsupported("quantifier under mixed polarity")
            universal = (k == "forall") == pol
            if mode == "assert" and universal:
                env2 = dict(env)
                for v in e[1]:
                    env2[v] = self.fresh(f"q_{v}_", self.W)
                self.notes.append(f"\\{k} {','.join(e[1])}: symbolic (solver-chosen) value, exact")
                return self.tr(e[2], mode, view, pol, old, env2)
            if mode == "assume" and not universal:
                raise Unsupported("existential quantifier in an assumption")
            if not self.nmax:
                raise Unsupported("quantifier over a domain with no array bound")
            terms = [dict(env)]
            for v in e[1]:
                terms = [dict(t, **{v: self.lit(i)}) for t in terms for i in range(self.nmax + 1)]
            self.notes.append(f"\\{k} {','.join(e[1])}: enumerated over 0..{self.nmax} "
                              f"({'weaker assumption' if mode == 'assume' else 'stronger assertion'})")
            parts = [self.tr(e[2], mode, view, pol, old, t) for t in terms]
            return "(" + (" && " if k == "forall" else " || ").join(parts) + ")"
        if k == "range":
            raise Unsupported("range outside \\valid")
        raise Unsupported(f"construct {k}")

    def clause(self, tree, mode, view, cutoff):
        """Translate one clause; returns (cond, notes)."""
        self.notes, self.side, self.calls = [], [], 0
        ex = expand(tree, self.fn.defs, cutoff, self.ptrs, types=self.types)
        self.max_mul = max(self.max_mul, mul_depth(ex))
        cond = self.tr(ex, mode, view)
        if len(cond) > 2_000_000:
            raise Unsupported("translated condition too large")
        if self.side:
            s = " && ".join(self.side)
            cond = f"(({s}) && {cond})" if mode == "assert" else f"(!({s}) || {cond})"
        return cond, list(dict.fromkeys(self.notes))


def _ixkey(ix):
    """Stable short key of an index expression (same expression -> same out-of-model cell)."""
    import hashlib
    return hashlib.sha1(ix.encode()).hexdigest()[:8]


def mul_depth(e):
    """Nesting depth of multiplications/shifts (decides the integer width needed)."""
    if not isinstance(e, tuple):
        return 0
    kids = [mul_depth(x) for x in e[1:] if isinstance(x, tuple)] + \
        [mul_depth(y) for x in e[1:] if isinstance(x, list) for y in x if isinstance(y, tuple)]
    d = max(kids, default=0)
    return d + 1 if e[0] == "bin" and e[1] in ("*", "<<") else d


def _assigns_item(item, fn, typenames):
    """An assigns location -> (name, lo_tree, hi_tree, field|None) or None.
    Forms: '* ( a + ( lo .. hi ) )', '* ( a + i )', '* p', 'a [ lo .. hi ]', 'a [ i ]',
           'p -> x', '( a + ( lo .. hi ) ) -> x', 'a [ i ] . x'."""
    item = item.strip()
    fm = re.fullmatch(r"(.+?)\s*(->|\.)\s*(\w+)", item)
    if fm and not re.fullmatch(r".*\.\..*", fm.group(3)):
        base_txt, op, fld = fm.groups()
        if op == "->":
            base = _assigns_item("* ( " + base_txt + " )", fn, typenames)
        else:
            base = _assigns_item(base_txt, fn, typenames)
        return (base[0], base[1], base[2], fld) if base else None
    m = re.fullmatch(r"(\w+) \[ (.+) \.\. (.+) \]", item)
    if m:
        p = lambda s_: Parser(tokenize(s_), typenames).expr()
        return m.group(1), p(m.group(2)), p(m.group(3)), None
    toks = tokenize(item)
    if toks and toks[0] == "*":
        e = Parser(toks[1:], typenames).unary()
        while e[0] == "old":
            e = e[1]
        if e[0] == "var":
            return e[1], ("int", 0), ("int", 0), None
        if e[0] == "bin" and e[1] == "+" and e[2][0] in ("var", "old"):
            v = e[2][1] if e[2][0] == "var" else e[2][1][1]
            r = e[3]
            return (v, r[1], r[2], None) if r[0] == "range" else (v, r, r, None)
    m = re.fullmatch(r"(\w+) \[ (.+) \]", item)
    if m:
        t = Parser(tokenize(m.group(2)), typenames).expr()
        return m.group(1), t, t, None
    return None


def build_properties(fn, itf, lat_max, W, cutoff):
    """-> (props, skipped, translator)"""
    tr = Translator(fn, itf, W)
    props, skipped = [], []
    counter = [0]

    def add(kind, origin, src, beh, build):
        counter[0] += 1
        pid = f"P{counter[0]}"
        try:
            cond, notes = build()
            props.append(Prop(pid, kind, origin, src, beh, cond, notes))
        except Unsupported as ex:
            skipped.append({"id": pid, "kind": kind, "origin": origin, "source": src, "reason": str(ex)})

    ranges = []
    for beh in [fn.default] + fn.behaviors:
        bname = None if beh is fn.default else beh.name
        A = None
        if beh.assumes:
            A = beh.assumes[0][2]
            for x in beh.assumes[1:]:
                A = ("bin", "&&", A, x[2])
        for label, txt, tree in beh.requires:
            try:            # inline predicates so that \valid ranges inside them are seen too
                tree_x = expand(tree, fn.defs, cutoff, tr.ptrs, types=tr.types)
            except Unsupported:
                tree_x = tree
            for c in conjuncts(tree_x):
                if c[0] == "valid":
                    ranges.append((c[1], c[2], c[3], txt, bname, A))
                    continue
                t = c if A is None else ("bin", "==>", A, c)
                add("assume", "requires", f"requires {txt}", bname,
                    lambda t=t: tr.clause(t, "assume", "start", cutoff))
        for label, txt, tree in beh.ensures:
            t = tree if A is None else ("bin", "==>", ("old", A), tree)
            add("post", "ensures", f"ensures {txt}", bname, lambda t=t: tr.clause(t, "assert", "post", cutoff))

    for arr, lo, hi, txt, bname, A in ranges:
        if arr in itf.objects:
            continue                                     # single objects: nothing to range-check
        if arr not in itf.memories:
            skipped.append({"id": "-", "kind": "assume", "origin": "valid", "source": f"requires {txt}",
                            "reason": f"{arr} is not a memory"})
            continue
        mem = itf.memories[arr]
        guard = (lambda c: c) if A is None else (lambda c, A=A: ("bin", "==>", A, c))
        fits = ("bin", "||", ("cmp", ["<"], [hi, lo]),
                ("bin", "&&", ("cmp", ["<="], [("int", 0), lo]), ("cmp", ["<"], [hi, ("int", mem.size)])))
        add("assume", "valid", f"requires {txt}  [range fits the {mem.size}-cell memory]", bname,
            lambda f=guard(fits): tr.clause(f, "assume", "start", cutoff))
        if mem.protocol == "cells":
            continue                                     # per-element ports: no address to check
        for b, bank in enumerate(mem.banks):
            shift = (bank.word_width // 8).bit_length() - 1 if bank.byte_addr else 0
            for n in ("0", "1"):
                ce, ad = bank.ports.get("ce" + n), bank.ports.get("address" + n)
                if not ce or not ad:
                    continue
                word = f"($signed({{{{{W - bank.aw}{{1'b0}}}}, {ad}}}) >>> {shift})"
                idxs = [mem.index_expr(b, word, s_) for s_ in range(mem.slots)]
                def build(lo=lo, hi=hi, ce=ce, idxs=idxs, A=A):
                    l, _ = tr.clause(lo, "assert", "run", cutoff)
                    h, _ = tr.clause(hi, "assert", "run", cutoff)
                    inr = " || ".join(f"(({l} <= {ix}) && ({ix} <= {h}))" for ix in idxs)
                    cond = f"!{ce} || ({inr})"
                    if A is not None:          # !A || check: A sits in the negative position of an assertion
                        na, _ = tr.clause(("un", "!", A), "assert", "run", cutoff)
                        cond = f"({na}) || {cond}"
                    notes = ["word holds several elements: access counts as in range if any is"] if len(idxs) > 1 else []
                    return cond, notes
                add("access", "valid", f"requires {txt}  [access in range: {arr} bank {b} port {n}]",
                    bname, build)
                words = max(mem.locate(i)[1] for i in range(mem.size) if mem.locate(i)[0] == b) + 1
                add("access_phys", "valid", f"physical memory bound: {arr} bank {b} port {n} address < {words}",
                    None, lambda ce=ce, word=word, words=words: (f"!{ce} || (({word}) < {tr.lit(words)})", []))

    # frame conditions from assigns: default behavior, and each named behavior under \old(assumes)
    typenames = set(fn.typedefs)
    objects = {a: (1, None) for a in itf.objects}
    objects.update({a: (m.size, m) for a, m in itf.memories.items()})

    def can_write(a):
        if a in itf.objects:
            return bool(itf.objects[a].vld_port)
        m = itf.memories[a]
        return any(b.ports.get(k) for b in m.banks for k in ("we0", "we1")) or any(c.vld_port for c in m.cells)

    for beh in [fn.default] + fn.behaviors:
        if not beh.assigns:
            continue
        bname = None if beh is fn.default else beh.name
        A = None
        if beh.assumes:
            A = beh.assumes[0][2]
            for x in beh.assumes[1:]:
                A = ("bin", "&&", A, x[2])
        windows = {a: [] for a in objects}
        for item in beh.assigns:
            if item.strip() in ("\\nothing", "\\result"):
                continue
            try:
                win = _assigns_item(item, fn, typenames)
            except Unsupported:
                win = None
            if win and win[0] in windows:
                windows[win[0]].append((win[1], win[2], win[3]))
            elif win and (win[0] in itf.scalars or win[0] not in tr.pmap):
                head = win[0]
                if head in getattr(fn, "globals", ()):
                    skipped.append({"id": "-", "kind": "post", "origin": "assigns", "source": f"assigns {item}",
                                    "reason": "global variable: global state is not modelled"})
                continue                                  # locals / by-value params: not observable
            else:
                head = re.match(r"\W*(\w+)", item)
                why = ("global variable: global state is not modelled"
                       if head and head.group(1) in getattr(fn, "globals", ()) else "location form not supported")
                skipped.append({"id": "-", "kind": "post", "origin": "assigns", "source": f"assigns {item}",
                                "reason": why})
        for a, wins in windows.items():
            size, mem = objects[a]
            src = f"assigns {' , '.join(beh.assigns)}  [frame of {a}]"
            if not can_write(a) and A is None:
                counter[0] += 1
                props.append(Prop(f"P{counter[0]}", "discharged", "assigns", src, bname, "1'b1",
                                  ["holds by construction: the RTL cannot write this object"]))
                continue
            def build(a=a, wins=wins, size=size, A=A):
                lay = tr.pmap[a].layout if a in tr.pmap else None
                fields = [x[0] for x in lay] if lay else [None]
                bounds = []
                for lo, hi, fld in wins:
                    l, _ = tr.clause(("old", lo), "assert", "post", cutoff)
                    h, _ = tr.clause(("old", hi), "assert", "post", cutoff)
                    bounds.append((l, h, fld))
                parts = []
                for i in range(size):
                    for f in fields:
                        inw = [f"(({l} <= {tr.lit(i)}) && ({tr.lit(i)} <= {h}))" for l, h, fld in bounds
                               if fld is None or fld == f]
                        sfx = f"__{f}" if f else ""
                        keep = f"({a}_e{i}{sfx} == {a}_o{i}{sfx})"
                        parts.append(f"({' || '.join(inw)} || {keep})" if inw else keep)
                cond = "(" + " && ".join(parts) + ")"
                if A is not None:              # !\old(A) || frame, translated as one assertion
                    ng, _ = tr.clause(("un", "!", ("old", A)), "assert", "post", cutoff)
                    cond = f"(({ng}) || {cond})"
                return cond, [f"exact frame over all {size} element(s) of {a}" + (" (per field)" if lay else "")]
            add("post", "assigns", src, bname, build)
    counter[0] += 1
    props.append(Prop(f"P{counter[0]}", "term", "termination",
                      "termination: every call eventually completes (checked bounded by the latency search)",
                      None, "", []))
    return props, skipped, tr


def ir_json(fn, itf, props, skipped):
    import json
    return json.dumps({"function": fn.name, "tool": itf.tool,
                       "properties": [asdict(p) for p in props], "not_translated": skipped}, indent=2)
