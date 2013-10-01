/****************************************************************************
 * Copyright (C) 2012 Rodrigo Aldecoa and Ignacio Marín                     *
 *                                                                          *
 *  This program is free software: you can redistribute it and/or modify    *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, either version 3 of the License, or       *
 *  (at your option) any later version.                                     *
 *                                                                          *
 *  This program is distributed in the hope that it will be useful,         *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *                                                                          *
 * Contact info: Rodrigo Aldecoa <raldecoa@ibv.csic.es>                     *
 ****************************************************************************/



/*
 computeSurprise calculates the Surprise of the partition of a network
 This program needs Surprise.cpp and Surprise.h to work properly. 
 You should have received Surprise.cpp and Surprise.h along with 
 this program.

 If you use this program, please cite:
       Aldecoa R, Marín I (2011)
       Deciphering network community structure by Surprise
       PLoS ONE 6(9): e24195

 The program receives two input files:
  - networkFile: A network represented by a list of links (pairs of nodes)
                 Each line contains two nodes separated by a 'tab'
  - partitionFile: Describes a given partition of the network
                 Each line contains a node and the community to which it
                 is assigned, separated by a 'tab'.
                 (The partition identifier must be a number)
  
 ** Two toy examples of these files are included within this folder 
    (network.pairs and partition.part)
*/

					     
// Surprise parameters are F, M, n and p


#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "Surprise.cpp"

using namespace std;

void printHelp()
{
  cerr << "Usage: " << endl;
  cerr << "ga <Graph file> <partition (optional)>" << endl << endl;
}

void printErrorFile(const string file)
{
  cerr << "Error in " << file << " file" << endl;
  cerr << "Either it does not exist or the format is wrong" << endl << endl;
}

vector<vector<int> > * readGraph(const string& nwFile, map<string, int>& nodes)
{

  vector<vector<int> > * adjList = new vector<vector<int> >();

  //Open the file
  ifstream fStream(nwFile.c_str(), std::ios::in);
  if(!fStream)
    return NULL;

  //Opened. Let's read the graph
  map<string, int>::const_iterator it;
  const int maxLineLength = 256;
  char line[maxLineLength];
  int index = 0;


  while (fStream.getline(line, maxLineLength)){
    
    string x, y;
    std::istringstream ss(line);
    while(ss >> x >> y){
    
      string noEndOfLine;            // If there is something else in ss
      if(ss >> noEndOfLine)          // apart from the pair of nodes,
	return NULL;                 // the format is wrong
      else{

	// Check if x has appeared before
	it = nodes.find(x);
	if(it == nodes.end())
	  {
	    nodes[x] = index++;
	    vector<int> tmp;
	    adjList->push_back(tmp);
	  }
	// Check if y has appeared before
	it = nodes.find(y);
	if(it == nodes.end())
	  {
	    nodes[y] = index++;
	    vector<int> tmp;
	    adjList->push_back(tmp);
	  }
	// Add link
	int from = nodes[x];
	int to = nodes[y];
	if(from > to)
	  swap(from, to);
	adjList->at(from).push_back(to);
      }
    }
  }
  return adjList;
}

vector<int> * readPartition(const string partFile,
			    map<string, int>& nodes)
{
  vector<int> * partition = new vector<int>(nodes.size());
  //Open the file
  ifstream fStream(partFile.c_str(), std::ios::in);
  if(!fStream)
    return NULL;
  
  //Opened. Let's read the graph
  map<string, int>::const_iterator it;
  const int maxLineLength = 256;
  char line[maxLineLength];

  // Remove first line
  fStream.getline(line, maxLineLength);

  while (fStream.getline(line, maxLineLength)){
    
    string label, comm;
    std::istringstream ss(line);
    while(ss >> label >> comm){
      
      string noEndOfLine;            // If there is something else in ss
      if(ss >> noEndOfLine)          // apart from the label and its comm
	return NULL;                 // the format is wrong
      else{
	int c = atoi(comm.c_str());
	int index = nodes[label];
	partition->at(index) = c;
      }
    }
  }
  return partition;
}

vector<int> * computeMemberships(vector<int> * partition)
{
  vector<int> * memberships = new vector<int>();
  std::map<int, int> comms;
  std::map<int, int>::const_iterator itMap;
  int index = 0; 
  for(std::vector<int>::const_iterator it = partition->begin();
      it != partition->end(); ++it)
    {

      itMap = comms.find(*it);
      if(itMap == comms.end()) // community index not found
	{
 	  comms[*it] = index++;
	  memberships->push_back(1);
 	}
      else
	{
	  memberships->at(comms[*it])++;
	}
    }
  return memberships;
}

int main(int argc, char **argv){

  if(argc != 3){
    cerr << "Usage: " 
	 << argv[0] 
	 << " network_file partition_file" 
	 << endl;
  }
  else{

    char *nwFile = argv[1];
    char *partFile = argv[2];
    
    map<string, int> nodes; // Map label->index
    vector<vector<int> > * adjList = readGraph(nwFile, nodes);
    if(adjList == NULL)
      {
	printErrorFile(nwFile);
	printHelp();
	return 1;
      }

    vector<int> * partition = readPartition(partFile, nodes);
    if(partition == NULL)
      {
	printErrorFile(partFile);
	printHelp();
	return 1;
      }
    

    vector<int> * memberships = computeMemberships(partition);
        
    // Calculate parameter F
    double nNodes = nodes.size();
    double F = nNodes * (nNodes - 1) / 2;

    // Calculate parameter n
    double n = 0.0;
    vector<vector<int> >::const_iterator it;
    for(it = adjList->begin(); it != adjList->end(); ++it)
      n += (*it).size();

    // Calculate parameter M
    double M = 0.0;
    for(vector<int>::const_iterator it = memberships->begin();
	it != memberships->end(); ++it)
      {
	double s = *it;
	M += s * (s-1) / 2;
      }


    // Calculate parameter p
    double p = 0.0;
    for(int i = 0; i < (int) adjList->size(); ++i)
      for(vector<int>::const_iterator it = adjList->at(i).begin();
	it != adjList->at(i).end(); ++it)
	{
	  if(partition->at(i) == partition->at(*it))
	    p++;
	}
 

    //Read partition file
    double surprise = computeSurprise(F,M,n,p);
    cout << "Surprise = " << surprise << endl;
  }
}
