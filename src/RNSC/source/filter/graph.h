//requires inclusion of linkedList.h and cycList.h
#include <vector>
#include "linkedList.h"

void PrintWhichCluster(const long * whichCluster, const long size, const long cost, const long numClust);
void PrintScaledWhichCluster(const long * whichCluster, const long size, const double cost, const long numClust);

class Graph
{
 private:
 public:
	Graph();
	~Graph();
	long ReadGraph(const char *fName);
	long MakeGraph(const char *fName);
	int ReadAdjList(const char *fName);
	long HaveOrderConstructor();
	bool CheckAdjList();
	bool FillAdjList();

	void SetNumClust(long num);

	void InitClustering(int * whichClusterMCL);

	void FillClusterList();
	void FillClusterListSize();



	long NUM_CLUST;
	long Order;//The number of vertices in the graph

	long EdgeOrder;//The number of edges in the graph

	long CurrentCost;

	long BestMove[3];//The 'vertex', 'from', and 'into', for the best move.

	long* Degree;
	long* WhichCluster;//Which cluster is the vertex in? (Order)

	long* BestWhichCluster;//Best clustering (Order)
	
	long* ClusterSize;//The size of each cluster (NUM_CLUST)

	long* ClusterEdgeCount;


 	SLList* AdjList;//The adjacency list of the graph (Order)

	SLList* ClusterList;//The cluster list of the graph (Order)



};
