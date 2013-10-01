#include <sys/time.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include <string>
#include "graph.h"
#include "miscFunctions.h"

int * whichCluster, * ClusterSize, * numberMatched;
int numVerts, numClust, numNonemptyComplexes, numberOfComplexes;
int numVertsS, numVertsL, numClustS, numClustL, numClust1, numClust2;
int NumMatchedClusters, NumMatchedComplexes, * ComplexSize;

//global filenames
char * nameFilename, * graphFilename, * clusteringFilename;
char * outputFilename, *complexFilename;



int NumFunctions;//number of functional groups.
char * functionMapping;//maps numbers to letters.

long scaledCost;
SLList* ComplexList;
bool *AlreadyCounted, *ClusterMatched, *ComplexMatched, *ClusterSMatched, *ComplexSMatched;

double avgClusterSizeCL, avgClusterSizeVL, avgClusterDensityCL, avgClusterDensityVL;
double pctEdgesInClustersL;

double avgClusterSizeCS, avgClusterSizeVS, avgClusterDensityCS, avgClusterDensityVS;
double pctEdgesInClustersS;

//Matching criteria cutoff thresholds
float ClusterCutoff, ComplexCutoff, ComConCutoff, CluConCutoff;

float *ClusterDensity, *ComplexDensity;
double *ClusterP, *ComplexP, *ComplexPee;
int *ComplexFunction, *ClusterFunction;

int sizeCutoff; float densityCutoff; float pCutoff;//Cutoff thresholds.

int *MainInCluster, *UnknownInCluster;
int *MainInComplex, *UnknownInComplex;

long * Pi;
std::string * PiProteinName;
char * PiFunctionNum;

std::string * proteinName;
char * Function;
int * FunctionNum, * FunctionSize;

int * MaxMatch; int * MaxMatchWith;
float * MaxMatchPct; int * MaxMatchPctWith;


Graph graph;

long nChoose2(long n){
	if(n<0){
		fprintf(stderr,"Error: Performing nChoose2 on negative n.\n");
	}
	return((n*n - n)/2);
}

//*****************************************************************************
/* printUsageError ************************************************************
 **
 ** Prints a usage error
 */
void printUsageError(void)
{
	fprintf(stderr,"Usage: ./form -g GraphFilename -c ClusteringFilename -n NameFilename -s SizeCutoff -d DensityCutoff -p PCutoff -o OutputFilename\n\n");
	fprintf(stderr,"Only the graph filename and the clustering filename are necessary.\n\n");
	
	
}// printUsageError ***********************************************************
//*****************************************************************************



bool IsAMatch(int Clust, int Comp, int NumberMatched)
{
	
	if(NumberMatched > MaxMatch[Clust]){
		MaxMatch[Clust] = NumberMatched;
		MaxMatchWith[Clust] = Comp;
	}
	
	if((float) NumberMatched / ComplexSize[Comp] > MaxMatchPct[Clust]){
		MaxMatchPct[Clust] = (float) NumberMatched / ComplexSize[Comp];
		MaxMatchPctWith[Clust] = Comp;
	}
	
	return(
		   
		   (
			((float) NumberMatched/ClusterSize[Clust]) >=
			((float) ClusterCutoff/log10(7+ClusterSize[Clust]))
			&&
			((float) NumberMatched/ComplexSize[Comp]) >=
			((float) ComplexCutoff/log10(7+ComplexSize[Comp]))
			)
		   ||
		   (
			((float) NumberMatched/ClusterSize[Clust]) >= CluConCutoff
			||
			((float) NumberMatched/ComplexSize[Comp]) >= ComConCutoff
			)
		   
		   
		   );
	
}

long factorial(long n){
	long bang;
	if(n>=0){
		bang = 1;
		for(long i=2; i<=n; i++){
			bang *= i;
		}
	} else {
		fprintf(stderr,"Arithmetic exception: factorial of a negative.\n");
		exit(1);
	}
	return bang;
}

