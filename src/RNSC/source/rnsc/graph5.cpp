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


//Update all the data structures in the scaled scheme.  This is called after
//choosing a move.
void Graph::ScaledUpdate()
{
	if(SANITY_CHECK && Move[2] == -2 && GhostCluster == -1){
		printf("GhostCluster move, but there's no empty cluster!\n");
		while(1);
	}
	
	if(NEIGHBOURHOODS){
		long num2 = (ClusterSize[Move[1]]+ClusterSize[Move[2]])*(NumClust-NumEmptyClusters-3)+2*Order;
		long num = NumMovesTo[Move[1]] + NumMovesTo[Move[2]];
		ClusterList[Move[1]].RewindToHead();
		for(int i=0; i<ClusterSize[Move[1]]; i++){
			num += NumMovesOf[ClusterList[Move[1]].CurrentPtr->Vertex];
			ClusterList[Move[1]].Advance();
		}
		ClusterList[Move[2]].RewindToHead();
		for(int i=0; i<ClusterSize[Move[2]]; i++){
			num += NumMovesOf[ClusterList[Move[2]].CurrentPtr->Vertex];
			ClusterList[Move[2]].Advance();
		}
		FILE * fStream;
		fStream = fopen("nhoodsizes.dat","a");
		fprintf(fStream," 0   %9d   %9d  \n", (int) num, (int) num2);
		fclose(fStream);
		
	}
	
	
	//Update AdjVector.
	UpdateAdjVector();
	
	//Update the tabu list.
	UpdateTabu();//ok
	
	//Update ClusterSize.
	ClusterSize[Move[1]] --;
	if(Move[2] != -2)
		ClusterSize[Move[2]] ++;
	else
		ClusterSize[GhostCluster]++;
	
	
	//Update ClusterEdgeCount.
	ClusterEdgeOrder[Move[1]] -= GetClusterDegree(Move[0], Move[1]);
	if(Move[2] != -2)
		ClusterEdgeOrder[Move[2]] += GetClusterDegree(Move[0], Move[2]);
	else
		ClusterEdgeOrder[GhostCluster] += GetClusterDegree(Move[0], GhostCluster);
	
	//Update WhichCluster.
	if(Move[2] != -2)
		WhichCluster[Move[0]] = Move[2];
	else
		WhichCluster[Move[0]] = GhostCluster;
	
	//Rearrange AdjList
	UpdateAdjList();
	
	//Update ClusterList
	UpdateClusterList();//ok
	
	//Update EmptyClusterList and GhostCluster
	UpdateEmpties();
	
	//Update alpha_v and beta_v
	UpdateNumAndDom();
	
	//Update CostMatrix
	ScaledUpdateMoveList();
	
	//Set the AdjVector to false. (MUST BE THE LAST TASK OF UPDATE)
	NullifyAdjVector();
}


//Updates CostNumerator and CostDenominator for any vertex in C_i or C_j.
void Graph::UpdateNumAndDom()
{
	
	int v, u, C_i, C_j;
	v = Move[0];
	C_i = Move[1];
	C_j = Move[2];
	
	//The numerator and denominator can only change for vertices in C_i or C_j.
	ClusterList[C_i].RewindToHead();
	for(int i=0; i<ClusterSize[C_i]; i++){
		u = ClusterList[C_i].CurrentPtr->Vertex;
		
		if(!AdjVector[u]){
			CostNumerator[u]--;
			CostDenominator[u]--;
		} else CostNumerator[u]++;
		
		ClusterList[C_i].Advance();
	}
	
	
	if(C_j != -2){//if C_j is not the ghost cluster
		ClusterList[C_j].RewindToHead();
		for(int i=0; i<ClusterSize[C_j]-1; i++){
			u = ClusterList[C_j].CurrentPtr->Vertex;
			
			if(!AdjVector[u]){
				CostNumerator[u]++;
				CostDenominator[u]++;
			} else {
				CostNumerator[u]--;
			}
			
			ClusterList[C_j].Advance();
		}
	}
	
	
	CostNumerator[v] = Degree[v] + ClusterSize[WhichCluster[v]]-1 - 2*GetClusterDegree(v, WhichCluster[v]);
	CostDenominator[v] = Degree[v] + ClusterSize[WhichCluster[v]] - GetClusterDegree(v, WhichCluster[v]);
	
	
}





