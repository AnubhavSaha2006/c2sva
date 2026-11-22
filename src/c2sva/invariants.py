"""A1 — certificate translation through HLS: C proof artefacts -> RTL invariant candidates.

The C side ships with a deductive proof (Frama-C WP) whose key ingredients are loop
invariants, loop assigns and loop variants. An RTL model checker lacks exactly these: the
contract assertion at ap_done is not inductive. This module maps every loop annotation of
the top function to *candidate* invariants of the formal harness:

  loop invariant I      ->  guard  ==>  I[rtl(v)/v]      for C locals v
  loop assigns L        ->  guard  ==>  every cell outside L equals its \\old snapshot
  loop variant V        ->  guard  ==>  V >= 0
  contract assigns      ->  h_busy ==>  every cell outside the assigns keeps its value

where guard ranges over `h_busy` and `h_busy && <FSM state>` of the module that holds the
mapped registers, and rtl(v) ranges over the RTL registers Vitis derived from v (by name:
v_fu_N, v_reg_N, ap_sig_allocacmp_v_K, ap_phi_mux_v_..., narrowed widths zero/sign
extended). Parameters, memories and \\old/\\at(.,Pre) use the harness views of the contract
translation. HLS-structural candidates (FSM one-hot, idle state when no call is in flight,
input registers equal to the captured inputs) complete the set.

Candidates carry no soundness obligation: only those that a k-induction proof certifies
(Houdini fixpoint, see prove.py) are ever used, and then as assumptions about reachable
states. Internal RTL signals are read through taps: the harness declares an undriven wire,
and the SBY script connects it to the flattened signal (`connect -nounset -set t_k dut.path.sig`; without -nounset Yosys also
detaches the continuous assignments that read the tap).
"""
import json
import re
import subprocess
from dataclasses import dataclass, field
from itertools import product

from . import acsl, translate
from .acsl import Parser, Unsupported, tokenize
from .logic import expand


# ------------------------------------------------------------------ RTL signals

def rtl_signals(rtl_files, top, workdir):
    """Flattened signals of the DUT: {name: width}, names as after `flatten` ('inst.sig')."""
    out = workdir / "rtl_signals.json"
    reads = "; ".join(f"read_verilog -formal {f}" for f in rtl_files)
    p = subprocess.run(["yosys", "-q", "-p", f"{reads}; hierarchy -top {top}; proc; flatten; "
                        f"write_json {out}"], capture_output=True, text=True, timeout=600)
    if p.returncode != 0:
        raise RuntimeError(f"yosys: {p.stderr[-300:]}")
    nets = json.loads(out.read_text())["modules"][top]["netnames"]
    return {n: len(v["bits"]) for n, v in nets.items() if not v.get("hide_name")}


def _scope(name):
    return name.rsplit(".", 1)[0] if "." in name else ""


def _leaf(name):
    return name.rsplit(".", 1)[-1]


def fsm_states(sigs):
    """{scope: [state wires]} for every module instance with a Vitis FSM."""
    out = {}
    for n, w in sigs.items():
        if w == 1 and re.fullmatch(r"ap_CS_fsm_(state\d+|pp\d+_stage\d+)", _leaf(n)):
            out.setdefault(_scope(n), []).append(n)
    for k in out:
        out[k].sort(key=lambda s: [int(x) for x in re.findall(r"\d+", _leaf(s))])
    return out


def schedule_aliases(hls_dir):
    """C variable -> names of the LLVM values that hold it, from Vitis' schedule reports.
    A C local becomes `%v = alloca`; each read is `%x = load %v` and the RTL names the
    forwarded value `ap_sig_allocacmp_x` and its register `x_reg_N`. After inlining, x is often
    named after the caller variable (`%first = load %left` when `first = lower_bound(...)`), so
    name matching on `left` alone misses the value of `left` in the current iteration."""
    out = {}
    for f in sorted(hls_dir.rglob("*.verbose.sched.rpt")):
        text = f.read_text(errors="replace")
        allocas = set(re.findall(r'"%(\w+) = alloca ', text))
        for x, v in re.findall(r'"%(\w+) = load \w+ %(\w+)"', text):
            if v in allocas and x != v:
                out.setdefault(v, set()).add(x)
    return out


def var_signals(sigs, v, cap=8, aliases=None):
    """RTL signals Vitis derived from C variable v (by its naming conventions, and through the
    LLVM values that load it when the schedule reports are available)."""
    if aliases and aliases.get(v):
        seen = []
        for name in [v] + sorted(aliases[v]):
            seen += [n for n in var_signals(sigs, name, cap) if n not in seen]
        return seen[:2 * cap]
    v = re.escape(v)
    pats = [rf"ap_sig_allocacmp_{v}(_\d+)?", rf"{v}(_\d+)?_fu_\d+", rf"{v}(_\d+)?_reg_\d+",
            rf"ap_phi_mux_{v}(_\d+)?_phi_fu_\d+_p\d+", rf"{v}(_\d+)?", rf"{v}_load(_\d+)?_reg_\d+"]
    hits = [n for n in sigs if sigs[n] > 1 and any(re.fullmatch(p, _leaf(n)) for p in pats)]
    hits.sort(key=lambda n: (n.count("."), "allocacmp" not in n, "_fu_" not in n, n))
    return hits[:cap]


