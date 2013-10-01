#include <stdlib.h>
#include <math.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <fstream.h>
#include "linkedList.h"
#include "graph.h"
#include "definitions.h"
#include "miscFunctions.h"
#include <wait.h>

//*****************************************************************************
// moveScheme4.cpp: Use the scaled SvD criterion from Def 18, p.103.
// fast version: uses linked list to find best moves fast.
// 
// 
/* 
******************************************************************************/



//*****************************************************************************
/* UpdateTabu******************************************************************
******************************************************************************/
void Graph::UpdateTabu()
{

	if(TabuLength){
		TabuMatrix[BestMove[0]][BestMove[2]] = true;
		
		if(TabuCount < TabuLength){//Still filling TabuList
			
			TabuCount++;
			TabuList.CurrentPtr->Vertex = BestMove[0];
			TabuList.CurrentPtr->To = BestMove[2];
			TabuMatrix[BestMove[0]][BestMove[2]] = true;
			TabuList.Advance();
			
		} else {                   //TabuList is full. Cycling...
			
			TabuMatrix[TabuList.CurrentPtr->Vertex][TabuList.CurrentPtr->To] = false;
			TabuList.CurrentPtr->Vertex = BestMove[0];
			TabuList.CurrentPtr->To = BestMove[2];
			TabuMatrix[TabuList.CurrentPtr->Vertex][TabuList.CurrentPtr->To] = true;
			TabuList.Advance();
			
		}
	}

}


//*****************************************************************************
/* Update**********************************************************************
******************************************************************************/
void Graph::Update()
{
	//	printf("Cost is now %d.\n", (int)CurrentCost);

	//Update TabuMatrix
	UpdateTabu();

	//Update WhichCluster
	WhichCluster[BestMove[0]] = BestMove[2];

	//Update ClusterSize;
	ClusterSize[BestMove[1]] --;
	ClusterSize[BestMove[2]] ++;

	//Update ClusterEdgeCount;
	ClusterEdgeCount[BestMove[1]] -= ClusterListSize[BestMove[0]][BestMove[1]];
	ClusterEdgeCount[BestMove[2]] += ClusterListSize[BestMove[0]][BestMove[2]];

	//Update ClusterListSize;
	UpdateClusterListSize();

	//Update AdjList
	//Update ClusterListPtr
	UpdateAdjList();

	//Update ClusterList
	UpdateClusterList();

	//Update CostMatrix
	UpdateCostMatrix();
	UpdateScaledMoveList();

}


//*****************************************************************************
/* ToCost**********************************************************************
******************************************************************************/
//deals with: NumWithCost, FirstWithCost, and of course MoveList.
void Graph::ToCost(DListPtr point, long vertex, long to, long cost)
{
	long costTick;

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
		FirstWithCost[Order+ cost]->Cost = cost;

		//		printf("1 %3d +1      %d %d\n",cost, vertex, to);
		NumWithCost[Order+ cost] ++;
		
	} else {//point is not NULL.
		long prevCost = point->Cost;
		costTick = cost;

		//preserve correctness of FirstWithCost
		if(point == FirstWithCost[Order+ point->Cost]){
			if(point->Next->Cost == point->Cost){//then we just bump the pointer.
				FirstWithCost[Order+ point->Cost] = point->Next;
			} else {//there is no move with the cost anymore.
				FirstWithCost[Order+ point->Cost] = NULL;
			}
		}
		

		while((costTick < Order) && (!FirstWithCost[Order+ costTick])){//uses short circuit
			costTick++;
		}
		
		if(costTick == Order){//we are at the end of the list.
			MoveList.MoveToBefore(point, MoveList.Tail);
			
			
		} else {//not at the end.
			MoveList.MoveToBefore(point, FirstWithCost[Order+ costTick]);
		
		}
		FirstWithCost[Order+ cost] = point;
		//		printf("2 %3d -1      %d %d\n",prevCost, vertex, to);
		NumWithCost[Order+ prevCost] --;
		//		printf("3 %3d +1      %d %d\n",cost, vertex, to);
		NumWithCost[Order+ cost]++;
		
		FirstWithCost[Order+ cost]->Cost = cost;
		
	}
}