//*****************************************************************************
/* ScaledUpdateMoveList********************************************************
 ******************************************************************************/
//Updates FirstWithCost, NumWithCost, MovesOf, MovesTo, and MoveList.
//This is a major operation, this function.  It deletes, edits, and adds moves,
//maintaining other data structures as it goes along.
void Graph::ScaledUpdateMoveList(){
	
	int v = Move[0]; int C_i = Move[1]; int C_j = Move[2];
	int u, C_l;
	double CostOfTheMove = MovePtr->Cost;
	double cost;
	
	
	//UPDATE COSTS OF EXISTING MOVES.
	
	//update any move to C_i.
	MovesTo[C_i].RewindToHead();
	
	if(MovesTo[C_i].Head != NULL){
		while(1){
			u = MovesTo[C_i].CurrentPtr->Ptr->Vertex;
			
			cost = GetScaledCostForMove(u, C_i);
			
			ToCost(MovesTo[C_i].CurrentPtr->Ptr, u, C_i, (int)ceil(cost), cost);
			
			if(MovesTo[C_i].CurrentPtr == MovesTo[C_i].Tail) break;
			MovesTo[C_i].Advance();
		}
	}
	
	
	//update any move to C_j.
	//if C_j is the GhostCluster, C_j will be -2. Change it to NumClust for this while(1).
	if(C_j != -2){
		
		if(C_j == -2) C_j = NumClust;
		MovesTo[C_j].RewindToHead();
		while(1){
			u = MovesTo[C_j].CurrentPtr->Ptr->Vertex;
			
			cost = GetScaledCostForMove(u, C_j);
			
			if(C_j != NumClust)
				ToCost(MovesTo[C_j].CurrentPtr->Ptr, u, C_j, (int)ceil(cost), cost);
			else
				ToCost(MovesTo[C_j].CurrentPtr->Ptr, u, -2, (int)ceil(cost), cost);
			
			if(MovesTo[C_j].CurrentPtr == MovesTo[C_j].Tail) break;
			MovesTo[C_j].Advance();
		}
		if(C_j == NumClust) C_j = -2;//C_j is once again -2, or 0 <= C_j < Order.
		
	}
	
	//update any move from C_i.
	ClusterList[C_i].RewindToHead();
	while(1){
		if(ClusterSize[C_i] == 0) break; //there is nothing to do here.
		u = ClusterList[C_i].CurrentPtr->Vertex;
		
		MovesOf[u].RewindToHead();
		//for each move of u, cost -1 if u~v, cost +1 otherwise.
		while(1){
			if(MovesOf[u].Head == NULL) break;//if there are no moves to look at.
			
			C_l = MovesOf[u].CurrentPtr->Ptr->To;
			
			cost = GetScaledCostForMove(u, C_l);
			
			ToCost(MovesOf[u].CurrentPtr->Ptr, u, C_l, (int)ceil(cost), cost);
			
			if(MovesOf[u].CurrentPtr == MovesOf[u].Tail) break;
			MovesOf[u].Advance();
			
		}			
		if(ClusterList[C_i].CurrentPtr == ClusterList[C_i].Tail) break;
		ClusterList[C_i].Advance();
		
	}
	
	
	//update any move from C_j. (includes v, which should be the tail)
	if(C_j==-2){//if the destination C_j is GhostCluster
		MovesOf[v].RewindToHead();
		while(1){
			C_l = MovesOf[v].CurrentPtr->Ptr->To;
			//change the cost for v->whatever. subtract CostOfTheMove.
			
			cost = MovesOf[v].CurrentPtr->Ptr->Cost - CostOfTheMove;
			
			ToCost(MovesOf[v].CurrentPtr->Ptr, v, C_l, (int)ceil(cost), cost);
			
			if(MovesOf[v].CurrentPtr == MovesOf[v].Tail) break;
			MovesOf[v].Advance();
		}
		
	}else{
		ClusterList[C_j].RewindToHead();
		
		while(1){
			
			u = ClusterList[C_j].CurrentPtr->Vertex;
			MovesOf[u].RewindToHead();
			
			if(u != v){
				
				while(1){
					if(MovesOf[u].Head == NULL) break;//there are no moves of u.
					
					C_l = MovesOf[u].CurrentPtr->Ptr->To;
					cost = GetScaledCostForMove(u, C_l);
					ToCost(MovesOf[u].CurrentPtr->Ptr, u, C_l, (int)ceil(cost), cost);
					
					if(MovesOf[u].CurrentPtr == MovesOf[u].Tail) break;
					MovesOf[u].Advance();
				}
				
			} else{//if u==v
				
				while(1){
					C_l = MovesOf[v].CurrentPtr->Ptr->To;
					if(C_l != C_j){
						//change the cost for v->whatever. subtract CostOfTheMove.
						
						cost = MovesOf[v].CurrentPtr->Ptr->Cost - CostOfTheMove;
						ToCost(MovesOf[v].CurrentPtr->Ptr, v, C_l, (int)ceil(cost), cost);
						
					}
					if(MovesOf[v].CurrentPtr == MovesOf[v].Tail) break;
					MovesOf[v].Advance();
				}
			}
			
			if(ClusterList[C_j].CurrentPtr == ClusterList[C_j].Tail)	break;
			ClusterList[C_j].Advance();
		}//end if/else: if the destination is GhostCluster.
	}//end edit moves from C_j.
	
	
	//end edit existing costs.
	
	
	
	
	//NOW DELETE THE DEFUNCT MOVES.
	
	//if C_j is a singleton, nix v->GhostCluster. else nix v->C_j.
	//remember, in the MoveList, cluster '-2' is the ghost cluster.
	//in MovesTo, ghost cluster moves are stored in MovesTo[NumClust].
	if(C_j == -2 || C_j == NumClust){//this is kind of hacky/lazy.
		//we are moving into an empty cluster. nix v->GhostCluster.
		NixMove(v,NumClust);
	} else {
		//we are not moving into an empty cluster. nix v->C_j.
		if(C_j==NumClust) while(1);
		NixMove(v,C_j);
	}
	
	
	//if C_i is now a singleton and there was previously an empty cluster, 
	//nix u->GhostCluster for u in C_i.
	if(ClusterSize[C_i] == 1 && (C_j == -2 || GhostCluster >=0 )){
		NixMove(ClusterList[C_i].Head->Vertex, NumClust);
	}
	
	//if there WAS an empty cluster but now there isn't, nix all moves to GhostCluster.
	//	if((EmptyClusterList.Head != NULL) && (EmptyClusterList.Head->Next == NULL) &&
	//	ClusterSize[C_j] == 1){
	if(C_j == -2 && GhostCluster == -1){
		while(MovesTo[NumClust].Head != NULL){
			NixMove(MovesTo[NumClust].Head->Ptr->Vertex, NumClust);
		}
	}
	
	
	//for u~v not in C_i, nix u->C_i unless u has another neighbour in C_i.
	//remember, the AdjList has already been updated.
	AdjList[v].RewindToHead(); AdjList[v].Advance();
	if(Degree[v] > 0){
		while(1){
			u = AdjList[v].CurrentPtr->Vertex;
			if(WhichCluster[u] != C_i){
				
				//does u have a neighbour in C_i?
				if(!HasNeighbourInCluster(u,C_i)){
					//nix the move u->C_i.
					NixMove(u,C_i);
				}
				
			}
			if(AdjList[v].CurrentPtr == AdjList[v].Tail) break;
			AdjList[v].Advance();
		}
	}//end nix u->C_i.
	//Done deleting defunct moves.
	
	
	
	
	//ADD NEW MOVES TO THE MOVELISTS.
	
	//Add v->C_i if v has a neighbour in C_i (unless C_i is empty, in which case add v->Ghost.
	if(ClusterSize[C_i] > 0){
		if(HasNeighbourInCluster(v,C_i)){
			AddAScaledMove(v, C_i);
		}
	} else {
		AddAScaledMove(v, NumClust);
	}
	
	//Add u->GhostCluster if u is in C_j and |C_j| = 2.
	if(C_j != -2 && C_j != NumClust && GhostCluster >= 0 && ClusterSize[C_j] == 2){
		AddAScaledMove(ClusterList[C_j].Head->Vertex, NumClust);
	}
	
	//Add u->C_j if it doesn't exist and u is not in C_j and u~v.	
	AdjList[v].RewindToHead(); AdjList[v].Advance();
	if(Degree[v] > 0){
		while(1){
			if(AdjList[v].CurrentPtr == AdjHead2[v]){
				break;//AdjList has been updated.
			}
			u = AdjList[v].CurrentPtr->Vertex;
			
			if(C_j == -2){//if the destination cluster is a singleton
				//we already know it doesn't exist, because the cluster didn't exist.
				AddAScaledMove(u, WhichCluster[v]);
				
			} else { //the cluster already existed.
				if(!MoveIsInMovesOf(u, C_j)){
					AddAScaledMove(u, WhichCluster[v]);
				}
			}
			
			if(AdjList[v].CurrentPtr == AdjList[v].Tail){
				break;
			}
			AdjList[v].Advance();
		}
	}//end add u->C_j.
	
	//if there is wasn't an empty cluster before, but there is now, we need to clear
	//all the moves to the empty cluster and add them.
	if(ClusterSize[C_i] == 0 && EmptyClusterList.Head->Next == NULL){
		//clear all moves to the GhostCluster.
		while(MovesTo[NumClust].Head != NULL){
			NixMove(MovesTo[NumClust].Head->Ptr->Vertex, NumClust);
		}
		for(int i=0; i<Order; i++){
			if(ClusterSize[WhichCluster[i]] > 1)
				AddAScaledMove(i, NumClust);
		}
	}
	
	if(SANITY_CHECK) CheckScaledMoveList();
	
}






