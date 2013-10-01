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


#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <fstream>
#include <math.h>
#include "linkedList.h"
#include "graph.h"
#include "definitions.h"
#include "miscFunctions.h"



//*****************************************************************************
/* FindAGoodMove***************************************************************
 ******************************************************************************/
//Finds a near-optimal move, if possible. Otherwise, finds the best non-tabu
//move.
void Graph::FindAGoodMove()
{
	//this function does not need to be modified for FRNSC.
	
	int counter=0;
	int numSteps=-1;
	int bestCost = (int) ceil(MoveList.Head->Cost); //The best nonempty cost bin.
	
	
	for(counter=0; counter<20; counter++){
		numSteps = rand() % NumWithCost[Order+ bestCost];
		MoveList.RewindToHead();//we start at the head of the list.
		for(int i=0; i<numSteps; i++){ MoveList.Advance();	}
		
		
		if(
		   (TabuVertices[MoveList.CurrentPtr->Vertex] < TabuTol)
		   &&
		   (ClusterSize[WhichCluster[MoveList.CurrentPtr->Vertex]]>1
			||
			ClusterSize[MoveList.CurrentPtr->To]>0)
		   ){//it is not tabu and it is not a redundant move. take it.
			Move[0] = MoveList.CurrentPtr->Vertex;
			Move[1] = WhichCluster[Move[0]];
			Move[2] = MoveList.CurrentPtr->To;
			MovePtr = MoveList.CurrentPtr;
			break;
		}
	}
	if(counter==20){//didn't find one randomly. find another good move.
		MoveList.RewindToHead();
		while(
			  (TabuVertices[MoveList.CurrentPtr->Vertex] >= TabuTol)
			  ||
			  (ClusterSize[WhichCluster[MoveList.CurrentPtr->Vertex]]==1
			   &&
			   ClusterSize[MoveList.CurrentPtr->To]==0)
			  ){//tabu or redundant. check the next.

			if(MoveList.CurrentPtr->Next->Vertex < 1) break;//we are at the sentinel
			else MoveList.Advance();
		}
		//take the move.
		Move[0] = MoveList.CurrentPtr->Vertex;
		Move[1] = WhichCluster[Move[0]];
		Move[2] = MoveList.CurrentPtr->To;
		MovePtr = MoveList.CurrentPtr;
	}
	
	NaiveCost = NaiveCost + (int) MoveList.CurrentPtr->Cost;
	ScaledCost = ScaledCost + MoveList.CurrentPtr->Cost;
	
}



//*****************************************************************************
/* FindAShuffleMove************************************************************
 ******************************************************************************/
//Picks a shuffling diversification move: Uniformly chooses a vertex that can
//be moved, then uniformly chooses a move from MovesOf[v].
void Graph::FindAShuffleMove(){
        int vertex=-1;//, cluster=-1;
	PListPtr Ptr = NULL;
	int StepCount;
	int WhichOne = 0;
	
	do{
		vertex = rand() % Order;//pick a vertex.
		while(TabuVertices[vertex] >= TabuTol || MovesOf[vertex].Head == NULL ) 
			vertex = rand() % Order; //vertex is tabu or cannot be moved. start DO WHILE again.
		
		WhichOne = rand() % Degree[vertex];//which move to take? deg(v) is a reasonable bound.
		
		StepCount = 0;
		Ptr = MovesOf[vertex].Head;
		
		while(StepCount <= WhichOne){
			StepCount ++;
			Ptr = Ptr->Next;
			if(Ptr == NULL) Ptr = MovesOf[vertex].Head;
		}
		//cluster = Ptr->Ptr->To;
		
	} while(0);
	
	
	MovePtr = Ptr->Ptr;
	MoveList.CurrentPtr = MovePtr;
	Move[0] = MoveList.CurrentPtr->Vertex;
	Move[1] = WhichCluster[Move[0]];
	Move[2] = MoveList.CurrentPtr->To;
	
	NaiveCost = NaiveCost + (int) MovePtr->Cost;
	ScaledCost = ScaledCost + MovePtr->Cost;
}


//*****************************************************************************
/* FindADestroyMove************************************************************
 ******************************************************************************/