//*****************************************************************************
/* ScaledToCost****************************************************************
******************************************************************************/
//deals with: ScaledNumWithCost, ScaledFirstWithCost, and of course ScaledMoveList.
void Graph::ScaledToCost(DListDoublePtr point, long vertex, long to, long cost, double actualCost)
{
	long costTick;

	if(!point){//if the point is NULL (only case is the newly available move)
		costTick = cost;

		while((costTick < Order) && (!ScaledFirstWithCost[Order+ costTick])){//uses short circuit
			costTick++;
		}

		if(costTick == Order){//we are at the end of the list.
			ScaledFirstWithCost[Order+ cost] = 
				ScaledMoveList.InsertANodeBefore(ScaledMoveList.Tail);
		} else {//not at the end.
			ScaledFirstWithCost[Order+ cost] = 
				ScaledMoveList.InsertANodeBefore(ScaledFirstWithCost[Order+ costTick]);
		}
		ScaledFirstWithCost[Order+ cost]->Vertex = vertex;
		ScaledFirstWithCost[Order+ cost]->To = to;
		ScaledFirstWithCost[Order+ cost]->Cost = actualCost;

		//		printf("1 %3d +1      %d %d\n",cost, vertex, to);
		ScaledNumWithCost[Order+ cost] ++;
		
	} else {//point is not NULL.
		long prevCost = (long) ceil(point->Cost);
		costTick = cost;

		//preserve correctness of FirstWithCost
		if(point == ScaledFirstWithCost[Order+ prevCost]){
			if( ( (long) ceil(point->Next->Cost) ) == prevCost ){//then we just bump the pointer.
				ScaledFirstWithCost[Order+ prevCost] = point->Next;
			} else {//there is no move with the cost anymore.
				ScaledFirstWithCost[Order+ prevCost] = NULL;
			}
		}

		while((costTick < Order) && (!ScaledFirstWithCost[Order+ costTick])){//uses short circuit
			costTick++;
		}
		
		if(costTick == Order){//we are at the end of the list.
			ScaledMoveList.MoveToBefore(point, ScaledMoveList.Tail);
			
			
		} else {//not at the end.
			ScaledMoveList.MoveToBefore(point, ScaledFirstWithCost[Order+ costTick]);
		
		}
		ScaledFirstWithCost[Order+ cost] = point;
		//		printf("2 %3d -1      %d %d\n",prevCost, vertex, to);
		ScaledNumWithCost[Order+ prevCost] --;
		//		printf("3 %3d +1      %d %d\n",cost, vertex, to);
		ScaledNumWithCost[Order+ cost]++;
		
		ScaledFirstWithCost[Order+ cost]->Cost = actualCost;
		
	}
}


//*****************************************************************************
/* UpdateScaledMoveList********************************************************
******************************************************************************/
//Updates ScaledFirstWithCost, ScaledNumWithCost, ScaledMoveList, ScaledMoveListPtr.
//Also updates CostNumerator and CostDenominator.