long comb(long n, long k){
	if(n<k || n<0 || k<0){
		fprintf(stderr,"Error performing combination function\n");
		exit(1);
	} else {
		if(k==0) return 1;
		return factorial(n)/(factorial(k)*factorial(n-k));
	}
}

double getP(long F, long C, long k)
{
	
	//printf("Called getP %d %d %d. \n",F,C,k);
	//Calculates P-value for a function of size F, a cluster of size C
	//with k members of the function.
	long V=numVerts;
	
	if(F==V || F==0 || k==0){ return (double) 1;}
	if(k>F || k>C){ return (double) 0;}
	
	long double ret=1; //to be deducted from and returned.
	long double value; //odds of having i members for a given i<k.
	long double binomialTemp;//used for calculating n choose k.
	
	for(int i=0; i<k; i++){
		
		value=1;
		
		// F choose i
		binomialTemp=1;
		for(int j=0; j<i; j++){
			binomialTemp = binomialTemp/(j+1);
			binomialTemp = binomialTemp*(F-j);
		}
		value = value*binomialTemp;
		
		// V-F choose C-i
		binomialTemp=1;
		if(V-F < C-i){
			binomialTemp=0;
		} else {
			for(int j=0; j<C-i; j++){
				binomialTemp = binomialTemp/(j+1);
				binomialTemp = binomialTemp*(V-F-j);
			}
		}
		value = value*binomialTemp;
		
		// V choose C (inverted)
		binomialTemp=1;
		for(int j=0; j<C; j++){
			binomialTemp = binomialTemp/(j+1);
			binomialTemp = binomialTemp*(V-j);
		}
		value = value/binomialTemp;
		
		ret -= value;		
	}
	
	return (double) ret;
	
}


/*
 double getP(long C, long n, long i)
 {
 //calculates the i term in the P-value...
 //i.e. n!*(C)i*(G-C)n-i / (G)n*i!*(n-i)!
 int j,k;
 long double value, ret=1;
 
 for(k=0; k<i; k++){
 value=1;
 for(j=1;j<=n;j++){
 value *= (long double) j/(numVerts-j+1);
 }
 for(j=1;j<=k;j++){
 value *= (long double) (C-j+1)/j;
 }
 for(j=1;j<=n-k;j++){
 value *= (long double) (numVerts-C-j+1)/j;
 }
 ret -= value;
 }
 return (double) ret;
 
 }
 */



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
	//	printf("Returning \"%s\".\n",token);
	
	return atol(token);
}

void WriteComposition(char * filename){
	FILE * fStream;
	//	int flag;
	
	if((fStream = fopen(filename,"w"))){
		for(int i=0; i<numberOfComplexes; i++){
			//write the complex data.
		  //		if(!ComplexSize[i]) flag=-1;
		  //	else if(ComplexSMatched[i]) flag=2;
		  //	else if(ComplexMatched[i]) flag=1;
		  //	else flag=0;
			
			fprintf(fStream, "%d %d %d %d %1.16e\n",ComplexSize[i],MainInComplex[i],UnknownInComplex[i],ComplexFunction[i],ComplexP[i]);
		}
		
		
		for(int i=0; i<numClust; i++){
			//write the cluster data.
			//if(ClusterSMatched[i]) flag=2;
			//else if(ClusterMatched[i]) flag=1;
			//else flag=0;
			
			fprintf(fStream, "%d %d %d %d %1.16e\n",ClusterSize[i],MainInCluster[i],UnknownInCluster[i],ClusterFunction[i],ClusterP[i]);
		}
		
	}	else{
		fprintf(stderr,"Couldn't read clustering.\n");
		return;
	}
	
	fclose(fStream);
}


