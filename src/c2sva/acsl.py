"""ACSL front end: parse a function contract (from Frama-C's normalized print) into trees.

Trees are tuples:
  ('int',v) ('bool',b) ('var',n) ('result',) ('old',e) ('at',e,label) ('idx',arr,e)
  ('un',op,e) ('bin',op,l,r) ('cmp',[ops],[es]) ('ite',c,a,b) ('cast',width|None,signed,e)
  ('call',name,[args],[labels]) ('forall'|'exists',[vars],e) ('agg','sum'|'numof',lo,hi,var,e)
  ('valid',arr,lo,hi) ('true_by_hw',what) ('let',var,e,body)
"""
import re
from dataclasses import dataclass, field

C_WIDTH = {"char": 8, "short": 16, "int": 32, "long": 64, "_Bool": 1}
C_ALIASES = {"size_t": "unsigned long", "uint32_t": "unsigned int", "int32_t": "int",
             "uint8_t": "unsigned char", "int8_t": "signed char", "uint16_t": "unsigned short",
             "int16_t": "short", "uint64_t": "unsigned long", "int64_t": "long", "bool": "_Bool",
             "ssize_t": "long", "ptrdiff_t": "long"}
BASE_TYPE_WORDS = {"int", "unsigned", "signed", "char", "short", "long", "_Bool", "integer",
                   "boolean", "const", "volatile", "real", "float", "double", "void"}


class Unsupported(Exception):
    pass


@dataclass
class Param:
    name: str
    ctype: str
    width: int
    signed: bool
    is_array: bool = False
    size: int = 0
    is_pointer: bool = False
    from_pointer: bool = False
    fields: list = None        # struct element: [(name, width, signed)] in declaration order
    layout: list = None        # struct element: [(name, offset, width, signed)] matched to the RTL


@dataclass
class Behavior:
    name: str
    assumes: list = field(default_factory=list)     # (label, text, tree, defs)
    requires: list = field(default_factory=list)
    ensures: list = field(default_factory=list)
    assigns: list = field(default_factory=list)     # raw text items


@dataclass
class Function:
    name: str
    ret_width: int
    ret_signed: bool
    params: list
    default: Behavior = None
    behaviors: list = field(default_factory=list)
    defs: dict = field(default_factory=dict)
    typedefs: dict = field(default_factory=dict)
    ret_fields: list = None
    ret_layout: list = None


# ------------------------------------------------------------------ tokenizer / parser

TOKEN = re.compile(r"\s*(<==>|==>|->|<=|>=|==|!=|&&|\|\||\^\^|\.\.|<<|>>|\\\w+|[A-Za-z_]\w*|"
                   r"0[xX][0-9a-fA-F]+[uUlL]*|\d+[uUlL]*|'(?:\\.|[^'\\])'|"
                   r"[-+*/%<>!()\[\];,:?{}&|^~.=])")
CHAR_ESC = {"n": 10, "t": 9, "r": 13, "0": 0, "\\": 92, "'": 39, '"': 34}


def tokenize(s):
    s = re.sub(r"//[^\n]*", " ", s)
    pos, out, s = 0, [], s.rstrip()
    while pos < len(s):
        m = TOKEN.match(s, pos)
        if not m:
            raise Unsupported(f"cannot tokenize ACSL at: {s[pos:pos+30]!r}")
        out.append(m.group(1))
        pos = m.end()
    return out