void Graph::UpdateScaledMoveList()
{
	double newCost;

	//inefficient way of doing it first.
	//first update numerators and denominators.
	long vertex = BestMove[0]; long from = BestMove[1]; long to = BestMove[2];
	long otherVertex, cluster;
	ScaledCost = 0;


	ClusterList[from].RewindToHead();
	for(long j=0; j< ClusterSize[from]; j++){
		otherVertex = ClusterList[from].CurrentPtr->Data;
		if(AdjMatrix[vertex][otherVertex]){
			CostNumerator[otherVertex]++;
		} else {
			CostNumerator[otherVertex]--;
			CostDenominator[otherVertex]--;
		}
		ClusterList[from].Advance();
	}
	
	ClusterList[to].RewindToHead();
	for(long j=0; j< ClusterSize[to]; j++){
		otherVertex = ClusterList[to].CurrentPtr->Data;
		if(AdjMatrix[vertex][otherVertex]){
			CostNumerator[otherVertex]--;
		} else {//includes otherVertex = Vertex case
			if(otherVertex != vertex){
				CostNumerator[otherVertex]++;
				CostDenominator[otherVertex]++;
			} else { //otherVertex = vertex
				//printf("Setting num/dom for %d: %d / %d\n",vertex,(int) CostNumerator[otherVertex],(int) CostDenominator[otherVertex]);
				CostNumerator[otherVertex] += 2*ClusterListSize[vertex][from]
					- 2*ClusterListSize[vertex][to] +ClusterSize[to]-1 - ClusterSize[from];
				
				CostDenominator[otherVertex] += ClusterListSize[vertex][from]
					- ClusterListSize[vertex][to] +ClusterSize[to]-1 - ClusterSize[from];
				//printf("Setting num/dom for %d: %d / %d\n",vertex,(int) CostNumerator[otherVertex],(int) CostDenominator[otherVertex]);
				
			}
		}
		ClusterList[to].Advance();
	}


	//now get the cost for everything in from and to, and everything to from and to.
	for(otherVertex = 0; otherVertex < Order; otherVertex++){
		if(WhichCluster[otherVertex] == from){//cluster i
			for(cluster=0; cluster<NUM_CLUST; cluster++){
				if(cluster != from){
					// from i to any cluster.
					newCost = GetCostForMove(otherVertex, from, cluster);
					ScaledToCost(ScaledMoveListPtr[otherVertex][cluster], otherVertex, cluster, (long) ceil(newCost), newCost);
					ScaledMoveListPtr[otherVertex][cluster] = ScaledFirstWithCost[Order + (long) ceil(newCost)];
				}
			}
		} else if(WhichCluster[otherVertex] == to){//cluster j
			for(cluster=0; cluster<NUM_CLUST; cluster++){
				if(cluster != to){
					// from j to any cluster.
					newCost = GetCostForMove(otherVertex, to, cluster);
					ScaledToCost(ScaledMoveListPtr[otherVertex][cluster], otherVertex, cluster, (long) ceil(newCost), newCost);
					ScaledMoveListPtr[otherVertex][cluster] = ScaledFirstWithCost[Order + (long) ceil(newCost)];
				}
			}
		} else {//cluster k
			// from k to j.
			newCost = GetCostForMove(otherVertex, WhichCluster[otherVertex], to);
			ScaledToCost(ScaledMoveListPtr[otherVertex][to], otherVertex, to, (long) ceil(newCost), newCost);
			ScaledMoveListPtr[otherVertex][to] = ScaledFirstWithCost[Order + (long) ceil(newCost)];

			// from k to i.
			newCost = GetCostForMove(otherVertex, WhichCluster[otherVertex], from);
			ScaledToCost(ScaledMoveListPtr[otherVertex][from], otherVertex, from, (long) ceil(newCost), newCost);
			ScaledMoveListPtr[otherVertex][from] = ScaledFirstWithCost[Order + (long) ceil(newCost)];
		}

		CostCoverage[otherVertex] = (double) (CostNumerator[otherVertex])/(CostDenominator[otherVertex]);
		CostCoverage[otherVertex] *= ScaledMultiplier;
		ScaledCost += CostCoverage[otherVertex];

	}//for every vertex


	//add the new move
	ScaledToCost(ScaledMoveListPtr[vertex][from], vertex, from, (long) ceil(-ScaledMoveListPtr[vertex][to]->Cost),-ScaledMoveListPtr[vertex][to]->Cost);
	ScaledMoveListPtr[vertex][from] = ScaledFirstWithCost[Order + ((long) ceil(-ScaledMoveListPtr[vertex][to]->Cost))];
	//	ScaledNumWithCost[Order -((long) ceil(ScaledMoveListPtr[vertex][to]->Cost))]--;

	//get rid of the defunct move.
	if(ScaledMoveListPtr[vertex][to] == ScaledFirstWithCost[Order+ (long) ceil(ScaledMoveListPtr[vertex][to]->Cost)]){
		if((long) ceil(ScaledMoveListPtr[vertex][to]->Cost) == (long) ceil(ScaledMoveListPtr[vertex][to]->Next->Cost)){
			ScaledFirstWithCost[Order+ (long) ceil(ScaledMoveListPtr[vertex][to]->Cost)] =
				ScaledFirstWithCost[Order+ (long) ceil(ScaledMoveListPtr[vertex][to]->Cost)]->Next;
		} else {
			ScaledFirstWithCost[Order+ (long) ceil(ScaledMoveListPtr[vertex][to]->Cost)]=NULL;
		}

	}
	//  	ScaledNumWithCost[Order+ (long)ceil(ScaledMoveListPtr[vertex][to]->Cost)] --;
	// I DON"T KNOW WHY THIS WORKS< BUT IT DOES!!!!
	//	ScaledNumWithCost[Order+ 1] --;

	for(long i=1; i < 2*Order-1; i++){//fix ScaledNumWithCost
		ScaledNumWithCost[i] = 0;
		if(ScaledFirstWithCost[i]){
			ScaledMoveList.CurrentPtr = ScaledFirstWithCost[i];
			//printf("i = %d, long ceil = %d\n",(int)i, Order+ (int)ceil(ScaledFirstWithCost[i]->Cost));
			while(Order + (long) ceil(ScaledMoveList.CurrentPtr->Cost) == i){
				ScaledNumWithCost[i]++;
				ScaledMoveList.Advance();
			}
		} 
	}
	//ScaledNumWithCost[Order+ (long) ceil(ScaledMoveListPtr[vertex][to]->Cost)]--;



	ScaledMoveList.DeleteANode(ScaledMoveListPtr[vertex][to]);
	ScaledMoveListPtr[vertex][to] = NULL;



}