void WriteData(char * filename){
	FILE * fStream;
	int flag;
	
	if((fStream = fopen(filename,"w"))){
		for(int i=0; i<numberOfComplexes; i++){
			//write the complex data.
			if(!ComplexSize[i]) flag=-1;
			else if(ComplexSMatched[i]) flag=2;
			else if(ComplexMatched[i]) flag=1;
			else flag=0;
			
			fprintf(fStream, "%d %f %d %d %1.16f\n",ComplexSize[i],ComplexDensity[i],flag,ComplexFunction[i],ComplexP[i]);
		}
		
		
		for(int i=0; i<numClust; i++){
			//write the cluster data.
			if(ClusterSMatched[i]) flag=2;
			else if(ClusterMatched[i]) flag=1;
			else flag=0;
			
			fprintf(fStream, "%d %f %d %d %1.16f\n",(int) graph.ClusterSize[i],ClusterDensity[i],flag,ClusterFunction[i],ClusterP[i]);
		}
		
	}	else{
		fprintf(stderr,"Couldn't read clustering.\n");
		return;
	}
	fclose(fStream);
}

void GenVertexPermutation(long order)
//generates a random permutation of size "order". Used to find P-values of clusters with shuffled labels.
{
	Pi = new long[order];
	int i, swapRand;
	for(i=0; i<order; i++){
		Pi[i]=i;
	}
	for(i=1; i<order; i++){//remember, now Pi[i]=i.
		swapRand=rand()%(i+1);
		Pi[i] = Pi[swapRand];
		Pi[swapRand]=i;
	}
	
	//this little trick shuffles the labels.
	PiProteinName = new std::string[order];
	PiFunctionNum = new char[order];
	for(i=0; i<order; i++){
		PiProteinName[i]=proteinName[Pi[i]];
		PiFunctionNum[i]=FunctionNum[Pi[i]];
	}
	for(i=0; i<order; i++){
		proteinName[i]=PiProteinName[i];
		FunctionNum[i]=PiFunctionNum[i];
	}
	
	
	
}

void WriteVerbalData(char * filename){
	FILE * fStream;
	int i,j;
	//const char* mapping = "EGMPTBFOARDCU";
	
	if((fStream = fopen(filename,"w"))){
		
		
		//print the complex data.
		for(j=0; j<numberOfComplexes; j++){
			//if(ComplexMatched[j]){
			fprintf(fStream,"Complex #%d. Size %d. Density %1.4f. P-value %4.4e. Function %c. %4.4e\n",j, ComplexSize[j],
					ComplexDensity[j], ComplexP[j], functionMapping[ComplexFunction[j]], ComplexP[j]);
			//if(ComplexSMatched[j]) fprintf(fStream,"Supermatched.\n");
			//else if(ComplexMatched[j]) fprintf(fStream,"Matched.\n");
			//else fprintf(fStream,"Unmatched.\n");
			ComplexList[j].RewindToHead();
			for(i=0; i<ComplexSize[j]; i++){
				fprintf(fStream,"%s(%c)  ",proteinName[ComplexList[j].CurrentPtr->Data].c_str(),
						Function[ComplexList[j].CurrentPtr->Data]);
				ComplexList[j].Advance();
			}
			fprintf(fStream,"\n----------\n");
			//}
		}
		
		
		
		
	}	else{
		fprintf(stderr,"Couldn't read clustering.\n");
		return;
	}
	fclose(fStream);
}


void WriteVerbalData2(char * filename){
	FILE * fStream;
	int i,j, counter=0;
	//	const char* mapping = "EGMPTBFOARDCU";
	
	
	if((fStream = fopen(filename,"w"))){
		
		//print the cluster data.
		for(j=0; j<numClust; j++){
			if(ClusterSize[j]>=sizeCutoff && ClusterDensity[j]>=densityCutoff && ClusterP[j]<=pCutoff){
				fprintf(fStream,"Cluster #%d.  Size %d.  Density %f.  Predicted function is %c with P-value %4.4e.\n",counter, (int)graph.ClusterSize[j],
						ClusterDensity[j], functionMapping[ClusterFunction[j]], ClusterP[j]);
				graph.ClusterList[j].RewindToHead();
				for(i=0; i<ClusterSize[j]; i++){
					fprintf(fStream,"%s(%c) ",proteinName[graph.ClusterList[j].CurrentPtr->Data].c_str(), functionMapping[FunctionNum[graph.ClusterList[j].CurrentPtr->Data]]);
					graph.ClusterList[j].Advance();
				}
				fprintf(fStream,"\n----------\n");
				counter++;
			}
		}
		
		printf("%d predicted complexes written to file %s.\n",counter,filename);
		
	}	else{
		fprintf(stderr,"Couldn't write data to output file.  Check write permissions on directory and file.\n");
		return;
	}
	
	fclose(fStream);
	
}