//Picks a destructive diversification move. (Just moves the vertex at the head
//of the cluster list randomly.  We know that the size of cluster is > 1 and
//therefore the vertex can be moved.
//Actually, we need to make sure the vertex can be moved.  Otherwise we check the next vertex, etc.,
//eventually returning 0 if it fails.
int Graph::FindADestroyMove(int cluster){
	
	int vertex;
	ListPtr VPtr = NULL;
	PListPtr MPtr = NULL;
	int NumberOfSteps, StepCount;
	
	
	VPtr = ClusterList[cluster].Head;
	
	while(VPtr != NULL)
	{
		vertex = VPtr->Vertex;
		NumberOfSteps = Degree[vertex];//sloppy upper bound.
		
		StepCount = 0;
		MPtr = MovesOf[vertex].Head;
		if(MPtr == NULL){
			VPtr = VPtr->Next;
			continue;
		}
		
		while(StepCount<=NumberOfSteps && MPtr != MovesOf[vertex].Tail){
			StepCount++;
			MPtr = MPtr->Next;
		}
		int NewCluster = MPtr->Ptr->To;
		
		Move[0] = vertex;
		Move[1] = cluster;
		Move[2] = NewCluster;
		MovePtr = MPtr->Ptr;
		
		NaiveCost = NaiveCost + (int) MovePtr->Cost;
		ScaledCost = ScaledCost + MovePtr->Cost;
		
		return 1;
		
		VPtr = VPtr->Next;
	}
	
	
	return 0;
	
}



//*****************************************************************************
/* UpdateAdjList***************************************************************
 ******************************************************************************/
//Maintains the adjacency list for the vertex being moved such that its
//neighbours in the same cluster are at the end, located by AdjHead2[v].
void Graph::UpdateAdjList()
{
	int u=-1;
	int v = Move[0];
	ListPtr tempPrevious;
	
	//First update for u in C_i.
	AdjList[v].CurrentPtr = AdjHead2[v];
	while(AdjList[v].CurrentPtr != NULL){
		u = AdjList[v].CurrentPtr->Vertex;
		AdjList[u].CurrentPtr = AdjHead2[u];
		while(AdjList[u].CurrentPtr->Vertex != v){
			AdjList[u].Advance();
		}
		
		AdjList[u].InsertANodeAfter(AdjList[u].Head);
		AdjList[u].Head->Next->Vertex = v;
		if(AdjList[u].CurrentPtr == AdjHead2[u])
			AdjHead2[u] = AdjHead2[u]->Next;
		tempPrevious = AdjList[u].CurrentPtr;
		
		AdjList[u].Advance();
		AdjList[u].DeleteANode(tempPrevious);
		
		if(AdjList[v].CurrentPtr == AdjList[v].Tail) break;
		AdjList[v].Advance();
	}
	
	
	//Then update for v.
	AdjList[v].RewindToHead();
	tempPrevious=AdjList[v].CurrentPtr;
	AdjList[v].Advance();
	
	for(int j=0; j<Degree[v]; j++){//run through and throw in-cluster
		//neighbours to the end of the list.
		if(SANITY_CHECK){
			if(tempPrevious->Next != AdjList[v].CurrentPtr)
				printf("SANITY CHECK: Problem in Graph::UpdateAdjList. i=%d, j=%d.\n",v,j);
		}
		if(WhichCluster[AdjList[v].CurrentPtr->Vertex] == WhichCluster[v]){
			AdjList[v].AddANode();
			AdjList[v].Tail->Vertex = AdjList[v].CurrentPtr->Vertex;
			AdjList[v].DeleteANodeFast(AdjList[v].CurrentPtr,tempPrevious);
			AdjList[v].CurrentPtr = tempPrevious;
		}
		tempPrevious=AdjList[v].CurrentPtr;
		AdjList[v].Advance();
	}
	if(WhichCluster[AdjList[v].CurrentPtr->Vertex] == WhichCluster[v])
		AdjHead2[v]=AdjList[v].CurrentPtr;
	else AdjHead2[v]=NULL;
	
	//Finally, update for u in C_j.
	AdjList[v].CurrentPtr = AdjHead2[v];
	while(AdjList[v].CurrentPtr != NULL){
		u = AdjList[v].CurrentPtr->Vertex;
		AdjList[u].RewindToHead(); AdjList[u].Advance();
		while(AdjList[u].CurrentPtr->Vertex != v){
			AdjList[u].Advance();
		}
		AdjList[u].AddANode();
		AdjList[u].Tail->Vertex = v;
		if(AdjList[u].CurrentPtr == AdjHead2[u])
			AdjHead2[u] = AdjHead2[u]->Next;
		else if(AdjHead2[u] == NULL)
			AdjHead2[u] = AdjList[u].Tail;
		tempPrevious = AdjList[u].CurrentPtr;
		AdjList[u].Advance();
		AdjList[u].DeleteANode(tempPrevious);
		
		if(AdjList[v].CurrentPtr == AdjList[v].Tail) break;
		AdjList[v].Advance();
	}
	
	if(SANITY_CHECK) CheckAdjList();
}



