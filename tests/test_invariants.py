"""Unit tests of the A1 candidate generator (no HLS / solver needed)."""
from c2sva import invariants as I

NORM = """
typedef unsigned int size_type;
size_type find(int const *a, size_type n, int v)
{
  size_type __retres;
  {
    size_type i = 0u;
    /*@ loop invariant bound: 0 <= i <= n;
        loop invariant \\forall integer k; 0 <= k < i ==> *(a + k) != v;
        loop assigns i, a[0 .. n - 1];
        loop variant n - i;
    */
    while (i < n) { i ++; }
  }
  __retres = n;
  return __retres;
}
"""


def test_loop_annotations():
    loops, body = I.loop_annotations(NORM, "find", {"size_type"}, {"size_type": "unsigned int"})
    assert len(loops) == 1
    lp = loops[0]
    assert [lbl for lbl, _, _ in lp.invariants] == ["bound", ""]
    assert lp.assigns == ["i", "a [ 0 .. n - 1 ]"]
    assert lp.variant[0] == "n - i"
    assert I.local_types(body, {"i"}, {"size_type": "unsigned int"}) == {"i": (32, False)}


def test_assigned_params():
    assert I.assigned_params("n--; x = n; m += 2; k == 3;", ["n", "m", "k", "x"]) == {"n", "m", "x"}


def test_var_signals_vitis_names():
    sigs = {"i_fu_36": 32, "ap_sig_allocacmp_i_1": 32, "i_1_reg_125": 32, "icmp_ln12_reg_131": 1,
            "grp_x_fu_46.i_fu_42": 3, "ii_fu_3": 32, "ap_phi_mux_i_2_phi_fu_68_p4": 32, "add_ln12_fu_93_p2": 32}
    got = I.var_signals(sigs, "i")
    assert set(got) == {"i_fu_36", "ap_sig_allocacmp_i_1", "i_1_reg_125", "grp_x_fu_46.i_fu_42",
                        "ap_phi_mux_i_2_phi_fu_68_p4"}
    assert got[0] == "ap_sig_allocacmp_i_1"            # current-iteration value first, top level first
    assert got[-1] == "grp_x_fu_46.i_fu_42"


def test_fsm_states_by_scope():
    sigs = {"ap_CS_fsm_state1": 1, "ap_CS_fsm_state10": 1, "ap_CS_fsm_state2": 1,
            "grp.ap_CS_fsm_pp0_stage0": 1, "ap_CS_fsm": 3}
    st = I.fsm_states(sigs)
    assert st[""] == ["ap_CS_fsm_state1", "ap_CS_fsm_state2", "ap_CS_fsm_state10"]
    assert st["grp"] == ["grp.ap_CS_fsm_pp0_stage0"]


def test_llm_domain_all_premises_and_call_renaming():
    from c2sva import llm
    lem = "/*@ lemma f_eq: \\forall integer b, integer a; a > 0 ==> b >= 0 ==> f(a, b) == f_c2sva(a, b); */"
    assert llm.domain_of(lem, [("integer", "x"), ("integer", "y")], "f") == "(x > 0) && (y >= 0)"
    assert llm.domain_of("/*@ lemma f_eq: \\true; */", [("integer", "x")], "f") == "\\true"