class Parser:
    def __init__(self, toks, typenames=()):
        self.t, self.i = toks, 0
        self.typenames = set(typenames) | BASE_TYPE_WORDS | set(C_ALIASES)

    def peek(self, k=0):
        j = self.i + k
        return self.t[j] if j < len(self.t) else None

    def eat(self, tok=None):
        cur = self.peek()
        if cur is None or (tok is not None and cur != tok):
            raise Unsupported(f"ACSL parse: expected {tok!r}, got {cur!r}")
        self.i += 1
        return cur

    def labels(self):
        if self.peek() != "{":
            return []
        self.eat("{")
        out = []
        while self.peek() != "}":
            tok = self.eat()
            if tok != ",":
                out.append(tok)
        self.eat("}")
        return out

    def binder(self):
        """`integer i, integer j;` or `integer i, j;` -> [i, j] (consumes the ';')."""
        names, seg = [], []
        while self.peek() != ";":
            tok = self.eat()
            if tok == ",":
                names.append(seg[-1])
                seg = []
            else:
                seg.append(tok)
        names.append(seg[-1])
        self.eat(";")
        return names

    def expr(self):
        if self.peek() in ("\\forall", "\\exists"):
            q = self.eat()[1:]
            names = self.binder()
            return (q, names, self.expr())
        if self.peek() == "\\let":
            self.eat()
            v = self.eat()
            self.eat("=")
            e = self.expr()
            self.eat(";")
            return ("let", v, e, self.expr())
        c = self.equiv()
        if self.peek() == "?":
            self.eat()
            a = self.expr()
            self.eat(":")
            return ("ite", c, a, self.expr())
        return c

    def equiv(self):
        l = self.impl()
        if self.peek() == "<==>":
            self.eat()
            r = self.impl()
            return ("bin", "&&", ("bin", "==>", l, r), ("bin", "==>", r, l))
        return l

    def impl(self):
        l = self.lor()
        if self.peek() == "==>":
            self.eat()
            r = self.expr() if self.peek() in ("\\forall", "\\exists", "\\let") else self.impl()
            return ("bin", "==>", l, r)
        return l

    def _left(self, sub, ops):
        e = sub()
        while self.peek() in ops:
            op = self.eat()
            e = ("bin", op, e, sub())
        return e

    # ACSL / Frama-C precedence (checked against frama-c -print): && > ^^ > || > ==> > <==> > ?:
    def lor(self):
        return self._left(self.lxor, ("||",))

    def lxor(self):
        e = self.land()
        while self.peek() == "^^":
            self.eat()
            r = self.land()
            e = ("bin", "||", ("bin", "&&", e, ("un", "!", r)), ("bin", "&&", ("un", "!", e), r))
        return e

    def land(self):
        return self._left(self.bor, ("&&",))

    def bor(self):
        return self._left(self.bxor, ("|",))

    def bxor(self):
        return self._left(self.band, ("^",))

    def band(self):
        return self._left(self.cmp, ("&",))

    def cmp(self):
        es, ops = [self.shift()], []
        while self.peek() in ("<", "<=", ">", ">=", "==", "!="):
            ops.append(self.eat())
            es.append(self.shift())
        return es[0] if not ops else ("cmp", ops, es)

    def shift(self):
        return self._left(self.add, ("<<", ">>"))

    def add(self):
        return self._left(self.mul, ("+", "-"))

    def mul(self):
        return self._left(self.unary, ("*", "/", "%"))

    def is_cast(self):
        if self.peek() != "(":
            return False
        j = 1
        if self.peek(j) not in self.typenames or self.peek(j) in ("\\forall",):
            return False
        while self.peek(j) in self.typenames or self.peek(j) == "*":
            j += 1
        return self.peek(j) == ")"

    def unary(self):
        if self.peek() in ("-", "!", "~"):
            op = self.eat()
            return ("un", op, self.unary())
        if self.peek() == "+":
            self.eat()
            return self.unary()
        if self.peek() == "*":
            self.eat()
            return deref(self.unary())
        if self.peek() == "&":
            raise Unsupported("address-of (&) in contract")
        if self.is_cast():
            self.eat("(")
            words = []
            while self.peek() != ")":
                words.append(self.eat())
            self.eat(")")
            return ("cast_words", words, self.unary())
        return self.postfix()

    def postfix(self):
        e = self.primary()
        while self.peek() in ("[", ".", "->"):
            if self.peek() in (".", "->"):
                op = self.eat()
                fname = self.eat()
                base = deref(e) if op == "->" else e
                if base[0] == "old" and base[1][0] in ("var", "result"):
                    base = base[1] if base[1][0] == "var" else base
                e = ("field", base, fname)
                continue
            self.eat("[")
            idx = self.expr()
            if self.peek() == "..":
                raise Unsupported("array range a[lo..hi] in expression")
            self.eat("]")
            if e[0] == "old" and e[1][0] == "var":     # \old(p)[i]: pointer value is unchanged
                e = e[1]
            if e[0] != "var":
                raise Unsupported("indexing a non-variable")
            e = ("idx", e[1], idx)
        return e

    def args(self):
        self.eat("(")
        out = []
        if self.peek() != ")":
            out.append(self.expr())
            while self.peek() == ",":
                self.eat()
                out.append(self.expr())
        self.eat(")")
        return out

    def primary(self):
        if self.peek() in ("\\forall", "\\exists", "\\let"):
            return self.expr()
        t = self.eat()
        if re.match(r"\d", t):
            return ("int", c_int(t))
        if t.startswith("'"):
            body = t[1:-1]
            return ("int", CHAR_ESC.get(body[1], ord(body[1])) if body.startswith("\\") else ord(body))
        if t == "\\result":
            return ("result",)
        if t in ("\\true", "\\false"):
            return ("bool", t == "\\true")
        if t == "\\old":
            a = self.args()
            return ("old", a[0])
        if t == "\\at":
            self.eat("(")
            e = self.expr()
            self.eat(",")
            lab = self.eat()
            self.eat(")")
            return ("at", e, lab)
        if t in ("\\valid_read", "\\valid"):
            self.eat("(")
            e = self.add()
            if self.peek() == "..":
                raise Unsupported("unsupported \\valid form")
            self.eat(")")
            return valid_of(e)
        if t in ("\\separated", "\\initialized", "\\dangling", "\\freeable"):
            self.args()
            return ("true_by_hw", t)
        if t in ("\\sum", "\\numof"):
            self.eat("(")
            lo = self.expr()
            self.eat(",")
            hi = self.expr()
            self.eat(",")
            self.eat("\\lambda")
            v = self.binder()
            body = self.expr()
            self.eat(")")
            return ("agg", t[1:], lo, hi, v[0], body)
        if t in ("\\abs", "\\min", "\\max"):
            a = self.args()
            if len(a) != (1 if t == "\\abs" else 2):
                raise Unsupported(f"{t} with {len(a)} arguments")
            return ("call", t, a, [])
        if t == "(":
            if self.peek(1) == ".." or (self.peek() and self.peek(1) == ".."):
                pass
            e = self.expr()
            if self.peek() == "..":                     # a range (lo .. hi): returned to \valid
                self.eat()
                hi = self.expr()
                self.eat(")")
                return ("range", e, hi)
            self.eat(")")
            return e
        if t.startswith("\\"):
            raise Unsupported(f"ACSL builtin {t}")
        if re.match(r"[A-Za-z_]\w*$", t):
            labs = self.labels()
            if self.peek() == "(":
                return ("call", t, self.args(), labs)
            return ("var", t)
        raise Unsupported(f"unexpected token {t!r}")


