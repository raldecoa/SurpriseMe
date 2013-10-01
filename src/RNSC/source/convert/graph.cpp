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
#include <iostream>
#include <cstring>
#include "linkedList.h"
#include "math.h"
#include "graph.h"
#include "definitions.h"
#include "miscFunctions.h"


/* constructor ***************************************************************/
Graph::Graph()
{ 
	//initialize scalars.
	Order = -1;
	NumClust = -1;
	EdgeOrder = -1;
	NaiveCost = -1;
	ScaledCost = -1;
	TabuLength = 1;
	TabuCount = -1;
	TabuTol = 1;
	ScaledMultiplier = -1;
	HighestCostUsed = 0;
	LowestCostUsed = 0;
	Move[0]= -1; Move[1]= -1; Move[2] = -1;
	NumEmptyClusters = -1;
	GhostCluster = -1;
	LookupBound = -1;
	
	MovePtr = NULL;
	
	ClusteringFN = "";
	OutputFN = "";
	
}

/* the real constructor, called after we have ********************************
 the number of vertices and clusters. ****************************************/
void Graph::Initialize()
{
	if(SILENCE<=2) printf("Initializing graph...");
	
	//if no maximum number of clusters is prescribed in the command line, set it to
	//the number of vertices in the graph.
	if(NumClust <= 1 || NumClust > Order) NumClust = Order;
	
	
	ScaledMultiplier = (float)((float) Order-1.1)/(3);//the .1 is to prevent borderline
	//floating-point errors with ceil(). remember, .1 isn't representable ;-)
	
	//vertex scalar information
	Degree = new int[Order];
	WhichCluster = new int[Order];
	ClusterListPtr = new ListPtr[Order];
	AdjHead2 = new ListPtr[Order];
	AdjVector = new bool[Order];
	VectorForScaledCost = new bool[Order];
	TabuVertices = new int[Order];
	CostNumerator = new int[Order];
	CostDenominator = new int[Order];
	for(int i=0; i<Order; i++){
		Degree[i]=0;
		WhichCluster[i]=-1;
		ClusterListPtr[i]=NULL;
		AdjHead2[i]=NULL;
		TabuVertices[i]=0;
		CostNumerator[i]=-1;
		CostDenominator[i]=-1;
	}
	
	
	//neighbourhood monitoring data
	NumMovesOf = new int[Order];
	NumMovesTo = new int[NumClust];
	for(int i=0; i<Order; i++) NumMovesOf[i]=0;
	for(int i=0; i<NumClust; i++) NumMovesTo[i]=0;
	
	
	//vertex list information
	AdjList = new SLList[Order];
	for(int i=0; i<Order; i++){
		AdjVector[i] = false;
		VectorForScaledCost[i] = false;
	}
	
	//cluster information
	ClusterSize = new int[NumClust];
	ClusterEdgeOrder = new int[NumClust];
	ClusterList = new SLList[NumClust];
	MoveIsNew = new bool[NumClust];
	MovesTo = new PLList[NumClust+1];
	MovesOf = new PLList[Order];
	for(int j=0; j<NumClust; j++){
		ClusterSize[j] = 0;
		ClusterEdgeOrder[j] = 0;
		MovesTo[j].DeleteANode(MovesTo[j].Head);
	}
	MovesTo[NumClust].DeleteANode(MovesTo[NumClust].Head);
	for(int j=0; j<Order; j++){
		MovesOf[j].DeleteANode(MovesOf[j].Head);
	}
	
	//list information
	FirstWithCost = new DListDoublePtr[2*Order+1];
	NumWithCost = new int[2*Order+1];
	for(int i=0; i<2*Order+1; i++){
		FirstWithCost[i] = NULL;
		NumWithCost[i] = 0;
	}
	
	
	//set sentinel in MoveList.
	MoveList.Head->Vertex = -1;
	MoveList.Head->To = -1;
	MoveList.Head->Cost = BIG_COST;
	
	//initialize tabu list.
	for(int i=0; i<TabuLength; i++){
		TabuList.AddANode();
	}
	TabuList.Tail->Next = TabuList.Head;
	if(SILENCE<=2) printf(" done.\n");
}


