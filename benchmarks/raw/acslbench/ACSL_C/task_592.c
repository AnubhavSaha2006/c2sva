/*
 * Task 592 Natural Language:
 * Write an ACSL-annotated C function that compares two Contact structures based on their
 * firstName, lastName, and emails fields. The comparison follows a hierarchical order: first
 * compare firstName, then lastName if firstNames are equal, then emails if both firstNames and
 * lastNames are equal. The function returns -1, 0, or 1 following standard comparison semantics.
 * If any required field is zero, the comparison proceeds to the next available field, returning 0
 * if no fields are available for comparison.
 */


struct Contact{
  int getFirstName;
  int getLastName;
  int getEmails;
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
int compare(struct Contact o1, struct Contact o2) {
  int compareFirstName = 0;
  if ((o1.getFirstName != 0) && (o2.getFirstName != 0)) {
    compareFirstName = IntCompare(o1.getFirstName, o2.getFirstName);

    if (compareFirstName == 0) {
      int compareLastName = 0;
      if ((o1.getLastName != 0) && (o2.getLastName != 0)) {
	compareLastName = IntCompare(o1.getLastName, o2.getLastName);

	if (compareLastName == 0) {
	  int compareEmail = 0;
	  if ((o1.getEmails != 0) && (o2.getEmails != 0)) {
	    compareEmail = IntCompare(o1.getEmails, o2.getEmails);
	    return compareEmail;
	  } else {
	    return 0;
	  }
	} else {
	  return compareLastName;
	}
      } else {
	int compareEmail = 0;
	if ((o1.getEmails != 0) && (o2.getEmails != 0)) {
	  compareEmail = IntCompare(o1.getEmails, o2.getEmails);
	  return compareEmail;
	} else {
	  return 0;
	}
      }
    } else {
      return compareFirstName;
    }
  } else {
    int compareLastName = 0;
    if ((o1.getLastName != 0) && (o2.getLastName != 0)) {
      compareLastName = IntCompare(o1.getLastName, o2.getLastName);

      if (compareLastName == 0) {
	int compareEmail = 0;
	if ((o1.getEmails != 0) && (o2.getEmails != 0)) {
	  compareEmail = IntCompare(o1.getEmails, o2.getEmails);
	  return compareEmail;
	} else {
	  return 0;
	}
      } else {
	return compareLastName;
      }
    } else {
      int compareEmail = 0;
      if ((o1.getEmails != 0) && (o2.getEmails != 0)) {
	compareEmail = IntCompare(o1.getEmails, o2.getEmails);
	return compareEmail;
      } else {
	return 0;
      }
    }
  }
}
