"""Unit tests for the translation rules (run: PYTHONPATH=src pytest tests -q)."""
import re

import pytest

from c2sva import acsl, logic, mutate, translate
from c2sva.hls import Bank, Interface, Memory


# ---------------------------------------------------------------- parser

def test_chained_comparison():
    e = acsl.parse_expr("0 <= i < n")
    assert e == ("cmp", ["<=", "<"], [("int", 0), ("var", "i"), ("var", "n")])


def test_multi_variable_quantifier():
    e = acsl.parse_expr("\\forall integer i, integer j; 0 <= i < j < n ==> a[i] <= a[j]")
    assert e[0] == "forall" and e[1] == ["i", "j"]


def test_deref_forms():
    assert acsl.parse_expr("*(a + i)") == ("idx", "a", ("var", "i"))
    assert acsl.parse_expr("*p") == ("idx", "p", ("int", 0))
    assert acsl.parse_expr("*(\\old(a) + \\result)") == ("idx", "a", ("result",))


def test_valid_range():
    e = acsl.parse_expr("\\valid_read(a + (0 .. n - 1))")
    assert e[0] == "valid" and e[1] == "a"


def test_c_integer_literals():
    assert acsl.c_int("010") == 8 and acsl.c_int("0x1F") == 31 and acsl.c_int("7u") == 7


def test_equivalence_desugars_to_two_implications():
    e = acsl.parse_expr("p <==> q")
    assert e[0] == "bin" and e[1] == "&&"


# ---------------------------------------------------------------- logic layer

SRC = """
typedef int value_type;
/*@ predicate Equal{K,L}(value_type *a, integer m, integer n, value_type *b) =
      \\forall integer i; m <= i < n ==> \\at(a[i],K) == \\at(b[i],L); */
/*@ predicate Equal{K,L}(value_type *a, integer m, integer n, integer p) =
      Equal{K,L}(a, m, n, a + p); */
/*@ predicate AllEqual{L}(value_type *a, integer m, integer n) = \\forall integer i; m <= i < n ==> a[i] == a[m]; */
/*@ predicate AllEqual{L}(value_type *a, integer n, value_type v) = \\forall integer i; 0 <= i < n ==> a[i] == v; */
/*@ requires \\valid(a + (0 .. n-1)) && \\valid(b + (0 .. n-1));
    ensures Equal{Here,Here}(a, 0, n, b);
    ensures AllEqual(a, n, v); */
int f(value_type *a, value_type *b, int n, value_type v) { return 0; }
"""


def test_overload_by_pointer_kind_and_c_type():
    fn = acsl.parse_function(SRC, "f")
    tr = translate.Translator(fn, _itf(fn), 64)
    e1 = logic.expand(fn.default.ensures[0][2], fn.defs, 6, tr.ptrs, types=tr.types)
    assert any(x[0] == "idx" and x[1] == "b" for x in acsl.walk(e1))      # pointer overload chosen
    e2 = logic.expand(fn.default.ensures[1][2], fn.defs, 6, tr.ptrs, types=tr.types)
    assert any(x == ("var", "v") for x in acsl.walk(e2))                   # value_type overload chosen


def test_pointer_terms_offsets():
    t = logic.ptr_term(("bin", "+", ("old", ("var", "a")), ("var", "k")), {"a"})
    assert t == ("ptr", "a", ("var", "k"))


def test_recursion_is_cut_off_with_free_value():
    src = """/*@ logic integer S{L}(int *a, integer n) = n <= 0 ? 0 : S(a, n-1) + a[n-1]; */
/*@ requires \\valid(a + (0 .. n-1)); ensures \\result == S(a, n); */ int g(int *a, int n) { return 0; }"""
    fn = acsl.parse_function(src, "g")
    e = logic.expand(fn.default.ensures[0][2], fn.defs, 3, frozenset({"a"}))
    assert any(x[0] == "free" for x in acsl.walk(e))


# ---------------------------------------------------------------- layouts

@pytest.mark.parametrize("part,resh", [(None, None), (("cyclic", 2), None), (("block", 2), None),
                                       (None, ("cyclic", 2)), (None, ("block", 2)), (("cyclic", 4), None)])
def test_locate_and_index_are_inverse(part, resh):
    m = Memory("a", 8, 32, True, partition=part, reshape=resh)
    for i in range(8):
        b, w, s = m.locate(i)
        expr = m.index_expr(b, str(w), s)
        assert eval(expr) == i


# ---------------------------------------------------------------- soundness polarity

def _itf(fn):
    itf = Interface("t", "m", dict(clk="c", rst="r", start="s", done="d", rst_active_low=False))
    itf.ret, itf.ret_width = "ret", 32
    for p in fn.params:
        if p.is_array:
            itf.memories[p.name] = Memory(p.name, p.size, p.width, p.signed,
                                          banks=[Bank({"address0": "x", "ce0": "y", "q0": "z"}, 2, p.width)])
        else:
            itf.scalars[p.name] = p.name
    return itf


