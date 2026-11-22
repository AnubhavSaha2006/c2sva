/* rule: single struct through a pointer (packed ap_ovld word), field-level assigns */
struct Point { int x; short y; char z; };
typedef struct Point Point;
/*@ requires \valid(p);
    requires -1000 <= p->x <= 1000;
    assigns p->x, p->y;
    ensures p->x == \old(p->x) + 1;
    ensures p->y == 0; */
void bump(Point *p)
{
  p->x = p->x + 1;
  p->y = 0;
}