//*****************************************************************************
/* UpdateCostMatrix************************************************************
******************************************************************************/
//Updates FirstWithCost, NumWithCost, MoveList, MoveListPtr.

void Graph::UpdateCostMatrix()
{
	long i,j,temp;
	long v, cost;

	v = BestMove[0];

	temp = MoveListPtr[BestMove[0]][BestMove[2]]->Cost;

	
	//for all vertices w.r.t. the clusters moved to and from
	for(i=0; i<Order; i++){
		if(AdjMatrix[v][i]){
			
			if(MoveListPtr[i][BestMove[1]]){
				cost = MoveListPtr[i][BestMove[1]]->Cost;
				ToCost(MoveListPtr[i][BestMove[1]], i, BestMove[1], cost+1);
				MoveListPtr[i][BestMove[1]] = FirstWithCost[Order+ cost+1];
				//				printf("1Moving %d,%d from cost %d to cost %d.\n",i ,BestMove[1] ,cost ,cost+1 ); MoveList.PrintList(); MoveList.PrintListBackwards(); 
			}
			if(MoveListPtr[i][BestMove[2]]){
				cost = MoveListPtr[i][BestMove[2]]->Cost;
				ToCost(MoveListPtr[i][BestMove[2]], i, BestMove[2], cost-1);
				MoveListPtr[i][BestMove[2]] = FirstWithCost[Order+ cost-1];
				//				printf("2Moving %d,%d from cost %d to cost %d.\n",i ,BestMove[2] ,cost ,cost-1 ); MoveList.PrintList(); MoveList.PrintListBackwards(); 
			}
			
		} else {
			
			if(MoveListPtr[i][BestMove[1]]){
				cost = MoveListPtr[i][BestMove[1]]->Cost;
				ToCost(MoveListPtr[i][BestMove[1]], i, BestMove[1], cost-1);
				MoveListPtr[i][BestMove[1]] = FirstWithCost[Order+ cost-1];
				//				printf("3Moving %d,%d from cost %d to cost %d.\n",i ,BestMove[1] ,cost , cost-1); MoveList.PrintList(); MoveList.PrintListBackwards(); 
			}
			if(MoveListPtr[i][BestMove[2]]){
				cost = MoveListPtr[i][BestMove[2]]->Cost;
				ToCost(MoveListPtr[i][BestMove[2]], i, BestMove[2], cost+1);
				MoveListPtr[i][BestMove[2]] = FirstWithCost[Order+ cost+1];
				//				printf("4Moving %d,%d from cost %d to cost %d.\n",i ,BestMove[2] , cost, cost+1); MoveList.PrintList(); MoveList.PrintListBackwards(); 
			}			
		}
	}
	
	//for the cluster moved from
	ClusterList[BestMove[1]].RewindToHead();
	for(i=0; i<ClusterSize[BestMove[1]]; i++){
		if(AdjMatrix[v][ClusterList[BestMove[1]].CurrentPtr->Data]){
			for(j=0; j<NUM_CLUST; j++){
				if(MoveListPtr[ClusterList[BestMove[1]].CurrentPtr->Data][j]){
					cost = MoveListPtr[ClusterList[BestMove[1]].CurrentPtr->Data][j]->Cost;
					ToCost(MoveListPtr[ClusterList[BestMove[1]].CurrentPtr->Data][j], 
							 ClusterList[BestMove[1]].CurrentPtr->Data, j, cost-1);
					MoveListPtr[ClusterList[BestMove[1]].CurrentPtr->Data][j] = 
						FirstWithCost[Order+ cost-1];
					//					printf("5Moving %d,%d from cost %d to cost %d.\n", ClusterList[BestMove[1]].CurrentPtr->Data,j ,cost , cost-1); MoveList.PrintList(); MoveList.PrintListBackwards(); 
				}
			}
		} else {
			for(j=0; j<NUM_CLUST; j++){
				if(MoveListPtr[ClusterList[BestMove[1]].CurrentPtr->Data][j]){
					cost = MoveListPtr[ClusterList[BestMove[1]].CurrentPtr->Data][j]->Cost;
					ToCost(MoveListPtr[ClusterList[BestMove[1]].CurrentPtr->Data][j], 
							 ClusterList[BestMove[1]].CurrentPtr->Data, j, cost+1);
					MoveListPtr[ClusterList[BestMove[1]].CurrentPtr->Data][j] = 
						FirstWithCost[Order+ cost+1];
					//					printf("6Moving %d,%d from cost %d to cost %d.\n",ClusterList[BestMove[1]].CurrentPtr->Data ,j ,cost , cost+1); MoveList.PrintList(); MoveList.PrintListBackwards(); 
				}
			}
		}
		ClusterList[BestMove[1]].Advance();
	}
	
	//for the cluster moved to
	ClusterList[BestMove[2]].RewindToHead();
	for(i=0; i<ClusterSize[BestMove[2]]-1/*not v itself*/ ; i++){
		if(AdjMatrix[v][ClusterList[BestMove[2]].CurrentPtr->Data]){
			for(j=0; j<NUM_CLUST; j++){
				if(MoveListPtr[ClusterList[BestMove[2]].CurrentPtr->Data][j]){
					cost = MoveListPtr[ClusterList[BestMove[2]].CurrentPtr->Data][j]->Cost;
					ToCost(MoveListPtr[ClusterList[BestMove[2]].CurrentPtr->Data][j],
							 ClusterList[BestMove[2]].CurrentPtr->Data, j, cost+1);
					MoveListPtr[ClusterList[BestMove[2]].CurrentPtr->Data][j] = 
						FirstWithCost[Order+ cost+1];
					//					printf("7Moving %d,%d from cost %d to cost %d.\n", ClusterList[BestMove[2]].CurrentPtr->Data, j,cost , cost+1); MoveList.PrintList(); MoveList.PrintListBackwards(); 
				}
				
			}
		} else {
			for(j=0; j<NUM_CLUST; j++){
				if(MoveListPtr[ClusterList[BestMove[2]].CurrentPtr->Data][j]){
					cost = MoveListPtr[ClusterList[BestMove[2]].CurrentPtr->Data][j]->Cost;
					ToCost(MoveListPtr[ClusterList[BestMove[2]].CurrentPtr->Data][j],
							 ClusterList[BestMove[2]].CurrentPtr->Data, j, cost-1);
					MoveListPtr[ClusterList[BestMove[2]].CurrentPtr->Data][j] = 
						FirstWithCost[Order+ cost-1];
					//					printf("8Moving %d,%d from cost %d to cost %d.\n", ClusterList[BestMove[2]].CurrentPtr->Data, j,cost ,cost-1 ); MoveList.PrintList(); MoveList.PrintListBackwards(); 
				}
			}
		}
		ClusterList[BestMove[2]].Advance();
	}
	
	//fix v itself
	for(i=0; i<NUM_CLUST; i++){
		//subtract temp from any move of v.
		
		if(MoveListPtr[v][i]){
			cost = MoveListPtr[v][i]->Cost;
			ToCost(MoveListPtr[v][i], v, i, cost-temp);
			MoveListPtr[v][i] = FirstWithCost[Order+ cost-temp];
			
			//			printf("9Moving %d,%d from cost %d to cost %d.\n", v,i ,cost ,cost-temp ); MoveList.PrintList(); MoveList.PrintListBackwards(); 
		}
	}
	
	//set the move which was previously unavailable.
	
	ToCost(MoveListPtr[v][BestMove[1]], v, BestMove[1], -temp);
	MoveListPtr[v][BestMove[1]] = FirstWithCost[Order -temp];
	//	printf("0Adding %d,%d from cost %d to cost %d.\n",v , BestMove[1],cost ,-temp ); MoveList.PrintList(); MoveList.PrintListBackwards(); 
	
	
	//remove the move that was just made.
 
	if(MoveListPtr[v][BestMove[2]] == FirstWithCost[Order+ MoveListPtr[v][BestMove[2]]->Cost]){
		if(MoveListPtr[v][BestMove[2]]->Cost == MoveListPtr[v][BestMove[2]]->Next->Cost){
			FirstWithCost[Order+ MoveListPtr[v][BestMove[2]]->Cost] =
				FirstWithCost[Order+ MoveListPtr[v][BestMove[2]]->Cost]->Next;
		} else {
			FirstWithCost[Order+ MoveListPtr[v][BestMove[2]]->Cost]=NULL;
		}

	}
	MoveList.DeleteANode(MoveListPtr[v][BestMove[2]]);
   MoveListPtr[v][BestMove[2]] = NULL;	
	//	NumWithCost[Order+ temp] --;
	// I DON"T KNOW WHY THIS WORKS< BUT IT DOES!!!!
	NumWithCost[Order+ 1] --;

	//	printf("Removing %d,%d from cost %d.\n",v , BestMove[2],cost ); MoveList.PrintList(); MoveList.PrintListBackwards(); 

}