/* initialize the lookup tables.  requires knowing LookupBound. **************/
void Graph::InitializeLUT()
{
	
	//lookup tables
	LUT_InvB = new double[LookupBound];
	LUT_BigPos.assign(LookupBound,(double *) 0);
	LUT_BigNeg.assign(LookupBound,(double *) 0);
	for(int i=0; i<LookupBound; i++){
		LUT_InvB[i] = (double) 1/i;
		LUT_BigPos[i] = new double[LookupBound];
		LUT_BigNeg[i] = new double[LookupBound];
		for(int j=0; j<LookupBound; j++){
			LUT_BigPos[i][j] = (double) (j-i) / (j*j + j);
			LUT_BigNeg[i][j] = (double) (i-j) / (j*j - j);
		}
	}
}


/* destructor ****************************************************************/
//This is sloppy, but I'm a bad programmer, so there you go.
Graph::~Graph()
{
	
	TabuList.Tail->Next = NULL;
	/*
	 for(int v=0; v<Order; v++){
	 while(AdjList[v].Head != NULL){ AdjList[v].DeleteANode(AdjList[v].Head); }
	 }
	 
	 for(int c=0; c<NumClust; c++){
	 while(ClusterList[c].Head != NULL){ ClusterList[c].DeleteANode(ClusterList[c].Head); }
	 }
	 
	 delete [] ClusterList;
	 delete [] ClusterListPtr;
	 delete [] AdjList;
	 delete [] AdjVector;
	 delete [] VectorForScaledCost;
	 delete [] Degree;
	 delete [] WhichCluster;
	 delete [] AdjHead2;
	 delete [] ClusterSize;
	 delete [] ClusterEdgeOrder;
	 delete [] TabuVertices;
	 delete [] NumWithCost;
	 delete [] FirstWithCost;
	 delete [] CostNumerator;
	 delete [] CostDenominator;
	 */
}


/*****************************************************************************/
// getNumToken takes the next numerical token from fStream (in scope).
// Returns -2 on EOF.
//
long getNumToken(FILE *fStream)
{
	char currentChar;
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
	return atol(token);
}



//*****************************************************************************
/* ReadGraph*******************************************************************
 ******************************************************************************/
// Reads the graph file and gets the number of vertices by counting the
// number of -1 tokens in the file.
int Graph::ReadGraph()
{
	Order = 0;
	int token = -2;
	FILE * fStream;
	EdgeOrder = 0;
	
	if((fStream = fopen(GraphFN,"r"))){ //read file open
		while((token = getNumToken(fStream))>-2){
			if(token==-1){ Order++; }
		}
		
		Initialize();
		rewind(fStream);
		
		//init the adjacency list.
		for(int vertex = 0; vertex < Order; vertex++){
			AdjList[vertex].Head->Vertex = vertex;
		}
		for(int vertex = 0; vertex < Order; vertex++){
			vertex = getNumToken(fStream);
			while((token = getNumToken(fStream)) > vertex){
				if(!IsInAdjListOf(vertex, token)){
					//vertex is adjacent to token.
					AdjList[vertex].AddANode();
					AdjList[vertex].Tail->Vertex = token;
					Degree[vertex]++;
					
					//token is adjacent to vertex.
					AdjList[token].AddANode();
					AdjList[token].Tail->Vertex = vertex;
					Degree[token]++;
					
					//we have another edge.
					EdgeOrder++;
				}else{
					printf("Ignoring duplicate edge %d to %d.\n", vertex, token);
				}
				
			}
			if(token != -1){//something is wrong
				printf("Error in graph input file %s. Bailing out.\n",GraphFN); while(1){};
				return 0;
			}
		}
		
		LookupBound = 2*(int) ceil(sqrt((double) EdgeOrder));
		printf("LookupBound = %d.\n", LookupBound);
		InitializeLUT();
		
		fclose(fStream);
	} else {//input file read failed.
		fprintf(stderr,"Graph file open failed. (%s)\n", GraphFN);
		return -1;
	}//if else
	return Order;
}






//*****************************************************************************
/* WriteGraph****************************************************************
 ******************************************************************************/
