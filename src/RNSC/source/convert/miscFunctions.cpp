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
#include <sys/time.h>
#include <sys/resource.h>
#include "linkedList.h"
#include "graph.h"
#include "statsBook.h"
#include "experiment.h"
#include "definitions.h"


long nChoose2(long n){
        if(n<0){
                fprintf(stderr,"Error: Performing nChoose2 on negative n.\n");
        }
        return((n*n - n)/2);
}

void PrintStats(double Samples [], int NumSamples, const char * name)//Stats. Returns min, mean, and standard deviation.
{
	double StdDev=0, Mean=0, Min=BIG_COST;
	
	for(int i=0; i<NumSamples; i++){
		Mean += Samples[i];
		if(Samples[i]<Min) Min = Samples[i];
	}
	Mean = Mean / NumSamples;
	
	for(int i=0; i<NumSamples; i++){ 
		StdDev += (Mean - Samples[i])*(Mean - Samples[i]);
	}
	
	StdDev = StdDev / NumSamples;
	StdDev = sqrt(StdDev);
	
	printf("%s:   Min: %11.2f\t Mean: %11.2f\t  StdDev: %11.2f\n", name, Min, Mean, StdDev);

}

void PrintStatsShort(int Samples [], int NumSamples, const char * name)//Stats. Returns min, mean, and standard deviation. Minimal printing for data.
{
	double StdDev=0, Mean=0, Min=BIG_COST;
	
	for(int i=0; i<NumSamples; i++){
		Mean += Samples[i];
		if(Samples[i]<Min) Min = Samples[i];
	}
	Mean = Mean / NumSamples;
	
	for(int i=0; i<NumSamples; i++){ 
		StdDev += (Mean - Samples[i])*(Mean - Samples[i]);
	}
	
	StdDev = StdDev / NumSamples;
	StdDev = sqrt(StdDev);
	
	//	printf("%s: Min: %11.2f  Avg: %11.2f  Dev: %11.2f  ", name, Min, Mean, StdDev);
	printf("%11.3f  ",Min);//Mean);
}

void PrintStatsShort(double Samples [], int NumSamples, const char * name)//Stats. Returns min, mean, and standard deviation. Minimal printing for data.
{
	double StdDev=0, Mean=0, Min=BIG_COST;
	
	for(int i=0; i<NumSamples; i++){
		Mean += Samples[i];
		if(Samples[i]<Min) Min = Samples[i];
	}
	Mean = Mean / NumSamples;
	
	for(int i=0; i<NumSamples; i++){ 
		StdDev += (Mean - Samples[i])*(Mean - Samples[i]);
	}
	
	StdDev = StdDev / NumSamples;
	StdDev = sqrt(StdDev);
	
	//	printf("%s: Min: %11.2f  Avg: %11.2f  Dev: %11.2f  ", name, Min, Mean, StdDev);
	printf("%11.3f  ",Min);//Mean);
}

double getTime()
{
	struct rusage resources;

	if(getrusage(RUSAGE_SELF, &resources) != 0){
		fprintf(stderr,"ERROR in timings\n"); exit(-1);
	}
	return( (double) resources.ru_utime.tv_usec/1000000
		+ (double) resources.ru_utime.tv_sec);
}