def deref(e):
    """*p -> p[0];  *(p + e) -> p[e];  *(p - e) -> p[-e]"""
    if e[0] == "old" and e[1][0] == "var":
        e = e[1]
    if e[0] == "var":
        return ("idx", e[1], ("int", 0))
    if e[0] == "bin" and e[1] in ("+", "-"):
        l, r = e[2], e[3]
        if l[0] == "old" and l[1][0] == "var":
            l = l[1]
        if l[0] == "var":
            return ("idx", l[1], r if e[1] == "+" else ("un", "-", r))
        if e[1] == "+" and r[0] == "var":
            return ("idx", r[1], l)
    raise Unsupported("dereference of a complex pointer expression")


def valid_of(e):
    """\\valid(p) / \\valid(p + (lo .. hi)) / \\valid(p + i)."""
    if e[0] == "old" and e[1][0] == "var":
        e = e[1]
    if e[0] == "var":
        return ("valid", e[1], ("int", 0), ("int", 0))
    if e[0] == "bin" and e[1] == "+" and e[2][0] == "var":
        r = e[3]
        if r[0] == "range":
            return ("valid", e[2][1], r[1], r[2])
        return ("valid", e[2][1], r, r)
    raise Unsupported("unsupported \\valid argument")


def c_int(text):
    t = re.sub(r"[uUlL]+$", "", text.strip())
    if re.fullmatch(r"0[0-7]+", t):
        return int(t, 8)
    try:
        return int(t, 0)
    except ValueError:
        raise Unsupported(f"non-constant array size {text!r}")