int CountClusters(char * filename){
	FILE * fStream;
	int cluster = 0;
	long currentToken = 0;
	
	if((fStream = fopen(filename,"r"))){
		currentToken = getNumToken(fStream);
		while(currentToken != -2){
			while((currentToken = getNumToken(fStream)) >= 0){
				
			}
			cluster++;
		}
		return cluster-1;
	}	else{
		fprintf(stderr,"Couldn't read clustering.\n");
		return -1;
	}
}
int ReadClustering(char * filename){
	FILE * fStream;
	int cluster = 0;
	long currentToken = 0;
	
	if((fStream = fopen(filename,"r"))){
		
		while(currentToken != -2){
			ClusterSize[cluster]=0;
			while((currentToken = getNumToken(fStream)) >= 0){
				
				if(currentToken>=numVerts){//Too many vertices in clustering.
					fprintf(stderr,"Vertex %d in the clustering file does not exist in the graph!  QUITTING.\n\n",(int) currentToken);
					exit(1);
				}
				
				
				whichCluster[currentToken] = cluster;
				ClusterSize[cluster]++;
			}
			ClusterMatched[cluster]=false;
			ClusterSMatched[cluster]=false;
			cluster++;
		}
		
		
		return cluster-1;
	}	else{
		fprintf(stderr,"Couldn't read clustering file %s.\n",filename);
		return -1;
	}
	fclose(fStream);
}

int ReadClusteringNumber(char * filename){//GET THE NUMBER OF CLUSTERS
	FILE * fStream;
	int cluster = 0;
	long currentToken = 0;
	
	if((fStream = fopen(filename,"r"))){
		
		currentToken = getNumToken(fStream);
		while(currentToken != -2){
			while((currentToken = getNumToken(fStream)) >= 0){
			}
			cluster++;
		}
		//printf("CLUSTER COUNT: %d.\n",cluster-1);
		return cluster-1;
	}	else{
		fprintf(stderr,"ERROR: Couldn't parse data from file %s.  File misformatted or file not readable.\n",filename);
		return -1;
	}
	
	fclose(fStream);
	
}



void GetStats(){
	
	//get cluster density
	for(int i=0; i<numClust; i++){
		if(graph.ClusterSize[i]>1){
			ClusterDensity[i]=(float) graph.ClusterEdgeCount[i] / nChoose2(graph.ClusterSize[i]);
		}else{
			ClusterDensity[i]=1;
		}
	}
	
	
	//get average cluster densities:
	float MClusterDensity=0, UClusterDensity=0;//, AClusterDensity=0;//matched, unmatched, and overall averages.
	for(int i=0; i<numClust; i++){
		if(ClusterMatched[i]) MClusterDensity += ClusterDensity[i];
		else UClusterDensity +=ClusterDensity[i];
	}
	//	AClusterDensity = (float) (MClusterDensity + UClusterDensity)/numClust;
	MClusterDensity = (float) MClusterDensity/NumMatchedClusters;
	UClusterDensity = (float) UClusterDensity/(numClust-NumMatchedClusters);
	
	
	//print average densities.
	//printf("Average cluster densities (matched, unmatched, overall)\n%f %f %f\n",MClusterDensity, UClusterDensity, AClusterDensity);
	//printf("Average complex densities (matched, unmatched, overall)\n%f %f %f\n",MComplexDensity, UComplexDensity, AComplexDensity);
	
	
	//get average cluster sizes:
	float MClusterSize=0, UClusterSize=0;//, AClusterSize=0;//matched, unmatched, and overall averages.
	for(int i=0; i<numClust; i++){
		if(ClusterMatched[i]) MClusterSize += graph.ClusterSize[i];
		else UClusterSize += graph.ClusterSize[i];
	}
	//	AClusterSize = (float) (MClusterSize + UClusterSize)/numClust;
	MClusterSize = (float) MClusterSize/NumMatchedClusters;
	UClusterSize = (float) UClusterSize/(numClust-NumMatchedClusters);
	
}




