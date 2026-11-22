/*
 * Task 231 Natural Language:
 * Write an ACSL-annotated C function that increments the `hd` field of all nodes in a linked list
 * starting from pointer p. The function requires that all nodes reachable from p are valid until
 * reaching a null pointer. The function modifies the `hd` values of all reachable nodes in the
 * list.
 */

struct list {

	int hd;

	struct list *next;

};





/*@ inductive reachable{L}(struct list *root, struct list *to) {

   @  case empty: \forall struct list *l; reachable(l,l);

   @  case non_empty: \forall struct list *l1, *l2; \valid(l1) && reachable(l1->next, l2) ==> reachable(l1, l2);

   @ }

 */

/*@ requires reachable(p, \null);

    assigns { q->hd | struct list *q; reachable(p,q)};

 */

void incr_list(struct list *p)

{

	while (p) { p->hd++; p = p->next;}

}



