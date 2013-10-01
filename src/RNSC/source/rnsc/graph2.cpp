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
/* InitNaiveScheme*************************************************************
******************************************************************************/
// Initialize MoveList, FirstWithCost, NumWithCost, NaiveCost.
//
void Graph::InitNaiveScheme()
{
   //THE LIST SHOULD AT THIS POINT BE ONE NODE, A SENTINEL FOR THE END OF THE LIST.

   int C_i, C_j;

   //list information
   for(int i=0; i<2*Order+1; i++){
      NumWithCost[i] = 0;
      FirstWithCost[i] = NULL;
   }
   FirstWithCost[0] = MoveList.Head;//this pointer should always remain at the head.

   for(int v=0; v<Order; v++){
      ToGhostIsNew = true;
      C_i = WhichCluster[v];

      for(int c=0; c<NumClust; c++){//Reset MoveIsNew. No clusters have been used for v.
	 MoveIsNew[c]=true;
      }
      AdjList[v].RewindToHead();

      //for each neighbour of v, we want to make a move to the neighbour's cluster without duplicating.
      do{
	 if( ToGhostIsNew && GhostCluster >= 0 && ClusterSize[C_i] > 1 ){
	    //special first case. move v to a singleton cluster.
	    C_j = NumClust;
	    ToGhostIsNew = false;
	 } else {
	    AdjList[v].Advance();
	    C_j = WhichCluster[AdjList[v].CurrentPtr->Vertex];
	 }

	 if((MoveIsNew[C_j] || C_j == NumClust) && (C_i != C_j)){
	    AddANaiveMove(v,C_j);
	    MoveIsNew[C_j] = false;
	 }

      }while(AdjList[v].CurrentPtr != AdjList[v].Tail);//end do while
   }//end for v


   //initialize naive cost.
   NaiveCost = EdgeOrder;
   for(int i=0;i<NumClust;i++){
      NaiveCost -= 2*ClusterEdgeOrder[i];
      NaiveCost += nChoose2(ClusterSize[i]);
   }
}



//*****************************************************************************
/* InitScaledScheme************************************************************
******************************************************************************/
// Initialize MoveList, FirstWithCost, NumWithCost, 
//
void Graph::InitScaledScheme()
{
   //THE LIST SHOULD AT THIS POINT BE ONE NODE, A SENTINEL FOR THE END OF THE LIST.

   int C_i, C_j, ClusterDegree;

   //initialize alpha_v and beta_v. Also initialize scaled cost.
   ScaledCost = 0;
   for(int i=0; i<Order; i++){
      ClusterDegree = GetClusterDegree(i, WhichCluster[i]);
      CostNumerator[i] = Degree[i] + ClusterSize[WhichCluster[i]]-1 - 2*ClusterDegree;
      CostDenominator[i] = Degree[i] + ClusterSize[WhichCluster[i]] - ClusterDegree;
      ScaledCost += (double) CostNumerator[i]/CostDenominator[i];
   }
   ScaledCost *= ScaledMultiplier;


   //list information
   for(int i=0; i<2*Order+1; i++){
      NumWithCost[i] = 0;
      FirstWithCost[i] = NULL;
   }
   FirstWithCost[0] = MoveList.Head;//this pointer should always remain at the head.


   for(int v=0; v<Order; v++){
      ToGhostIsNew = true;
      C_i = WhichCluster[v];

      for(int c=0; c<NumClust; c++){//Reset MoveIsNew. No clusters have been used for v.
	 MoveIsNew[c]=true;
      }
      AdjList[v].RewindToHead();

      //for each neighbour of v, we want to make a move to the neighbour's cluster without duplicating.
      do{
	 if( ToGhostIsNew && GhostCluster >= 0 && ClusterSize[C_i] > 1 ){
	    //special first case. move v to a singleton cluster.
	    C_j = NumClust;
	    ToGhostIsNew = false;
	 } else {
	    AdjList[v].Advance();
	    C_j = WhichCluster[AdjList[v].CurrentPtr->Vertex];
	 }

	 if((MoveIsNew[C_j] || C_j == NumClust) && (C_i != C_j)){
	    AddAScaledMove(v,C_j);
	    MoveIsNew[C_j] = false;
	 }

      }while(AdjList[v].CurrentPtr != AdjList[v].Tail);//end do while
   }//end for v

}




//*****************************************************************************
/* GetScaledCostForClustering**************************************************
******************************************************************************/
// This is to get the scaled cost when the scaled scheme is not run.
double Graph::GetScaledCostForClustering(){
  //int ClusterDegree;
   ScaledCost = 0;
   for(int i=0; i<Order; i++){
     //ClusterDegree = GetClusterDegree(i, WhichCluster[i]);
      CostNumerator[i] = Degree[i] + ClusterSize[WhichCluster[i]]-1 - 2*GetClusterDegree(i, WhichCluster[i]);
      CostDenominator[i] = Degree[i] + ClusterSize[WhichCluster[i]] - GetClusterDegree(i, WhichCluster[i]);
      ScaledCost += (double) CostNumerator[i]/CostDenominator[i];
   }
   ScaledCost *= ScaledMultiplier;
   return ScaledCost;
}