void GetProteinNames(char * filename){
	FILE * fStream;
	int vertex;
	char buffer[100];
	char name[16];
	int i;
	
	if((fStream = fopen(filename,"r"))){
		for(vertex = 0; vertex < numVerts; vertex++){
			fgets(buffer, sizeof(buffer), fStream);
			sscanf(buffer, "%s", name);
			proteinName[vertex] = name;
			fgets(buffer, sizeof(buffer), fStream);
			sscanf(buffer, "%s", name);
			Function[vertex] = name[0];
			for(i=0; i<NumFunctions; i++){
				if(name[0]==functionMapping[i]) FunctionNum[vertex]=i;
			}
			//			printf("vertex %d function %c, %c, %d\n",vertex, Function[vertex],name[0],FunctionNum[vertex]);
		}
		
		
	}	else{
		fprintf(stderr,"Couldn't open protein name list file.\nPROCEEDING WITH NUMERICAL PROTEIN NAMES AND UNKNOWN PROTEIN FUNCTIONS.\n");
		
		//Set unknown functions and numerical names for vertices.
		for(vertex=0; vertex<numVerts; vertex++){
			Function[vertex] = functionMapping[0];
			FunctionNum[vertex]=0;
			sprintf(name, "%d", vertex);
			proteinName[vertex]=name;
		}
	}
	
	//get the function sizes.
	for(i=0; i<NumFunctions; i++) FunctionSize[i]=0;
	for(i=0; i<numVerts; i++) FunctionSize[FunctionNum[i]]++;
	
	//	for(i=0; i<NumFunctions; i++) printf("Function %c has size %d.\n",functionMapping[i],FunctionSize[i]);
	
	if(fStream) fclose(fStream);
	
}


void ReadComplexes(char * filename)
{
	long currentToken;
	int currentComplex=0;
	FILE * fStream;
	numNonemptyComplexes=0;
	
	if((fStream = fopen(filename,"r"))){
		for(currentComplex = 0; currentComplex<numberOfComplexes; currentComplex++){
			ComplexMatched[currentComplex]=false;
			ComplexList[currentComplex].DeleteANode(ComplexList[currentComplex].Head);
			for(int i=0; i<numVerts; i++){ AlreadyCounted[i]=0; }
			ComplexSize[currentComplex]=0;
			while(0 <= (currentToken = getNumToken(fStream))){
				if(!AlreadyCounted[currentToken]){
					AlreadyCounted[currentToken]=1;
					ComplexSize[currentComplex]++;
					ComplexList[currentComplex].AddANode();
					ComplexList[currentComplex].Tail->Data = currentToken;
				}
			}
			if(ComplexSize[currentComplex]) numNonemptyComplexes++;
		}
		
	} else {
		fprintf(stderr,"Couldn't read complexes. %s\n",filename);
	}
	
	fclose(fStream);
}



