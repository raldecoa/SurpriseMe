#ifndef UPGMA_H
#define UPGMA_H

#include <vector>
#include <list>
#include <iostream>

#include "UpgmaCluster.h"
#include "Surprise.h"


class Upgma
{
 public:
  Upgma(std::vector<std::vector<double> > * distances,
	std::vector<std::vector<bool> > * adjacencyMatrix);
  ~Upgma();
  
  UpgmaCluster* getResult();
  std::vector<int> * getBestPartition();
    
 private:
  int N;
  
  std::vector<UpgmaCluster*> clusters;
  std::vector<std::vector<double> >* clustersDistances;

  std::vector<int> *partition;
  std::vector<int> *bestPartition;
  std::vector<std::vector<int> > * communities;

  std::vector<std::vector<bool> > * adjMatrix;

  UpgmaCluster* result;

  // Surprise parameters
  double F, n, M, p;
  double bestS;

  //methods
  void computeDistances();
  bool isLink(int i, int j);

  void runUpgma();
  void performOnePass(int x);
};

#endif
