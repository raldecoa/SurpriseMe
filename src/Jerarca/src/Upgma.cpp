#include "Upgma.h"

Upgma::Upgma(std::vector<std::vector<double> > * distances,
	     std::vector<std::vector<bool> > * adjacencyMatrix)
{
  adjMatrix = adjacencyMatrix;
  clustersDistances = distances;
  N = clustersDistances->size();
  result = 0;
  bestS = 0.0;

  partition = new std::vector<int>();
  bestPartition = new std::vector<int>();
  communities = new std::vector<std::vector<int> >();

  //creates initial clusters, each one containing a single node
  for(int i = 0; i < N; i++){
    UpgmaCluster * c = new UpgmaCluster();
    c->index = 1;
    //c->height = 0;
    clusters.push_back(c);

    //create initial partition
    partition->push_back(i);
    bestPartition->push_back(i);
    std::vector<int> tmp(1, i);
    communities->push_back(tmp);

  }


  // Surprise parameters
  F = N * (N - 1) / 2;
  n = 0.0;
  for(unsigned int i = 0; i < adjMatrix->size(); ++i)
    for(unsigned int j = i+1; j < adjMatrix->size(); ++j)
      n += adjMatrix->at(i).at(j);
  M = 0.0;
  p = 0.0;
}

Upgma::~Upgma()
{
}

std::vector<int> * Upgma::getBestPartition(){
  runUpgma();
  return bestPartition;
}


//performs the tree building operations
void Upgma::runUpgma(){
  int index = N;
  while(clusters.size() != 1){
    performOnePass(index);
    index++;
  }

  delete partition;
  for(unsigned int i = 0; i < communities->size(); ++i)
    communities->at(i).clear();
  delete communities;
}

//merges the two nearest clusters into one and adds it to the tree
void Upgma::performOnePass(int x){

  int newComm = x + N;

  //find nearest clusters
  double mindist = clustersDistances->at(0).at(1);
  int ind1 = 0, ind2 = 1;

  for(size_t i = 0; i < clusters.size(); i++){
    for(size_t j = i+1; j < clusters.size(); j++){
      double dis = clustersDistances->at(i).at(j-i-1);
      if(dis < mindist){
	mindist = dis;
	ind1 = i;
	ind2 = j;
      }
    }
  }
  
  // WILL JOIN ind1 and ind2
  // Update Suprise parameters
  
  // p  
  int size1 = communities->at(ind1).size();
  int size2 = communities->at(ind2).size();
  for(int i = 0; i < size1; ++i)
    for(int j = 0; j < size2; ++j)
      p += isLink(communities->at(ind1).at(i), communities->at(ind2).at(j));
  
  // M
  M -= (size1 * (size1 - 1)) / 2.0;
  M -= (size2 * (size2 - 1)) / 2.0;
  
  int newSize = size1 + size2;
  M += (newSize * (newSize - 1) / 2.0);

  // Update partition
  for(int i = 0; i < size1; ++i){
    int node = communities->at(ind1).at(i);
    partition->at(node) = newComm;
  }
  for(int i = 0; i < size2; ++i){
    int node = communities->at(ind2).at(i);
    partition->at(node) = newComm;
  }

  // Update communities
  communities->at(ind1).insert(communities->at(ind1).end(),
			       communities->at(ind2).begin(),
			       communities->at(ind2).end());
  communities->erase(communities->begin() + ind2);


  // Best partition?
  double s = computeSurprise(F, M, n, p);
  if(s > bestS)
    {
      bestS = s;
      for(int i = 0; i < N; ++i)
	bestPartition->at(i) = partition->at(i);
    }

  //new cluster
  UpgmaCluster * newcluster = new UpgmaCluster();
  
  //merges the two nearest clusters into one
  newcluster->index = clusters[ind1]->index + clusters[ind2]->index;

  
  //cluster height
  //newcluster->height = x;
  
  //store old clusters
  newcluster->left = clusters[ind1];
  newcluster->right = clusters[ind2];
  
  //calculates new distances (fast method)
  double c1 = newcluster->left->index;
  double c2 = newcluster->right->index;

  std::vector<double> v;
  for(int i = 0; i < (int) clusters.size(); i++){
    if(i == ind1 || i == ind2)
      v.push_back(0);
    else
      if(i < ind1)
	v.push_back((clustersDistances->at(i).at(ind1-i-1) * c1 
		     + clustersDistances->at(i).at(ind2-i-1) * c2) 
		    / (c1 + c2));
      else if(i < ind2)
	v.push_back((clustersDistances->at(ind1).at(i-ind1-1) * c1 
		     + clustersDistances->at(i).at(ind2-i-1) * c2) 
		    / (c1 + c2));
      else
	v.push_back((clustersDistances->at(ind1).at(i-ind1-1) * c1 
		     + clustersDistances->at(ind2).at(i-ind2-1) * c2) 
		    / (c1 + c2));
  }

  //inserts new distances
  clustersDistances->at(ind1) = v;
  for(int i = 0; i < ind1; i++)
    clustersDistances->at(i).at(ind1-i-1) = v[i];
  for(size_t i = ind1+1; i < clusters.size(); i++)
    clustersDistances->at(ind1).at(i-ind1-1) = v[i];
  
  // Remove references to ind2
  for(int i = 0; i < ind2; i++){
    std::vector< double >::iterator Iter;
    Iter = clustersDistances->at(i).begin() + (ind2-i-1) ;
    clustersDistances->at(i).erase(Iter);
  }

  // Remove ind2
  std::vector<std::vector<double> >::iterator Iter2;
  Iter2 = clustersDistances->begin() + ind2;
  clustersDistances->erase(Iter2);
  
  //inserts new merged cluster;
  //  delete clusters[ind1];
  clusters[ind1] = newcluster;
  
  //removes old cluster
  std::vector<UpgmaCluster*>::iterator Iter;
  Iter = clusters.begin() + ind2;
  clusters.erase(Iter);  
}

bool Upgma::isLink(int i, int j)
{
  return adjMatrix->at(i).at(j);
}
