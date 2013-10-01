#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <fstream>
#include "graph.h"
#include "definitions.h"
#include "miscFunctions.h"

FILE *fStream;

/* constructor ***************************************************************/
Graph::Graph()
{ 
  //long Order;
	//Order = -1;
	//long EdgeOrder;
	//EdgeOrder = 0;
	//long TabuCount;
	//TabuCount = 0;
	//	long NUM_CLUST;
	//	NUM_CLUST = 0;
}

/* adjacency list, etc. constructor. requires known order. *******************/
long Graph::HaveOrderConstructor()
{
	int i,j;


	AdjList = new SLList [Order];
	Degree = new long [Order];
	WhichCluster = new long [Order];
	ClusterList = new SLList [NUM_CLUST];
	ClusterEdgeCount = new long [NUM_CLUST];



	ClusterSize = new long [NUM_CLUST];
	for(j=0;j<NUM_CLUST;j++){
		ClusterSize[j]=0;
		ClusterEdgeCount[j]=0;
	}



	for(i=0;i<Order;i++){
		Degree[i] = 0;
	}

	return Order;
}


/* destructor ****************************************************************/
Graph::~Graph()
{
	delete [] AdjList;
	delete [] ClusterList;
	delete [] WhichCluster;
	//	delete [] CostMatrix;
	//	delete [] AdjListPtr;
	//	delete [] ClusterListSize;
	delete [] ClusterSize;

}


void Graph::SetNumClust(long num)
{
	NUM_CLUST = num;
}



//*****************************************************************************
/* CheckAdjList****************************************************************
******************************************************************************/
// Checks that the adjacency list conforms to input protocol.
bool Graph::CheckAdjList()
{
	long i;
	bool checkHead=true, checkDiag=true, checkMax=true;

	for(i=0;i<Order;i++){

		AdjList[i].RewindToHead();
		if((AdjList[i].Head->Data != i) && checkHead){
			fprintf(stderr,"Input Error: Vertices should be 0 to n-1, in order. Vertex %d does not comply.\n",(int) i);
			AdjList[i].PrintList();
			checkHead = false;
		}

		while(AdjList[i].CurrentPtr != AdjList[i].Tail){
			AdjList[i].Advance();

			if((AdjList[i].CurrentPtr->Data < i) && checkDiag){
				fprintf(stderr,"Input Error: Adjacency list should be in upper-diagonal form. Vertex %d should not be in the list for vertex %d.\n", (int) AdjList[i].CurrentPtr->Data, (int) i);    
				checkDiag = false;
			}

			if((AdjList[i].CurrentPtr->Data > Order) && checkMax){
				fprintf(stderr,"Input Error: You have only %d vertices, but vertex %d is listed as being adjacent to vertex %d.\n", (int) Order, (int) AdjList[i].CurrentPtr->Data, (int) i);
				checkMax = false;
			}

		}
		
	}
	printf("Graph checked.\n");
	return (checkHead && checkDiag && checkMax);
}

/*****************************************************************************/
// Fill the list up so that it is symmetric
//
//Also sets EdgeOrder and Degree.
bool Graph::FillAdjList()
{
	long i, data;
	for(i=0;i<Order;i++){
		AdjList[i].RewindToHead();
		while(AdjList[i].CurrentPtr != AdjList[i].Tail){
			AdjList[i].Advance();
			data = AdjList[i].CurrentPtr->Data; 
			if(data > i){
				AdjList[data].AddANode();
				AdjList[data].Tail->Data = i;

				Degree[i]++;
				Degree[data]++;
				EdgeOrder ++;
			}
		}
	}

	printf("List filled. %d vertices and %d edges.\n",(int) Order, (int) EdgeOrder);
	return true;
}