//*****************************************************************************
/* FindBestMove****************************************************************
******************************************************************************/
void Graph::FindBestMove() //Takes a random move with best cost.
  //this can be made more efficient with another data structure... but it would be huge.
{
	long i, rNum;

	long best = MoveList.Head->Cost; //The best cost.
	rNum = rand() % NumWithCost[Order+ best];

	MoveList.CurrentPtr = FirstWithCost[Order+ best];

	for(i=0; i<rNum; i++){
		MoveList.Advance();
	}

	BestMove[0] = MoveList.CurrentPtr->Vertex;
	BestMove[1] = WhichCluster[BestMove[0]];
	BestMove[2] = MoveList.CurrentPtr->To;

	CurrentCost = CurrentCost + best;
}

//*****************************************************************************
/* ScaledFindBestMove**********************************************************
******************************************************************************/
void Graph::ScaledFindBestMove() //Takes a random move with best cost.
  //this can be made more efficient with another data structure... but it would be huge.
{
	long i, rNum;

	long best = (long) ceil(ScaledMoveList.Head->Cost); //The best cost.
	rNum = rand() % ScaledNumWithCost[Order+ best];

	ScaledMoveList.CurrentPtr = ScaledFirstWithCost[Order+ best];

	for(i=0; i<rNum; i++){
		ScaledMoveList.Advance();
	}

	if(TabuMatrix[ScaledMoveList.CurrentPtr->Vertex][ScaledMoveList.CurrentPtr->To]){
		//It's tabu. We need to pick another move.
		ScaledMoveList.CurrentPtr = ScaledFirstWithCost[Order+ best];
		while(TabuMatrix[ScaledMoveList.CurrentPtr->Vertex][ScaledMoveList.CurrentPtr->To]){
			ScaledMoveList.Advance();
		}

	}
	BestMove[0] = ScaledMoveList.CurrentPtr->Vertex;
	BestMove[1] = WhichCluster[BestMove[0]];
	BestMove[2] = ScaledMoveList.CurrentPtr->To;

	CurrentCost = CurrentCost + best;
}



