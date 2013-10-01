/* community.h
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

#ifndef COMMUNITY_H
#define COMMUNITY_H

#include <set>

#include "graph.h"
#include "info.h"

#define NO_NULL    1
#define ER_NULL    2
#define CONF_NULL  3
#define FIXED_NULL 4

#define POSITIVE  1
#define NEGATIVE  -1

using namespace std;

class Community {
 public:
  // network to compute communities for
  Graph* g;

  // number of nodes in the network
  int size;

  //the highest community
  int nb_comm;

  // community to which each node belongs
  int* n2c;
  int* csize; //community size

  // Since not all communities have links to all layers, it pays to have
  // a sparse representation of it. Whenever we reach a level such that
  // all communities are connected to almost all layers, there are not that
  // many communities to consider anyhow.

  // The null model used for the various layer of networks included.
  // Ordinarily the CONF_NULL model k_ik_j/m would be used, while
  // the ER_NULL model m/n(n-1) is not that common. The NO_NULL model
  // is always zero, and is mainly used for the interslice links.
  int* null_model_per_layer;

  // The sign indicates whether it is a positive or a negative contribution.
  // For negative layers the contribution is subtracted, while for positive layers
  // this is added. This is done in this way so that the second phase of
  // each pass can be executed correctly.
  int* sign_per_layer;

  // The resolution parameters for the various slices
  double* lambda_per_layer;

  // total weight from in- and outlinks per community (per layer).
  //deque< deque<double> > total_weight_in,total_weight_out;
  double* total_weight_in_; // Internal variables
  double* total_weight_out_;

  // total weight strictly inside the community (per layer).
  double* total_weight_comm_; // Internal variables
  //deque< deque<double> > total_weight_comm;

  bool communities_initialized;

  // constructors:
  // reads graph from file using graph constructor
  // layer defined the weighted/unweighted status of the graph file
  Community (char *filename, int* conf, int* sign, double* lambda);
  // copy graph
  Community (Graph* g, int* conf, int* sign, double* lambda);

  ~Community();

  // remove the node from its current community with which it has dnodecomm links
  void remove(int node, int comm, map<int, double> weight_to_comm, map<int, double> weight_from_comm);

  // insert the node in comm with which it shares dnodecomm links
  void insert(int node, int comm, map<int, double> weight_to_comm, map<int, double> weight_from_comm);

  // get the unique communities.
  deque<int> getCommunities();

  // compute the set of neighboring communities of node
  // for each community, gives the number of links from node to comm
  void neigh_comm(int node, int direction, map< int, map<int, double> >*  res, set<int>* comm);
  void total_weight_node_comm(int node, map< int, map<int, double> >*  res, set<int>* comm);
  void total_weight_comm_node(int node, map< int, map<int, double> >*  res, set<int>* comm);

  // compute the modularity of the current partition
  double modularity();
  double modularity(int* sign, double* lambda, int* model);

  // displays the current partition
  void display_partition();
  void display_partition(ostream& out);

  // reset the community assignments based on another hierarchical community object
  void reinit_communities(Community* c);
  void reinit_weights( );

  //renumber the communities so to go from 0 - n.
  void renumber_communities();

  // generates the binary graph of communities as computed by one_level
  Graph* partition2graph();

  // compute the gain of modularity if node where inserted in comm
  // given that node has dnodecomm links to comm.  The formula is:
  // [(In(comm)+2d(node,comm))/2m - ((tot(comm)+deg(node))/2m)^2]-
  // [In(comm)/2m - (tot(comm)/2m)^2 - (deg(node)/2m)^2]
  // where In(comm)    = number of half-links strictly inside comm
  //       Tot(comm)   = number of half-links inside or outside comm (sum(degrees))
  //       d(node,com) = number of links from node to comm
  //       deg(node)   = node degree
  //       m           = number of links
  double modularity_gain(int node, int comm, map<int, double> weight_to_comm, map<int, double> weight_from_comm);

  double& total_weight_in(int comm, int layer);
  double& total_weight_out(int comm, int layer);
  double& total_weight_comm(int comm, int layer);

  void init_communities();


private:
  // Remember whether we created the graph (using filename) and are hence
  // responsible for the deletion of it.
  bool delete_graph;

  void init(int* conf, int* sign, double* lambda);
};

inline double& Community::total_weight_in(int comm, int layer)
{
  return total_weight_in_[comm*g->nb_layers + layer];
}

inline double& Community::total_weight_out(int comm, int layer)
{
  return total_weight_out_[comm*g->nb_layers + layer];
}

inline double& Community::total_weight_comm(int comm, int layer)
{
  return total_weight_comm_[comm*g->nb_layers + layer];
}

inline void Community::remove(int node, int comm, map<int, double> weight_to_comm, map<int, double> weight_from_comm) {
  assert(node>=0 && node<size);

  int nb_node_layers = g->nb_nonnull_layers(node);
  int* node_layers = g->nonnull_layers(node);
  for (int layer_ind = 0; layer_ind < nb_node_layers; layer_ind++)
  {
    int layer = node_layers[layer_ind];

    total_weight_in(comm,  layer)  -= g->weighted_degree(node, layer, INCOMING);
    total_weight_out(comm, layer)  -= g->weighted_degree(node, layer, OUTGOING);

    double node_comm    = weight_to_comm[layer];
    double comm_node    = weight_from_comm[layer];

    total_weight_comm(comm, layer) -= node_comm + comm_node + g->self_weight(node, layer);
    //cerr << "Remove node " << node << " to community " << comm << " (size " << csize[comm] << ") difference, " << node_comm + comm_node + g->self_weight(node, layer) << endl;
  }

  csize[comm] -= g->nsize[node];

  n2c[node]  = -1;

}

inline void Community::insert(int node, int comm, map<int, double> weight_to_comm, map<int, double> weight_from_comm) {
  assert(node>=0 && node<size);

  int nb_node_layers = g->nb_nonnull_layers(node);
  int* node_layers = g->nonnull_layers(node);
  for (int layer_ind = 0; layer_ind < nb_node_layers; layer_ind++)
  {
    int layer = node_layers[layer_ind];

    total_weight_in(comm,  layer)  += g->weighted_degree(node, layer, INCOMING);
    total_weight_out(comm, layer)  += g->weighted_degree(node, layer, OUTGOING);

    double node_comm    = weight_to_comm[layer];
    double comm_node    = weight_from_comm[layer];

    total_weight_comm(comm, layer) += node_comm + comm_node + g->self_weight(node, layer);
    //cerr << "Add node " << node << " to community " << comm << " (size " << csize[comm] << ") difference, " << node_comm + comm_node + g->self_weight(node, layer) << endl;
  }

  csize[comm] += g->nsize[node];

  n2c[node]=comm;
}

inline double Community::modularity_gain(int node, int comm, map<int, double> weight_to_comm, map<int, double> weight_from_comm) {
  assert(node>=0 && node<size);

  double gain = 0.0;
  double n    = g->total_nodes;

  int nb_node_layers  = g->nb_nonnull_layers(node);
  int* node_layers    = g->nonnull_layers(node);
  for (int layer_ind = 0; layer_ind < nb_node_layers; layer_ind++)
  {
    int layer = node_layers[layer_ind];

    double comm_in      = (double)total_weight_in(comm, layer);
    double comm_out     = (double)total_weight_out(comm, layer);
    double node_in      = (double)g->weighted_degree(node, layer, INCOMING);
    double node_out     = (double)g->weighted_degree(node, layer, OUTGOING);
    double m            = (double)g->total_weight(layer);
    double p            = m/(n*n);

    double node_comm    = weight_to_comm[layer];
    double comm_node    = weight_from_comm[layer];

    double total_weight = (double)g->total_weight(layer);

    if (total_weight != 0)
    {
      double expected = 0.0;
      if (null_model_per_layer[layer] == NO_NULL)
        expected = 0.0;
      else if (null_model_per_layer[layer] == ER_NULL)
        expected = 2*g->nsize[node]*csize[comm]*p;
      else if (null_model_per_layer[layer] == FIXED_NULL)
        expected = 2*g->nsize[node]*csize[comm];
      else if (null_model_per_layer[layer] == CONF_NULL)
        expected = node_out*comm_in/total_weight + comm_out*node_in/total_weight;

      double weight = 1.0;
      if (lambda_per_layer != NULL)
        weight = lambda_per_layer[layer];

      double gain_t = sign_per_layer[layer]*(node_comm + comm_node - weight*expected);

      //cerr << "Node " << node << " in community " << comm << ", layer " << layer << ", gain: " << gain_t << ", weight: " << node_comm + comm_node << ", expected: " << expected << ", resolution: " << weight << endl ;

      gain += gain_t;

    }
  }

  return gain;
}

#endif
