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
#include <algorithm>
#include <fstream>
#include <sstream>
using std::string;


#include "Graph.h"


Graph::Graph(){}

Graph::~Graph(){}

const FileErrorType Graph::readGraphFromFile(const string& name)
{
  //Open the file
  std::ifstream fStream(name.c_str(), std::ios::in);
  if(!fStream)
    return OpenError;
  
  //Opened. Let's read the graph
  std::map<string, int> tmpMap; // Mapping: node_name -> node_index
  
  const int maxLineLength = 256;
  char line[maxLineLength];
  while (fStream.getline(line, maxLineLength)){
    
    string nodeX, nodeY;
    int indexX, indexY;
    bool isNewX, isNewY;
    std::istringstream ss(line);
    while(ss >> nodeX >> nodeY){
    
      string noEndOfLine;            // If there is something else in ss
      if(ss >> noEndOfLine)          // apart from the pair of nodes,
	return WrongFormatError;     // the format is wrong
      else{
	//Add nodes to tmpMap
	indexX = insertNodeToTmpMap(nodeX, tmpMap, isNewX);
	indexY = insertNodeToTmpMap(nodeY, tmpMap, isNewY);
	
	if(isNewX){
	  addNode();
	  nodesNames_.push_back(nodeX);
	}
	if(isNewY){
	  addNode();
	  nodesNames_.push_back(nodeY);
	}
	addEdge(indexX, indexY);
      }
      if(nodeY.empty())             // If there is only a node in the line,
	return WrongFormatError;    // the format is wrong
    }
  }
  fStream.close();
  nNodes_ = adjacencyList_.size();
  createAdjacencyMatrix();  // If everything's ok, it creates the adjacency matrix
  for(int i = 0; i < nNodes_; ++i)
    adjacencyList_[i].clear();
  adjacencyList_.clear();
  return Ok;
}

std::vector<std::vector<bool> > * Graph::getAdjacencyMatrix()
{
  return &adjacencyMatrix_;
}

std::vector<std::list<int> > * Graph::getAdjacencyList()
{
  return &adjacencyList_;
}

const std::vector<std::string> Graph::getNodesNames() const
{
  return nodesNames_;
}

void Graph::addNode()
{
  std::list<int> l;
  adjacencyList_.push_back(l);
  invAdjacencyList_.push_back(l);
}

void Graph::createAdjacencyMatrix()
{
  adjacencyMatrix_.resize(nNodes_);
  for(int i = 0; i < nNodes_; ++i){
    adjacencyMatrix_[i].resize(nNodes_);
    for(int j = 0; j < nNodes_; ++j)
      adjacencyMatrix_[i][j] = false;
  }


  std::list<int>::const_iterator it;
  for(int i = 0; i < nNodes_; ++i)
    for(it = adjacencyList_[i].begin();	it != adjacencyList_[i].end(); ++it)
      adjacencyMatrix_[i][*it] = adjacencyMatrix_[*it][i] = true;
}

void Graph::addEdge(int x, int y)
{
  if(x > y)
    std::swap(x,y);
  
  adjacencyList_[x].push_back(y);
  invAdjacencyList_[y].push_back(x);
}

void Graph::getNeighbors(const int x, std::list<int>& neighbors) const
{
  neighbors.assign(adjacencyList_[x].begin(), adjacencyList_[x].end());
  neighbors.insert(neighbors.end(), 
		   invAdjacencyList_[x].begin(), invAdjacencyList_[x].end());
}

void Graph::updateAdjacencyMatrix(const int rows, const int cols)
{
  adjacencyMatrix_.resize(rows);
  for(int i = 0; i < rows; ++i)
    adjacencyMatrix_[i].resize(cols);
}

void Graph::printAdjacencyList() const
{
  std::list<int>::const_iterator it;
  for(int i = 0; i < (int)adjacencyList_.size(); ++i){
    std::cout << "[" << nodesNames_[i] << "] -> [ ";
    for(it = adjacencyList_[i].begin(); it != adjacencyList_[i].end(); ++it){
      int indexY = *it;
      std::cout << nodesNames_[indexY] << " ";
    }
    std::cout << "]" << std::endl;
  }
}

void Graph::printAdjacencyMatrix() const
{
  for(int i = 0; i < nNodes_; ++i){
    for(int j = 0; j < nNodes_; ++j)
      std::cout << adjacencyMatrix_[i][j] << " ";
    std::cout << std::endl;
  }
}

const int insertNodeToTmpMap(const string& nodeName,
			     std::map<string, int>& tmpMap,
			     bool& isNew)
{
  std::map<string, int>::const_iterator found = tmpMap.find(nodeName);
  if( found == tmpMap.end() ){
    int newIndex = (int) tmpMap.size();
    isNew = true;
    tmpMap.insert(std::pair<string, int>(nodeName, newIndex));
    return newIndex;
  }
  else{
    int oldIndex = (*found).second;
    isNew = false;
    return oldIndex;
  }
}
