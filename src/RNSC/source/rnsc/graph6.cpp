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



//Returns whether or not the move is already in MovesTo
bool Graph::MoveIsInMovesTo(int vertex, int cluster)
{
   if(MovesTo[cluster].Head == NULL) return false;
   MovesTo[cluster].RewindToHead();
   while(1){
      if(MovesTo[cluster].CurrentPtr->Ptr->Vertex == vertex) return true;

      if(MovesTo[cluster].CurrentPtr == MovesTo[cluster].Tail) break;
      MovesTo[cluster].Advance();
   }
   return false;
}


//Returns whether or not the move is already in MovesFrom
bool Graph::MoveIsInMovesOf(int vertex, int to)
{
   if(MovesOf[vertex].Head == NULL) return false;
   MovesOf[vertex].RewindToHead();
   while(1){
      if(MovesOf[vertex].CurrentPtr->Ptr->To == to) 
	 return true;

      if(MovesOf[vertex].CurrentPtr == MovesOf[vertex].Tail) break;
      MovesOf[vertex].Advance();
   }
   return false;
}


void Graph::UpdateAdjVector()
{//Set up the adjacency vector (AdjVector) so that it reflects the adjacency to v, the
   //vertex being moved. (i.e. Move[0]).
   ListPtr vPtr = AdjList[Move[0]].Head->Next;
   while(vPtr){
      AdjVector[vPtr->Vertex] = true;
      vPtr = vPtr->Next;
   }
}

void Graph::NullifyAdjVector()
{//Reset the adjacency vector (AdjVector) so that every entry is FALSE.
   ListPtr vPtr = AdjList[Move[0]].Head->Next;
   while(vPtr){
      AdjVector[vPtr->Vertex] = false;
      vPtr = vPtr->Next;
   }

   if(SANITY_CHECK){//make sure it's null.
      for(int i=0; i<Order; i++){
	 if(AdjVector[i])
	    printf("SANITY_CHECK: AdjVector should be null, but it isn't.\n");
      }
   }

}


bool Graph::HasNeighbourInCluster(int v, int c)
{//Does v have a neighbour in cluster c?
   ListPtr vPtr;
   if(WhichCluster[v] == c){
      vPtr = AdjHead2[v];

      while(vPtr){
	 if(WhichCluster[vPtr->Vertex] == c) return true;
	 vPtr = vPtr->Next;
      }		
   } else {
      vPtr = AdjList[v].Head->Next;
      while(vPtr != NULL && vPtr != AdjHead2[v]){
	 if(WhichCluster[vPtr->Vertex] == c) return true;
	 vPtr = vPtr->Next;
      }
   }
   return false;
}

bool Graph::IsInAdjListOf(int v, int u)
{//is u in AdjList[v]?
   ListPtr Ptr = AdjList[v].Head->Next;
   while(Ptr != NULL){
      if(Ptr->Vertex == u) return true;
      Ptr = Ptr->Next;
   }
   return false;
}

int Graph::GetClusterDegree(int v, int c)
{//how many neighbours does v have in cluster c?
   int ClusterDegree = 0;
   ListPtr Ptr = AdjList[v].Head->Next;
   while(Ptr != NULL){
      if(WhichCluster[Ptr->Vertex] == c) ClusterDegree ++;
      Ptr = Ptr->Next;
   }
   return ClusterDegree;
}

//Removes v->c from MoveList, MovesTo[c], and MovesOf[v].
//To delete a ghost move, let c=Order.
void Graph::NixMove(int v, int c)
{
   using namespace std;

   int ClustNum;
   if(c == NumClust) ClustNum=-2;
   else ClustNum=c;
   MovesOf[v].RewindToHead();
   while(MovesOf[v].CurrentPtr->Ptr->To != ClustNum) MovesOf[v].Advance();
   MovesTo[c].RewindToHead();
   while(MovesTo[c].CurrentPtr->Ptr->Vertex != v) MovesTo[c].Advance();
   
   int cost = (int) ceil(MovesTo[c].CurrentPtr->Ptr->Cost);
   
   if(MovesTo[c].CurrentPtr->Ptr == FirstWithCost[Order + cost]){
      //We need to adjust the FirstWithCost pointer.
      
      if((int) ceil(FirstWithCost[Order + cost]->Next->Cost) == cost){
	 FirstWithCost[Order + cost] = FirstWithCost[Order + cost]->Next;
      } else {
	 FirstWithCost[Order + cost] = NULL;
      }
      
   }
   
   
   NumWithCost[Order + cost] --;
   
   if(SANITY_CHECK){
      if(MovesOf[v].CurrentPtr->Ptr->To != ClustNum  ||  MovesOf[v].CurrentPtr->Ptr->Vertex != v){
	 printf("SANITY_CHECK: Error in NixMove for %d,%d.\n", v, c); while(1);}
      if(MovesOf[v].CurrentPtr->Ptr->To != ClustNum  ||  MovesTo[c].CurrentPtr->Ptr->Vertex != v){
	 printf("SANITY_CHECK: Error in NixMove for %d,%d.\n", v, c); while(1);}

   }

   
   MoveList.DeleteANode(MovesTo[c].CurrentPtr->Ptr);
   MovesOf[v].DeleteANode(MovesOf[v].CurrentPtr);
   MovesTo[c].DeleteANode(MovesTo[c].CurrentPtr);
   
   FirstWithCost[0] = MoveList.Head;


   int i=Order+cost;
   if((NumWithCost[i]==0) != (FirstWithCost[i]==NULL)){
      printf("Problem with First/Num: %d.\n", i); while(1);
   }

   
   NumMoves--;
   NumMovesOf[v]--;
   NumMovesTo[c]--;
}