// Writes the graph to GraphFN in RNSC format.
int Graph::WriteGraph()
{
	
	
	ofstream fStream(GraphFN);
	if(fStream){
		for(int vertex = 0; vertex < Order; vertex++){//for each vertex
			
//			fStream << vertex << " ";
			
			AdjList[vertex].RewindToHead();
			for(int v = 0; v<Degree[vertex]+1; v++){
				
				if( AdjList[vertex].CurrentPtr->Vertex >= vertex ){
					fStream << AdjList[vertex].CurrentPtr->Vertex << " ";
				}
				AdjList[vertex].Advance();
				
			}
			fStream << "-1" << endl;
			
		}	
		fStream.close();
	}
	else {
		fprintf(stderr,"Output file open failed. (%s)\n", GraphFN);
		return(-1);
	}

		
	return(1);
	
}


//*****************************************************************************
/* WriteNames  ****************************************************************
 ******************************************************************************/
// Writes the names file.
int Graph::WriteNames()
{
		
	ofstream fStream(NamesFilename);
	if(fStream){
		for(int vertex = 0; vertex < Order; vertex++){//for each vertex
			
			fStream << vertexName[vertex] << endl << Function[vertex] << endl;
			
		}	
		fStream.close();
	}
	else {
		fprintf(stderr,"Output file open failed. (%s)\n", NamesFilename);
		return(-1);
	}
	
	
	return(1);
	
	
}
	
//*****************************************************************************
/* ReadEdgeList****************************************************************
 ******************************************************************************/
