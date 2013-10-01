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


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include "linkedList.h"
#include "graph.h"
#include "definitions.h"
#include "miscFunctions.h"
#include "experiment.h"


Graph graph;
Experiment experiment;
int NumExper;

int randSeed;

const char * OutputFilename;

double BestScaledCost;//this is the scaled cost of the final cluster.
int * ClusteringF;//this is the final clustering.

double * BestCosts;
double * Times;
double * CC;
int * ScaledLengths;
int * NaiveLengths;
double TimesSum;//running total of times.

//global flags.
bool GF_ReadClustering = false;
bool GF_SkipNaive = false;



void PrintHelp()
{
	printf("\nRNSC.  Usage:\n\n\trnsc -g GraphFilename"
		   "\n\t     [-o OutputFilename]"
		   "\n\t     [-i inputname]"
		   "\n\t     [-c MaxNumberOfClusters]"
		   "\n\t     [-t TabuLength]"
		   "\n\t     [-n NaiveStoppingTolerance]"
		   "\n\t     [-N ScaledStoppingTolerance]"
		   "\n\t     [-e NumberOfExperiments]"
		   "\n\t     [-D DiversificationFrequency]"
		   "\n\t     [-d DiversificationLength]"
		   "\n\t     [-s]"
		   
		   "\n\n\tFOR MORE INFORMATION ON PARAMETERS, SEE README.TXT.\n\n"
		   );

	
	exit(1);
}

void ReadParameters(int number, char ** vector)
{
	int c;
	while ((c = getopt(number, vector, "hr:sc:t:T:g:i:n:N:e:D:d:o:")) != EOF) {
		switch (c) {
				
			case 'h': //Print help and exit.
				PrintHelp();
				break;
			case 'r': //Sets the random seed.
				randSeed = atoi(optarg);
				break;
			case 's': //Skips the naive scheme
				GF_SkipNaive = true;
				break;
			case 'i': //Takes an input clustering.
				GF_ReadClustering = true;
				graph.ClusteringFN = optarg;
				break;
			case 'o': //Writes the final clustering.
				OutputFilename = optarg;
				break;
			case 'g': //Sets the graph filename.
				graph.GraphFN = optarg;
				break;
			case 'c': //Sets the max number of clusters. (default graph.Order)
				graph.NumClust = atoi(optarg);
				break;
			case 't': //Sets the tabu length. (default 1)
				graph.TabuLength = atoi(optarg);
				break;
			case 'T': //Sets the tabu tolerance. (default 1)
				graph.TabuTol = atoi(optarg);
				break;
			case 'n': //Sets the naive stopping tolerance. (default 5)
				experiment.P_NaiveStoppingTol = atoi(optarg);
				break;
			case 'N': //Sets the scaled stopping tolerance. (default 5)
				experiment.P_ScaledStoppingTol = atoi(optarg);
				break;
			case 'e': //Sets the number of experiments.
				NumExper = atoi(optarg);
				break;
			case 'D': //Sets the diversification frequency.
				experiment.P_ShuffleFreq = atoi(optarg);
				experiment.P_DestroyFreq = atoi(optarg);			
				break;
			case 'd': //Sets the shufflind diversification length.
				experiment.P_ShuffleLength = atoi(optarg);
				break;
			case '?': //invalid command line format
				exit(1);
				return;
				break;
			default: //invalid command line format
				exit(1);
				return;
				break;
		}//END SWITCH
		
	}//END WHILE c (taking command line arguments)   
	
	if (optind < number) { //More invalid input
		fprintf(stderr,"Too many arguments.\n");
		//exit(1);
		return;
	}
	
}