//*****************************************************************************
/* UpdateTabu******************************************************************
 ******************************************************************************/
//Update the tabu list when a move is made.
void Graph::UpdateTabu()
{
	
	if(TabuLength){
		if(TabuVertices[Move[0]] < TabuTol)
			TabuVertices[Move[0]]++;
		
		if(TabuCount < TabuLength){//Still filling TabuList
			
			TabuCount++;
			TabuList.CurrentPtr->Vertex = Move[0];
			TabuList.Advance();
			
		} else {                   //TabuList is full. Cycling...
			
			TabuVertices[TabuList.CurrentPtr->Vertex]--;
			TabuList.CurrentPtr->Vertex = Move[0];
			TabuList.Advance();
		}
	}
}



//*****************************************************************************
/* UpdateClusterList***********************************************************
 ******************************************************************************/
//Updates the cluster lists for the source and destination clusters of the
//vertex being moved.
void Graph::UpdateClusterList()
{
	int C_j = Move[2];
	if(C_j == -2) C_j = GhostCluster;
	
	ClusterList[Move[1]].DeleteANode(ClusterListPtr[Move[0]]);
	ClusterList[C_j].AddANode();
	ClusterList[C_j].Tail->Vertex = Move[0];
	ClusterListPtr[Move[0]] = ClusterList[C_j].Tail;
}


//*****************************************************************************
/* UpdateEmpties***************************************************************
 ******************************************************************************/
//Maintain the list of empty clusters, which is used in generating ghost moves.
void Graph::UpdateEmpties()
{
	
	if(GhostCluster >= 0){//if there was an empty cluster
		if(ClusterSize[GhostCluster]){//it was empty, now it isn't.
			EmptyClusterList.DeleteANode(EmptyClusterList.Head);
			NumEmptyClusters--;
		}
	}
	if(ClusterSize[Move[1]] == 0){//we have a new empty cluster.
		EmptyClusterList.AddANode();
		EmptyClusterList.Tail->Vertex = Move[1];
		NumEmptyClusters++;
	}
	SetGhostCluster();
}


//*****************************************************************************
/* ToCost**********************************************************************
 ******************************************************************************/
//Deals with: NumWithCost, FirstWithCost, and of course MoveList.
//This function takes the move in MoveList located at 'point' and moves it to
//keep MoveList 'nearly sorted'.
void Graph::ToCost(DListDoublePtr point, int vertex, int to, int cost, double actualCost)
{
	int costTick;
	
	if(!point){//if the point is NULL (only case is the newly available move)
		
		costTick = cost;
		
		while((costTick < Order) && (!FirstWithCost[Order+ costTick])){//uses short circuit
			costTick++;
		}
		
		if(costTick == Order){//we are at the end of the list.
			FirstWithCost[Order+ cost] = 
			MoveList.InsertANodeBefore(MoveList.Tail);
		} else {//not at the end.
			FirstWithCost[Order+ cost] = 
			MoveList.InsertANodeBefore(FirstWithCost[Order+ costTick]);
		}
		FirstWithCost[Order+ cost]->Vertex = vertex;
		FirstWithCost[Order+ cost]->To = to;
		FirstWithCost[Order+ cost]->Cost = actualCost;
		NumWithCost[Order+ cost] ++;
		
	} else {//point is not NULL.
		int prevCost = (int) ceil(point->Cost);
		costTick = cost;
		
		//preserve correctness of FirstWithCost
		if(point == FirstWithCost[Order+ prevCost]){
			if( ( (int) ceil(point->Next->Cost) ) == prevCost ){//then we just bump the pointer.
				FirstWithCost[Order+ prevCost] = point->Next;
			} else {//there is no move with the cost anymore.
				FirstWithCost[Order+ prevCost] = NULL;
			}
		}
		
		while((costTick < Order) && (!FirstWithCost[Order+ costTick])){//uses short circuit
			costTick++;
		}
		
		if(costTick == Order){//we are at the end of the list.
			MoveList.MoveToBefore(point, MoveList.Tail);
		} else {//not at the end.
			//a bug here is oxymoronic...
			if(SANITY_CHECK && !FirstWithCost[Order+ costTick]){
				printf("Trying to pass a null pointer to MoveToBefore. Busy halting...\n");
				while(1){}
			}
			MoveList.MoveToBefore(point, FirstWithCost[Order+ costTick]);
		}
		FirstWithCost[Order+ cost] = point;
		NumWithCost[Order+ prevCost] --;
		NumWithCost[Order+ cost]++;
		
		FirstWithCost[Order+ cost]->Cost = actualCost;
	}
	
}

