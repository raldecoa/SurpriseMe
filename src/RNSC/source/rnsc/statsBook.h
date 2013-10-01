/******************************************************************************
*******************************************************************************
*******************************************************************************
// All files written by Andrew D. King, 2004, except some of the linked list
// classes, but I honestly can't remember where I got them.  Feel free to
// distribute this in its unaltered form.  The author takes no responsibility
// for loss of sanity resulting from poorly written or documented code.
//
// Compiled under Fedora Core 2 Linux using the GNU C++ compiler verision 2.96.
// Probably won't work with newer or older versions or under different
// operating systems.
//
// As of September, 2004, the author will be reachable at the Department of
// Computer Science, McGill University, Montreal, Quebec, Canada.  The email
// address andrew.king@utoronto.ca will probably be in service until at least
// mid-2005.  Feel free to send any comments.
*******************************************************************************
*******************************************************************************
******************************************************************************/

class StatsBook
{
 private:
 public:
   StatsBook();
   ~StatsBook();
   void SecondConstructor(long num, long numExper);

   void WriteStats(const char* data, const char* vars, const char* minima,
			      const long params[6]);

   void Update(double costChange);
   void GetMinimumStats(long numExper);
   long NumMoves;
   double* Minima;
   double* TotalChange;
   double* AvgChange;
   long MoveNum;
   long ExperCount;

};