// Reads an edge list in which each vertex has a sort of alphanumeric label.  Returns the number of vertices.
int Graph::ReadEdgeList()
{
	Order = 0;
	int token = -2;
	FILE * fStream;
	EdgeOrder = 0;
	char tokenWord [80];
	//	std::string * nameList;
	std::string currentName;
	int LoopCount = 0; int MultiEdgeCount = 0;
	
	int tokenCount =0;//number of tokens in the edge list.
	
	
	if((fStream = fopen(EdgeListFilename,"r"))){ //read file open
		
		//First job is to make a list of the graph labels.
		
		
		//Count the tokens in the list and use this number as an upper bound on the number of labels.
		while(1){
			token = fscanf(fStream,"%s", tokenWord);
			if(token <= 0) break;
			if(strlen(tokenWord)>1 || isdigit(tokenWord[0])) tokenCount++;//skip token if length 1 and not a number
		}
		fprintf(stdout, "Counted %d tokens.  Rewinding.\n", tokenCount);
		//nameList = new std::string[tokenCount];
		vertexName = new std::string[tokenCount];
		Function = new char[tokenCount];
		for(int i=0; i<tokenCount;i++){
			Function[i] = 'U';
		}
		
		
		//Rewind and read the tokens again, this time writing them uniquely to a list.
		rewind(fStream);
		tokenCount=0;
		int uniqueTokenCount=0;
		
		bool namesMatch;
		
		
		
		
		//While the buffer isn't EOF
		while(1){
			token = fscanf(fStream,"%s", tokenWord);
			if(token <= 0) break;//break if EOF

			
			if(strlen(tokenWord)==1 && isalpha(tokenWord[0])){//set a function.
				if(uniqueTokenCount > 0){
					Function[uniqueTokenCount-1]=toupper(tokenWord[0]);
					//cout << "The vertex " << uniqueTokenCount-1 << " should have label " <<  vertexName[uniqueTokenCount-1] << " and function " <<tokenWord[0] << endl;
																											
				}
			}
			if(strlen(tokenWord)>1 || isdigit(tokenWord[0])){//skip token if length 1 and not a number.
				
				currentName = tokenWord;
				namesMatch = false;
				
				//Check token for matches
				for(int i=0; i<uniqueTokenCount; i++){
					if(!currentName.compare(vertexName[i])){
						namesMatch=true;
						break;
					}
				}
				//If the token is new
				if(!namesMatch){
					vertexName[uniqueTokenCount] = currentName;
					uniqueTokenCount++;
				}
								
			}
			tokenCount++;
		}
		fprintf(stdout,"Read %d tokens and wrote %d unique tokens to memory.\n", tokenCount,uniqueTokenCount);
		//if(isodd( tokenCount)) fprintf(stderr, "Edge list token count is odd! \n");
		if(tokenCount%2==1) fprintf(stderr, "Edge list token count is odd! \n");
		int numEdges = (int) ((tokenCount+(tokenCount%2))/2);
		EdgeOrder = numEdges;//TEMPORARY.
		
		
		//Set the order as the number of names in the list.
		Order = uniqueTokenCount;
		
		//Initialize the graph.
		Initialize();
		
		//Now make another 2*n' array, being a numerical edge list.
		EdgeListA = new int[EdgeOrder];
		EdgeListB = new int[EdgeOrder];
		rewind(fStream);
		
		int verbalTokenCount = 0;
		for(int i=0; i<tokenCount; i++){
			
			token = fscanf(fStream,"%s", tokenWord);
			if(token <= 0){
				cout<< "Breaking for end of file.\n";
				break;//break if EOF
			}
						
			if(strlen(tokenWord)>1 || isdigit(tokenWord[0]))
			{//if token nontrivial
			
				//Find the matching vertex.
				currentName = tokenWord;
				bool matchedFlag=false;
				for(int j=0; j<Order; j++)
				{
					if(!currentName.compare(vertexName[j]))
					{//names match.
						if(verbalTokenCount%2==0)
						{//i is even
							EdgeListA[(int) verbalTokenCount/2]=j;
							//cout<< "Matched " << currentName << " with " << vertexName[j] << " " << (int) verbalTokenCount/2 << "\n";
						}
						else 
						{
							EdgeListB[(int) (verbalTokenCount-1)/2]=j;
							//cout<< "Matched " << currentName << " with " << vertexName[j] << " now " << EdgeListA[(int) verbalTokenCount/2] << " and " << EdgeListB[(int) (verbalTokenCount-1)/2] << "\n";
						}
						matchedFlag=true;
						
					}//if
				}//for
				if(!matchedFlag) cout << "Failed to match "<< currentName << "\n";
				//if(matchedFlag) cout << "Matched "<< currentName << "\n";
				verbalTokenCount++;
				matchedFlag=false;

			}//if
		}
		
		
		numEdges = (int) ((verbalTokenCount+(verbalTokenCount%2))/2);
		cout << verbalTokenCount << " verbal tokens and numEdges is " << numEdges << endl;
		//Go through the edge list and fill the adjacency list.
		
		
		//init the adjacency list.
		for(int vertex = 0; vertex < Order; vertex++){
			AdjList[vertex].Head->Vertex = vertex;
		}
		
		EdgeOrder=0;
		//Fill the adjacency list.
		for(int i=0; i<numEdges; i++){
			if(EdgeListA[i]==EdgeListB[i]){
				fprintf(stdout,"Ignoring loop %d to %d.\n",EdgeListA[i], EdgeListB[i]);
				LoopCount++;
			} else{
				if(!IsInAdjListOf(EdgeListA[i], EdgeListB[i])){
					AdjList[EdgeListA[i]].AddANode();
					AdjList[EdgeListA[i]].Tail->Vertex = EdgeListB[i];
					Degree[EdgeListA[i]]++;
					
					AdjList[EdgeListB[i]].AddANode();
					AdjList[EdgeListB[i]].Tail->Vertex = EdgeListA[i];
					Degree[EdgeListB[i]]++;
					
					EdgeOrder++;
				}else{
					fprintf(stdout,"Ignoring duplicate edge %d to %d.\n",EdgeListA[i], EdgeListB[i] );
					MultiEdgeCount++;
				}
			}
			
			
		}
		
		fprintf(stdout,"Read %d edges and put %d of them in the graph.\n",numEdges, EdgeOrder);
		if(LoopCount+MultiEdgeCount) fprintf(stdout,"Ignored %d loops and %d multiple edges.\n",LoopCount,MultiEdgeCount);
		
		
		//LookupBound = 2*(int) ceil(sqrt((double) EdgeOrder));
		//printf("LookupBound = %d.\n", LookupBound);
		//InitializeLUT();
		
		fclose(fStream);
	} else {//input file read failed.
		fprintf(stderr,"Graph file open failed. (%s)\n", EdgeListFilename);
		return -1;
	}//if else
	return Order;
}


//*****************************************************************************
/* InitSortAdjList(); *********************************************************
 ******************************************************************************/
