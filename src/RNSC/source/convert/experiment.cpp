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
#include <math.h>
#include <fstream>
#include <ctype.h>
#include "linkedList.h"
#include "graph.h"
#include "statsBook.h"
#include "experiment.h"
#include "definitions.h"


Experiment::Experiment(){
   BestNaiveCost = BIG_COST;
   BestScaledCost = BIG_COST;

   P_NaiveStoppingTol = 0;
   P_ScaledStoppingTol = 0;
   P_ShuffleFreq = BIG_COST;
   P_ShuffleLength = 0;//never diversify.
   P_DestroyFreq = BIG_COST;//never diversify.
   P_SkipNaive = false;
   P_WriteNaive = false;
   P_WriteScaled = true;

   NMatlabFN = "mat_n.dat";
   SMatlabFN = "mat_s.dat";
	

}

void Experiment::ClusteringSAlloc(Graph& graph)
{
   ClusteringS = new int [graph.Order];
}

//No destructor needed, since there is no object memory allocated.
Experiment::~Experiment(){
   //if(ClusteringS) delete [] ClusteringS;//getting a seg fault for some reason.
   ClusteringS = NULL;
}



//*****************************************************************************
  /* RunExperiment***************************************************************
******************************************************************************/
// RunExperiment performs a single experiment of RNSC. See Fig 3.1 in my
// Master's thesis for an explanation.
void Experiment::RunExperiment(Graph& graph)
{


   if(!P_SkipNaive){//Run the naive scheme.

      if(SILENCE<=2) printf("\tRunning naive scheme....\n");

      //Initialize the clustering C_0 for the experiment.
      graph.InitClustering();

      //Initialize the naive move scheme, i.e. MoveList, NaiveCost,
      //FirstWithCost, NumWithCost, MoveListPtr.
      graph.InitNaiveScheme();
		
      //Set the best clustering to the initial clustering.
      BestNaiveCost = graph.NaiveCost;
      for(int i=0; i<graph.Order; i++){
	 ClusteringS[i] = graph.WhichCluster[i];
      }

      //Run the naive scheme.
      if(SILENCE<=1) printf("\t\tInitial naive cost  \t=%14d\n",BestNaiveCost);
      RunNaiveScheme(graph);
      if(SILENCE<=1) printf("\t\tFinal naive cost    \t=%14d\n", BestNaiveCost);
		
      //Delete the move list.
      graph.PurgeMoveList();

      //if(SILENCE<=1) printf("%d\t",graph.LoadClustering(ClusteringS));
      //Adjust the clustering, i.e. set the graph's current clustering to C_n.
      graph.InitClustering(ClusteringS);

   } else {//the naive scheme was not run.
      //We need to initialize the clustering C_0 for the experiment.
      //That is, we don't run the naive scheme, so C_n = C_0.
      graph.InitClustering();
   }

   if(P_ScaledStoppingTol > 0){//Run the scaled scheme.
      if(SILENCE<=2) printf("\tRunning scaled scheme....\n");

      //Initialize the scaled move scheme, i.e. MoveList, ScaledCost,
      //FirstWithCost, NumWithCost, MoveListPtr.
      graph.InitScaledScheme();
		
      //Set the best clustering to the initial clustering, i.e. C_s := C_n.
      BestScaledCost = graph.ScaledCost;
      for(int i=0; i<graph.Order; i++){
	 ClusteringS[i] = graph.WhichCluster[i];
      }


      //Run the scaled scheme.
      if(SILENCE<=1) printf("\t\tInitial scaled cost \t= %13.3f\n", BestScaledCost);
      RunScaledScheme(graph);
      if(SILENCE<=1) printf("\t\tFinal scaled cost   \t= %13.3f\n", BestScaledCost);
		
      //Clean up the scaled move scheme.
      graph.PurgeMoveList();

      //if(SILENCE<=1) printf("     %d\n",graph.LoadClustering(ClusteringS));
      //Adjust the clustering, i.e. set the graph's current clustering to C_s.
      graph.InitClustering(ClusteringS);

   } else {//The scaled scheme is not run. Get the scaled cost for C_n.
      BestScaledCost = graph.GetScaledCostForClustering();
   }

}