def c_globals(src):
    """Names of file-scope variables (contracts that mention them depend on global state)."""
    code = _strip_comments(src)
    depth, out, buf = 0, set(), []
    for ch in code:
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
        elif depth == 0:
            buf.append(ch)
    for stmt in "".join(buf).split(";"):
        stmt = stmt.strip()
        if not stmt or stmt.startswith(("typedef", "#", "extern")) or "(" in stmt.split("=")[0]:
            continue
        m = re.match(r"[\w\s\*]+?\b(\w+)\s*(\[[^\]]*\])*\s*(=.*)?$", stmt, re.S)
        if m:
            out.add(m.group(1))
    return out


def parse_expr(text, typenames=()):
    p = Parser(tokenize(text), typenames)
    e = p.expr()
    if p.peek() is not None:
        raise Unsupported(f"trailing tokens {p.t[p.i:p.i+5]}")
    return e


# ------------------------------------------------------------------ C types

def _strip_comments(src):
    return re.sub(r"/\*.*?\*/|//[^\n]*", lambda m: " " * len(m.group(0)), src, flags=re.S)


def c_typedefs(src):
    code = _strip_comments(src)
    return {n: t.strip() for t, n in re.findall(r"\btypedef\s+([\w\s\*]+?)\s*\b(\w+)\s*;", code)}


def resolve_type(t, typedefs):
    words = t.replace("*", " * ").split()
    if {"struct", "union", "enum"} & set(words):
        return words
    for _ in range(10):
        if {"struct", "union", "enum"} & set(words):
            break
        new = []
        for w in words:
            rep = typedefs.get(w) or C_ALIASES.get(w)
            new.extend(rep.replace("*", " * ").split() if rep else [w])
        if new == words:
            break
        words = new
    return words


def c_structs(src):
    """{tag: [(field, type-text)]} for struct definitions (Frama-C prints `struct T { ... };`)."""
    code = _strip_comments(src)
    out = {}
    for m in re.finditer(r"\bstruct\s+(\w+)\s*\{([^{}]*)\}\s*;", code):
        fields = []
        for fm in re.finditer(r"([\w\s\*]+?)\s*\b(\w+)\s*(\[[^\]]*\])?\s*;", m.group(2)):
            fields.append((fm.group(2), fm.group(1).strip() + (" []" if fm.group(3) else "")))
        out[m.group(1)] = fields
    return out


def struct_fields(t, typedefs, structs):
    """Fields [(name, width, signed)] if the type is a struct of integer fields, else None."""
    words = resolve_type(t, typedefs)
    if "struct" not in words:
        return None
    tag = words[words.index("struct") + 1] if words.index("struct") + 1 < len(words) else None
    if tag not in structs:
        raise Unsupported(f"struct {tag} without a definition")
    out = []
    for name, ft in structs[tag]:
        if "[]" in ft or "*" in ft:
            raise Unsupported(f"struct {tag}: array/pointer member {name}")
        if "struct" in resolve_type(ft, typedefs):
            raise Unsupported(f"struct {tag}: nested struct member {name}")
        w, sg, _ = c_type(ft, typedefs)
        out.append((name, w, sg))
    return out