//sorts the adjacency list so that in-cluster neighbours come last, and sets
//AdjHead2 to the first in-cluster neighbour in the list.
void Graph::InitSortAdjList()
{
	ListPtr tempPrevious=NULL;
	for(int i=0; i<Order; i++){
		//Set the current pointer and its previous properly.
		AdjList[i].RewindToHead();
		tempPrevious=AdjList[i].CurrentPtr;
		AdjList[i].Advance();
		
		for(int j=0; j<Degree[i]; j++){//run through and throw in-cluster
			//neighbours to the end of the list.
			if(SANITY_CHECK){
				if(tempPrevious->Next != AdjList[i].CurrentPtr)
					printf("SANITY CHECK: Problem in Graph::InitSortAdjList. i=%d, j=%d.\n",i,j);
			}
			
			if(WhichCluster[AdjList[i].CurrentPtr->Vertex] == WhichCluster[i]){
				AdjList[i].AddANode();
				AdjList[i].Tail->Vertex = AdjList[i].CurrentPtr->Vertex;
				AdjList[i].DeleteANodeFast(AdjList[i].CurrentPtr,tempPrevious);
				AdjList[i].CurrentPtr = tempPrevious;
			}
			
			tempPrevious=AdjList[i].CurrentPtr;
			AdjList[i].Advance();
			
		}
		
		if(WhichCluster[AdjList[i].CurrentPtr->Vertex] == WhichCluster[i])
			AdjHead2[i]=AdjList[i].CurrentPtr;
		else AdjHead2[i]=NULL;
		
	}
	
}


//*****************************************************************************
/* InitClustering**************************************************************
 ******************************************************************************/
//this either creates a random clustering or reads from a file.
void Graph::InitClustering()	
{
	if(ClusteringFN[0] == '\0') SetRandomClustering();
	else SetExplicitClustering();
	FillClusterList();
	FillClusterListSize();
	
	InitSortAdjList();
	InitEmptyClusterList();
}

//this takes a clustering as a parameter (by reference)
void Graph::InitClustering(int InputClustering []){
	for(int cluster=0; cluster< NumClust; cluster++){
		ClusterSize[cluster]=0;
	}
	if(TabuLength){
		for(int vertex=0; vertex < Order; vertex++){
			TabuVertices[vertex] = 0;
		}
	}
	for(int vertex=0; vertex < Order; vertex++){
		WhichCluster[vertex] = InputClustering[vertex];
		ClusterSize[WhichCluster[vertex]]++;
	}
	
	FillClusterList();
	FillClusterListSize();
	
	InitSortAdjList();
	InitEmptyClusterList();
}

//this one is just for post-run analysis. doesn't fill ClusterDegree.
int Graph::LoadClustering(int InputClustering []){
	int NumNonempty = 0;//number of non-empty clusters.
	for(int cluster=0; cluster< NumClust; cluster++){
		ClusterSize[cluster]=0;
	}
	if(TabuLength){
		for(int vertex=0; vertex < Order; vertex++){
			TabuVertices[vertex] = 0;
		}
	}
	for(int vertex=0; vertex < Order; vertex++){
		WhichCluster[vertex] = InputClustering[vertex];
		ClusterSize[WhichCluster[vertex]]++;
	}
	for(int cluster=0; cluster<NumClust; cluster++){
		if(ClusterSize[cluster]>0) NumNonempty ++;
	}
	
	FillClusterList();
	
	InitSortAdjList();
	InitEmptyClusterList();
	
	return(NumNonempty);
}


//Sets a random clustering, i.e. sets the cluster of each vertex randomly and
//uniformly. 
void Graph::SetRandomClustering()
{
	long vertex, cluster;
	for(cluster=0; cluster< NumClust; cluster++){
		ClusterSize[cluster]=0;
	}
	if(TabuLength){
		for(vertex=0; vertex < Order; vertex++){
			TabuVertices[vertex] = 0;
		}
	}
	for(vertex=0; vertex < Order; vertex++){
		WhichCluster[vertex] = rand() % (NumClust);
		ClusterSize[WhichCluster[vertex]]++;
	}
}