void Experiment::RunNaiveScheme(Graph& graph)
{
   int MovesSinceImprovement = 0;
   NaiveLength = 0;

   while(1){

      if(MovesSinceImprovement > P_NaiveStoppingTol) break;

      if(NEIGHBOURHOODS){
	 FILE * fS = fopen("nhoodsizes.dat","a");
	 fprintf(fS,"%4d",(int)NaiveLength);
	 fclose(fS);
      }

      //Make a move and update everything.
      MakeANaiveMove(graph);
		

      if(graph.NaiveCost < BestNaiveCost){
	 //this is the best naive clustering so far.
	 MovesSinceImprovement = 0;
	 for(int i=0; i < graph.Order; i++){
	    ClusteringS[i] = graph.WhichCluster[i];
	 }
	 BestNaiveCost = graph.NaiveCost;

	 //printf("\t\tNew best naive cost %d.\n",BestNaiveCost);
      } else {
	 MovesSinceImprovement ++;
      }

      NaiveLength++;

   }
}

void Experiment::RunScaledScheme(Graph& graph)
{
	int DeadCluster = -1;
	int MovesSinceImprovement = 0;
	int MoveNumber = 0;
	int DestroyMoveNumber = 0;
	FILE * fStream;
	bool fileOpen=false;
	
	if(MATLAB_WRITE){
		fileOpen = true;
		if(!(fStream = fopen(SMatlabFN,"w"))){ //write file open
			printf("Failed to open scaled matlab file. (%s)\n",SMatlabFN);
			fileOpen = false;
		}
	}
	
	while(1){//using tolerance instead of length.
		
		
		if(MovesSinceImprovement > P_ScaledStoppingTol) break;
		
		
		if(((MoveNumber % P_DestroyFreq) == P_DestroyFreq-1) && !P_ShuffleLength){
			//diversify, i.e. destroy a cluster.
			//we don't hold out any hope that this will actually improve the cost.
			
			//pick a non-empty, non-singleton cluster randomly, uniformly.
			DeadCluster = rand() % graph.NumClust;
			while(graph.ClusterSize[DeadCluster] <= 1) DeadCluster = rand() % graph.NumClust;
			
			while(graph.ClusterSize[DeadCluster] > 1){//leaves the cluster as a singleton.
				DestroyMoveNumber ++;
				if(NEIGHBOURHOODS){
					FILE * fS = fopen("nhoodsizes.dat","a");
					fprintf(fS,"%4d",(int)MoveNumber);
					fclose(fS);
				}
				MakeADestroyMove(graph, DeadCluster);
			}
			
		} else if(((MoveNumber % P_ShuffleFreq) >= (P_ShuffleFreq - P_ShuffleLength)) && P_ShuffleLength){
			//Shuffling diversification being used.
			if(NEIGHBOURHOODS){
				FILE * fS = fopen("nhoodsizes.dat","a");
				fprintf(fS,"%4d",(int)MoveNumber);
				fclose(fS);
			}
			MakeAShuffleMove(graph);
			
		} else {//we just make a global move.
			//Make a move and update everything.
			if(NEIGHBOURHOODS){
				FILE * fS = fopen("nhoodsizes.dat","a");
				fprintf(fS,"%4d",(int)MoveNumber);
				fclose(fS);
			}
			MakeAScaledMove(graph);
			if(graph.ScaledCost < BestScaledCost - .00001){//allows for .00001 floating-point error.
				//this is the best scaled clustering so far.
				MovesSinceImprovement = 0;
				for(int i=0; i < graph.Order; i++){
					ClusteringS[i] = graph.WhichCluster[i];
				}
				BestScaledCost = graph.ScaledCost;
			} else {
				MovesSinceImprovement ++;
			}
		}
		
		if(fileOpen && MATLAB_WRITE){
			fprintf(fStream,"%f\n",graph.ScaledCost);
		}
		
		
		MoveNumber++;
	}
	
	ScaledLength = MoveNumber + DestroyMoveNumber;
}


void Experiment::MakeANaiveMove(Graph& graph){
	graph.FindAGoodMove();
	//printf("MOVE: %d from %d to %d. (cost = %d)\n", graph.Move[0], graph.Move[1], graph.Move[2],
	//		 (int) graph.MovePtr->Cost);
	
	graph.NaiveUpdate();
}


void Experiment::MakeAScaledMove(Graph& graph){
	graph.FindAGoodMove();
	//printf("MOVE: %d from %d to %d. (cost = %.2f)\n", graph.Move[0], graph.Move[1], graph.Move[2],
	//		 graph.MovePtr->Cost);
	
	graph.ScaledUpdate();
}

void Experiment::MakeADestroyMove(Graph& graph, int cluster){
	graph.FindADestroyMove(cluster);
	graph.ScaledUpdate();
}

void Experiment::MakeAShuffleMove(Graph& graph){
	graph.FindAShuffleMove();
	graph.ScaledUpdate();
}

