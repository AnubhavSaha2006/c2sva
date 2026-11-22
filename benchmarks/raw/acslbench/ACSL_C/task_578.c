/*
 * Task 578 Natural Language:
 * Write an ACSL-annotated C function that compares two NzbFile structures based on file extensions
 * and content. The function prioritizes files with specific extensions (".nfo", ".sfv", ".srr",
 * ".nzb", ".srt") in a fixed order, then compares file names if both files have non-null FileName
 * pointers, and finally compares Subject fields if both FileName pointers are null. The function
 * requires valid read access to FileName pointers when they are non-null.
 */


struct NzbFile{
  int* FileName;
  int getFileName_toLowerCase_endsWith[5];
  int Subject;
};

/*@ type invariant NzbFile_is_bool(struct NzbFile s) = 
  \forall integer k; 0 <= k < 5
  ==> (s.getFileName_toLowerCase_endsWith[k] == 0 || s.getFileName_toLowerCase_endsWith[k] == 1);*/

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

/*@ requires (o1.FileName != 0) ==> \valid_read(o1.FileName);
    requires (o2.FileName != 0) ==> \valid_read(o2.FileName);
    assigns \result \from o1.FileName,o1.getFileName_toLowerCase_endsWith[0..4],o1.Subject,o2.FileName,o2.getFileName_toLowerCase_endsWith[0..4],o2.Subject;
 */
int compare(struct NzbFile o1, struct NzbFile o2){
  if ((o1.FileName != 0) && (o2.FileName != 0)){
    if (o1.getFileName_toLowerCase_endsWith[0]) // ".nfo"
      return -1000;
    else if (o2.getFileName_toLowerCase_endsWith[0]) // ".nfo"
      return 1000;
    else if (o1.getFileName_toLowerCase_endsWith[1]) // ".sfv"
      return -999;
    else if (o2.getFileName_toLowerCase_endsWith[1]) // ".sfv"
      return 1001;
    else if (o1.getFileName_toLowerCase_endsWith[2]) // ".srr"
      return -998;
    else if (o2.getFileName_toLowerCase_endsWith[2]) // ".srr"
      return 1002;
    else if (o1.getFileName_toLowerCase_endsWith[3]) // ".nzb"
      return -997;
    else if (o2.getFileName_toLowerCase_endsWith[3]) // ".nzb"
      return 1003;
    else if (o1.getFileName_toLowerCase_endsWith[4]) //".srt"
      return -996;
    else if (o2.getFileName_toLowerCase_endsWith[4]) // ".srt"
      return 1004;
    else
      return IntCompare(*o1.FileName, *o2.FileName);
  }
  else if ((o1.FileName != 0) && (o2.FileName == 0))
    {
      return -995;
    }
  else if ((o1.FileName == 0) && (o2.FileName != 0))
    {
      return 1005;
    }
  else
    {
      return IntCompare(o1.Subject, o2.Subject);
    }

}