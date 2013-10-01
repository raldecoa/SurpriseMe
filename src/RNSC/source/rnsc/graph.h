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

#include <vector>
#include "definitions.h"

using namespace std;

class Graph
{
 private:
 public:

   //CONTENTS OF graph.cpp
   Graph();
   ~Graph();
	
   void Initialize();
   void InitializeLUT();
   int  ReadGraph();
   void FillClusterList();
   void FillClusterListSize();
   void InitSortAdjList();
   void InitEmptyClusterList();
   void SetGhostCluster();
   void InitClustering();
   void InitClustering(int InputClustering []);
   int  LoadClustering(int InputClustering []);
   void SetRandomClustering();
   void SetExplicitClustering();
   //END CONTENTS OF graph.cpp

   //CONTENTS OF printing.cpp
   void PrintAdjList();
   void CheckAdjList();
   void PrintNumWithCost();
   void PrintClusters();
   void PrintMovesTo();
   void PrintMovesOf();
   void CheckFirsts();

   //CONTENTS OF graph2.cpp
   void InitNaiveScheme();
   void InitScaledScheme();
   double GetScaledCostForClustering();
   double GetScaledCostForMove(int vertex, int C_j);
   double GetScaledCostForGhostMove(int vertex);
   void PurgeMoveList();//also purges MovesOf and MovesTo.
   //END CONTENTS OF graph2.cpp

	
   //CONTENTS OF graph3.cpp
   void FindAGoodMove();
   void FindAShuffleMove();
   int FindADestroyMove(int cluster);
   void UpdateAdjList();
   void UpdateTabu();
   void UpdateClusterList();
   void UpdateEmpties();
   void ToCost(DListDoublePtr point, int vertex, int to, int cost, double actualCost);
   //END CONTENTS OF graph3.cpp


   //CONTENTS OF graph4.cpp
   void NaiveUpdate();
   void NaiveUpdateMoveList();
   void CheckNaiveMoveList();
   void AddANaiveMove(int v, int c);
   //END CONTENTS OF graph4.cpp

   //CONTENTS OF graph5.cpp
   void ScaledUpdate();
   void UpdateNumAndDom();
   void ScaledUpdateMoveList();
   void CheckScaledMoveList();
   void AddAScaledMove(int v, int C_j);
   //END CONTENTS OF graph5.cpp

   //CONTENTS OF graph6.cpp
   bool MoveIsInMovesTo(int vertex, int cluster);
   bool MoveIsInMovesOf(int vertex, int to);
   void UpdateAdjVector();
   void NullifyAdjVector();
   bool IsInAdjListOf(int v, int u);
   int GetClusterDegree(int v, int c);
   bool HasNeighbourInCluster(int v, int c);
   void NixMove(int v, int c);
   //END CONTENTS OF graph6.cpp

   //Filename data
   const char * GraphFN;//graph filename.
   const char * ClusteringFN;//clustering filename (for input).
   const char * OutputFN;//clustering output filename.


   //Static graph data
   //
   int NumClust;//number of clusters
   int Order;//number of vertices
   int EdgeOrder;//number of edges
   int* Degree;//degree for each vertex
   SLList* AdjList;//adjacency list
   ListPtr* AdjHead2;//pointer to the first in-cluster neighbour of v.
   bool* AdjVector;//when we want to query adjacency to a given vertex often, (i.e. v when moving v),
   //we set AdjVector to the vth row of the AdjMatrix.


   //General clustering data
   //
   int* WhichCluster;//which cluster is the vertex in?
   int* ClusterSize;//number of vertices in the cluster
   int* ClusterEdgeOrder;//number of edges in the cluster
   SLList* ClusterList;//list of vertices in a cluster
   ListPtr* ClusterListPtr;//pointer to a vertex in ClusterList
   int Move[3];//the chosen move: [vertex from to]
   DListDoublePtr MovePtr;//pointer to the current move in MoveList

   int NumEmptyClusters;//the number of empty clusters
   SLList EmptyClusterList;//the list of empty clusters
   int GhostCluster;//the present empty cluster to be used. it is the HEAD of EmptyClusterList.



   //Tabu data
   //
   SLList TabuList;//Tabu List. It's a ringed linked list.
   int TabuLength;//Length of the Tabu List.
   int TabuTol;//Max number of times a vertex can be on the tabu list.
   int TabuCount;//Number of nodes actually in the tabu list.
   int* TabuVertices;//Number of times a vertex is on the tabu list.

   //Cost data
   int NaiveCost;//the naive cost of a clustering. only applicable under naive scheme.
   double ScaledCost;//the scaled cost of a clustering. only applicable under scaled scheme.
   DLListDouble MoveList;//the move list. now there's only one. length is 2*Order+1: A sentinel
   //at the beginning and end, and 2(Order-1)+1 bins.
   PLList* MovesTo;//List of pointers to MoveList by destination cluster.
   //has size NumClust+1. The last one, i.e. MovesTo[NumClust], is the 'ghost cluster'. it's more
   //efficient than maintaining moves to singletons explicitly.
   PLList* MovesOf;//List of pointers to MoveList by source cluster.
   int NumMoves;//The number of moves in the MoveList.
   bool * VectorForScaledCost;//Used in GetScaledCostForMove(). When the function begins and
   //ends, the vector is null. VFSC is the row in the adjacency matrix for v, the vertex that
   //is being moved.

   bool* MoveIsNew;//used to initialize schemes in graph2.cpp.
   bool ToGhostIsNew;

   //Data for monitoring neighbourhood sizes
   int * NumMovesTo;
   int * NumMovesOf;


	
   DListDoublePtr* FirstWithCost;//the first in the move list with cost Order+x.
   int* NumWithCost;//the number of moves in the list with cost Order+x.
   int* CostNumerator;// alpha_v.
   int* CostDenominator;// beta_v.

   //Safeguards
   float ScaledMultiplier;//for when things go wrong and we need to hack things.
   float HighestCostUsed;
   float LowestCostUsed;


   //Lookup tables. They speed up scaled updating.
   double * LUT_InvB;
   vector < double * > LUT_BigPos;
   vector < double * > LUT_BigNeg;
   int LookupBound;//this is the dimension of the lookup tables.  We want them to take
   //no more than O(m) memory, so it is set to 2*ceil(sqrt(EdgeOrder)).


};
