"""RQ6 — LLM assistance under formal guardrails (A3).

An LLM may only contribute *checked* artefacts. This module implements the first use:
lifting an axiomatic/inductive/abstract ACSL definition, which the rule-based translation
rejects, into an explicit (possibly recursive) definition that the translation supports.

  1. prompt     the axiomatic block, its uses in the contract, the translatable fragment
  2. response   an explicit definition `name_c2sva` and an equivalence lemma over a domain
  3. validate   Frama-C/WP proves, in the original file (axioms in scope):
                  (a) the equivalence lemma (with the axioms as hypotheses), and
                  (b) consistency probe: the axioms plus the LLM's instance lemmas do not
                      prove `\\false` (an inconsistent axiomatisation makes every proof
                      vacuous; reported as a finding, the lifting is then refused)
  4. apply      a validated definition replaces the axiomatic one in the translator
                (logic.set_overrides); clauses using it become translatable

Soundness: the explicit definition is used only if WP proves it equal to the axiomatic one on
the stated domain, and the domain is conjoined as a side condition where the definition is
used (assumption: guard, weaker; assertion: conjunct, stronger) — the polarity rule.

Backends: `anthropic` (ANTHROPIC_API_KEY set) or `files` (prompt written to
<dir>/<id>.prompt.md, response read from <dir>/<id>.response.md — any LLM, or a person).
"""
import json
import os
import re
import subprocess
from pathlib import Path

PROMPT = """You are helping a verification tool that translates ACSL contracts into hardware
properties. The tool cannot use the axiomatic definition below. Give an EQUIVALENT explicit
ACSL definition that it can use: a `logic` function or `predicate` defined by an expression,
recursion allowed (it is unrolled), no axioms, no inductive cases, no \\sum/\\numof over
unbounded ranges.

Axiomatic block:
```
{axiomatic}
```

How the contract uses `{name}`:
```
{uses}
```

ACSL syntax reminders (Frama-C 33): `\\numof(lo, hi, \\lambda integer k; P(k))` counts k in
lo..hi INCLUSIVE; `\\sum(lo, hi, \\lambda integer k; e(k))`; there is no `\\numof(integer k; ...)`
form. Conditional: `c ? a : b`. Memory labels on predicates: `p{{L1,L2}}(...)`, read with
`\\at(e, L1)`. Recursive definitions: `logic integer f(integer n) = n <= 0 ? 1 : n * f(n - 1);`.

Answer with exactly two ACSL annotations and nothing else:
1. the definition, named `{name}_c2sva`, with the same parameters (and labels) as `{name}`;
2. a lemma named `{name}_c2sva_eq` stating `{name}(...) == {name}_c2sva(...)` (or `<==>` for a
   predicate) for all arguments in the domain where the contract uses it (state the domain as
   an implication premise; keep it as large as the axioms justify).
Use the form:
/*@ logic integer {name}_c2sva(...) = ...; */
/*@ lemma {name}_c2sva_eq: \\forall ...; ... ==> {name}(...) == {name}_c2sva(...); */
"""


class FileBackend:
    def __init__(self, d):
        self.d = Path(d)
        self.d.mkdir(parents=True, exist_ok=True)

    def complete(self, key, prompt):
        (self.d / f"{key}.prompt.md").write_text(prompt)
        r = self.d / f"{key}.response.md"
        return r.read_text() if r.exists() else None


class AnthropicBackend:
    def __init__(self, model="claude-sonnet-5-5"):
        import anthropic
        self.client, self.model = anthropic.Anthropic(), model

    def complete(self, key, prompt):
        msg = self.client.messages.create(model=self.model, max_tokens=2000,
                                          messages=[{"role": "user", "content": prompt}])
        return "".join(b.text for b in msg.content if getattr(b, "type", "") == "text")


def backend(kind, d):
    if kind == "anthropic" or (kind == "auto" and os.environ.get("ANTHROPIC_API_KEY")):
        return AnthropicBackend()
    return FileBackend(d)