def c_type(t, typedefs):
    """-> (width, signed, is_pointer)"""
    words = resolve_type(t, typedefs)
    ptr = "*" in words
    words = [w for w in words if w not in ("const", "volatile", "*", "restrict", "register")]
    if {"float", "double"} & set(words):
        raise Unsupported("floating-point type")
    if "struct" in words or "union" in words or "enum" in words:
        raise Unsupported(f"C type {' '.join(words)!r}")
    base = [w for w in words if w in C_WIDTH]
    if not base and not ({"unsigned", "signed"} & set(words)):
        raise Unsupported(f"C type {t.strip()!r}")
    signed = "unsigned" not in words
    width = 64 if "long" in words else (C_WIDTH[base[0]] if base else 32)
    if base == ["_Bool"]:
        signed = False
    return width, signed, ptr


def cast_type(words, typedefs):
    """('cast', width|None, signed, e) parameters for a cast's type words."""
    ws = resolve_type(" ".join(words), typedefs)
    if "integer" in ws or "boolean" in ws:
        return None, True
    if "*" in ws:
        raise Unsupported("pointer cast")
    w, s, _ = c_type(" ".join(ws), {})
    return w, s


def resolve_casts(e, typedefs):
    if not isinstance(e, tuple):
        return e
    if e[0] == "cast_words":
        w, s = cast_type(e[1], typedefs)
        inner = resolve_casts(e[2], typedefs)
        return inner if w is None else ("cast", w, s, inner)
    return tuple(resolve_casts(x, typedefs) if isinstance(x, tuple)
                 else [resolve_casts(y, typedefs) if isinstance(y, tuple) else y for y in x]
                 if isinstance(x, list) else x for x in e)


# ------------------------------------------------------------------ contracts

def parse_contract(text, typenames):
    toks = tokenize(re.sub(r"^\s*@", "", text, flags=re.M))
    p = Parser(toks, typenames)
    default = Behavior("default")
    cur, behaviors = default, []
    while p.peek() is not None:
        kw = p.eat()
        if kw == "behavior":
            cur = Behavior(p.eat())
            p.eat(":")
            behaviors.append(cur)
            continue
        if kw in ("complete", "disjoint", "terminates", "exits", "decreases", "allocates", "frees",
                  "check", "admit"):
            if kw in ("check", "admit") and p.peek() in ("requires", "ensures"):
                continue                          # 'check ensures' == ensures for our purpose
            while p.eat() != ";":
                pass
            continue
        if kw not in ("requires", "ensures", "assigns", "assumes"):
            raise Unsupported(f"contract clause {kw!r}")
        label = ""
        if re.match(r"[A-Za-z_]\w*$", p.peek() or "") and p.peek(1) == ":":
            label = p.eat()
            p.eat(":")
        start = p.i
        if kw == "assigns":
            items, buf, d = [], [], 0
            while not (p.peek() == ";" and d == 0):
                tok = p.eat()
                if tok == "\\from":
                    while not (p.peek() == ";" and d == 0):
                        p.eat()
                    break
                if tok == "," and d == 0:
                    items.append(buf)
                    buf = []
                    continue
                d += tok in ("(", "[") and 1 or 0
                d -= tok in (")", "]") and 1 or 0
                buf.append(tok)
            items.append(buf)
            cur.assigns.extend(" ".join(b) for b in items if b)
            p.eat(";")
            continue
        tree = p.expr()
        txt = " ".join(toks[start:p.i])
        p.eat(";")
        {"requires": cur.requires, "ensures": cur.ensures, "assumes": cur.assumes}[kw].append(
            (label, txt, tree))
    return default, behaviors


FUNC_RE = re.compile(r"/\*@((?:(?!\*/).)*?)\*/\s*((?:[\w\*]+\s+)+?\**)\s*(\w+)\s*\(([^)]*)\)\s*\{", re.S)