void Match()
{
	int currentComplex=0;
	bool ClusterHasMatch;
	//const char* mapping = "EGMPTBFOARDCU";
	
	for(int cluster=0; cluster<numClust; cluster++){
		ClusterHasMatch=false;
		ClusterSMatched[cluster]=false;
		
		for(currentComplex = 0; currentComplex<numberOfComplexes; currentComplex++){
			for(int i=0; i<numVerts; i++){ AlreadyCounted[i]=0; }
			numberMatched[currentComplex]=0;
			ComplexList[currentComplex].RewindToHead();
			for(int i=0; i<ComplexSize[currentComplex]; i++){
				if(whichCluster[ComplexList[currentComplex].CurrentPtr->Data] == cluster){
					numberMatched[currentComplex]++;
				}
				//printf("%d ",numberMatched[currentComplex]);
				ComplexList[currentComplex].Advance();
			}
			if( IsAMatch(cluster, currentComplex, numberMatched[currentComplex]) ){
				printf("Cluster %d matches complex %d %d times. Sizes %d and %d. ",cluster,currentComplex,numberMatched[currentComplex], ClusterSize[cluster], ComplexSize[currentComplex]);
				ClusterHasMatch = true;
				ClusterMatched[cluster]=true;
				ComplexMatched[currentComplex]=true;
				if(ComplexFunction[currentComplex]==ClusterFunction[cluster]){//SUPERMATCH!
					ClusterSMatched[cluster]=true;
					ComplexSMatched[currentComplex]=true;
					printf("Both function %c. (SUPERMATCH)\n",functionMapping[ComplexFunction[currentComplex]]);
				} else {
					printf("Functions %c and %c.\n", functionMapping[ClusterFunction[cluster]], functionMapping[ComplexFunction[currentComplex]]);
				}
				
			}
			
		}
		if(ClusterHasMatch){ NumMatchedClusters++; }
 	}
	NumMatchedComplexes=0;
	for(currentComplex = 0; currentComplex<numberOfComplexes; currentComplex++){
		if(ComplexMatched[currentComplex] && ComplexSize[currentComplex]){
			NumMatchedComplexes++;
		}
	}
}


void GetComplexP()
{
	int j, fcn;
	double pvalue;
	int votes[NumFunctions];
	//Get the most frequent function, then the P value.
	for(int comp=0; comp<numberOfComplexes; comp++){
		for(j=0; j<NumFunctions; j++){ votes[j]=0; }
		ComplexList[comp].RewindToHead();
		for(j=0; j<ComplexSize[comp]; j++){
			votes[FunctionNum[ComplexList[comp].CurrentPtr->Data]]++;
			ComplexList[comp].Advance();
		}
		
		//get the P value.
		fcn=0;
		ComplexP[comp]=1;
		ComplexPee[comp]=1;
		int en = 0;
		
		for(j=0; j<NumFunctions; j++){
			pvalue = getP(FunctionSize[j],ComplexSize[comp],votes[j]);
			ComplexPee[comp] *= pvalue;
			if(pvalue<ComplexP[comp]){
				fcn = j;
				ComplexP[comp]=pvalue;
			}
			if(votes[j]) en++;
		}
		if(ComplexPee[comp] < 0) ComplexPee[comp] = 0;
		ComplexPee[comp]= pow(ComplexPee[comp], (float)1/en);
		
		
		//set the letter
		ComplexFunction[comp]=fcn;
		MainInComplex[comp] = votes[fcn];
		UnknownInComplex[comp] = votes[NumFunctions-1];
		
		//printf("P value %f for %d %d %d\n",ComplexP[comp],FunctionSize[maxVotes],ComplexSize[comp],votes[maxVotes]);
	}
}

void GetClusterP()
{
	int j, fcn;
	double pvalue;
	int votes[NumFunctions];
	//Get the most frequent function, then the P value.
	for(int cluster=0; cluster<numClust; cluster++){
		for(j=0; j<NumFunctions; j++){ votes[j]=0; }
		graph.ClusterList[cluster].RewindToHead();
		for(j=0; j<graph.ClusterSize[cluster]; j++){
			votes[FunctionNum[graph.ClusterList[cluster].CurrentPtr->Data]]++;
			graph.ClusterList[cluster].Advance();
		}
		
		//get the P value.
		fcn=0;
		ClusterP[cluster]=1;
		for(j=0; j<NumFunctions; j++){
			pvalue= getP(FunctionSize[j],graph.ClusterSize[cluster],votes[j]);
			
			if(pvalue<=ClusterP[cluster] && votes[j] > 0){
				fcn = j;
				ClusterP[cluster]=pvalue;
			}
			
		}
		
		
		//set the letter
		ClusterFunction[cluster]=fcn;
		MainInCluster[cluster] = votes[fcn];
		UnknownInCluster[cluster] = votes[0];
		
		//printf("P value %f for %d %d %d\n",ClusterP[cluster],FunctionSize[maxVotes],graph.ClusterSize[cluster],votes[maxVotes]);
	}
}








