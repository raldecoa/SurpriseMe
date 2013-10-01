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
const char * EdgeListFilename;

double BestScaledCost;//this is the scaled cost of the final cluster.
int * ClusteringF;//this is the final clustering.

double * BestCosts;
double * Times;
double * CC;
int * ScaledLengths;
int * NaiveLengths;
double TimesSum;//running total of times.

//global flags.
bool GF_InputGiven = false;
bool GF_SkipNaive = false;
bool GF_ReadEdgeList = false;











void PrintHelp()
{
	printf("\nRNSCCONVERT.  Usage:\n\n\trnscconvert -g OutputGraphFilename"
		   "\n\t            -i InputEdgelistFilename"
		   "\n\t            -n OutputNameFilename"
		   
		   "\n\n\tFOR MORE INFORMATION ON PARAMETERS, SEE README.TXT.\n\n"
		   );
	
	
	exit(1);
}


void ReadParameters(int number, char ** vector)
{
	int c;
	while ((c = getopt(number, vector, "i:g:n:")) != EOF) {
		switch (c) {
				
			case 'h': //Print help and exit.
				PrintHelp();
				break;
				
			case 'i': //An input text-based edge list.  Whitespace delimited, newline split.
				GF_ReadEdgeList = true;
				graph.EdgeListFilename = optarg;
				break;
			case 'g': //Sets the graph filename.
				graph.GraphFN = optarg;
				break;
			case 'n': //Sets the names filename
				graph.NamesFilename = optarg;
				break;
			case '?': //invalid command line format
				fprintf(stderr,"Invalid parameter.  Use \n   -i for the input file\n   -g for the graph output filename\n   -n for the names output filename.\n\n");
				
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
		exit(1);
		return;
	}
	
}



/********************************
 *
 *
 *
 */
void WriteClusteringF(){
	
	ofstream fStream(OutputFilename);
	if(fStream){
		for(int cluster = 0; cluster<graph.NumClust; cluster++){
			graph.ClusterList[cluster].RewindToHead();
			for(int v = 0; v<graph.ClusterSize[cluster]; v++){
				fStream << graph.ClusterList[cluster].CurrentPtr->Vertex << " ";
				graph.ClusterList[cluster].Advance();
			}
			fStream << "-1" << endl;
		}	
		fStream.close();
	}
	else {
		fprintf(stderr,"Output file open failed. (%s)\n", OutputFilename);
	}
	
}


/********************************
 *
 *
 *
 */
int main(int argc, char **argv)
{
	//Only input parameters are input file, output graph file, and output names file.
	if(argc==1) PrintHelp();
	
	ReadParameters(argc, argv);
	
	if(graph.EdgeListFilename==NULL){
		printf("Must specify an input edge list filename with -i flag.\n");
	}
	if(graph.GraphFN==NULL){
		printf("Must specify an output graph filename with -g flag.\n");
	}
	if(graph.NamesFilename==NULL){
		printf("Must specify an output names/functions filename with -n flag.\n");
	}
	
	if(graph.EdgeListFilename==NULL || graph.GraphFN==NULL || graph.NamesFilename==NULL) exit(1);
	
	
	
	if(graph.ReadEdgeList() == -1){
		printf("Couldn't read verbal edge list.  Filename %s failed.\n", graph.EdgeListFilename);
		exit(1);
	}
	
	if(graph.WriteGraph()) fprintf(stdout, "\n    RNSC formatted graph file written to %s.\n\n",graph.GraphFN);
	if(graph.WriteNames()) fprintf(stdout, "    Protein names/functions written to %s.\n\n", graph.NamesFilename);
	
	
	
}