def axiomatic_blocks(src):
    """{name: block text} for every logic name declared inside an axiomatic/inductive block."""
    out = {}
    for m in re.finditer(r"/\*@(.*?)\*/", src, re.S):
        t = m.group(1)
        for ax in re.finditer(r"\baxiomatic\s+\w+\s*\{", t):
            i, d = ax.end(), 1
            while i < len(t) and d:
                d += {"{": 1, "}": -1}.get(t[i], 0)
                i += 1
            blk = t[ax.start():i]
            for n in re.findall(r"\b(?:logic\s+[\w\s\*]+?|predicate)\s+(\w+)\s*(?:\{[^}]*\})?\s*\(", blk):
                out[n] = blk
        for ind in re.finditer(r"\binductive\s+(\w+)", t):
            out[ind.group(1)] = t
    return out


def prompt_for(name, block, uses):
    return PROMPT.format(name=name, axiomatic=block.strip(), uses="\n".join(uses) or "(not shown)")


REPAIR = """Your previous answer for `{name}` was rejected by Frama-C with this error:
```
{error}
```
Previous answer:
```
{answer}
```
Fix the ACSL syntax only (keep the meaning). Answer again with exactly the two annotations.
"""


def repair_prompt(name, answer, error):
    return REPAIR.format(name=name, answer=answer.strip(), error="\n".join(error)[:1500])


def parse_response(text, name):
    anns = re.findall(r"/\*@.*?\*/", text or "", re.S)
    defn = [a for a in anns if re.search(rf"\b{re.escape(name)}_c2sva\b\s*(\{{[^}}]*\}})?\s*\(", a)
            and re.search(r"\b(logic|predicate)\b", a) and "lemma" not in a]
    lem = [a for a in anns if re.search(rf"\blemma\s+{re.escape(name)}_c2sva_eq\b", a)]
    return (defn[0], lem[0]) if defn and lem else (None, None)


