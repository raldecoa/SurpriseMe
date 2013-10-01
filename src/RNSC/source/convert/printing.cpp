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
#include "linkedList.h"
#include "graph.h"
#include "definitions.h"
#include "miscFunctions.h"



void Graph::PrintAdjList()
{
   for(int i=0; i<Order; i++){
      AdjList[i].RewindToHead();
      printf("%d{%d}:  ",i,WhichCluster[i]);
      while(AdjList[i].CurrentPtr != AdjList[i].Tail)
	 {
	    AdjList[i].Advance();
	    if(AdjList[i].CurrentPtr == AdjHead2[i])
	       printf(" #%d{%d}",AdjList[i].CurrentPtr->Vertex, WhichCluster[AdjList[i].CurrentPtr->Vertex]);
	    else
	       printf(" > %d{%d}",AdjList[i].CurrentPtr->Vertex, WhichCluster[AdjList[i].CurrentPtr->Vertex]);
	 }
      printf("\n");
   }
}

void Graph::CheckAdjList()
{
	int countToDegree;
	bool inNeighbours;
	for(int v=0; v<Order; v++){
		countToDegree=0;
		inNeighbours=false;
		
		AdjList[v].RewindToHead(); AdjList[v].Advance();
		while(AdjList[v].CurrentPtr){
			countToDegree++;
			if(AdjList[v].CurrentPtr == AdjHead2[v]){
				inNeighbours=true;
			}
			if(WhichCluster[AdjList[v].CurrentPtr->Vertex] == WhichCluster[v] && !inNeighbours){
				printf("SANITY_CHECK: Problem in Graph::CheckAdjList 1 (in %d).\n", v); while(1);}
			if(WhichCluster[AdjList[v].CurrentPtr->Vertex] != WhichCluster[v] && inNeighbours){
				printf("SANITY_CHECK: Problem in Graph::CheckAdjList 2.\n"); getchar(); while(1);}
			
			if(AdjList[v].CurrentPtr == AdjList[v].Tail) break; AdjList[v].Advance(); 
			
		}
		
		if(countToDegree != Degree[v]){
			printf("SANITY_CHECK: Problem in Graph::CheckAdjList. Degree is %d, not %d.\n",
				   Degree[v], countToDegree);
			while(1);
		}
		
		
	}
	
	
	
}

void Graph::PrintClusters()
{
	printf("CLUSTER LISTS:\n");
	for(int c = 0; c<NumClust; c++){
		ClusterList[c].PrintList();
	}
	
}

void Graph::PrintMovesTo()
{
	printf("\"MOVES TO\" LISTS:\n");
	for(int c = 0; c<NumClust; c++){
		MovesTo[c].PrintList();
	}
	printf("Ghost cluster is %d...\n",GhostCluster);
	MovesTo[NumClust].PrintList();
}

void Graph::PrintMovesOf()
{
	printf("\"MOVES OF\" LISTS:\n");
	for(int v = 0; v<Order; v++){
		MovesOf[v].PrintList();
	}
}

void Graph::CheckFirsts()
{
	for(int i=0; i<2*Order; i++){
		printf("%4d | %4d | %f\n", i-Order, NumWithCost[i], FirstWithCost[i]->Cost);
	}
}

void Graph::PrintNumWithCost()
{
	printf("     ");
	for(int i=0; i<Order; i++){
		printf("%3d",i);
	}
	printf("\n+    ");
	for(int i=0; i<Order; i++){
		printf("%3d",NumWithCost[Order+i]);
	}
	printf("\n-    ");
	for(int i=0; i>-Order; i--){
		printf("%3d",NumWithCost[Order+i]);
	}
	printf("\n\n");
}

