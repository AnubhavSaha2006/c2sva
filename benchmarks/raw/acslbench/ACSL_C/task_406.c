/*
 * Task 406 Natural Language:
 * Write an ACSL-annotated C function that updates a cricket scorecard structure. The function
 * takes a pointer to a scorecard and increments the total runs, wickets, and completed overs by
 * the specified amounts. It requires the input values to be within valid cricket ranges (runs:
 * 0-999, wickets: 0-10, completed overs: 0-20).
 */

#include<stdio.h>

struct scorecard{
    int total_runs;
    int wickets;
    int total_overs;
    int over_complete;
    int target_score;
};
/*@
requires 0<=runs<1000 && 0<=wic<=10 && 0<=comp<=20;
ensures a->total_runs==\old(a->total_runs)+runs;
ensures a->wickets==\old(a->wickets)+wic;
ensures a->over_complete==\old(a->over_complete)+comp;
*/
void update_scorecard(struct scorecard *a,int runs, int wic,int comp){
    a->total_runs+=runs;
    a->wickets+=wic;
    a->over_complete+=comp;
}
