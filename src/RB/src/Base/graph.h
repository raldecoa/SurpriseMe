/* graph.h
 * Copyright (C) (2011)  V.A. Traag, P. Van Dooren, Y. Nesterov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In case of any problems or bugs, please contact Vincent Traag at
 * vincent (dot) traag (at) uclouvain (dot) be
 *
 * This software is based on the article
 *
 * V.A. Traag, P. Van Dooren, Y. Nesterov, "Narrow scope for resolution-free
 * community detection" (2011) arXiv:1104.3083v1.
 *
 */
// Originally based on:
//-----------------------------------------------------------------------------
// Community detection
// Based on the article "Fast unfolding of community hierarchies in large networks"
// Copyright (C) 2008 V. Blondel, J.-L. Guillaume, R. Lambiotte, E. Lefebvre
//
// This program must not be distributed without agreement of the above mentionned authors.
//-----------------------------------------------------------------------------
// Author   : E. Lefebvre, adapted by J.-L. Guillaume
// Email    : jean-loup.guillaume@lip6.fr
// Location : Paris, France
// Time	    : February 2008
//-----------------------------------------------------------------------------

#ifndef GRAPH_H
#define GRAPH_H

//#define NDEBUG
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <deque>

#define WEIGHTED   0
#define UNWEIGHTED 1

#define OUTGOING   0
#define INCOMING   1

#define UNDIRECTED 0
#define DIRECTED   1

using namespace std;

class Graph {
 public:
  int nb_nodes;
  double total_nodes; //sum of node size
  int nb_links;
  int nb_layers;
  int is_weighted;
  int is_directed;

  int *nsize;
  int *degrees;
  double *weighted_degree_array;
  int *links;
  double *weights;
  double *self_weights;

  int *nb_nonnull_layers_per_node;
  int *nonnull_layers_per_node;
  int total_layer_per_node;

  double total_w;

  //the total_weight per layer
  double *total_weight_per_layer;

  Graph();
  Graph(Graph const& g);
  ~Graph();

  void free_mem();

  // binary file format is
  // 4 bytes to indicate whether the graph is weighted
  // 4 bytes to indicate whether it is directed
  // 4 bytes for the number of nodes in the graph
  // 4 bytes for the number of layers in the graph
  // 4*(nb_nodes) bytes for the cumulative degree for each node:
  //    deg(0)=degrees[0]
  //    deg(k)=degrees[k]-degrees[k-1]
  // 4*(sum_degrees) bytes for the links
  // IF WEIGHTED 4*(sum_degrees) bytes for the weights
  Graph(char *filename);

  Graph(int nb_nodes, int nb_layers, int nb_links, int *degrees, int *links, double *wweights);

  void init_weighted_degree();
  void init();
  void init_layers_per_node();
  void init_self_weights();

  void display(char* outfile);
  void display_binary(char *outfile);

  // return the number of neighbors (degree) of the node
  int nb_neighbors(int node);
  int nb_neighbors(int node, int layer, int direction);

  // return the weight of self loop of the node
  double self_weight(int node, int layer);

  // return the weighted degree of the node
  double weighted_degree(int node);
  double weighted_degree(int node, int layer, int direction);

  // return pointers to the first neighbor and first weight of the node
  pair<int *, double *> neighbors(int node);
  pair<int *, double *> neighbors(int node, int layer, int direction);

  //return total_weight per layer
  double total_weight(int layer);
  double total_weight();

  int nb_nonnull_layers(int node);
  int* nonnull_layers(int node);

 private:
  //function to aid looking up the correct degree in the cumulative
  //degree sequence. The actual cumulative degree can then be used
  //to obtain the index for the links.
  int degree_index(int node, int layer, int direction);
};

inline double Graph::total_weight(int layer)
{
  return total_weight_per_layer[layer];
}

inline double Graph::total_weight()
{
  double w = 0.0;
  for (int i = 0; i < nb_layers; i++)
    w += total_weight_per_layer[i];

  return w;
}

inline int
Graph::nb_neighbors(int node) {
  assert(node>=0 && node<nb_nodes);

  if (node==0)
    return degrees[degree_index(0, nb_layers-1, is_directed)];
  else
    return degrees[degree_index(node, nb_layers-1, is_directed)]-degrees[degree_index(node - 1, nb_layers-1, is_directed)];
}

inline int
Graph::nb_neighbors(int node, int layer, int direction)
{
  assert(node>=0 && node<nb_nodes && layer >= 0 && layer < nb_layers && direction >= 0 && direction < 2);

  int index = degree_index(node, layer, direction);
  if (index == 0)
    return degrees[index];
  else
    return degrees[index]-degrees[index-1];
}

inline int
Graph::degree_index(int node, int layer, int direction)
{
  return node*nb_layers*(is_directed+1) //dimension 1 - node
       + layer*(is_directed+1)          //dimension 2 - layer
       + direction*is_directed;         //dimension 3 - direction
}

inline double
Graph::self_weight(int node, int layer) {
  assert(node>=0 && node<nb_nodes);

  return self_weights[node*nb_layers+layer];

  return 0;
}

inline double
Graph::weighted_degree(int node) {
  assert(node>=0 && node<nb_nodes);

  double w = 0.0;
  int idx = 0;
  idx = degree_index(node, 0, OUTGOING);
  for (int layer = 0; layer < nb_layers; layer++)
  {
   w += weighted_degree_array[idx++]; //Get one and move on to the next (incoming)
   w += weighted_degree_array[idx++]; //Get one and move on to the next (outgoing)
  }

  return w;
}

inline double
Graph::weighted_degree(int node, int layer, int direction) {
   assert(node>=0 && node<nb_nodes && layer >= 0 && layer < nb_layers && direction >= 0 && direction < 2);

  double alt_weight =  weighted_degree_array[degree_index(node, layer, direction)];

  /*double res = 0;
  pair<int *,double *> p = neighbors(node, layer, direction);
   int nn = nb_neighbors(node, layer, direction);
   if (p.second==NULL)
     return (double)nn;
   else {
     for (int i=0 ; i< nn ; i++)
       res += *(p.second+i);
   }
   cerr << " correct: " << res << ",  and: " << alt_weight << endl;
   */
  return alt_weight;
}


inline pair<int *,double *>
Graph::neighbors(int node) {
  return neighbors(node, 0, 0);
  //the links and weights starting at layer 0 direction 0 are always the
  //first links, so the neighbours start from there.
}

inline pair<int *,double *>
Graph::neighbors(int node, int layer, int direction) {
  assert(node>=0 && node<nb_nodes && layer >= 0 && layer < nb_layers && direction >= 0 && direction < 2);

  long rel_index=0;

  if (node==0 && layer==0 && direction==0)
    rel_index = 0;
  else
    rel_index = (long)degrees[degree_index(node, layer, direction)-1];

  if (weights!=NULL)
    return make_pair(links+rel_index, weights+rel_index);
  else
    return make_pair(links+rel_index, weights);
}

inline int Graph::nb_nonnull_layers(int node)
{
  assert(node>=0 && node<nb_nodes);

  if (node == 0)
    return nb_nonnull_layers_per_node[node];
  else
    return nb_nonnull_layers_per_node[node]-nb_nonnull_layers_per_node[node-1];
}

inline int* Graph::nonnull_layers(int node)
{
  assert(node>=0 && node<nb_nodes);

  long rel_index=0;

  if (node == 0)
    rel_index = 0;
  else
    rel_index = nb_nonnull_layers_per_node[node - 1];

  return nonnull_layers_per_node + rel_index;

}

#endif // GRAPH_H