# ------------------------------------------------------------------ loop annotations

@dataclass
class Loop:
    invariants: list = field(default_factory=list)   # (label, text, tree)
    assigns: list = field(default_factory=list)      # location texts
    variant: tuple = None                            # (text, tree)
    func: str = ""                                   # function containing the loop
    body: str = ""                                   # its body (declarations of the locals)


def _body(src, name):
    m = re.search(rf"\b{re.escape(name)}\s*\([^;{{]*\)\s*\{{", src)
    if not m:
        return ""
    i, d = m.end(), 1
    while i < len(src) and d:
        d += {"{": 1, "}": -1}.get(src[i], 0)
        i += 1
    return src[m.end():i - 1]


def loop_annotations(norm, fname, typenames, typedefs):
    """Loop annotations of function `fname` in Frama-C-normalized source."""
    body = _body(norm, fname)
    loops = []
    for m in re.finditer(r"/\*@(.*?)\*/", body, re.S):
        text = m.group(1)
        if not re.search(r"\bloop\b", text):
            continue
        toks = tokenize(re.sub(r"^\s*@", "", text, flags=re.M))
        p = Parser(toks, typenames)
        L = Loop()

        def skip():
            while p.peek() is not None and p.eat() != ";":
                pass
        while p.peek() is not None:
            kw = p.eat()
            if kw in ("check", "admit"):
                kw = p.eat()
            if kw != "loop":
                skip()
                continue
            what = p.eat()
            try:
                if what == "invariant":
                    label = ""
                    if re.match(r"[A-Za-z_]\w*$", p.peek() or "") and p.peek(1) == ":":
                        label = p.eat()
                        p.eat(":")
                    s = p.i
                    tree = acsl.resolve_casts(p.expr(), typedefs)
                    L.invariants.append((label, " ".join(toks[s:p.i]), tree))
                    p.eat(";")
                elif what == "assigns":
                    items, buf, dd = [], [], 0
                    while not (p.peek() == ";" and dd == 0):
                        tok = p.eat()
                        if tok == "," and dd == 0:
                            items.append(" ".join(buf))
                            buf = []
                            continue
                        dd += tok in ("(", "[") and 1 or 0
                        dd -= tok in (")", "]") and 1 or 0
                        buf.append(tok)
                    items.append(" ".join(buf))
                    L.assigns.extend(x for x in items if x)
                    p.eat(";")
                elif what == "variant":
                    s = p.i
                    tree = acsl.resolve_casts(p.expr(), typedefs)
                    L.variant = (" ".join(toks[s:p.i]), tree)
                    skip()
                else:
                    skip()
            except (Unsupported, IndexError, AssertionError, TypeError):
                skip()
        L.func, L.body = fname, body
        loops.append(L)
    return loops, body


_NOT_FUNCS = {"if", "for", "while", "switch", "return", "sizeof"}


def all_loops(sources, typedefs, skip=()):
    """Loops of every function defined in `sources` (callees of the top function in the same or
    other files). Callee parameters with a top-level name use the top-level views; others are
    looked up as RTL registers like locals. Wrong guesses are harmless: Houdini discards them."""
    out, seen = [], set(skip)
    for src in sources:
        tds = dict(typedefs)
        tds.update(acsl.c_typedefs(src))
        code = re.sub(r"/\*(?!@).*?\*/|//[^\n]*", "", src, flags=re.S)
        for m in re.finditer(r"\b(\w+)\s*\([^;{)]*\)\s*\{", code):
            name = m.group(1)
            if name in _NOT_FUNCS or name in seen:
                continue
            seen.add(name)
            try:
                loops, _ = loop_annotations(src, name, set(tds), tds)
            except Exception:
                continue
            out += [lp for lp in loops if lp.invariants or lp.assigns or lp.variant]
    return out


def local_types(body, names, typedefs):
    """{local: (width, signed)} from the declarations in the function body."""
    out = {}
    for v in names:
        m = re.search(rf"((?:const\s+|unsigned\s+|signed\s+|long\s+|short\s+)*\w+)\s+{re.escape(v)}\s*(?:=|;|,)",
                      body)
        try:
            w, s, _ = acsl.c_type(m.group(1), typedefs) if m else (32, True, False)
        except Unsupported:
            w, s = 32, True
        out[v] = (w, s)
    return out


def assigned_params(body, params):
    return {p for p in params
            if re.search(rf"(\b{re.escape(p)}\s*(=(?!=)|\+\+|--|[-+*/|&^]=))|((\+\+|--)\s*{re.escape(p)}\b)", body)}