//*****************************************************************************
/* CheckScaledMoveList()********************************************************
 ******************************************************************************/
//Makes sure everything is ok in MoveList under the scaled scheme.
void Graph::CheckScaledMoveList()
{
	int v, C_i, C_j;
	double cost, actualcost;
	float tol = .00001;
	
	for(int i=1; i<2*Order; i++){
		if((NumWithCost[i]==0) != (FirstWithCost[i]==NULL)){
			printf("Problem with First/Num: %d.\n", i); while(1);
		}
	}
	
	MoveList.RewindToHead();
	
	while(MoveList.CurrentPtr->Vertex >= 0){
		v = MoveList.CurrentPtr->Vertex;
		C_i = WhichCluster[MoveList.CurrentPtr->Vertex];
		C_j = MoveList.CurrentPtr->To;
		cost = MoveList.CurrentPtr->Cost;
		
		
		//first check the cost.
		actualcost = GetScaledCostForMove(v,C_j);
		
		if(fabs(actualcost - cost) > tol){
			printf("%d %d %d Cost in the list is %.2f, but it should be %.2f.\n",
				   v, C_i, C_j, cost, actualcost);
			while(1);
		}
		
		//check viability of the move.
		if(
		   (C_j == -2 && ClusterSize[C_i] < 2) ||
		   (C_j != -2 && !HasNeighbourInCluster(v,C_j))
		   ){
			printf("%d %d %d Move is not viable.\n", v, C_i, C_j);
			while(1);
		}
		
		MoveList.Advance();
	}
	
	//check that all moves are present.
	for(int vv=0; vv<Order; vv++){
		for(int cc=0; cc<NumClust; cc++){
			if(HasNeighbourInCluster(vv,cc) && cc != WhichCluster[vv]){
				if(!MoveIsInMovesOf(vv,cc)){
					printf("Move %d to %d should exist, but doesn't.\n", vv, cc);
					while(1);
				}
			}
		}
		if(GhostCluster >= 0 && ClusterSize[WhichCluster[vv]] > 1){
			if(!MoveIsInMovesOf(vv,-2)){
				printf("Move %d to GhostCluster should exist, but doesn't.\n", vv);
				while(1);
			}					
		}
	}
	
}