//*****************************************************************************
/* FindRandomMove**************************************************************
******************************************************************************/
void Graph::FindRandomMove()
{
	long vertex, cluster, cost;

	while(true){
		vertex = rand();
		cluster = rand();
		vertex = vertex % Order;
		cluster = cluster % NUM_CLUST;

		if((!TabuMatrix[vertex][cluster]) &&
			MoveListPtr[vertex][cluster]){

			BestMove[0] = vertex;
			BestMove[2] = cluster;
			BestMove[1] = WhichCluster[vertex];
			cost = MoveListPtr[vertex][cluster]->Cost;
			break;
		}
	}
	CurrentCost = CurrentCost + cost;
}


//*****************************************************************************
/* SetInitialNumAndDom*********************************************************
******************************************************************************/
void Graph::SetInitialNumAndDom()
{
	//Sets the initial numerators and denominators for the vertex coverage,
	//i.e. 1out+0in and |n_v union c_v|.

	long CurrentCluster, j, i;

	ScaledCost=0;

	for(i=0; i<Order; i++){
		CurrentCluster = WhichCluster[i];
		CostNumerator[i] = 0;
		CostDenominator[i] = 0;
		
		
		//First count 1out and n'hood - intersection (they are equal)
		AdjList[i].RewindToHead();
		while(AdjList[i].CurrentPtr != AdjList[i].Tail){
			AdjList[i].Advance();
			if(WhichCluster[AdjList[i].CurrentPtr->Data] != CurrentCluster){
				CostNumerator[i] ++;
			}
		}
		CostDenominator[i] = CostNumerator[i] + ClusterSize[CurrentCluster];

		//Now count 0in
		ClusterList[CurrentCluster].RewindToHead();
		for(j=0; j< ClusterSize[CurrentCluster]; j++){
			if(!AdjMatrix[i][ClusterList[CurrentCluster].CurrentPtr->Data]){
				CostNumerator[i] ++;
			}
			ClusterList[CurrentCluster].Advance();
		}
		CostNumerator[i]--;

		CostCoverage[i] = (double) (CostNumerator[i])/(CostDenominator[i]);
		CostCoverage[i] *= ScaledMultiplier;
		ScaledCost += CostCoverage[i];
	}
}


