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
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "Graph.h"
#include "SCluster.h"
#include "UVCluster.h"
#include "UpgmaCluster.h"
#include "Upgma.h"

using namespace std;

void printHelp()
{
  cerr << "Usage: " << endl;
  cerr << "jerarca <Graph file> <Algorithm [scluster or uvcluster]> <Iterations>" 
       << endl << endl;
  cerr << "For help and a more detailed explanation, "
       << "please visit http://xxx" << endl;
}

void printPartition(vector<int>* partition, 
		    vector<string> names,
		    const char* alg)
{
  string a = alg;
  string s = "partition_" + a + ".txt";
  ofstream off;
  off.open(s.c_str());
  for(unsigned int i = 0; i < partition->size(); ++i)
    off << names[i] << "\t" << partition->at(i) << endl;
  off.close();
}

int main(int argc, char** argv)
{

  if(argc != 4){
    printHelp();
    return 1;
  }

  const char* file = argv[1];
  const char* alg = argv[2];
  int nIter = atoi(argv[3]);
  
  if(nIter < 1){
    cerr << "ERROR: The number of iterations must be an integer positive" 
	 << endl << endl;
    return 1;
  }


  Graph G;
  FileErrorType fError = G.readGraphFromFile(file); 
  if(fError){
    cerr << "Problem with graph file => ";
    if(fError == OpenError)
      cerr << "Unable to open: " << file << endl;
    else if(fError == WrongFormatError)
      cerr << "Wrong file format" << endl;
    return 1;
  }



  // Iterative algorithm.
  // Creation of the matrix of secondary distances
  vector<string> nodesNames = G.getNodesNames();
  vector<vector<bool> > * adjacencyMatrix = G.getAdjacencyMatrix();

  DoubleMatrix * secondaryDistances;  

  if(!strcmp(alg, "scluster"))
    {
      SCluster u(G);
      u.run(nIter);
      secondaryDistances = u.getSecondaryDistances();
    }
  else if(!strcmp(alg, "uvcluster"))
    {
      UVCluster u(G);
      u.run(nIter);
      secondaryDistances = u.getSecondaryDistances();
    }
  else
    {
      cerr << "Wrong algorithm. [scluster or uvcluster]" << endl;
      return 1;
    }

  // Run UPGMA
  Upgma uu(secondaryDistances, adjacencyMatrix);
  vector<int>* partition = uu.getBestPartition();
  delete secondaryDistances;

  // Print best partition to file
  printPartition(partition, nodesNames, alg);
  delete partition;
}
