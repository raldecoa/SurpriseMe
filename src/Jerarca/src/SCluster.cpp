/***********************************************************************
Jerarca - Efficient analysis of complex networks
This file is part of Jerarca.

Copyright (C) 2010 Rodrigo Aldecoa <raldecoa@ibv.csic.es>

    Jerarca is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jerarca is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jerarca.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/
#include <algorithm>
#include "SCluster.h"

SCluster::SCluster(const Graph& graph)
{
  graph_ = graph;
  R_.seed();
  nNodes_ = graph.nNodes();

  // Initialize countMatrix to 0
  countMatrix_.resize(nNodes_);
  for(int i = 0; i < nNodes_; ++i)
    for(int j = i+1; j < nNodes_; ++j)
      countMatrix_[i].push_back(0);
}

SCluster::~SCluster(){}


DoubleMatrix * SCluster::getSecondaryDistances()
{
  secondaryDistances_ = new DoubleMatrix(nNodes_);
  for(int i = 0; i < nNodes_; ++i){
    for(int j = i+1; j < nNodes_; ++j)
      secondaryDistances_->at(i).push_back(1.0 - (double) countMatrix_[i][j-i-1] / nIter_);
  }
  return secondaryDistances_;
}

void SCluster::run(const int nIter)
{
  nIter_ = nIter;
  for(int iter = 0; iter < nIter_; ++iter){
    
    //std::cout << iter << std::endl;    

    //Initialize nodesLeft
    std::vector<int> nodesLeft;
    for(int i = 0; i < nNodes_; ++i)
      nodesLeft.push_back(i);
    
    while(nodesLeft.size() > 0){
      
      // Select a random node and insert every adjacent node
      // in the same cluster
      std::vector<int> cluster;
      int index = R_.randInt(nodesLeft.size() - 1);
      int nodeX = nodesLeft[index];
      cluster.push_back(nodeX);
      
      nodesLeft.erase(nodesLeft.begin() + index);
      for(int i = 0; i < (int)nodesLeft.size(); ++i){
	int nodeY = nodesLeft[i];
	if(graph_.isLink(nodeX, nodeY)){
	  cluster.push_back(nodeY);
	  nodesLeft.erase(nodesLeft.begin() + i);
	  i--;
	}
      }

      // countMatrix +1
      for(int i = 0; i < (int)cluster.size(); ++i){
	for(int j = i+1; j < (int)cluster.size(); ++j){
	  int x = cluster[i];
	  int y = cluster[j];
	  if(x > y)
	    std::swap(x,y);
	  countMatrix_[x][y-x-1]++;
	}
      }

    } // while
  }
}