//*****************************************************************************
/* GetScaledCostForMove********************************************************
******************************************************************************/
//This function is THE load bearing function for the scaled scheme.

// Simply returns the cost of moving 'vertex' to 'cluster'.
double Graph::GetScaledCostForMove(int vertex, int C_j)
{
   if(SANITY_CHECK){//make sure VectorForScaledCost is null.
      for(int i=0; i<Order; i++){
	 if(VectorForScaledCost[i] != false){
	    printf("SANITY_CHECK in GetScaledCostForMove: VectorForScaledCost (v=%d) is not initially null: %d\n", vertex, i);
	 }
      }
   }

   //initialize VectorForScaledCost.
   ListPtr VertAdjPtr = AdjList[vertex].Head->Next;
   while(VertAdjPtr){
      VectorForScaledCost[VertAdjPtr->Vertex] = true;
      VertAdjPtr = VertAdjPtr->Next;
   }


	
   //If the move creates a new singleton cluster, run a special version of the function.
   if(C_j == NumClust || C_j == -2){
      double ReturnValue = GetScaledCostForGhostMove(vertex);
      
      //clear VectorForScaledCost
      VertAdjPtr = AdjList[vertex].Head->Next;
      while(VertAdjPtr){
	 VectorForScaledCost[VertAdjPtr->Vertex] = false;
	 VertAdjPtr = VertAdjPtr->Next;
      }
      
      if(SANITY_CHECK){//make sure VectorForScaledCost is null.
	 for(int i=0; i<Order; i++){
	    if(VectorForScaledCost[i] != false){
	       printf("SANITY_CHECK in GetScaledCostForMove (gh): VectorForScaledCost (v=%d) is not null on exit: %d\n",vertex,i);
	    }
	 }
      }
      return ReturnValue;
   }


   //the vertex being moved is CURRENTLY IN C_i.
   int u;//u in C_i or C_j
   int C_i = WhichCluster[vertex];
   double cost;//the cost of the move.
   int newNum, newDom;//new numerator and denominator for v.

   ListPtr Ptr_i = ClusterList[C_i].CurrentPtr;
   ListPtr Ptr_j = ClusterList[C_j].CurrentPtr;

   ClusterList[C_i].RewindToHead();
   ClusterList[C_j].RewindToHead();


   //make the cost equal to the cost incurred by v.
   int ClusterDegree = GetClusterDegree(vertex, C_j);
   newNum = Degree[vertex] + ClusterSize[C_j] + 1 - 1 - 2*ClusterDegree;
   newDom = Degree[vertex] + ClusterSize[C_j] + 1 - ClusterDegree;
   cost = (double) (-CostNumerator[vertex]*newDom + newNum*CostDenominator[vertex]) /
      (CostDenominator[vertex]*newDom);
   
   for(int i=0; i<ClusterSize[C_i]; i++){//u in C_i
      u = ClusterList[C_i].CurrentPtr->Vertex;
      if(u != vertex){
	 if(VectorForScaledCost[u]){ //equiv to if(AdjMatrix[u][vertex]){
	    // + 1/B_u
	    if(CostDenominator[u] < LookupBound){//implies CostNumerator[u] < LookupBound.
	       cost += LUT_InvB[CostDenominator[u]];
	    }else{
	       cost += (double) 1/CostDenominator[u];
	    }
	 }else{
	    // + (a_u - B_u)/(B_u^2 - B_u)
	    if(CostDenominator[u] < LookupBound){//implies CostNumerator[u] < LookupBound.
	       cost += LUT_BigNeg[CostNumerator[u]][CostDenominator[u]];
	    }else{
	       cost += (double) (CostNumerator[u] - CostDenominator[u])/
		  (CostDenominator[u]*CostDenominator[u] - CostDenominator[u]);
	    }
	 }
      }
      ClusterList[C_i].Advance();
   }
   
   for(int i=0; i<ClusterSize[C_j]; i++){//u in C_j
      u = ClusterList[C_j].CurrentPtr->Vertex;
      if(VectorForScaledCost[u]){ //equiv to if(AdjMatrix[u][vertex]){
	 // - 1/B_u
	 if(CostDenominator[u] < LookupBound){//implies CostNumerator[u] < LookupBound.
	    cost -= LUT_InvB[CostDenominator[u]];
	 }else{
	    cost -= (double) 1/CostDenominator[u];
	 }
      }else{
	 // + (B_u - a_u)/(B_u^2 + B_u)
	 if(CostDenominator[u] < LookupBound){//implies CostNumerator[u] < LookupBound.
	    cost += LUT_BigPos[CostNumerator[u]][CostDenominator[u]];
	 }else{
	    cost += (double) (CostDenominator[u] - CostNumerator[u])/
	       (CostDenominator[u]*CostDenominator[u] + CostDenominator[u]);
	 }
      } 
      ClusterList[C_j].Advance();
   }
   
   cost *= ScaledMultiplier; //recall that ScaledMultiplier is typically (n-1)/3.
   if(cost>Order || cost < -Order){
      printf("Major problem. Out-of-bounds cost %f.\n", (float) cost);
      
   }
	
   ClusterList[C_i].CurrentPtr = Ptr_i;//Return the cluster pointer to its original position.
   ClusterList[C_j].CurrentPtr = Ptr_j;//Return the cluster pointer to its original position.


   //clear VectorForScaledCost
   VertAdjPtr = AdjList[vertex].Head->Next;
   while(VertAdjPtr){
      VectorForScaledCost[VertAdjPtr->Vertex] = false;
      VertAdjPtr = VertAdjPtr->Next;
   }

   if(SANITY_CHECK){//make sure VectorForScaledCost is null.
      for(int i=0; i<Order; i++){
	 if(VectorForScaledCost[i] != false){
	    printf("SANITY_CHECK in GetScaledCostForMove: VectorForScaledCost (v=%d) is not null on exit: %d\n",vertex,i);
	 }
      }
   }

   return cost;
	
}