/*****************************************************************************/
// Takes the next numerical token from fStream (in scope). Returns -2 on EOF.
//
long getNumToken()
{
	char currentChar;
	long longToken;
	int tokenPtr = 0;
	char *token = new char[32];

	//Advance until a digit is found. If a - is found, keep it.
	currentChar = '.';
	while(!isdigit(currentChar)){
		currentChar = getc(fStream);
		if(currentChar == EOF) return -2; 
		if(currentChar== '-') {token[tokenPtr++] = currentChar;}
	}

	//Write the number to the token string.
	while(isdigit(currentChar)){
		token[tokenPtr++] = currentChar;
		currentChar = getc(fStream);
		if(currentChar == EOF) return -2; 
	}

	//Add the terminating character
	token[tokenPtr] = '\0';

	//We now have the token string.
	//printf("Returning \"%s\".\n",token);
	longToken = atol(token);
	delete [] token;
	return longToken;
}


//*****************************************************************************
/* ReadGraph*******************************************************************
******************************************************************************/
long Graph::ReadGraph(const char *fName)
{

	//printf("Filename %s\n", fName);

	Order = 0;
	int currentVertex =0;
	long data=0;

	char currentChar = '.';

  	if((fStream = fopen(fName,"r"))){ //read file open

		//Count the number of vertices, then reset the filestream.
		while(currentChar != EOF){
			if(currentChar == '-') Order++;
			currentChar = getc(fStream);
		}
		rewind(fStream);
		//printf("Read graph order as %d",(int) Order);
		HaveOrderConstructor(); //Now allocate the graph's memory!

		while(true){
			data = getNumToken();
			if(data==-2){//-2 is the end of file token
//								printf("JUMPING! %d %d\n",(int)data, (int)currentVertex);
				break;
			} else if(data<0){
				AdjList[currentVertex].DeleteANode(AdjList[currentVertex].Tail);
				currentVertex++; //printf("NEW VERTEX\n");
			} else {
				AdjList[currentVertex].CurrentPtr->Data = data;
				AdjList[currentVertex].AddANode();
				AdjList[currentVertex].Advance();
				//				printf("(%d) ",(int) data);
			}

		}

		fclose(fStream);
	} else {//input file read failed.
		fprintf(stderr,"Graph file open failed.\n");
		return -1;
	}//if else

	return Order;
}





//*****************************************************************************
/* MakeGraph*******************************************************************
******************************************************************************/
long Graph::MakeGraph(const char *fName)
{
	return ReadGraph(fName);
}



//*****************************************************************************
/* FillClusterList*************************************************************
******************************************************************************/
void Graph::FillClusterList()
{
	delete [] ClusterList;
	ClusterList = new SLList [NUM_CLUST];
	long vertex, cluster;
	for(vertex=0; vertex < Order; vertex++){
		ClusterList[WhichCluster[vertex]].Tail->Data = vertex;
		ClusterList[WhichCluster[vertex]].AddANode();
	}

	for(cluster=0; cluster < NUM_CLUST; cluster++){
		ClusterList[cluster].DeleteANode(ClusterList[cluster].Tail);
	}
}







//*****************************************************************************
/* FillClusterListSize*********************************************************
******************************************************************************/
//Also fills ClusterEdgeCount.
//Also initializes CurrentCost.
void Graph::FillClusterListSize()
{
	long i,j;
	for(j=0;j<NUM_CLUST;j++){
		ClusterEdgeCount[j]=0;
	}

	for(i=0;i<Order;i++){
		AdjList[i].RewindToHead();
		while(AdjList[i].CurrentPtr != AdjList[i].Tail){
			AdjList[i].Advance();
			if(WhichCluster[i]==WhichCluster[AdjList[i].CurrentPtr->Data])
				ClusterEdgeCount[WhichCluster[AdjList[i].CurrentPtr->Data]]++;
		}
	}

	for(i=0;i<NUM_CLUST;i++){
		ClusterEdgeCount[i] /= 2;
	}

}







//*****************************************************************************
/* InitClustering********************************************************
******************************************************************************/
void Graph::InitClustering(int * whichClusterMCL)
{
	long vertex, cluster;

	for(cluster=0; cluster< NUM_CLUST; cluster++){
		ClusterSize[cluster]=0;
	}
	for(vertex=0; vertex < Order; vertex++){
		WhichCluster[vertex] = whichClusterMCL[vertex];
		ClusterSize[WhichCluster[vertex]]++;
	}

	FillClusterList();
	FillClusterListSize();
}