//This function takes the clustering specified in the file whose name is stored
//in ClusteringFN and sets it as the current clustering.
void Graph::SetExplicitClustering()
{
	FILE * fStream;
	long vertex, cluster;
	long currentToken = 0;
	
	for(cluster=0; cluster< NumClust; cluster++){
		ClusterSize[cluster]=0;
	}
	
	if(TabuLength){
		for(vertex=0; vertex < Order; vertex++){
			TabuVertices[vertex] = 0;
		}
	}
	
	for(vertex = 0; vertex < Order; vertex++)
		WhichCluster[vertex] = -1;
	
	cluster = 0;
	
	//Input clustering from file
	if((fStream = fopen(ClusteringFN,"r"))){
		rewind(fStream);
		currentToken = 0;
		while(currentToken != -2){
			while((currentToken = getNumToken(fStream)) >= 0){
				if(currentToken >= Order){//bad input
					fprintf(stderr,"Bad input clustering. Too many vertices.\n");
					exit(1);
				}
				WhichCluster[currentToken] = cluster;
				ClusterSize[cluster]++;
			}
			cluster++;
		}
		fclose(fStream);
	}	else{
		fprintf(stderr,"Couldn't read clustering file %s. Using random clustering.\n",ClusteringFN);
		SetRandomClustering();
	}
	if(cluster > NumClust+1){//bad input
		fprintf(stderr,"Bad input clustering. Too many clusters.\n");
		exit(1);
	}
	
	for(vertex = 0; vertex < Order; vertex++){
		if(WhichCluster[vertex] == -1){
			fprintf(stderr,"Bad input clustering. Ill-defined for vertex %d\n",(int)vertex);
			getchar();
			exit(1);
		}
	}
}



//*****************************************************************************
/* InitEmptyClusterList********************************************************
 ******************************************************************************/
//Makes a list of the empty clusters.
void Graph::InitEmptyClusterList()
{
	//clear the list.
	NumEmptyClusters = 0;
	while(EmptyClusterList.Head != NULL) 
		EmptyClusterList.DeleteANode(EmptyClusterList.Head);
	
	for(int cluster=0; cluster<NumClust; cluster++){
		if(ClusterSize[cluster]==0){
			EmptyClusterList.AddANode();
			EmptyClusterList.Tail->Vertex = cluster;
			NumEmptyClusters++;
		}
	}
	
	//	printf("%d ", NumEmptyClusters); EmptyClusterList.PrintList();
	//	PrintAdjList();
	
	SetGhostCluster();
}


//*****************************************************************************
/* SetGhostCluster*************************************************************
 ******************************************************************************/
void Graph::SetGhostCluster()
{
	//Note the GhostCluster is the HEAD of the list, not the tail.
	if(EmptyClusterList.Head != NULL) GhostCluster = EmptyClusterList.Head->Vertex;
	else GhostCluster = -1;
}


//*****************************************************************************
/* FillClusterList*************************************************************
 ******************************************************************************/
//This function uses the clusters specified in the vector WhichCluster to fill
//the cluster lists in ClusterList.
void Graph::FillClusterList()
{
	for(int c=0; c<NumClust; c++){
		while(ClusterList[c].Head != NULL)
			ClusterList[c].DeleteANode(ClusterList[c].Head);
	}
	
	for(int vertex=0; vertex < Order; vertex++){
		ClusterList[WhichCluster[vertex]].AddANode();
		ClusterList[WhichCluster[vertex]].Tail->Vertex = vertex;
		ClusterListPtr[vertex] =
		ClusterList[WhichCluster[vertex]].Tail;
	}
}


//This function sets the number of edges in each cluster.
void Graph::FillClusterListSize()
{
	long i,j;
	for(j=0;j<NumClust;j++){
		ClusterEdgeOrder[j]=0;
	}
	
	for(i=0;i<Order;i++){
		AdjList[i].RewindToHead();
		while(AdjList[i].CurrentPtr != AdjList[i].Tail){
			AdjList[i].Advance();
			if(WhichCluster[i]==WhichCluster[AdjList[i].CurrentPtr->Vertex])
				ClusterEdgeOrder[WhichCluster[AdjList[i].CurrentPtr->Vertex]]++;
		}
	}
	
	for(i=0;i<NumClust;i++){
		ClusterEdgeOrder[i] /= 2;
	}
	
}