//*****************************************************************************
  /* GetScaledCostForGhostMove***************************************************
******************************************************************************/
// Simply returns the cost of moving 'vertex' to an empty cluster.
//This function is THE load bearing function for the scaled scheme.
double Graph::GetScaledCostForGhostMove(int vertex)
{
   //the vertex being moved is CURRENTLY IN C_i.
   int u;//u in C_i or C_j
   int C_i = WhichCluster[vertex];
   double cost;//the cost of the move.
   int newNum, newDom;//new numerator and denominator for v.

   ListPtr Ptr_i = ClusterList[C_i].CurrentPtr;

   ClusterList[C_i].RewindToHead();
	
   //make the cost equal to the cost incurred by v.
   newNum = Degree[vertex];
   newDom = Degree[vertex] + 1;
   cost = (double) (-CostNumerator[vertex]*newDom + newNum*CostDenominator[vertex]) /
      (CostDenominator[vertex]*newDom);
	
   for(int i=0; i<ClusterSize[C_i]; i++){//u in C_i
      u = ClusterList[C_i].CurrentPtr->Vertex;
      if(u != vertex){
	 if(VectorForScaledCost[u]){ //equiv to if(AdjMatrix[u][vertex]){
	    // + 1/B_u
	    if(CostDenominator[u] < LookupBound){//implies CostNumerator[u] < LookupBound.
	       cost += LUT_InvB[CostDenominator[u]];
	    }else{
	       cost += (double) 1/CostDenominator[u];
	    }
	 }else{
	    // + (a_u - B_u)/(B_u^2 - B_u)
	    if(CostDenominator[u] < LookupBound){//implies CostNumerator[u] < LookupBound.
	       cost += LUT_BigNeg[CostNumerator[u]][CostDenominator[u]];
	    }else{
	       cost += (double) (CostNumerator[u] - CostDenominator[u])/
		  (CostDenominator[u]*CostDenominator[u] - CostDenominator[u]);
	    }
	 }
      }
      ClusterList[C_i].Advance();
   }
	
   cost *= ScaledMultiplier; //recall that ScaledMultiplier is typically (n-1.1)/3.
   if(SANITY_CHECK && (cost>Order || cost < -Order))
      printf("Major problem. Out-of-bounds cost %f.\n", (float) cost);		
   
   
   ClusterList[C_i].CurrentPtr = Ptr_i;//Return the cluster pointer to its original position.
	
   return cost;
	
}




//*****************************************************************************
/* PurgeMoveList***************************************************************
******************************************************************************/
// Purge the move list. Leave only the sentinel.
// Also completely empties all MovesTo and MovesOf lists.
void Graph::PurgeMoveList()
{
   while(MoveList.Head != MoveList.Tail){
      MoveList.DeleteANode(MoveList.Tail);
   }
   //	MoveList.AddANode();
   MoveList.Head->Vertex = -1;
   MoveList.Head->To = -1;
   MoveList.Head->Cost = BIG_COST;


   for(int v = 0; v<Order; v++){
      while(MovesOf[v].Head != NULL)
	 MovesOf[v].DeleteANode(MovesOf[v].Head);
      NumMovesOf[v] = 0;
   }
   for(int c = 0; c<NumClust+1; c++){
      while(MovesTo[c].Head != NULL)
	 MovesTo[c].DeleteANode(MovesTo[c].Head);
      NumMovesTo[c]=0;
   }

   //We don't need to worry about FirstWithCost and NumWithCost; they are dealt
   //with when the move list is filled.

}