//*****************************************************************************
/* AddAScaledMove**************************************************************
 ******************************************************************************/
//Adds a move to the MoveList, updating FirstWithCost, NumWithCost, MovesTo,
//and MovesOf. Special case for Ghost move is c=NumClust.
void Graph::AddAScaledMove(int v, int C_j)
{
	
	
	//find the cost of the move
	double cost = GetScaledCostForMove(v, C_j);
	
	//Fill the list
	if(MoveList.Head->Next == NULL){//if the list is empty (i.e. just a sentinel node)
		
		FirstWithCost[Order+ (int)ceil(cost)] = MoveList.InsertANodeBefore(MoveList.Head);
		
	}else{//The list is NOT empty
		
		if(NumWithCost[Order+ (int)ceil(cost)] == 0){//If this is the first one with this cost
			if(SANITY_CHECK && FirstWithCost[Order+(int)ceil(cost)])
				printf("SANITY CHECK: AddAScaledMove error 1.\n");
			
			
			int costTick = (int)ceil(cost);
			while(!FirstWithCost[Order+ (costTick)])
				costTick--;//costTick is the greatest smaller nonempty cost.
			//possibly -Order.
			
			MoveList.CurrentPtr = FirstWithCost[Order+ (costTick)];
			for(int ii=0; ii<NumWithCost[Order+ costTick]; ii++){
				MoveList.Advance();
			}//we are now at the first of the > costTick nodes.
			
			if(costTick == -Order){
				FirstWithCost[Order+ (int)ceil(cost)] = 
				MoveList.InsertANodeBefore(MoveList.CurrentPtr);//currentPtr should be head.
			} else { //we insert before.
				FirstWithCost[Order+ (int)ceil(cost)] = 
				MoveList.InsertANodeBefore(MoveList.CurrentPtr);
			}
			
		}else{//If there was already a node with this cost
			if(SANITY_CHECK && !FirstWithCost[Order+(int)ceil(cost)]){
				printf("SANITY CHECK: AddAScaledMove error 2. %d, %f\n",(int)ceil(cost),cost);
				while(1);
			}
			FirstWithCost[Order+ (int)ceil(cost)] = 
			MoveList.InsertANodeBefore(FirstWithCost[Order+ (int)ceil(cost)]);
			
		}
	}//end if(listIsEmpty)
	
	//Set the data for the new move.
	FirstWithCost[Order+ (int)ceil(cost)]->Vertex = v;
	if(C_j<NumClust)
		FirstWithCost[Order+ (int)ceil(cost)]->To = C_j;
	else//it's a move to the ghost cluster.
		FirstWithCost[Order+ (int)ceil(cost)]->To = -2;
	FirstWithCost[Order+ (int)ceil(cost)]->Cost = cost;
	
	//Set the pointers to the new move.
	MovesOf[v].AddANode();
	MovesOf[v].Tail->Ptr = FirstWithCost[Order + (int)ceil(cost)];
	MovesTo[C_j].AddANode();
	MovesTo[C_j].Tail->Ptr = FirstWithCost[Order + (int)ceil(cost)];
	
	FirstWithCost[0] = MoveList.Head; // maintain the sentinel.
	
	NumWithCost[Order+ (int)ceil(cost)] ++;
	NumMoves++;
	NumMovesTo[C_j]++;
	NumMovesOf[v]++;
	
	
	int i=Order+(int) ceil(cost);
	if(SANITY_CHECK && (NumWithCost[i]==0) != (FirstWithCost[i]==NULL)){
		printf("Problem with First/Num: %d.\n", i); while(1);
	}
	
}