//*****************************************************************************
/* GetCostForMove(vertex,j,cluster)********************************************
******************************************************************************/
//Also sets cost.
double Graph::GetCostForMove(long vertex, long from, long to)
{
	long finalDenominator, finalNumerator, initialNumerator, initialDenominator;
	double scaledCost;
	long otherVertex;

	initialNumerator = Degree[vertex] + ClusterSize[from]-1 - 2*ClusterListSize[vertex][from];
	initialDenominator = Degree[vertex] + ClusterSize[from] - ClusterListSize[vertex][from];

	finalNumerator = Degree[vertex] + ClusterSize[to] - 2*ClusterListSize[vertex][to];
	finalDenominator = Degree[vertex] + ClusterSize[to]+1 - ClusterListSize[vertex][to];

	scaledCost = (double) (-initialNumerator*finalDenominator + finalNumerator*initialDenominator) /
		(initialDenominator*finalDenominator);

	//	printf("n0 = %d     n1 = %d     d0 = %d     d1 = %d\n",initialNumerator,finalNumerator,initialDenominator,finalDenominator);

	//  	printf("%d from %d to %d \nAdding for v: %f\n",(int) vertex, (int) from, (int) to, (double) (-initialNumerator*finalDenominator + finalNumerator*initialDenominator) / (initialDenominator*finalDenominator));


	//scaledCost is now the cost incurred by v.
	//we must now add the cost incurred by vertices in i and j.

	ClusterList[from].RewindToHead();
	for(long j=0; j< ClusterSize[from]; j++){
		otherVertex = ClusterList[from].CurrentPtr->Data;
		if(AdjMatrix[vertex][otherVertex]){
			//num++, cost += 1/den
			scaledCost += (double) 1/CostDenominator[otherVertex];
			//			printf("Adding for %d (1): %f\n",(int)otherVertex, (double) 1/CostDenominator[otherVertex]);


		} else {//includes otherVertex = Vertex case
			if(otherVertex != vertex){
				//num--, den--, cost -= (d-n)/(d^2-d)
				scaledCost -= (double) (CostDenominator[otherVertex]-CostNumerator[otherVertex])
					/ (CostDenominator[otherVertex]*CostDenominator[otherVertex]-CostDenominator[otherVertex]);
				//				printf("Adding for %d (2): %f\n",(int)otherVertex, (double) (-CostDenominator[otherVertex]+CostNumerator[otherVertex])
				//					/ (CostDenominator[otherVertex]*CostDenominator[otherVertex]-CostDenominator[otherVertex]));

			}
		}
		ClusterList[from].Advance();
	}
	
	ClusterList[to].RewindToHead();
	for(long j=0; j< ClusterSize[to]; j++){
		otherVertex = ClusterList[to].CurrentPtr->Data;
		if(AdjMatrix[vertex][otherVertex]){
			//num --, cost -= 1/den
			scaledCost -= (double) 1/CostDenominator[otherVertex];
			//			printf("Adding for %d (3): %f\n",(int)otherVertex,  (double) -1/CostDenominator[otherVertex]);
		} else {
			//num++, den++, cost += (d-n)/(d^2+d)
				scaledCost += (double) (CostDenominator[otherVertex]-CostNumerator[otherVertex])
					/ (CostDenominator[otherVertex]*CostDenominator[otherVertex]+CostDenominator[otherVertex]);
				//				printf("Adding for %d (4): %f\n",(int)otherVertex,(double) (CostDenominator[otherVertex]-CostNumerator[otherVertex])
				//					/ (CostDenominator[otherVertex]*CostDenominator[otherVertex]+CostDenominator[otherVertex]));
		}
		ClusterList[to].Advance();
	}

	scaledCost *= ScaledMultiplier;
	//printf("\n\n");
	
	return scaledCost;
}