void PrintHelp()
{
	printf("\nRNSCFILTER.  Usage:\n\n\trnscfilter -g GraphFilename"
		   "\n\t           -c ClusteringFilename"
		   "\n\t           [-n NameFilename]"
		   "\n\t           [-o OutputFilename]"
		   "\n\t           [-s SizeCutoff]"
		   "\n\t           [-d DensityCutoff]"
		   "\n\t           [-p PValueCutoff]"
		   
		   "\n\n\tFOR MORE INFORMATION ON PARAMETERS, SEE README.TXT.\n\n"
		   );
	
	
	exit(1);
}



/* READ INPUT PARAMETERS 
 * 
 * */

void ReadParameters(int number, char ** vector)
{
	int c;
	while ((c = getopt(number, vector, "g:c:n:f:o:s:d:p:")) != EOF) {
		switch (c) {
				
			case 'h': //Print help and exit.
				PrintHelp();
				break;
				
			case 'g': //Sets the graph filename.
				graphFilename = optarg;
				break;
			case 'c': //Sets the clustering filename.
				clusteringFilename = optarg;
				break;
				// case 'C': //Sets the complex filename.
				//complexFilename = optarg;
				//break;
			case 'n': //Sets the protein name filename.
				nameFilename = optarg;
				break;
			case 'f': //Sets the function name string.
				functionMapping = optarg;
				break;
			case 'o': //Sets the output filename
				outputFilename = optarg;
				break;
			case 's': //Sets the minimum cluster size
				sizeCutoff = atoi(optarg);
				break;
			case 'd': //Sets the minimum cluster density
				densityCutoff = atof(optarg);
				break;
			case 'p': //Sets the maximum P-value
				pCutoff = atof(optarg);
				break;
			case '?': //invalid command line format
				printUsageError();
				exit(1);
				return;
				break;
			default: //invalid command line format
				printUsageError();
				exit(1);
				
				return;
				break;
		}//END SWITCH
		
	}//END WHILE c (taking command line arguments)   
	
	if (optind < number) { //More invalid input
		fprintf(stderr,"Too many arguments.\n");
		printUsageError();
		exit(1);
		return;
	}
	
}





/* MAIN METHOD
 * 
 * */