void RNSC()
{
	ClusteringF = new int[graph.Order];
	BestCosts = new double[NumExper];
	Times = new double[NumExper];
	NaiveLengths = new int[NumExper];
	ScaledLengths = new int[NumExper];
	
	BestScaledCost = BIG_COST;
	TimesSum = 0;//getTime();
	
	experiment.ClusteringSAlloc(graph);
	
	experiment.P_SkipNaive = GF_SkipNaive;
	if(SILENCE<=1) printf("\n");
	for(int Exper=0; Exper<NumExper; Exper++){
		if(SILENCE<=1) printf("Running experiment %d.\n",Exper+1);
		experiment.RunExperiment(graph);
		
		if(experiment.BestScaledCost < BestScaledCost){
			//we have a new C_F.
			for(int i=0; i<graph.Order; i++){
				ClusteringF[i] = experiment.ClusteringS[i];
			}
		}
		BestCosts[Exper]=experiment.BestScaledCost;
		ScaledLengths[Exper]=experiment.ScaledLength;
		NaiveLengths[Exper]=experiment.NaiveLength;
		
		
		Times[Exper] = getTime()-TimesSum;
		if(SILENCE<=2) printf("\tExperiment %d took %.2f seconds.\n",Exper+1,Times[Exper]);
		TimesSum += Times[Exper];
	}
}


void WriteClusteringF(){
	
	ofstream fStream(OutputFilename);
	if(fStream){
		for(int cluster = 0; cluster<graph.NumClust; cluster++){
			graph.ClusterList[cluster].RewindToHead();
			for(int v = 0; v<graph.ClusterSize[cluster]; v++){
				fStream << graph.ClusterList[cluster].CurrentPtr->Vertex << " ";
				graph.ClusterList[cluster].Advance();
			}
			if(graph.ClusterSize[cluster]) fStream << "-1" << endl;
		}	
		fStream.close();
	}
	else {
		fprintf(stderr,"Output file open failed. (%s)\n", OutputFilename);
	}
	
}


int main(int argc, char **argv)
{
	if(NEIGHBOURHOODS){//clear the file.
		FILE * nhoodfile = fopen("nhoodsizes.dat","w");
		fprintf(nhoodfile," ");
		fclose(nhoodfile);
	}
	
	
	OutputFilename = "out.rnsc";
	//setting some basic parameters. they can be overridden in the command line
	graph.GraphFN = "graph";
	graph.NumClust = -1;//actually defaults to graph.Order
	NumExper = 1;
	experiment.P_ScaledStoppingTol = 5;
	experiment.P_NaiveStoppingTol = 5;
	
	time_t startTime;
	randSeed = (int) time(&startTime);
	
	
	if(argc==1) PrintHelp();
	
	
	ReadParameters(argc, argv);
	srand(randSeed);	
	
	if(SILENCE<=1) printf("Random seed is %d.\n", randSeed);
	if(graph.ReadGraph() == -1){
		printf("Couldn't read graph file %s.\n", graph.GraphFN);
		exit(1);
	}
	
	if(graph.TabuLength / graph.TabuTol >= graph.Order-1){
		printf("Your tabu list is too long.  Choose a shorter length.  Setting to zero.\n");
		graph.TabuLength = 0;
		graph.TabuTol = 1;
	}
	
	RNSC();
	
	
	//Write best clustering to file.
	WriteClusteringF();
	
	//if(SILENCE<=1) printf("\n");
	//	PrintStatsShort(NaiveLengths, NumExper, "Lengths");
	//	PrintStatsShort(ScaledLengths, NumExper, "Lengths");
	int * BestCostsInt = new int[NumExper];
	for(int i=0; i<NumExper; i++) BestCostsInt[i] = (int)BestCosts[i];
	//	PrintStatsShort(BestCostsInt, NumExper, "sdf");
	//	PrintStatsShort(Times, NumExper, "sdflj");
	//printf("\n");
	if(SILENCE<=3) PrintStats(BestCosts, NumExper, "Costs");
	if(SILENCE<=4) PrintStats(Times, NumExper, "Times");
	
	//	printf("%d\n",graph.LoadClustering(ClusteringF));
	
	
}