//*****************************************************************************
/* FillCostMatrix**************************************************************
******************************************************************************/
//Also sets cost.
void Graph::FillCostMatrix()
{

	long vertex, i, j, cluster, cost, costTick, ii, scaledCostTick;
	bool listIsEmpty = true;
	double scaledCost;
	long roundedScaledCost;

	SetInitialNumAndDom();

	for(vertex=0;vertex<Order;vertex++){
		cluster = WhichCluster[vertex];
		for(j=0;j<NUM_CLUST;j++){
				

			if(cluster != j){
				cost = (ClusterSize[j] - ClusterSize[cluster] + 1) + 
					2*(ClusterListSize[vertex][cluster] - ClusterListSize[vertex][j]);
				
				scaledCost = GetCostForMove(vertex,cluster,j);
				
				roundedScaledCost = (long) ceil(scaledCost);
				
				
				/*Fill the list*/
				if(listIsEmpty){
					
					listIsEmpty = false;
					
					FirstWithCost[Order+ cost] = MoveList.InsertANodeBefore(MoveList.Head);
					FirstWithCost[Order+ cost]->Vertex = vertex;
					FirstWithCost[Order+ cost]->To = j;
					FirstWithCost[Order+ cost]->Cost = cost;
					
					ScaledFirstWithCost[Order+ roundedScaledCost] = ScaledMoveList.InsertANodeBefore(ScaledMoveList.Head);
					ScaledFirstWithCost[Order+ roundedScaledCost]->Vertex = vertex;
					ScaledFirstWithCost[Order+ roundedScaledCost]->To = j;
					ScaledFirstWithCost[Order+ roundedScaledCost]->Cost = scaledCost;
					
					
				}else{//The list is NOT empty
					
					costTick = cost;
					while(!FirstWithCost[Order+ (costTick)])
						costTick--;//costTick is the greatest smaller nonempty cost.
 						              //possibly -Order.
					if(NumWithCost[Order+ cost] == 0){//If this is the first one with this cost
						MoveList.CurrentPtr = FirstWithCost[Order+ (costTick)];
						for(ii=0; ii<NumWithCost[Order+ costTick]; ii++){
							MoveList.Advance();
						}//we are now at the first of the > costTick nodes.
						
						if(costTick == -Order){
							FirstWithCost[Order+ cost] = 
								MoveList.InsertANodeBefore(MoveList.CurrentPtr);//currentPtr should be head.
						} else { //we insert after. no, before.
							FirstWithCost[Order+ cost] = 
								MoveList.InsertANodeBefore(MoveList.CurrentPtr);
						}
						
						FirstWithCost[Order+ cost]->Vertex = vertex;
						FirstWithCost[Order+ cost]->To = j;
						FirstWithCost[Order+ cost]->Cost = cost;
						
					}else{//If there was already a node with this cost
						FirstWithCost[Order+ cost] = 
							MoveList.InsertANodeBefore(FirstWithCost[Order+ cost]);
						FirstWithCost[Order+ cost]->Vertex = vertex;
						FirstWithCost[Order+ cost]->To = j;
						FirstWithCost[Order+ cost]->Cost = cost;
					}
					
					

					
					
					
					
					scaledCostTick = roundedScaledCost;
					while(!ScaledFirstWithCost[Order+ (scaledCostTick)])
						scaledCostTick--;//costTick is the greatest smaller nonempty cost.
 						              //possibly -Order.
					if(ScaledNumWithCost[Order+ roundedScaledCost] == 0){//If this is the first one with this cost
						ScaledMoveList.CurrentPtr = ScaledFirstWithCost[Order+ scaledCostTick];
						for(ii=0; ii<ScaledNumWithCost[Order+ scaledCostTick]; ii++){
							ScaledMoveList.Advance();
						}//we are now at the first of the > costTick nodes.
						
						if(scaledCostTick == -Order){
							ScaledFirstWithCost[Order+ roundedScaledCost] = 
								ScaledMoveList.InsertANodeBefore(ScaledMoveList.CurrentPtr);//currentPtr should be head.
						} else { //we insert after. no, before.
							ScaledFirstWithCost[Order+ roundedScaledCost] = 
								ScaledMoveList.InsertANodeBefore(ScaledMoveList.CurrentPtr);
						}
						
						ScaledFirstWithCost[Order+ roundedScaledCost]->Vertex = vertex;
						ScaledFirstWithCost[Order+ roundedScaledCost]->To = j;
						ScaledFirstWithCost[Order+ roundedScaledCost]->Cost = scaledCost;
						
					}else{//If there was already a node with this cost
						ScaledFirstWithCost[Order+ roundedScaledCost] = 
							ScaledMoveList.InsertANodeBefore(ScaledFirstWithCost[Order+ roundedScaledCost]);
						ScaledFirstWithCost[Order+ roundedScaledCost]->Vertex = vertex;
						ScaledFirstWithCost[Order+ roundedScaledCost]->To = j;
						ScaledFirstWithCost[Order+ roundedScaledCost]->Cost = scaledCost;
					}
					
					
				}//end {the list is not empty}
				FirstWithCost[0] = MoveList.Head; // maintain the sentinel.
				MoveListPtr[vertex][j] = FirstWithCost[Order+ cost];
				NumWithCost[Order+ cost] ++;
				
				ScaledFirstWithCost[0] = ScaledMoveList.Head; // maintain the sentinel.
				ScaledMoveListPtr[vertex][j] = ScaledFirstWithCost[Order+ roundedScaledCost];
				ScaledNumWithCost[Order+ roundedScaledCost] ++;
				
			}			
		}
	}
	//	printf("Cost matrix filled.\n");

	CurrentCost = EdgeOrder;
	for(i=0;i<NUM_CLUST;i++){
		CurrentCost -= 2*ClusterEdgeCount[i];
		CurrentCost += nChoose2(ClusterSize[i]);
	}

	//PrintNumAndDom();
	//PrintScaledCostMatrix();
	//PrintScaledNumWithCost();
	//	PrintCostMatrix2();
	//MoveList.PrintList();
  	//PrintClusterList();
	//ScaledMoveList.PrintList();
	//getchar();

}
