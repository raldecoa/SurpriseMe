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

#ifndef _GRAPH_H
#define _GRAPH_H

#include <vector>
#include <list>
#include <map>
#include <string>

enum FileErrorType{
  Ok                 = 0,
  OpenError          = 1,
  WrongFormatError   = 2
};

class Graph{

 public:
 
  Graph();
  ~Graph();

  // Reads a file containing a pair of nodes in each line.
  // Each pair of nodes represents an edge of the graph
  // Returns an error if something goes wrong
  const FileErrorType readGraphFromFile(const std::string& name);

  // Returns the number of nodes
  inline const int nNodes () const
  {
    return nNodes_;
  }

  // Returns the adjacency matrix
  std::vector<std::vector<bool> > * getAdjacencyMatrix();

  // Returns the adjacency list
  std::vector<std::list<int> > * getAdjacencyList();

  // Returns a vector containing the name of the nodes
  const std::vector<std::string> getNodesNames() const;

  // Returns true if the link exists and false otherwise
  inline const bool isLink(int x, int y) const
  {
    return adjacencyMatrix_[x][y];
  }

  // Updates the adjacency matrix
  void updateAdjacencyMatrix(const int rows, const int cols);

  // Sets position (i, j) to value
  inline void setAdjacencyMatrix(const int i, const int j, const bool value)
  {
    adjacencyMatrix_[i][j] = value;
  }

  // Returns a list containing every neighbor of node x
  void getNeighbors(const int x, std::list<int>& neighbors) const;

  // Prints the adjacency list
  void printAdjacencyList() const;

  // Prints the adjacency matrix
  void printAdjacencyMatrix() const;


 private:
  
  // Number of nodes
  int nNodes_;

  // The names of the nodes
  std::vector<std::string> nodesNames_;

  // The adjacency list (x < y)
  std::vector<std::list<int> > adjacencyList_;
  
  // The inverse adjacency list (x > y)
  std::vector<std::list<int> > invAdjacencyList_;

  // The adjacency matrix
  std::vector<std::vector<bool> > adjacencyMatrix_;

  // Adds a new node
  void addNode();

  // Adds a new edge
  void addEdge(int x, int y);

  // Creates the adjacency matrix
  void createAdjacencyMatrix();

};


const int insertNodeToTmpMap(const std::string& nodeName,
			     std::map<std::string, int>& tmpMap,
			     bool& isNew);

#endif // _GRAPH_H