# ------------------------------------------------------------------ candidate translation

class CandTranslator(translate.Translator):
    """Contract translator for candidates: quantifiers enumerated over the modelled cells
    (a candidate is only a guess, so any finite instance is admissible), current state =
    the harness 'post' views (memory now, captured inputs), \\old = call-start snapshot."""

    def __init__(self, fn, itf, W, modified):
        super().__init__(fn, itf, W)
        self.modified = modified

    def tr(self, e, mode, view, pol=True, old=False, env=None):
        if e[0] in ("forall", "exists"):
            if not self.nmax:
                raise Unsupported("quantifier without an array bound")
            terms = [dict(env or {})]
            for v in e[1]:
                terms = [dict(t, **{v: self.lit(i)}) for t in terms for i in range(self.nmax)]
            parts = [self.tr(e[2], mode, view, pol, old, t) for t in terms]
            return "(" + (" && " if e[0] == "forall" else " || ").join(parts) + ")"
        if e[0] == "old" and e[1][0] == "var" and e[1][1] in self.modified:
            return f"{e[1][1]}_c"
        return super().tr(e, mode, view, pol, old, env)

    def cand(self, tree, env, cutoff):
        self.notes, self.side, self.calls = [], [], 0
        ex = expand(tree, self.fn.defs, cutoff, self.ptrs, types=self.types)
        cond = self.tr(ex, "assert", "post", True, False, env)
        if self.side:
            cond = f"(({' && '.join(self.side)}) && {cond})"
        if len(cond) > 200_000:
            raise Unsupported("candidate too large")
        return cond


def _names(e, acc):
    if isinstance(e, tuple):
        if e[0] == "var":
            acc.add(e[1])
        for x in e[1:]:
            _names(x, acc)
    elif isinstance(e, list):
        for x in e:
            _names(x, acc)
    return acc


def _subst_store(e, arr, j, in_at=False):
    """Replace the current-state cell arr[j] by the store data (__wd) and j by the store address
    (__wa); cells under \\at/\\old keep reading the call-start state at that address."""
    if not isinstance(e, tuple):
        return e
    k = e[0]
    if k == "var" and e[1] == j:
        return ("var", "__wa")
    if k == "idx" and e[1] == arr and e[2] == ("var", j) and not in_at:
        return ("var", "__wd")
    if k in ("at", "old"):
        return (k, _subst_store(e[1], arr, j, True)) + tuple(e[2:])
    if k in ("forall", "exists") and j in e[1]:
        return e
    return tuple(_subst_store(x, arr, j, in_at) if isinstance(x, tuple) else
                 ([_subst_store(y, arr, j, in_at) for y in x] if isinstance(x, list) else x) for x in e)


def _quantified(e):
    if isinstance(e, tuple):
        return e[0] in ("forall", "exists") or any(_quantified(x) for x in e[1:])
    if isinstance(e, list):
        return any(_quantified(x) for x in e)
    return False


def _bound(e, acc):
    if isinstance(e, tuple):
        if e[0] in ("forall", "exists"):
            acc.update(e[1])
        if e[0] in ("let",):
            acc.add(e[1])
        if e[0] == "agg":
            acc.add(e[4])
        for x in e[1:]:
            _bound(x, acc)
    elif isinstance(e, list):
        for x in e:
            _bound(x, acc)
    return acc


@dataclass
class Cand:
    id: str
    origin: str          # loop-invariant | loop-assigns | loop-variant | call-frame | structural
    source: str
    cond: str            # Verilog boolean (guard already applied)