def find_functions(src):
    """[(name, contract, rtype, params)] for definitions preceded by an ACSL contract."""
    out = []
    for m in FUNC_RE.finditer(src):
        contract, rtype, name, plist = m.groups()
        if re.search(r"\b(predicate|lemma|axiomatic|loop|logic)\b", contract):
            continue
        out.append((name, contract, rtype, plist))
    return out


def parse_function(src, top, array_size=4, single_size=1):
    """Parse `top` from (Frama-C-normalized) C source.
    Pointer parameters become arrays: `array_size` cells when the contract asks for a valid
    range, `single_size` (1) when it asks for a single valid object."""
    from .logic import logic_definitions
    typedefs = c_typedefs(src)
    match = [f for f in find_functions(src) if f[0] == top]
    if not match:
        raise Unsupported(f"no ACSL contract before function {top}")
    name, contract, rtype, plist = match[-1]
    typenames = set(typedefs)
    defs = logic_definitions(src, typenames)
    for dl in defs.values():
        for d in dl:
            if d.body is not None:
                d.body = resolve_casts(d.body, typedefs)
    default, behaviors = parse_contract(contract, typenames)
    for b in [default] + behaviors:
        for lst in (b.requires, b.ensures, b.assumes):
            lst[:] = [(l, t, resolve_casts(tr, typedefs)) for l, t, tr in lst]

    structs = c_structs(src)

    def typed(t):
        """(width, signed, ptr, fields) for a parameter/return type; structs get width = packed sum."""
        fl = struct_fields(t, typedefs, structs)
        if fl is not None:
            return sum(f[1] for f in fl), False, "*" in resolve_type(t, typedefs), fl
        w, sg, ptr = c_type(t, typedefs)
        return w, sg, ptr, None

    ret_fields = None
    if rtype.split() == ["void"]:
        rw, rs = 0, False
    else:
        rw, rs, rptr, ret_fields = typed(rtype)
        if rptr:
            raise Unsupported("pointer return value")
    params = []
    for p in [x.strip() for x in plist.split(",") if x.strip() and x.strip() != "void"]:
        am = re.match(r"(.*?)(\w+)\s*\[\s*([^\]]*)\]$", p)
        if am:
            w, sg, _, fl = typed(am.group(1))
            size = am.group(3).strip()
            if size:
                params.append(Param(am.group(2), am.group(1).strip(), w, sg, True, c_int(size), fields=fl))
            else:
                params.append(Param(am.group(2), am.group(1).strip() + " *", w, sg, is_pointer=True, fields=fl))
            continue
        pm = re.match(r"(.*?)(\w+)$", p)
        w, sg, ptr, fl = typed(pm.group(1))
        params.append(Param(pm.group(2), pm.group(1).strip(), w, sg, is_pointer=ptr, fields=fl))

    fn = Function(name, rw, rs, params, default, behaviors, defs, typedefs)
    fn.ret_fields = ret_fields
    fn.structs = structs
    fn.globals = c_globals(src)
    ranged, single = set(), set()
    from .logic import expand
    for b in [default] + behaviors:
        for _, _, t in b.requires:
            try:
                t = expand(t, defs, 2, frozenset(p.name for p in params if p.is_pointer or p.is_array))
            except Unsupported:
                pass
            for x in walk(t):
                if x[0] == "valid":
                    (single if x[2:] == (("int", 0), ("int", 0)) else ranged).add(x[1])
    for p in params:
        if p.is_pointer:
            p.is_array, p.is_pointer, p.from_pointer = True, False, True
            p.size = array_size if (p.name in ranged or p.name not in single) else single_size
    return fn


def walk(e):
    if isinstance(e, tuple):
        yield e
        for x in e[1:]:
            if isinstance(x, tuple):
                yield from walk(x)
            elif isinstance(x, list):
                for y in x:
                    yield from walk(y)
