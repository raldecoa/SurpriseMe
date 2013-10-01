/* community.cpp
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

#include "community.h"

using namespace std;

// Community(char * filename, int* conf, int* sign, double* lambda)
// filename - file to read
// conf     - configuration per layer (which model to use: ER, Conf, CPM, ...)
// sign     - sign per layer
// lambda   - resolution parameters per layer
//
// This function will read a graph in <filename> and create a new graph
// object. Hence, we will need to take care of the deletion of the graph
// object ourselves as well.
Community::Community(char * filename, int* conf, int* sign, double* lambda)
{
  g = new Graph(filename); delete_graph = true;
  init(conf, sign, lambda);
}

// Community(Graph* gc, int* conf, int* sign, double* lambda)
// filename - file to read
// conf     - configuration per layer (which model to use: ER, Conf, CPM, ...)
// sign     - sign per layer
// lambda   - resolution parameters per layer
//
// This function will use a graph already created by somebody else. Hence,
// we are not responsible for the deletion of the object.
Community::Community(Graph* gc, int* conf, int* sign, double* lambda)
{
  g = gc; delete_graph = false;
  init(conf, sign, lambda);
}

// ~Community()
//
// Destructor: Make sure that the graph is deleted, when we have created it.
Community::~Community()
{
  //cout << "Community (" << size << " nodes) deleted (delete_graph) " << delete_graph << ".\n";
  if (delete_graph)
    delete g;

  if (communities_initialized)
  {
    free(total_weight_in_);
    free(total_weight_out_);
    free(total_weight_comm_);
    free(csize);
    free(n2c);
  }
}

// init(int* conf, int* sign, double* lambda)
// conf     - configuration per layer (which model to use: ER, Conf, CPM, ...)
// sign     - sign per layer
// lambda   - resolution parameters per layer
//
// Initialize the community object and parameters, by using
// the specified conf, sign and lambda values (all indicated
// per layer). Then make sure all the bookkeeping is thight
// by calling init_communities.
void Community::init(int* conf, int* sign, double* lambda)
{
  communities_initialized = false;
  this->null_model_per_layer = conf;
  this->sign_per_layer = sign;
  this->lambda_per_layer = lambda;
  size = g->nb_nodes;
  nb_comm = size;

  init_communities();
}

// init_communities()
//
// We set the initial community assignments here,
// and initialze the bookkeeping variables, such as the
// total weight per community.
void Community::init_communities()
{
  if (communities_initialized)
  {
    free(total_weight_in_);
    free(total_weight_out_);
    free(total_weight_comm_);
    free(csize);
    free(n2c);
  }

  communities_initialized = true;

  total_weight_in_    = (double*)malloc(g->nb_nodes*g->nb_layers*sizeof(double));
  total_weight_out_   = (double*)malloc(g->nb_nodes*g->nb_layers*sizeof(double));
  total_weight_comm_  = (double*)malloc(g->nb_nodes*g->nb_layers*sizeof(double));
  csize               = (int*)malloc(g->nb_nodes*sizeof(int));
  n2c                 = (int*)malloc(g->nb_nodes*sizeof(int));

  //init community weights
  for (int i=0; i<size ; i++)
  {
    n2c[i] = i;
    int nb_node_layers = g->nb_nonnull_layers(i);
    int* node_layers = g->nonnull_layers(i);
    for (int layer_ind = 0; layer_ind < nb_node_layers; layer_ind++)
    {
      int layer = node_layers[layer_ind];
      total_weight_in(i,   layer)  = g->weighted_degree(i, layer, INCOMING);
      total_weight_out(i,  layer)  = g->weighted_degree(i, layer, OUTGOING);
      total_weight_comm(i, layer)  = g->self_weight(i, layer);
    }
    csize[i] = g->nsize[i];
  }
}

// modularity()
//
// Retrieve the modularity as specified by the internal variables
// of the sign, lambda and model per layer.
double Community::modularity()
{
  double mod = modularity(sign_per_layer, lambda_per_layer, null_model_per_layer);
  return mod;
}

// modularity(int* sign, double* lambda, int* model)
// sign   - sign per layer
// lambda - resolution parameter per layer
// model  - model per layer (ER, Conf, CPM, ...)
//
// Retrieve the modularity as specified by the variables
// of the sign, lambda and model per layer.
double Community::modularity(int* sign, double* lambda, int* model)
{
  double q  = 0.;

  deque<int> comm = getCommunities();

  // Consider each layer
  for (int layer=0; layer < g->nb_layers; layer++)
  {
    double m = (double)g->total_weight(layer);
    double n = g->total_nodes;
    double p = m/(n*n);
    // Consider all communities. If the bookkeeping is correct, this should be
    // correct (hence the importance of maintaining correct bookkeeping). Since
    // we only need to consider the communities, the function should be
    // reasonably fast.
    for (deque<int>::iterator it=comm.begin(); it != comm.end(); it++)
    {
      int c = *it;

      // Only if there are any links at all (possibly sometimes layers can be empty)
      if (m > 0)
      {
        double internal_weight  = (double)total_weight_comm(c, layer);
        double cs = (double)csize[c];
        double current_lambda = lambda[layer];

        if (model[layer] == CONF_NULL)
        {
          double total_out = total_weight_out(c, layer);
          double total_in  = total_weight_in(c, layer);
          //cerr << "Community " << c << ", layer " << layer
          //   << ", in " << total_in
          //   << ", out " << total_out
          //   << ", comm " << internal_weight << endl;
          q += sign[layer]*( internal_weight - current_lambda*total_out*total_in/m );
        }
        else if (model[layer] == ER_NULL)
        {
          q += sign[layer]*(internal_weight - current_lambda*cs*cs*p );
        }
        else if (model[layer] == FIXED_NULL)
        {
          q += sign[layer]*(internal_weight - current_lambda*cs*cs);
        }
        else
        {
          q += sign[layer]*internal_weight;
        }
      }
    }
  }

  double m = (double)g->total_weight();
  //cerr << "Modularity: " << q/m << endl;
  return q/m;
}

// getCommunities()
//
// Get the unique communities. If the communities have been correctly
// renumbered, these are within the range 0,...,nb_comm-1, but if it
// has not yet been renumbered, this might be relatively arbitrary
// between any of the original community indices 0,...,nb_nodes-1, since we
// started out with these communities.
deque<int> Community::getCommunities()
{
  deque<int> comm = get_deque(g->nb_nodes, n2c);
  sort( comm.begin(), comm.end() );

  deque<int>::iterator new_end_pos;
  new_end_pos = unique( comm.begin(), comm.end() );

  comm.erase( new_end_pos, comm.end() );

  return comm;
}

// neigh_comm(int node, int direction, map< int, map<int, double> >* res, set<int>* comm)
// node       - which node should be examined
// direction  - consider only incoming or outgoing links
// res        - map of total weight per community
// comm       - the unique number of communities
//
// This function is responsible for calculating the total weight from <node> to all the other
// communities and stores the result in <res>. For directed graphs, the total weight can either be
// from node to a community (<direction>=OUTGOING) or the total weight can be frrom a community to
// a node (<direction>=INCOMING).
void Community::neigh_comm(int node, int direction, map< int, map<int, double> >* res, set<int>* comm)
{
  int nb_node_layers = g->nb_nonnull_layers(node);
  int* node_layers = g->nonnull_layers(node);
  int is_negative_present = false;
  
  for (int layer_ind = 0; layer_ind < nb_node_layers; layer_ind++)
  {
    int layer = node_layers[layer_ind];
    if (sign_per_layer[layer] == NEGATIVE)
      is_negative_present = true;

    pair< int*, double* > neighbors = g->neighbors(node, layer, direction);

    int deg = g->nb_neighbors(node, layer, direction);

    for (int i=0 ; i<deg ; i++)
    {
      int neigh        = *(neighbors.first+i);
      int neigh_comm   = n2c[neigh];
      double neigh_weight = (g->weights==NULL)?1:*(neighbors.second+i);

      if (neigh!=node)
      {
        (*res)[neigh_comm][layer] += neigh_weight;
        comm->insert(neigh_comm);
      }
    }
  }
  
  // if there was a negative layer being examined, we will have to loop through *all* communities
  // since it might be that the others have only a negative contribution.
  if (is_negative_present)
  {
    for (int i=0 ; i<nb_comm ; i++)
        comm->insert(i);
  }
}

// total_weight_node_comm(int node, map< int, map<int, double> >* res, set<int>* comm)
// node   - node to examine
// res    - map to store the result int
// comm   - set of unique communities
//
// Returns the total weight from a <node> to all communities, and stores the result in <res>.
void Community::total_weight_node_comm(int node, map< int, map<int, double> >* res, set<int>* comm)
{
  neigh_comm(node, OUTGOING, res, comm);
}

// total_weight_comm_node(int node, map< int, map<int, double> >* res, set<int>* comm)
// node   - node to examine
// res    - map to store the result int
// comm   - set of unique communities
//
// Returns the total weight from all communities to a <node>, and stores the result in <res>.
void Community::total_weight_comm_node(int node, map< int, map<int, double> >* res, set<int>* comm)
{
  neigh_comm(node, INCOMING, res, comm);
}

// display_partition()
//
// Display the partition to the standard out stream <cout>.
void Community::display_partition()
{
  display_partition(std::cout);
}

// display_partition(ostream& out)
// out  - stream to which to output the partition
//
// Outputs the partition in a tab-separated format
// <node> <community>
// per line.
void Community::display_partition(ostream& out)
{
  for (int i=0 ; i<size ; i++)
    out << i << "\t" << n2c[i] << endl;
}

// partition2graph()
//
// Create a new graph based on the partition. That is, we contract all communities
// to a single node, and the links between these new nodes consist simply of the sum
// of the links of the old nodes.
Graph* Community::partition2graph()
{
  // Compute per community the number of nodes inside it.
  deque< deque<int> > comm_nodes(nb_comm);
  for (int node=0 ; node<size ; node++)
  {
    comm_nodes[n2c[node]].push_back(node);
    //cerr << "Node " << node << ", Community " << renumber[n2c[node]] << endl;
  }

  // unweigthed to weighted
  Graph* g2 = new Graph();
  g2->nb_nodes = comm_nodes.size();
  g2->nb_links = 0;
  g2->nb_layers = g->nb_layers;
  g2->is_directed = g->is_directed;
  g2->is_weighted = 1;
  g2->total_nodes = g->total_nodes;

  //cerr << "Number of new nodes: " << g2->nb_nodes << endl;
  //cerr << "Allocating memory for degree: " << g2->nb_nodes*g2->nb_layers*(g2->is_directed + 1)*4 << " bytes" << endl;
  //cerr << "Allocating memory for links and weights: " << g2->nb_nodes*g2->nb_nodes*g2->nb_layers*4 << " bytes" << endl;

  // Allocate sufficient memory so that our new graph will definitely fit in there. We know exactly how many
  // nodes there will be, but we are not sure yet about the number of links. So, we will simply allocate as
  // much memory as we needed for the previous graph, which provides (a bad) upperbound on the possible number
  // of links.
  if( !(g2->degrees                 = (int *)malloc((long)g2->nb_nodes*g2->nb_layers*(g2->is_directed +1)*sizeof(int))) )
  {
    cerr << "Could not allocate " << g2->nb_nodes*g2->nb_layers*(g2->is_directed +1)*sizeof(int) << " bytes of memory for degrees." << endl;
    exit(-1);
  }
  if ( !(g2->weighted_degree_array   = (double *)malloc((long)g2->nb_nodes*g2->nb_layers*(g2->is_directed +1)*sizeof(double))) )
  {
    cerr << "Could not allocate " << g2->nb_nodes*g2->nb_layers*(g2->is_directed +1)*sizeof(double) << " bytes of memory for weighted_degree_array." << endl;
    exit(-1);
  }
  int link_ub = g->nb_links;
  if ( !(g2->links                   = (int *)malloc((long)link_ub*2*sizeof(int))) )
  {
    cerr << "Could not allocate " << link_ub*sizeof(int) << " bytes of memory for links." << endl;
    exit(-1);
  }
  if ( !(g2->weights                 = (double *)malloc((long)link_ub*2*sizeof(double))) )
  {
    cerr << "Could not allocate " << link_ub*sizeof(int) << " bytes of memory for weights." << endl;
    exit(-1);
  }

  if ( !(g2->nsize                   = (int *)malloc((long)g->nb_nodes*sizeof(int))) )
  {
    cerr << "Could not allocate " << g->nb_nodes*sizeof(int) << " bytes of memory for weights." << endl;
    exit(-1);
  }

  if ( !(g2->total_weight_per_layer  = (double *)malloc(g2->nb_layers*sizeof(double))) )
  {
    cerr << "Could not allocate " << g2->nb_layers*sizeof(double) << " bytes of memory for total_weight_per_layer." << endl;
    exit(-1);
  }

  for (int layer=0; layer < g2->nb_layers; layer++)
    g2->total_weight_per_layer[layer] = 0;

  long where = 0;
  int comm_deg = comm_nodes.size();
  int degree_index = 0;
  //Now consider each community
  for (int comm=0 ; comm<comm_deg ; comm++)
  {
    // and consider each layer per community
    for (int layer=0; layer<g2->nb_layers; layer++)
    {
      // and each direction per layer per community
      for (int direction=0; direction < 2; direction++)
      {
        map<int,double> m;
        map<int,double>::iterator it;

        int comm_size = comm_nodes[comm].size();
        // Consider all the nodes in this community
        for (int node=0 ; node<comm_size ; node++)
        {
          pair<int *,double *> p = g->neighbors(comm_nodes[comm][node], layer, direction);
          int deg = g->nb_neighbors(comm_nodes[comm][node], layer, direction);
          // and register the total weight of the neighbours of the node in other
          // communities.
          for (int i=0 ; i<deg ; i++)
          {
            int neigh        = *(p.first+i);
            int neigh_comm   = n2c[neigh];
            double neigh_weight = (g->weights==NULL)?1:*(p.second+i);

            it = m.find(neigh_comm);
            //double w = m[neigh_comm];
            if (it!=m.end())
              m[neigh_comm]+=neigh_weight;
            else
              m[neigh_comm] = neigh_weight;
            //cerr << node << "\t" << neigh << "\t" << neigh_weight << endl;
          }
        }
        // we insert the degrees in the right order:
        // node layer direction
        // 0    0    0
        // 0    0    1
        // 0    1    0
        // 0    1    1
        // ...

        // Set the correct degree for this entry (in terms of node,layer,direction)
        g2->degrees[degree_index]              = (degree_index==0)?m.size():g2->degrees[degree_index-1]+m.size();
        g2->weighted_degree_array[degree_index] = 0.0;
        //cerr << "Cum Degree " << degree_index << ": " << g2->degrees[degree_index] << endl;

        // Only add once, otherwise we're counting double
        if (direction == 0)
          g2->nb_links+=m.size();

        // Now consider all the communities we found (we don't need to make any links to any communnities
        // we haven't found by looping through all the neighbours of all the nodes in this community)
        for (it = m.begin() ; it!=m.end() ; it++)
        {
          // We will only add the total weight once, otherwise we are counting double.
          if (direction == 0)
            g2->total_weight_per_layer[layer]  += it->second;

          // Set the link to the communtiy (i.e. the new node), and it's weight.
          g2->links[where]   = it->first;
          g2->weights[where] = it->second;
          g2->weighted_degree_array[degree_index] += it->second;
          //cerr << "Added link (" << comm << ", " << it->first << ") " << ", layer: " << layer << ", direction: " << direction << ", weight: " << it->second << endl;
          where++;
        }

        degree_index++;
      }

    }

    // Maintain the node size per community, i.e. simply the sum of all the node
    // sizes in it's community. As the initial node size of nodes is always 1
    // this effectively counts the actual size of a community in terms of the
    // number of nodes.
    int total_node_size = 0;
    int comm_size = comm_nodes[comm].size();
    for (int node=0 ; node<comm_size ; node++)
      total_node_size += g->nsize[comm_nodes[comm][node]];

    g2->nsize[comm] = total_node_size; //save the community size as the size of the new node
    //cerr << "Community/node " << comm << " size " << total_node_size << endl;
  }

  // We overcharged the system at first by allocating as much as we could possible need (in order to avoid
  // any corrupted memory problems, and overflow problems of writing in ill-addressed memory space).
  // But in order to release the memory we have first allocated, we need to reallocate the memory now, sinc
  // we now know exactly how many links we will be needing.
  void* ptr   = realloc(g2->links,   (long)g2->nb_links*2*sizeof(int));
  if (ptr == NULL)
  {
    cerr << "Could not allocate " << g2->nb_links*2*sizeof(int) << " bytes for the links." << endl;
    exit(-1);
  }
  assert(ptr != NULL);
  g2->links   = (int*)ptr;

  ptr         = realloc(g2->weights, (long)g2->nb_links*2*sizeof(double));
  if (ptr == NULL)
  {
    cerr << "Could not allocate " << g2->nb_links*2*sizeof(int) << " bytes for the weights." << endl;
    exit(-1);
  }
  assert(ptr != NULL);
  g2->weights = (double*)ptr;

  // Make sure our bookkeeping of the layers is up to date.
  //g2->init();
  g2->init_self_weights();
  g2->init_layers_per_node();

  return g2;
}

// reinit_communities(Community* c)
// c - other communtiy with which to reinitialize this community
//
// Based on another community, we will reinitialize the actual
// community assignemnts, assuming they have been normalized. That is,
// we assume that the community in question is the community based
// on the graph as created by partition2graph. This function is mainly
// used to translate a higher level community to a lower level community,
// thereby allowing to keep track of the individual community assignements
// immediately.
void Community::reinit_communities(Community* c)
{
  for (int node=0 ; node<size ; node++)
  {
    //The community of a node is the node in c,
    //so the community of the node in c is the new community.
    n2c[node] = c->n2c[n2c[node]];
    //cerr << node << " - " << n2c[node] << " - " << c.n2c[n2c[node]] << endl;
  }

  // Renumber the communities to know for sure we have a correct range
  // of communities between 0,...,nb_comm-1
  renumber_communities();
}

// reinit_weights()
//
// Reinitialize all the weights associated with the communties, but *wihtout*
// reinitializaing the commmuniy assignemnts. In other words, this function
// sort of cleans up the bookkeeping, to make sure evertyhing is correct. For
// example, when a community assignment would be changed from the outside,
// the bookkeeping could be incorrect, which would be corrected again by this
// function.
void Community::reinit_weights()
{
  //Reallocate less memory for the number of communities

  communities_initialized = true;

  total_weight_in_    = (double*)realloc(total_weight_in_,   nb_comm*g->nb_layers*sizeof(double));
  total_weight_out_   = (double*)realloc(total_weight_out_,  nb_comm*g->nb_layers*sizeof(double));
  total_weight_comm_  = (double*)realloc(total_weight_comm_, nb_comm*g->nb_layers*sizeof(double));
  csize               = (int*)realloc(csize, nb_comm*sizeof(int));

  //init community weights
  for (int i=0; i<nb_comm ; i++)
  {
    for (int layer=0; layer<g->nb_layers; layer++)
    {
      total_weight_in(i,   layer)   = 0.0;
      total_weight_out(i,  layer)  = 0.0;
      total_weight_comm(i, layer) = 0.0;
    }
    csize[i] = 0;
  }

  //set community weights
  for (int node=0; node<size; node++)
  {
    for (int layer=0; layer<g->nb_layers; layer++)
    {
      total_weight_in(n2c[node], layer)  += g->weighted_degree(node, layer, INCOMING);
      total_weight_out(n2c[node], layer) += g->weighted_degree(node, layer, OUTGOING);

      //loop through neighbours, and if they are in the same community add the weight
      pair< int*, double* > neighbors = g->neighbors(node, layer, OUTGOING);

      int deg = g->nb_neighbors(node, layer, OUTGOING);
      for (int i=0 ; i < deg ; i++)
      {
        int neigh        = *(neighbors.first+i);
        //cerr << node << " - " << neigh << endl;
        int neigh_comm   = n2c[neigh];
        double neigh_weight = (g->weights==NULL)?1:*(neighbors.second+i);
        if (neigh_comm==n2c[node])
        {
          total_weight_comm(n2c[node], layer) += neigh_weight;
        }
      }
    }//layer
    csize[n2c[node]] += g->nsize[node];
  }

  /*for (int i=0; i<nb_comm ; i++)
  {
    for (int layer=0; layer<g->nb_layers; layer++)
    {
      cerr << "Community " << i << ", layer " << layer
           << ", in " << total_weight_in[i][layer]
           << ", out " << total_weight_out[i][layer]
           << ", comm " << total_weight_comm[i][layer] << endl;
    }
  } */
}

void Community::renumber_communities()
{
  deque<int> renumber(size, -1);
  for (int node=0 ; node<size ; node++)
  {
    renumber[n2c[node]]++;
  }

  nb_comm=0;
  for (int i=0 ; i<size ; i++)
    if (renumber[i]!=-1)
      renumber[i]=nb_comm++;

  for (int node=0 ; node<size ; node++)
  {
    //cerr << "Renumber " << n2c[node] << " to " << renumber[n2c[node]] << endl;
    n2c[node] = renumber[n2c[node]];
  }

  reinit_weights();
  //cerr << "After renumbering: " << this->modularity() << endl;
}
