"""Automatic first-order mutants of the top function body (standard operator classes).

  ROR  relational operator replacement     <  <=  >  >=  ==  !=  (to its closest neighbour)
  AOR  arithmetic operator replacement     +  <-> -,   *  -> +
  LCR  logical connector replacement       && <-> ||
  CRP  constant replacement                integer literal c -> c + 1   (expressions only)

Comments, pragmas, preprocessor lines, array declarators and string/char literals are never
touched. Sites are enumerated in source order; k sites are chosen deterministically,
round-robin over the operator classes present and evenly spread within each class, so the
selection is design-independent and reproducible.
"""
import re

ROR = {"<": "<=", "<=": "<", ">": ">=", ">=": ">", "==": "!=", "!=": "=="}
AOR = {"+": "-", "-": "+", "*": "+"}
LCR = {"&&": "||", "||": "&&"}
OPTOK = re.compile(r"<<=|>>=|<<|>>|<=|>=|==|!=|\+\+|--|\+=|-=|\*=|->|&&|\|\||[<>+\-*]|\b\d+[uUlL]*\b")


def _mask(s):
    s = re.sub(r"/\*.*?\*/|//[^\n]*", lambda m: " " * len(m.group(0)), s, flags=re.S)
    s = re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', lambda m: " " * len(m.group(0)), s)
    return re.sub(r"^\s*#[^\n]*", lambda m: " " * len(m.group(0)), s, flags=re.M)


def body_span(src, fname):
    masked = _mask(src)
    m = re.search(r"\b%s\s*\([^)]*\)\s*\{" % re.escape(fname), masked)
    if not m:
        return None
    depth, i = 0, m.end() - 1
    while i < len(masked):
        depth += masked[i] == "{"
        depth -= masked[i] == "}"
        if depth == 0:
            return m.end(), i
        i += 1
    return None


def _is_unary_or_deref(masked, pos):
    prev = masked[:pos].rstrip()[-1:] if masked[:pos].strip() else ""
    return prev in ("", "(", "[", ",", "=", "{", ";", "?", ":", "!", "<", ">", "&", "|", "+", "-", "*", "/", "%") \
        or re.search(r"\breturn\s*$", masked[:pos]) is not None


def sites(src, fname):
    span = body_span(src, fname)
    if not span:
        return []
    masked = _mask(src)
    out = []
    for m in OPTOK.finditer(masked, span[0], span[1]):
        tok, pos = m.group(0), m.start()
        if tok in ROR:
            out.append(("ROR", pos, tok, ROR[tok]))
        elif tok in LCR:
            out.append(("LCR", pos, tok, LCR[tok]))
        elif tok in AOR:
            if tok in ("*", "-") and _is_unary_or_deref(masked, pos):
                continue                                  # dereference / unary minus
            out.append(("AOR", pos, tok, AOR[tok]))
        elif tok[0].isdigit():
            before = masked[:pos].rstrip()
            if before.endswith("[") and re.search(r"\b(int|char|short|long|unsigned|\w+_t)\s+\w+\s*\[$", before):
                continue                                  # array declarator size
            num = re.match(r"\d+", tok).group(0)
            out.append(("CRP", pos, tok, str(int(num) + 1) + tok[len(num):]))
    return out


def mutants(src, fname, k=3):
    all_sites = sites(src, fname)
    if not all_sites or k <= 0:
        return []
    by_class = {}
    for s in all_sites:
        by_class.setdefault(s[0], []).append(s)
    for c, lst in by_class.items():                      # spread within each class
        n = len(lst)
        order = sorted(range(n), key=lambda i: (bin(i).count("1"), i))
        by_class[c] = [lst[(i * 7919) % n] if n > 1 else lst[0] for i in range(n)]
        seen, uniq = set(), []
        for s in by_class[c] + lst:
            if s[1] not in seen:
                seen.add(s[1])
                uniq.append(s)
        by_class[c] = uniq
    chosen, classes = [], sorted(by_class)
    while len(chosen) < k and any(by_class.values()):
        for c in classes:
            if by_class[c] and len(chosen) < k:
                chosen.append(by_class[c].pop(0))
    chosen.sort(key=lambda s: s[1])
    out = []
    for cls, pos, old, new in chosen:
        line = src.count("\n", 0, pos) + 1
        lo, hi = src.rfind("\n", 0, pos) + 1, src.find("\n", pos)
        snippet = src[lo:hi if hi >= 0 else None].strip()
        out.append((f"{cls} line {line}: '{old}' -> '{new}' in `{snippet[:48]}`",
                    src[:pos] + new + src[pos + len(old):]))
    return out
