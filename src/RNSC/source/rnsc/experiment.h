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

class Experiment
{
 private:
 public:


	//CONTENTS OF experiment.cpp
	Experiment();
	~Experiment();
	void ClusteringSAlloc(Graph& graph);

	void RunExperiment(Graph& graph);
	void InitExperiment(Graph& graph);
	void RunNaiveScheme(Graph& graph);
	void RunScaledScheme(Graph& graph);
	void MakeANaiveMove(Graph& graph);
	void MakeAScaledMove(Graph& graph);
	int MakeADestroyMove(Graph& graph, int cluster);
	void MakeAShuffleMove(Graph& graph);
	

	//END CONTENTS OF experiment.cpp

	//Parameters
	int P_NaiveStoppingTol;//naive stopping tolerance
	int P_ScaledStoppingTol;//scaled experiment length
	int P_ShuffleFreq;//shuffling diversification frequency
	int P_ShuffleLength;//shuffling diversification length
	int P_DestroyFreq;//destructive diversification frequency
	bool P_SkipNaive;//skip the naive scheme? useful when reading good clusterings.
	bool P_WriteNaive;//write the naive costs.
	bool P_WriteScaled;//write the scaled costs.

	int BestNaiveCost;//best naive cost achieved
	double BestScaledCost;//best scaled cost achieved
	int NaiveLength;//number of naive moves.
	int ScaledLength;//number of scaled moves.


	//Filenames for dumping cost curves.
	const char * NMatlabFN;//naive
	const char * SMatlabFN;//scaled
	

	//Data
	int * ClusteringS;//Clustering C_s, the best scaled clustering of the experiment.


};
