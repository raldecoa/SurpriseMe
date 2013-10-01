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
#ifndef _UVCLUSTER_H
#define _UVCLUSTER_H


#include "Graph.h"
#include "MersenneTwister.h"

typedef std::vector<std::vector<double> > DoubleMatrix;
typedef std::vector<std::vector<int> > IntMatrix;


class UVCluster{

 public:

  UVCluster(const Graph& graph);
  ~UVCluster();

  // Runs the UVCluster algorithm and computes the count matrix
  void run(const int nIter);
  DoubleMatrix * getSecondaryDistances();

 private:

  //
  int nNodes_;
  int nIter_;

  // Graph
  Graph graph_;

  // Random number generator
  MTRand R_;

  // Count matrix
  IntMatrix countMatrix_;

  // Secondary distances
  DoubleMatrix * secondaryDistances_;


  // Checks if node is linked to every node of cluster
  inline const bool linkedToAll(const int node, 
				const std::vector<int>& cluster) const;
  
};
#endif // _UVCLUSTER_H