def _fn(contract):
    return acsl.parse_function(f"/*@ {contract} */ int h(const int *a, int n) {{ return 0; }}", "h")


def test_forall_in_assertion_is_symbolic():
    fn = _fn("requires \\valid_read(a + (0 .. n-1)); ensures \\forall integer i; 0 <= i < n ==> a[i] >= 0;")
    props, skipped, tr = translate.build_properties(fn, _itf(fn), None, 64, 6)
    post = [p for p in props if p.kind == "post"][0]
    assert any("symbolic" in n for n in post.notes) and tr.anyconst


def test_exists_in_assumption_is_rejected():
    fn = _fn("requires \\valid_read(a + (0 .. n-1)); requires \\exists integer i; 0 <= i < n && a[i] == 0;")
    props, skipped, tr = translate.build_properties(fn, _itf(fn), None, 64, 6)
    assert any("existential" in s["reason"] for s in skipped)


def test_forall_in_assumption_is_enumerated():
    fn = _fn("requires \\valid_read(a + (0 .. n-1)); requires \\forall integer i; 0 <= i < n ==> a[i] > 0;")
    props, skipped, tr = translate.build_properties(fn, _itf(fn), None, 64, 6)
    ass = [p for p in props if p.kind == "assume" and p.origin == "requires"][0]
    assert any("enumerated" in n for n in ass.notes)


def test_out_of_range_cell_is_free_not_zero():
    fn = _fn("requires \\valid_read(a + (0 .. n-1)); ensures \\result == a[7];")
    props, skipped, tr = translate.build_properties(fn, _itf(fn), None, 64, 6)
    post = [p for p in props if p.kind == "post"][0]
    assert "a_x7" in post.cond and ("a_x7", 64) in tr.anyconst


def test_out_of_range_cell_same_before_and_after():
    # the RTL cannot write a cell outside the model: \old(a[7]) and a[7] are one solver-chosen value
    fn = _fn("requires \\valid_read(a + (0 .. n-1)); ensures a[7] == \\old(a[7]);")
    props, skipped, tr = translate.build_properties(fn, _itf(fn), None, 64, 6)
    post = [p for p in props if p.kind == "post" and p.origin == "ensures"][0]
    assert post.cond == "((a_x7 == a_x7))"


def test_nested_valid_is_range_fit():
    # \valid under a disjunction: exact meaning for an N-cell array, not 'true'
    fn = _fn("requires n == 0 || (n > 0 && \\valid_read(a + (0 .. n-1))); ensures \\result == 0;")
    props, skipped, tr = translate.build_properties(fn, _itf(fn), None, 64, 6)
    pre = [p for p in props if p.kind == "assume"][0]
    assert "< 64'sd4" in pre.cond and "1'b1" not in pre.cond


# ---------------------------------------------------------------- mutation

def test_mutation_skips_unary_minus_and_declarators():
    src = "int f(int a){ int b[4]; return a < 0 ? -a : a; }"
    kinds = [(s[0], s[2]) for s in mutate.sites(src, "f")]
    assert ("AOR", "-") not in kinds and ("CRP", "4") not in kinds and ("ROR", "<") in kinds


def test_behavior_guard_is_negative_position_of_assertion():
    """assumes with \\exists guards a frame condition: !A || frame must translate (A at - in an assert)."""
    src = """/*@ requires \\valid(a + (0 .. n-1));
    behavior some: assumes \\exists integer i; 0 <= i < n && a[i] == 0; assigns a[0 .. n-1];
    behavior none: assumes \\forall integer i; 0 <= i < n ==> a[i] != 0; assigns \\nothing;
    complete behaviors; disjoint behaviors; */
int z(int *a, int n) { return 0; }"""
    fn = acsl.parse_function(src, "z")
    props, skipped, tr = translate.build_properties(fn, _itf(fn), None, 64, 6)
    assert not skipped and any(p.origin == "assigns" and p.behavior for p in props)


def test_distinct_symbolic_out_of_model_cells():
    # review finding: a[i] and a[j] for different symbolic i, j beyond the model shared one
    # symbol, so `a[i] == a[j]` was provable (assertion weaker than ACSL)
    fn = _fn("requires \\valid_read(a + (0 .. n-1)); ensures \\forall integer i, j; i >= 4 && j >= 4 ==> a[i] == a[j];")
    props, skipped, tr = translate.build_properties(fn, _itf(fn), None, 64, 6)
    post = [p for p in props if p.origin == "ensures"][0]
    assert len(set(re.findall(r"a_xd\w+", post.cond))) == 2


def test_acsl_precedence_xor_between_and_or():
    # Frama-C: && > ^^ > || (a || b ^^ c == a || (b ^^ c); a && b ^^ c == (a && b) ^^ c)
    from c2sva.acsl import parse_expr
    e = parse_expr("a || b ^^ c")
    assert e[1] == "||" and e[2] == ("var", "a")
    x = parse_expr("a && b ^^ c")
    assert x[1] == "||" and x[2][2][1] == "&&"
