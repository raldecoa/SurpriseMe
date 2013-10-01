/* slicer.h
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

#ifndef SLICER_H
#define SLICER_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "../Base/graph.h"

#define WEIGHTED   0
#define UNWEIGHTED 1

// The file format for the slice is the following:
// First a list of node names.
// Node   Name
// 1      Node A
// 2      Node B
// ...
//
// Then a list of slices:
// Slice  Name
// 1      Slice A
// 2      Slice B
//
// Then a list of links in the format:
// From   To    Weight    Slice
// 1      2     2         1
// 3      1     1         2
// ...
// Each section is started with a line containing only a '>'. The complete
// file would thus look something like this:
// >
// 1  Node A
// 2  Node B
// >
// 1  Slice A
// 2  Slice B
// >
// 1  2  2  1
// 3  1  1  2
//
// The same node can appear in various slices, connected by an interslice link.
// This means the nodes are renumbered (that is done anyhow), taking into account
// the actual slice. For example, Node 1 in Slice 1 will be renumbered to 0, and
// Node 1 in Slice 2 to 1, etc...
//
// It is assumed that if the graph is undirected, that each link is two times in the
// file. In other words, we assume it is directed.

#define NO_NULL   1
#define ER_NULL   2 //currently not implemented
#define CONF_NULL 3

#define POSITIVE  1
#define NEGATIVE  -1

using namespace std;

class Slicer {
 public:
  struct LinkInfo
  {
      LinkInfo (int n, double w)
      {
        node = n;
        weight = w;
      };

      int node;
      double weight;
  };

  map<int, char*> node_names;
  map<int, char*> slice_names;

  //in_links[from_node][slice][neighbour_1]
  //in_links[from_node][slice][neighbour_2]
  //etc...
  map< int, map< int, vector< Slicer::LinkInfo > > > in_links;
  map< int, map< int, vector< Slicer::LinkInfo > > > out_links;

  map< int, map< int, vector< Slicer::LinkInfo > > > neg_in_links;
  map< int, map< int, vector< Slicer::LinkInfo > > > neg_out_links;

  map< int, vector< Slicer::LinkInfo > > interslice_in_links;
  map< int, vector< Slicer::LinkInfo > > interslice_out_links;

  /*vector< vector< vector< Slicer::LinkInfo > > > in_links;
  vector< vector< vector< Slicer::LinkInfo > > > out_links;

  vector< vector< vector< Slicer::LinkInfo > > > neg_in_links;
  vector< vector< vector< Slicer::LinkInfo > > > neg_out_links;

  vector< vector< Slicer::LinkInfo > > interslice_in_links;
  vector< vector< Slicer::LinkInfo > > interslice_out_links;
  */

  map<long,int> node_map;
  map<int,int>  slice_map;
  map<int,int>  slice_rmap;

  int nb_links;
  int nb_slices;
  int nb_nodes;
  int is_directed;
  int is_weighted;
  int is_single_slice;

  int* nb_links_per_slice;

  Slicer (char *filename, double weight, int is_directed, int is_weighted, int is_single_slice);
  Slicer (Slicer& s);

  ~Slicer();

  int max_node_id;
  int max_slice_id;

  void readNodes(ifstream& finput);
  void readSlices(ifstream& finput);
  void readLinks(ifstream& finput);

  void init_slice_links();

  long getNodeSliceId(int node, int slice);
  void setId(long nodeSliceId, int &node, int &slice);
  void add_interslice_links(double w);

  void clean();
  void renumber();

  Graph* get_graph();

  void display(char *filename);
  void display_node_mapping(char *filename);
  void display_binary(char *filename);
  void display_conf(char* filename, int modelType);
};


#endif // SLICER_H