int main(int argc, char **argv)
{
	if(argc==1) PrintHelp();

	scaledCost = 0;
	
	time_t startTime;
	srand(time(&startTime));
	
	outputFilename = (char *)"output.txt";
	
	functionMapping = (char *)"UABCDEFGHIJKLMNOPQRSTVWXYZ";//"UEGMPTBFOARDC";
	
	sizeCutoff = 1; densityCutoff=0; pCutoff=1.000001;
	
	ReadParameters(argc, argv);
	
	printf("Graph filename:         %s\n",graphFilename);
	printf("Number of nodes:        %d\n\n",numVerts = ReadClusteringNumber(graphFilename));
	if(numVerts== -1){
		fprintf(stderr,"Graph input error.  File %s is nonexistent, unreadable, or not a valid graph file.  QUITTING.\n",graphFilename);
		printUsageError();
		return 0;		
	}
	
	
	printf("Clustering filename:    %s\n",clusteringFilename);
	printf("Number of clusters:     %d\n\n",numClust = ReadClusteringNumber(clusteringFilename));
	if(numClust == -1){
		fprintf(stderr,"Clustering input error.  File %s is nonexistent, unreadable, or not a valid clustering file.  QUITTING.\n",clusteringFilename);
		printUsageError();
		return 0;		
	}
	
	//printf("Number of complexes:  %d\n",numberOfComplexes = ReadClusteringNumber(complexFilename));
	
	
	printf("Protein name filename:  %s\n\n",nameFilename);
	
	
	
	NumFunctions = strlen(functionMapping);
	printf("%-3dfunctional groups:   %s\n\n",NumFunctions, functionMapping);
	
	printf("Prediction cutoffs:\n     Size    >= %d.\n     Density >= %-1.3f.\n     P-value <= %-.1e.\n\n",
		   sizeCutoff, densityCutoff, pCutoff);
	
	
	
	/* FIX COMPUTATION OF BEST P-VALUE.
	 * WRITE THE REST OF THE CHAPTER.*/
	
	
	//initialize the arrays.
	
	ComplexList = new SLList[numberOfComplexes];
	proteinName = new std::string[numVerts];
	Function = new char[numVerts];
	FunctionNum = new int[numVerts];
	FunctionSize = new int[NumFunctions];
	GetProteinNames(nameFilename);
	
	whichCluster = new int[numVerts];
	AlreadyCounted = new bool[numVerts];
	ComplexSize = new int[numberOfComplexes];
	numberMatched = new int[numberOfComplexes];
	
	numClust = CountClusters(clusteringFilename);
	ClusterSize = new int[numClust];
	ClusterDensity = new float[numClust];
	ComplexDensity = new float[numberOfComplexes];
	
	ClusterP = new double[numClust];
	ComplexP = new double[numberOfComplexes];
	ComplexPee = new double[numberOfComplexes];
	
	ClusterFunction = new int[numClust];
	ComplexFunction = new int[numberOfComplexes];
	
	ClusterMatched = new bool[numClust];
	ComplexMatched = new bool[numberOfComplexes];
	ClusterSMatched = new bool[numClust];
	ComplexSMatched = new bool[numberOfComplexes];
	
	MainInComplex = new int[numberOfComplexes];
	MainInCluster = new int[numClust];
	UnknownInComplex = new int[numberOfComplexes];
	UnknownInCluster = new int[numClust];
	
	
	ComplexList = new SLList[numberOfComplexes];
	
	MaxMatch = new int[numClust];
	MaxMatchPct = new float[numClust];
	MaxMatchWith = new int[numClust];
	MaxMatchPctWith = new int[numClust];
	for(int clust = 1; clust < numClust; clust++){
		MaxMatch[clust]=0;
		MaxMatchPct[clust]=0;
		MaxMatchWith[clust]=0;
		MaxMatchPctWith[clust]=0;
	}
	
	
	ReadClustering(clusteringFilename);
	graph.SetNumClust(numClust);
	
	//printf("%d clusters....\n",(int) graph.NUM_CLUST);
	
	graph.MakeGraph(graphFilename);
	graph.CheckAdjList();
	graph.FillAdjList();
	
	graph.InitClustering(whichCluster);
	//graph.SetInitialNumAndDom();
	
	//Matching criteria
	//ComplexCutoff=.7; ClusterCutoff=.7;
	//ComConCutoff=1.1; CluConCutoff=.9;
	
	//ReadComplexes(complexFilename);
	
	
	//	GenVertexPermutation(graph.Order);
	
	//GetComplexP();
	GetClusterP();
	
	//	Match();
	
	GetStats();
	
	
  	//WriteData(outputFilename);
	
	printf("\n");
	WriteVerbalData2(outputFilename);
	
	//printf("PARAMS: %d %f %f\n",	sizeCutoff,densityCutoff, pCutoff);
	
	
	//WriteComposition(outputFilename);
	
	/*
	 for(int clust=0; clust<numClust; clust++){
	 if(!ClusterMatched[clust]){
	 printf("Cluster %d unmatched. Tops are %d/%d with %d (size %d) and %f with %d (size %d).\n",
	 clust, MaxMatch[clust], (int) graph.ClusterSize[clust],
	 MaxMatchWith[clust], ComplexSize[MaxMatchWith[clust]],
	 MaxMatchPct[clust], MaxMatchPctWith[clust],
	 ComplexSize[MaxMatchPctWith[clust]]);
	 }
	 
	 }
	 */
	
	return 0;
}
