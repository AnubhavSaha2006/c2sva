/*
 * Task 458 Natural Language:
 * Write an ACSL-annotated C function that compares two CatBPos structures for ordering. The
 * comparison first checks if both objects have valid HDOP times and compares them using
 * IntCompare. If equal, it compares the BPos times. Additional rules handle specific cases:
 * departure vs vacation returns 1, vacation vs arrival returns 1, and departure vs arrival at the
 * same station returns 1. The IntCompare helper function returns -1, 0, or 1 for less than, equal
 * to, or greater than comparisons respectively.
 */


struct CatBPos{
  int getHdopBPosGetTime;
  int getBPosGetTime;
  int getBPosGetStat;
  int getBPosIsDeparture;
  int getBPosIsVacation;
  int getBPosIsArrival;
  int getHdopBPosGetTimeIsNotVoid;
};

/*@ assigns \result \from x,y;
  @ ensures x < y ==> \result == -1;
  @ ensures x > y ==> \result == 1;
  @ ensures x == y ==> \result == 0;
 */
int IntCompare(int x, int y){
  if (x < y){
    return -1;
  }
  if(x > y){
    return 1;
  }

  return 0;
}

/*@ assigns \result \from o1,o2; 
 */
int compare(struct CatBPos o1, struct CatBPos o2) {
  int lCompare;

  if (o1.getHdopBPosGetTimeIsNotVoid && o2.getHdopBPosGetTimeIsNotVoid) {
    lCompare = IntCompare(o1.getHdopBPosGetTime, o2.getHdopBPosGetTime);
    if (lCompare != 0) {
      return lCompare;
    }
  }

  lCompare = IntCompare(o1.getBPosGetTime, o2.getBPosGetTime);
  if (lCompare != 0) {
    return lCompare;
  }

  if (o1.getBPosIsDeparture && o2.getBPosIsVacation) {
    return 1;
  } else if (o1.getBPosIsVacation && o2.getBPosIsArrival) {
    return 1;
  }

  // Ankunft und Abfahrt für denselben Bahnhof sollen in der richtigen Reihenfolge sein
  if (o1.getBPosIsDeparture && o2.getBPosIsArrival && (o1.getBPosGetStat == o2.getBPosGetStat)) {
    return 1;
  }

  return 0;
}