def _wp(path, prop, timeout=120):
    """Prove one property (`-wp-prop`) of a file; -> (proved: bool|None, log)."""
    try:
        p = subprocess.run(["frama-c", "-wp", "-wp-prover", "alt-ergo,z3,cvc5", "-wp-timeout", "20",
                            "-wp-prop", prop, str(path)], capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return False, "timeout"
    out = p.stdout + p.stderr
    if re.search(r"annot-error|syntax error|\[kernel\] User Error", out):
        return None, out
    m = re.search(r"Proved goals:\s+(\d+)\s*/\s*(\d+)", out)
    return (bool(m) and m.group(1) == m.group(2) and m.group(2) != "0"), out


def validate(src, name, defn, lemma, workdir):
    """-> {status: validated-equivalence | validated-unfolding | lemma-unproved |
    inconsistent-axioms | error, domain}. The equivalence lemma (needs induction in general)
    is tried first; the unfolding lemma (induction-free) is what the translation relies on."""
    workdir = Path(workdir)
    workdir.mkdir(parents=True, exist_ok=True)
    unf, dom = unfolding_lemma(name, defn, lemma)
    status = None
    f = workdir / f"{name}_lift.c"
    f.write_text(src + "\n" + defn + "\n" + lemma + "\n")       # axioms in scope as hypotheses
    ok, out = _wp(f, f"{name}_c2sva_eq")
    (workdir / f"{name}_lift.log").write_text(out)
    if ok is None:
        det = [l for l in out.splitlines() if "rror" in l][:3]
        lines = f.read_text().splitlines()
        for n in re.findall(r"_lift\.c:(\d+):", "\n".join(det))[:1]:
            if 0 < int(n) <= len(lines):
                det.append(f"offending line {n}: {lines[int(n) - 1].strip()[:400]}")
        return {"status": "error", "detail": det}
    proved = [lemma] if ok else []
    if ok:
        status = "validated-equivalence"
    if unf:
        g = workdir / f"{name}_unfold.c"
        g.write_text(src + "\n" + defn + "\n" + unf + "\n")
        ok_u, out_u = _wp(g, f"{name}_c2sva_unfold")
        (workdir / f"{name}_unfold.log").write_text(out_u)
        if ok_u:
            proved.append(unf)
            status = status or "validated-unfolding"
    if unf is None or unf not in proved:
        # the LLM's own lemma is never trusted for acceptance: only the unfolding lemma that
        # c2sva builds from the definition (Prop. 3' in docs/theory.md) admits a definition
        return {"status": "lemma-unproved" if unf else "unusable-domain", "domain": dom,
                "equivalence_lemma": bool(ok)}
    h = workdir / f"{name}_probe.c"                             # consistency probe
    h.write_text(src + "\n" + defn + "\n" + "\n".join(proved) + "\n/*@ lemma c2sva_probe_false: \\false; */\n")
    ok2, out2 = _wp(h, "c2sva_probe_false")
    (workdir / f"{name}_probe.log").write_text(out2)
    if ok2:
        return {"status": "inconsistent-axioms", "domain": dom}
    return {"status": "validated-unfolding", "domain": dom, "equivalence_lemma": bool(ok)}


_DEF = re.compile(r"(logic\s+[\w\s\*]+?|predicate)\s+(\w+)\s*(\{[^}]*\})?\s*\(([^)]*)\)\s*=\s*(.*?);\s*\*/", re.S)


def split_def(defn):
    """-> (kind 'logic'|'predicate', labels text, [(type, param)], body text) or None."""
    m = _DEF.search(defn)
    if not m:
        return None
    kind = "predicate" if m.group(1).startswith("predicate") else "logic"
    params = []
    for p in [x.strip() for x in m.group(4).split(",") if x.strip()]:
        w = p.replace("*", " * ").split()
        params.append((" ".join(w[:-1]), w[-1]))
    return kind, m.group(3) or "", params, m.group(5).strip()


def domain_of(lemma, params, name):
    """Domain = conjunction of ALL premises of the LLM's lemma (`A ==> B ==> concl` gives
    A && B), its variables renamed to the definition's parameters through the arguments of
    the `name(...)` call in the conclusion (not by binder order). Unusable -> None."""
    m = re.search(r"\\forall\s+(.*?);\s*(.*)\*/", lemma, re.S)
    if not m:
        return "\\true"
    body = m.group(2).strip().rstrip(";").strip()
    parts, depth, cur, i = [], 0, "", 0
    while i < len(body):                      # split on top-level ==>
        c = body[i]
        depth += c in "([{"
        depth -= c in ")]}"
        if depth == 0 and body.startswith("==>", i):
            parts.append(cur)
            cur, i = "", i + 3
            continue
        cur += c
        i += 1
    parts.append(cur)
    prem = " && ".join(f"({p.strip()})" for p in parts[:-1]) or "\\true"
    call = re.search(rf"\b{re.escape(name)}\s*(\{{[^}}]*\}})?\s*\(([^()]*)\)", parts[-1])
    if not call:
        return None
    args = [a.strip() for a in call.group(2).split(",")]
    if len(args) != len(params) or not all(re.fullmatch(r"\w+", a) for a in args):
        return None
    for a, (_, p) in zip(args, params):
        prem = re.sub(rf"\b{re.escape(a)}\b", f"__{p}", prem)
    return re.sub(r"\b__(\w+)", r"\1", prem)


def unfolding_lemma(name, defn, lemma):
    """`\\forall params; DOM ==> name(params) == BODY[name_c2sva := name]`: the axiomatic
    function satisfies the explicit definition's recursion equation on its domain. Provable
    from the axioms without induction; justifies every finite unrolling (deeper calls and
    calls outside DOM are solver-chosen in the translation)."""
    sp = split_def(defn)
    if not sp:
        return None, None
    kind, labels, params, body = sp
    dom = domain_of(lemma, params, name)
    if dom is None:
        return None, None
    body = re.sub(rf"\b{re.escape(name)}_c2sva\b", name, body)
    binder = ", ".join(f"{t} {p}" for t, p in params)
    args = ", ".join(p for _, p in params)
    rel = "<==>" if kind == "predicate" else "=="
    return (f"/*@ lemma {name}_c2sva_unfold{labels}: \\forall {binder}; ({dom}) ==> "
            f"({name}({args}) {rel} ({body})); */"), dom


def override_text(name, defn, domain="\\true"):
    """The validated definition under the original name, with its domain (outside the domain
    the translator uses a solver-chosen value)."""
    return {"definition": re.sub(rf"\b{re.escape(name)}_c2sva\b", name, defn), "domain": domain}