class Generator:
    def __init__(self, fn, itf, W, norm, sigs, latency, cutoff, max_cands=600, max_placements=12,
                 extra_sources=(), aliases=None):
        self.fn, self.itf, self.W, self.sigs = fn, itf, W, sigs
        self.aliases = aliases or {}
        self.latency, self.cutoff = latency, cutoff
        self.max_cands, self.max_placements = max_cands, max_placements
        typedefs = fn.typedefs
        self.loops, self.body = loop_annotations(norm, fn.name, set(typedefs), typedefs)
        self.loops += all_loops([norm] + list(extra_sources), typedefs, skip={fn.name})
        self.params = {p.name for p in fn.params}
        self.modified = assigned_params(self.body, [p for p in itf.scalars])
        self.tr = CandTranslator(fn, itf, W, self.modified)
        self.taps = {}            # rtl name -> tap index
        self.views = {}           # (rtl name, signed) -> view wire
        self.cands, self.skipped = [], []
        self.states = fsm_states(sigs)

    # ---- taps
    def tap(self, name):
        if name not in self.taps:
            self.taps[name] = len(self.taps)
        return f"t_{self.taps[name]}"

    def view(self, name, signed):
        key = (name, signed)
        if key not in self.views:
            w = self.sigs[name]
            t = self.tap(name)
            ext = (f"{{{{{self.W - w}{{{t}[{w - 1}]}}}}, {t}}}" if signed else f"{{{self.W - w}'d0, {t}}}") \
                if w < self.W else f"{t}[{self.W - 1}:0]"
            self.views[key] = (f"tv_{len(self.views)}", ext)
        return self.views[key][0]

    def placements(self, v, ctype):
        """W-bit views of the RTL signals that may hold C variable v."""
        cw, cs = ctype
        out = []
        for n in var_signals(self.sigs, v, aliases=self.aliases):
            w = self.sigs[n]
            if w > cw:
                continue
            if w < cw and cs:
                out += [self.view(n, False), self.view(n, True)]   # narrowed: either extension
            else:
                out.append(self.view(n, cs))
        return out, {n for n in var_signals(self.sigs, v, aliases=self.aliases)}

    def placements_of(self, n, ctype):
        cw, cs = ctype
        w = self.sigs[n]
        if w > cw:
            return []
        if w < cw and cs:
            return [self.view(n, False), self.view(n, True)]      # narrowed: either extension
        return [self.view(n, cs)]

    LAGS = (1, 2)                                # stage offsets tried for overlapped iterations
    MAX_COND, MAX_TOTAL = 20_000, 1_500_000      # characters; beyond that Yosys runs out of memory

    def add(self, origin, source, cond):
        if len(cond) > self.MAX_COND:
            self.skipped.append({"source": source, "reason": f"candidate too large ({len(cond)} chars)"})
            return
        self.total = getattr(self, "total", 0) + len(cond)
        if len(self.cands) < self.max_cands and self.total <= self.MAX_TOTAL:
            self.cands.append(Cand(f"I{len(self.cands) + 1}", origin, source, cond))

    def guards(self, scopes):
        g = [("h_busy", "busy")]
        for sc in sorted(scopes):
            # a sub-module's first FSM state is also its idle state: its registers are stale
            # until it is started, so states are also tried under the sub-module's ap_start
            run = f"{sc}.ap_start" if sc and self.sigs.get(f"{sc}.ap_start") == 1 else None
            if run:
                g.append((f"(h_busy && {self.tap(run)})", f"busy & {run}"))
            for s in self.states.get(sc, [])[:24]:
                g.append((f"(h_busy && {self.tap(s)})", f"busy & {s}"))
                if run:
                    g.append((f"(h_busy && {self.tap(run)} && {self.tap(s)})", f"busy & {run} & {s}"))
            for en in self.stage_enables(sc):       # later pipeline stages active
                g.append((f"(h_busy && {self.tap(en)})", f"busy & {en}"))
        return g

    def stage_enables(self, sc):
        pre = f"{sc}." if sc else ""
        return sorted(n for n, w in self.sigs.items() if w == 1 and n.startswith(pre) and _scope(n) == sc
                      and re.fullmatch(r"ap_enable_reg_pp\d+_iter[1-9]\d*", _leaf(n)))

    # ---- loop clauses with locals mapped to RTL
    def clause(self, origin, src, tree, body=None):
        names = _names(tree, set()) - _bound(tree, set()) - (self.params - self.modified)
        names -= set(getattr(self.fn, "globals", ()) or ())
        names = {n for n in names if not any(n == d for d in self.fn.defs)}
        types = local_types(body if body is not None else self.body, names, self.fn.typedefs)
        # placements grouped by module instance: all locals of one loop live in the same
        # (sub-)module, so a combination draws every variable from one scope (or the top level)
        by_scope = {}
        for v in sorted(names):
            per = {}
            for n in var_signals(self.sigs, v, aliases=self.aliases):
                for x in self.placements_of(n, types[v]):
                    per.setdefault(_scope(n), []).append(x)
            if not per:
                self.skipped.append({"source": src, "reason": f"no RTL signal found for C variable {v}"})
                return
            by_scope[v] = per
        scopes = set().union(*[set(p) for p in by_scope.values()]) if by_scope else {""}
        seen = set()
        for sc in sorted(scopes):
            choices = []
            for v in sorted(names):
                opts = by_scope[v].get(sc) or by_scope[v].get("")
                if not opts:
                    break
                choices.append([(v, x) for x in opts])
            else:
                combos = list(product(*choices))[:self.max_placements] if choices else [()]
                for combo in combos:
                    if combo in seen:
                        continue
                    seen.add(combo)
                    env = dict(combo)
                    try:
                        body = self.tr.cand(tree, env, self.cutoff)
                    except Unsupported as ex:
                        self.skipped.append({"source": src, "reason": str(ex)})
                        return
                    for g, gtxt in self.guards({sc}):
                        place = ", ".join(f"{v}={x}" for v, x in combo)
                        self.add(origin, f"{src}  [{gtxt}{'; ' + place if place else ''}]", f"(!{g} || {body})")
                    if combo and origin == "loop-invariant":
                        self.write_port_instances(src, tree, combo, sc)
                    # pipelined loops overlap iterations: at the header the last store(s) may still
                    # be in flight, so the invariant holds for the counter lagged by the store's stage
                    # (software-pipelining stage offsets). Only for clauses over memory.
                    if combo and origin == "loop-invariant" and _quantified(tree):
                        for lab, lag in self.lags(sc):
                            env_l = {v: f"({x} - {lag})" for v, x in combo}
                            try:
                                body_l = self.tr.cand(tree, env_l, self.cutoff)
                            except Unsupported:
                                break
                            for g, gtxt in self.guards({sc}):
                                place = ", ".join(f"{v}={x}-{lab}" for v, x in combo)
                                self.add("loop-invariant-lag", f"{src}  [{gtxt}; {place}]", f"(!{g} || {body_l})")

    def lags(self, sc):
        """Counter offsets for overlapped iterations: constants, and the dynamic commit count
        i - (v_1 + ... + v_d) over the stage-valid bits (exact in pipeline fill, steady state
        and drain; Puri-Ray-Hao-Xie's staircase)."""
        out = [(str(l), self.tr.lit(l)) for l in self.LAGS]
        acc = []
        for en in self.stage_enables(sc)[:3]:
            acc.append(f"$signed({{{self.W - 1}'d0, {self.tap(en)}}})")
            out.append(("+".join(_leaf(e) for e in self.stage_enables(sc)[:len(acc)]), "(" + " + ".join(acc) + ")"))
        return out

    # ---- contract persistence: after a call, the postcondition holds while the block is idle
    def contract_persistence(self):
        for beh in [self.fn.default] + self.fn.behaviors:
            A = None
            if beh.assumes:
                A = beh.assumes[0][2]
                for x in beh.assumes[1:]:
                    A = ("bin", "&&", A, x[2])
            for label, txt, tree in beh.ensures:
                t = tree if A is None else ("bin", "==>", ("old", A), tree)
                try:
                    body = self.tr.cand(t, {}, self.cutoff)
                except Unsupported:
                    continue
                self.add("contract-persist", f"ensures {txt} [idle after a call]",
                         f"(!(h_called && !h_busy && !h_start_evt) || {body})")

    # ---- pipeline copies: registers of later stages hold the loop counter of earlier iterations
    def pipeline_copies(self):
        for lp in self.loops:
            vs = self._loop_vars(lp)
            types = local_types(lp.body or self.body, vs, self.fn.typedefs)
            for v in sorted(vs):
                names = var_signals(self.sigs, v, aliases=self.aliases)
                for sc in sorted({_scope(n) for n in names}):
                    ens = self.stage_enables(sc)
                    if not ens:
                        continue
                    xs = [x for n in names if _scope(n) == sc for x in self.placements_of(n, types[v])][:3]
                    regs = [n for n, w in self.sigs.items() if _scope(n) == sc and 1 < w <= 64 and n not in names
                            and re.fullmatch(r"\w+_reg_\d+", _leaf(n)) and not _leaf(n).startswith("icmp")][:12]
                    for r in regs:
                        rv = self.view(r, False)
                        for x in xs:
                            for c in (-2, -1, 0, 1, 2):
                                for en in ens:
                                    self.add("pipeline-copy", f"{r} == {v} {c:+d} [busy & {en}; {v}={x}]",
                                             f"(!(h_busy && {self.tap(en)}) || ({rv} == {x} + {self.tr.lit(c)}))")

    # ---- in-flight stores: a quantified loop invariant instantiated at the RAM write port
    def write_port_instances(self, src, tree, combo, sc):
        """For `\\forall j; G(j) ==> ... a[j] ...`, a store in flight carries the value the loop
        is about to commit: on every enabled write (address w, data d), G(w) ==> ...d...
        (current-state a[j] -> d, j -> w, \\at(a[j], Pre) stays the call-start value at w).
        Nothing in the C proof or the HLS structure states this, yet induction over a
        pipelined loop needs it (the write lands after the loop header has moved on)."""
        if tree[0] != "forall" or len(tree[1]) != 1:
            return
        j = tree[1][0]
        for a, mem in self.itf.memories.items():
            if mem.protocol == "cells" or mem.slots != 1:
                continue
            for b, bank in enumerate(mem.banks):
                if bank.wen_bits != 1 or bank.byte_addr:
                    continue
                for n in ("0", "1"):
                    P = bank.ports
                    we, ce, ad, dd = P.get("we" + n), P.get("ce" + n), P.get("address" + n), P.get("d" + n)
                    if not (we and ce and ad and dd):
                        continue
                    body = _subst_store(tree[2], a, j)
                    if body is None:
                        continue
                    W, w = self.W, mem.width
                    dview = f"$signed({{{{{W - w}{{{dd}[{w - 1}]}}}}, {dd}}})" if mem.signed else f"$signed({{{W - w}'d0, {dd}}})"
                    aview = f"$signed({{{W - bank.aw}'d0, {ad}}})"
                    for lab, lag in [("0", None)] + self.lags(sc):
                        env = {v: (x if lag is None else f"({x} - {lag})") for v, x in combo}
                        env.update({"__wd": dview, "__wa": aview})
                        try:
                            cond = self.tr.cand(body, env, self.cutoff)
                        except Unsupported:
                            return
                        place = ", ".join(f"{v}={x}" + (f"-{lab}" if lag is not None else "") for v, x in combo)
                        for g, gtxt in self.guards({sc}):
                            self.add("loop-invariant-store", f"{src}  [write port {a}.{n}; {gtxt}; {place}]",
                                     f"(!({g} && {ce} && {we}) || {cond})")

    def frame(self, origin, src, items, guard_scopes, body=None):
        """Every cell outside the listed locations keeps its call-start value."""
        typenames = set(self.fn.typedefs)
        wins = {a: [] for a in list(self.itf.memories) + list(self.itf.objects)}
        for it in items:
            if it.strip() in ("\\nothing",) or re.fullmatch(r"\w+", it.strip()) and it.strip() not in wins:
                continue                                  # locals: registers, not memory
            try:
                w = translate._assigns_item(it, self.fn, typenames)
            except Unsupported:
                w = None
            if w and w[0] in wins:
                wins[w[0]].append(w)
            elif w is None:
                self.skipped.append({"source": src, "reason": f"location {it!r} not supported"})
                return
        for a, ws in wins.items():
            size = self.itf.memories[a].size if a in self.itf.memories else 1
            names = set()
            for w in ws:
                _names(w[1], names), _names(w[2], names)
            names -= self.params - self.modified
            types = local_types(body if body is not None else self.body, names, self.fn.typedefs)
            choices = []
            for v in sorted(names):
                pl, _ = self.placements(v, types[v])
                if not pl:
                    choices = None
                    break
                choices.append([(v, x) for x in pl])
            if choices is None:
                continue
            for combo in (list(product(*choices))[:self.max_placements] if choices else [()]):
                env = dict(combo)
                try:
                    bounds = [(self.tr.cand(w[1], env, self.cutoff), self.tr.cand(w[2], env, self.cutoff))
                              for w in ws if w[3] is None]
                except Unsupported:
                    continue
                parts = []
                lay = self.tr.pmap[a].layout if a in self.tr.pmap else None
                if lay and any(w[3] for w in ws):
                    continue                              # per-field windows: not generated
                for i in range(size):
                    inw = [f"(({l} <= {self.tr.lit(i)}) && ({self.tr.lit(i)} <= {h}))" for l, h in bounds]
                    keep = f"({a}_e{i} == {a}_o{i})" if not lay else \
                        " && ".join(f"({a}_e{i}__{f[0]} == {a}_o{i}__{f[0]})" for f in lay)
                    parts.append(f"({' || '.join(inw)} || ({keep}))" if inw else f"({keep})")
                body = "(" + " && ".join(parts) + ")"
                for g, gtxt in self.guards(guard_scopes):
                    self.add(origin, f"{src} [frame of {a}; {gtxt}]", f"(!{g} || {body})")

    # ---- memory-read consistency: the registered RAM output holds the cell at a loop index
    def memory_reads(self):
        idx_vars, types = set(), {}
        for lp in self.loops:
            vs = self._loop_vars(lp)
            idx_vars |= vs
            for v, t in local_types(lp.body or self.body, vs, self.fn.typedefs).items():
                types.setdefault(v, t)
        for a, mem in self.itf.memories.items():
            if mem.protocol == "cells":
                continue
            for b, bank in enumerate(mem.banks):
                cells = [(i,) + tuple(mem.locate(i)[1:]) for i in range(mem.size) if mem.locate(i)[0] == b]
                for n in ("0", "1"):
                    q = bank.ports.get("q" + n)
                    if not q:
                        continue
                    for v in sorted(idx_vars):
                        pl, names = self.placements(v, types[v])
                        scopes = {_scope(x) for x in names} or {""}
                        for x in pl:
                            parts = [f"(({x}) != {self.tr.lit(i)} || {q}[{s * mem.width + mem.width - 1}:{s * mem.width}]"
                                     f" == m_{a}_{i})" for i, word, s in cells]
                            body = "(" + " && ".join(parts) + ")"
                            for g, gtxt in self.guards(scopes)[1:]:
                                self.add("structural", f"{q} holds {a}[{v}] [{gtxt}; {v}={x}]", f"(!{g} || {body})")

    # ---- the precondition keeps holding on the captured inputs / call-start snapshot
    def call_requires(self):
        for beh in [self.fn.default] + self.fn.behaviors:
            A = None
            if beh.assumes:
                A = beh.assumes[0][2]
                for x in beh.assumes[1:]:
                    A = ("bin", "&&", A, x[2])
            for label, txt, tree in beh.requires:
                try:
                    tx = expand(tree, self.fn.defs, self.cutoff, self.tr.ptrs, types=self.tr.types)
                except Unsupported:
                    tx = tree
                for c in translate.conjuncts(tx):
                    if c[0] == "valid":
                        arr, lo, hi = c[1], c[2], c[3]
                        if arr not in self.itf.memories:
                            continue
                        n = self.itf.memories[arr].size
                        c = ("bin", "||", ("cmp", ["<"], [hi, lo]),
                             ("bin", "&&", ("cmp", ["<="], [("int", 0), lo]), ("cmp", ["<"], [hi, ("int", n)])))
                    t = c if A is None else ("bin", "==>", A, c)
                    try:
                        body = self.tr.cand(("old", t), {}, self.cutoff)
                    except Unsupported:
                        continue
                    self.add("call-requires", f"requires {txt} [captured inputs, during the call]",
                             f"(!(h_busy || h_done_d) || {body})")

    # ---- progress (loop variants -> latency): cycles of the call are bounded by the loop counter
    def progress(self, L):
        """Candidates `guard ==> h_cyc <= m * v + c` for loop counters v; with v bounded by the
        call-requires / loop invariants they make a latency bound inductive."""
        out = []

        def add(src, cond):
            out.append(Cand(f"G{len(out) + 1}", "progress", src, cond))
        for lp in self.loops:
            vs = self._loop_vars(lp)
            types = local_types(lp.body or self.body, vs, self.fn.typedefs)
            for v in sorted(vs):
                pl, names = self.placements(v, types[v])
                scopes = {_scope(x) for x in names} or {""}
                for x in pl:
                    for g, gtxt in self.guards(scopes):
                        for m in (1, 2, 3, 4):
                            for c in sorted({max(2, L // 2), L}):
                                add(f"h_cyc <= {m}*{v} + {c} [{gtxt}; {v}={x}]",
                                    f"(!{g} || ({{{self.W - 16}'d0, h_cyc}} <= {m} * {x} + {c}))")
        add("busy: h_cyc >= 1", "(!h_busy || h_cyc >= 1)")
        add(f"busy: h_cyc <= {L}", f"(!h_busy || h_cyc <= {L})")
        return out[:800]

    # ---- structural (HLS) candidates
    def structural(self):
        L = self.latency or 0
        self.add("structural", "no cycle counter while idle", "(h_busy || h_cyc == 0)")
        # harness/RTL coupling: the harness believes a call is in flight exactly when the RTL is busy
        idle = self.itf.control.get("idle")
        if idle:
            self.add("structural", "busy => RTL not idle", f"(!h_busy || !{idle})")
            self.add("structural", "idle harness => RTL idle (or a call being accepted)",
                     f"(h_busy || h_start_evt || {idle})")
        for sc in sorted({_scope(n) for n in self.sigs}):
            for en in self.stage_enables(sc) + [n for n in self.sigs if _scope(n) == sc and self.sigs[n] == 1
                                                and re.fullmatch(r"ap_enable_reg_pp\d+_iter0", _leaf(n))]:
                self.add("structural", f"{en} valid => call in flight", f"(!{self.tap(en)} || h_busy)")
        if L:
            self.add("structural", f"a call in flight is younger than the latency bound ({L})",
                     f"(!h_busy || h_cyc <= {L + 2})")
        for sc, sts in self.states.items():
            fsm = (sc + "." if sc else "") + "ap_CS_fsm"
            if self.sigs.get(fsm, 1) > 1:
                f = self.tap(fsm)
                self.add("structural", f"one-hot FSM {fsm}", f"(({f} != 0) && (({f} & ({f} - 1)) == 0))")
            first = sts[0]
            self.add("structural", f"idle: {first} when no call is in flight", f"(h_busy || {self.tap(first)})")
            for s in sts[1:]:
                self.add("structural", f"{s} only during a call", f"(!{self.tap(s)} || h_busy)")
        # control registers keep their post-reset value while no call is in flight
        # (sub-module start registers, loop-init / pipeline-enable flags, done registers, ...);
        # both polarities are proposed, Houdini keeps the true one
        ports = set(self.itf.ports) if hasattr(self.itf, "ports") else set()
        ctrl = [n for n, w in self.sigs.items() if w == 1 and n not in ports and
                re.fullmatch(r"(ap_\w*(_reg|_init\w*|_int|_iter\d+)|grp_\w+_ap_start_reg|\w+_reg(_\d+)?)", _leaf(n))]
        for n in sorted(ctrl, key=lambda x: (x.count("."), x))[:80]:
            t = self.tap(n)
            self.add("structural", f"{n} low while idle", f"(h_busy || !{t})")
            self.add("structural", f"{n} high while idle", f"(h_busy || {t})")
        pmap = {p.name: p for p in self.fn.params}
        for c, port in self.itf.scalars.items():
            if c in self.modified:
                continue
            pw = pmap[c].width
            self.add("structural", f"input {c} stable during a call", f"(!h_busy || {port} == {c}_cap)")
            for n, w in self.sigs.items():
                if w == pw and re.fullmatch(rf"{re.escape(c)}(_read)?(_\d+)?_reg_\d+", _leaf(n)):
                    for g, gtxt in self.guards({_scope(n)})[1:]:
                        self.add("structural", f"{n} holds input {c} [{gtxt}]",
                                 f"(!{g} || {self.tap(n)} == {c}_cap)")

    def generate(self):
        self.structural()
        self.call_requires()
        self.memory_reads()
        self.pipeline_copies()
        self.contract_persistence()
        if self.fn.default.assigns:                      # contract frame during the whole call
            self.frame("call-frame", "assigns " + ", ".join(self.fn.default.assigns), self.fn.default.assigns,
                       set())
        for k, lp in enumerate(self.loops):
            where = f"loop {k + 1}" + (f" ({lp.func})" if lp.func and lp.func != self.fn.name else "")
            for label, txt, tree in lp.invariants:
                self.clause("loop-invariant", f"{where}: loop invariant {label + ': ' if label else ''}{txt}",
                            tree, lp.body)
            if lp.variant:
                self.clause("loop-variant", f"{where}: loop variant {lp.variant[0]} >= 0",
                            ("cmp", [">="], [lp.variant[1], ("int", 0)]), lp.body)
            if lp.assigns:
                scopes = set()
                for v in self._loop_vars(lp):
                    scopes |= {_scope(n) for n in var_signals(self.sigs, v, aliases=self.aliases)}
                self.frame("loop-assigns", f"{where}: loop assigns {', '.join(lp.assigns)}", lp.assigns,
                           scopes, lp.body)
        return self.cands

    def _loop_vars(self, lp):
        acc = set()
        for _, _, t in lp.invariants:
            _names(t, acc)
        return acc - self.params

    # ---- harness text
    def harness_block(self, mode):
        """Verilog appended to the harness: taps, views, and the candidates as
        assertions (mode 'assert') or as assumptions (mode 'assume', proven invariants)."""
        clk = self.itf.control["clk"]
        out = ["  // ---- A1: ghost 'a call has completed since reset' (contract persistence)",
               f"  reg h_called = 0; always @(posedge {clk}) if (!h_init && h_done_d) h_called <= 1;",
               "  // ---- A1: taps on internal RTL signals (connected in the SBY script)"]
        for n, i in self.taps.items():
            out.append(f"  (* keep *) wire [{self.sigs[n] - 1}:0] t_{i};      // {n}")
        for (n, s), (v, ext) in self.views.items():
            out.append(f"  wire signed [{self.W - 1}:0] {v} = {ext};")
        return out

    def connects(self):
        return [f"connect -nounset -set t_{i} dut.{n}" for n, i in self.taps.items()]


def emit(base_text, gen, cands, mode, keep_contract, assumed=()):
    """Harness text with A1 candidates. mode: 'assert' | 'assume'; `assumed`: already proven
    invariants, emitted as assumptions. keep_contract=False disables the harness's own
    assertions (contract at ap_done, latency bound). Returns (text, {line: cand id})."""
    lines = base_text.rstrip().split("\n")
    assert lines[-1].strip() == "endmodule"
    lines = lines[:-1]
    if not keep_contract:
        lines = [("    // (harness assertion disabled for the invariant search)"
                  if re.match(r"\s*if \((h_done_d|h_busy)\) assert\(", l) else l) for l in lines]
    lines += gen.harness_block(mode)
    # solver-chosen symbols of the candidates (quantifier/cut-off values, cells outside the
    # model) that the contract harness does not declare
    used = " ".join(c.cond for c in list(cands) + list(assumed))
    for name, width in gen.tr.anyconst:
        if re.search(rf"\b{re.escape(name)}\b", used) and \
                not re.search(rf"\b(reg|wire)\b[^;]*\b{re.escape(name)}\s*(;|=)", base_text):
            lines.append(f"  (* anyconst *) reg {'signed ' if width > 1 else ''}[{width - 1}:0] {name};")
    if assumed:
        lines.append("  always @(*) if (!h_init) begin     // proven invariants")
        lines += [f"    assume({c.cond});   // {c.id}" for c in assumed]
        lines.append("  end")
    lines.append("  always @(*) if (!h_init) begin")
    lmap = {}
    for c in cands:
        lines.append(f"    // {c.id} {c.origin}: {c.source}")
        lmap[len(lines) + 1] = c.id
        lines.append(f"    {mode}({c.cond});")
    lines.append("  end")
    lines.append("endmodule")
    return "\n".join(lines) + "\n", lmap
