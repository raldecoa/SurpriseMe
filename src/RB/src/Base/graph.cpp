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

#include "graph.h"

Graph::Graph() {
  nb_nodes     = 0;
  nb_links     = 0;

  nsize                       = NULL;
  degrees                     = NULL;
  weighted_degree_array       = NULL;
  nb_nonnull_layers_per_node  = NULL;
  nonnull_layers_per_node     = NULL;
  links                       = NULL;
  weights                     = NULL;
  self_weights                = NULL;
}

Graph::Graph(Graph const& g)
{
  this->is_weighted           = g.is_weighted;
  this->is_directed           = g.is_directed;
  this->nb_nodes              = g.nb_nodes;
  this->nb_layers             = g.nb_layers;
  this->nb_links              = g.nb_links;
  this->total_nodes           = g.total_nodes;
  this->total_w               = g.total_w;
  this->total_layer_per_node  = g.total_layer_per_node;

  // read cumulative degree sequence: 4 bytes for each node, per layer, per direction
  // cum_degree[0]=degree(0); cum_degree[1]=degree(0)+degree(1), etc.
  long s = nb_nodes*nb_layers*(is_directed+1)*sizeof(int);
  if ( !(degrees = (int *)malloc(s)) )
  {
    cerr << "Could not allocate " << nb_nodes*nb_layers*(is_directed+1)*sizeof(int) << " bytes of memory for degrees." << endl;
    exit(-1);
  }
  //Copy degree
  memcpy(degrees, g.degrees, s);

  // read links: 4 bytes for each link (each link is counted twice)
  s = nb_links*sizeof(int)*2;
  if( !(links = (int *)malloc(s)) )
  {
    cerr << "Could not allocate " << nb_links*sizeof(int)*2 <<" bytes of memory for links." << endl;
    exit(-1);
  }
  memcpy(links, g.links, s);

  // IF WEIGHTED : read weights: 8 bytes for each link (each link is counted twice)
  if (is_weighted)
  {
    s = nb_links*sizeof(double)*2;
    if ( !(weights = (double *)malloc(s)) )
    {
      cerr << "Could not allocate " << nb_links*sizeof(int)*2 <<" bytes of memory for weights." << endl;
      exit(-1);
    }
    memcpy(weights, g.weights, s);
  }
  else
  {
    weights = NULL;
  }

  s = nb_nodes*sizeof(int);
  if( !(nsize = (int*)malloc(s)) )
  {
    cerr << "Could not allocated memory for node sizes." << endl;
    exit(-1);
  }

  memcpy(nsize, g.nsize, s);

  s = nb_nodes*nb_layers*(is_directed+1)*sizeof(double);
  if ( !(weighted_degree_array = (double *)malloc(s)) )
  {
    cerr << "Could not allocated memory for weighted degree array." << endl;
    exit(-1);
  }
  memcpy(weighted_degree_array, g.weighted_degree_array, s);

  s = nb_nodes*sizeof(int);
  if ( !(nb_nonnull_layers_per_node = (int*)malloc(s)) )
  {
    cerr << "Could not allocated memory for nb nonnull layers." << endl;
    exit(-1);
  }
  memcpy(nb_nonnull_layers_per_node, g.nb_nonnull_layers_per_node, s);

  s = total_layer_per_node*sizeof(int);
  if ( !(nonnull_layers_per_node = (int *)malloc(s)) )
  {
    cerr << "Could not allocated memory for nonnull layers." << endl;
    exit(-1);
  }
  memcpy(nonnull_layers_per_node, g.nonnull_layers_per_node, s);

  s = nb_nodes*nb_layers*sizeof(double);
  if ( !(self_weights = (double*)malloc(s)) )
  {
    cerr << "Could not allocated memory for self weights." << endl;
    exit(-1);
  }
  memcpy(self_weights, g.self_weights, s);

  //the total_weight per layer
  s = nb_layers*sizeof(double);
  if ( !(total_weight_per_layer = (double*)malloc(s)) )
  {
    cerr << "Could not allocated memory for total weight per layer." << endl;
    exit(-1);
  }
  memcpy(total_weight_per_layer, g.total_weight_per_layer, s);

}

Graph::~Graph()
{
  //cout << "Graph (" << nb_nodes << " nodes) deleted.\n";
  free_mem();
}

void Graph::free_mem()
{
  free(degrees);
  free(links);
  free(weights);
  free(total_weight_per_layer);
  free(nsize);
  free(nb_nonnull_layers_per_node);
  free(nonnull_layers_per_node);
  free(weighted_degree_array);
  free(self_weights);

  nsize                       = NULL;
  degrees                     = NULL;
  weighted_degree_array       = NULL;
  nb_nonnull_layers_per_node  = NULL;
  nonnull_layers_per_node     = NULL;
  links                       = NULL;
  weights                     = NULL;
  self_weights                = NULL;
}

Graph::Graph(char *filename) {

  nsize                       = NULL;
  degrees                     = NULL;
  weighted_degree_array       = NULL;
  nb_nonnull_layers_per_node  = NULL;
  nonnull_layers_per_node     = NULL;
  links                       = NULL;
  weights                     = NULL;
  self_weights                = NULL;

  ifstream finput;
  finput.open(filename,fstream::in | fstream::binary);

  if (! finput)
  {
    cerr << "Could not find file " << filename << "." << endl;
    exit(-1);
  }

  //read is_weighted, is_directed
  finput.read((char *)&is_weighted, sizeof(int));
  finput.read((char *)&is_directed, sizeof(int));
  // read number of nodes on 4 bytes
  finput.read((char *)&nb_nodes, sizeof(int));
  finput.read((char *)&nb_layers, sizeof(int));

  // read cumulative degree sequence: 4 bytes for each node, per layer, per direction
  // cum_degree[0]=degree(0); cum_degree[1]=degree(0)+degree(1), etc.
  if ( !(degrees = (int *)malloc((long)nb_nodes*nb_layers*(is_directed+1)*sizeof(int))) )
  {
    cerr << "Could not allocate " << nb_nodes*nb_layers*(is_directed+1)*sizeof(int) << " bytes of memory for degrees." << endl;
    exit(-1);
  }

  finput.read((char *)degrees, (long)nb_nodes*nb_layers*(is_directed+1)*sizeof(int));

  // read links: 4 bytes for each link (each link is counted twice)
  nb_links=degrees[nb_nodes*nb_layers*(is_directed+1)-1]/2;
  if( !(links = (int *)malloc((long)nb_links*sizeof(int)*2)) )
  {
    cerr << "Could not allocate " << nb_links*sizeof(int)*2 <<" bytes of memory for links." << endl;
    exit(-1);
  }
  finput.read((char *)links, (long)nb_links*sizeof(int)*2);
  cerr << "total : " << nb_links << endl;


  // IF WEIGHTED : read weights: 8 bytes for each link (each link is counted twice)
  if (is_weighted)
  {
    if ( !(weights = (double *)malloc((long)nb_links*sizeof(double)*2)) )
    {
      cerr << "Could not allocate " << nb_links*sizeof(int)*2 <<" bytes of memory for weights." << endl;
      exit(-1);
    }
    finput.read((char *)weights, (long)nb_links*sizeof(double)*2);
  }
  else
  {
    weights = NULL;
  }

  init();
  init_self_weights();
  init_layers_per_node();

  finput.close();
}

Graph::Graph(int nb_nodes, int nb_layers, int nb_links, int *degrees, int *links, double *weights) {

  this->nsize                       = NULL;
  this->degrees                     = NULL;
  this->weighted_degree_array       = NULL;
  this->nb_nonnull_layers_per_node  = NULL;
  this->nonnull_layers_per_node     = NULL;
  this->links                       = NULL;
  this->weights                     = NULL;
  this->self_weights                = NULL;

  is_weighted        = 1;
  is_directed        = 1;
  this->nb_nodes     = nb_nodes;
  this->nb_layers    = nb_layers;
  this->nb_links     = nb_links;
  this->degrees      = degrees;
  this->links        = links;
  this->weights      = weights;

  init();
  init_self_weights();
  init_layers_per_node();
}

void Graph::init()
{
  //initialize total weights
  total_w = 0;

  if ( !(total_weight_per_layer = (double*)malloc((long)nb_layers*sizeof(double))) )
  {
    cerr << "Could not allocated memory for total weight per layer." << endl;
    exit(-1);
  }

  if ( !(weighted_degree_array = (double *)malloc((long)nb_nodes*nb_layers*(is_directed+1)*sizeof(double))) )
  {
    cerr << "Could not allocated memory for weighted degree array." << endl;
    exit(-1);
  }

  for (int i = 0; i < nb_layers; i++)
  {
    total_weight_per_layer[i] = 0;
  }
  int degree_index = 0;
  int layer = 0;
  int node  = 0;
  weighted_degree_array[0] = 0.0;
  for (int i = 0 ; i<nb_links*2 ; i++)
  {
    //determine the layer from the cum_degrees
    while (degrees[degree_index] <= i && degree_index < nb_nodes*nb_layers*(is_directed+1))
    {
      degree_index++;
      weighted_degree_array[degree_index] = 0;
      layer = (degree_index % (nb_layers * (is_directed+1))) / (is_directed + 1);
    }
    assert(node >= 0 && node <= nb_nodes);
    if (is_weighted)
    {
      double w = weights[i];
      total_weight_per_layer[layer] += w;
      weighted_degree_array[degree_index] += w;
      total_w += w;
    }
    else
    {
      total_weight_per_layer[layer] += 1;
    }
  }

  //initialize total weights
  for (int i = 0; i < nb_layers; i++)
  {
    total_weight_per_layer[i] /= 2;
  }

  //init node size
  if( !(nsize = (int*)malloc((long)nb_nodes*sizeof(int))) )
  {
    cerr << "Could not allocated memory for node sizes." << endl;
    exit(-1);
  }
  for (int i = 0; i < nb_nodes; i++)
    nsize[i] = 1;

  total_nodes = nb_nodes;
}

void Graph::init_self_weights()
{
  // Initialize self weights
  if ( !(self_weights = (double*)malloc((long)nb_nodes*nb_layers*sizeof(double))) )
  {
    cerr << "Could not allocated memory for self weights." << endl;
    exit(-1);
  }
  for (int node = 0; node < nb_nodes; node++)
  {
    for (int layer = 0; layer < nb_layers; layer++)
    {
      pair<int *,double *> p = neighbors(node, layer, OUTGOING);
      int deg = nb_neighbors(node, layer, OUTGOING);
      //cerr << "Node " << node << ", degree " << deg << ", mem " << p.first << endl;
      // By default no self_weight
      self_weights[node*nb_layers+layer] = 0;
      for (int i=0 ; i < deg; i++)
      {
        //cerr << " Address " << p.first + i << endl;
        if (*(p.first+i)==node)
        {
          if (weights!=NULL)
          {
            self_weights[node*nb_layers+layer]  = *(p.second+i);
          }
          else
          {
            self_weights[node*nb_layers+layer] = 1;
          }
        }
      }

    }
  }
}

void Graph::init_layers_per_node()
{
  map<int, deque<int> > layers_per_node;

  total_layer_per_node = 0;
  for (int node=0 ; node < nb_nodes ; node++)
  {
    for (int layer=0; layer < nb_layers; layer++)
    {
      double tot_degree = (double)nb_neighbors(node, layer, OUTGOING) + (double)nb_neighbors(node, layer, INCOMING) + self_weight(node, layer);
      if  (tot_degree > 0)
      {
        layers_per_node[node].push_back(layer);
        total_layer_per_node++;
      }
    }
  }

  //Allocate memory for 'layer' degree and 'layers' per node
  if ( !(nb_nonnull_layers_per_node = (int*)malloc((long)nb_nodes*sizeof(int))) )
  {
    cerr << "Could not allocated memory for nb nonnull layers." << endl;
    exit(-1);
  }
  if ( !(nonnull_layers_per_node = (int *)malloc((long)total_layer_per_node*sizeof(int))) )
  {
    cerr << "Could not allocated memory for nonnull layers." << endl;
    exit(-1);
  }

  int prev = 0;
  int i = 0;
  for (int node=0; node < nb_nodes; node++)
  {
    int s = layers_per_node[node].size();
    nb_nonnull_layers_per_node[node] = s + prev;
    prev = nb_nonnull_layers_per_node[node];
    for (int layer_ind = 0; layer_ind < s; layer_ind++)
    {
      nonnull_layers_per_node[i++] = layers_per_node[node][layer_ind];
    }
  }

}

void
Graph::display(char *outfile)
{
  ofstream foutput;
  foutput.open(outfile ,fstream::out);

  for (int node=0 ; node<nb_nodes ; node++)
  {
    for (int layer=0; layer<nb_layers; layer++)
    {
      pair<int *,double *> p = neighbors(node, layer, OUTGOING);
      for (int i=0; i<nb_neighbors(node, layer, OUTGOING); i++)
        foutput << node << " " << *(p.first+i) << " " << *(p.second+i) << " " << layer << endl;
    }
  }
  foutput.close();
}

void
Graph::display_binary(char *outfile) {
  cerr << "Outputting binary network to " << outfile << "..." << endl;
  ofstream foutput;
  foutput.open(outfile ,fstream::out | fstream::binary);

  foutput.write((char *)(&is_weighted),4);
  foutput.write((char *)(&is_directed),4);
  foutput.write((char *)(&nb_nodes),4);
  foutput.write((char *)(&nb_layers),4);
  foutput.write((char *)(degrees),nb_nodes*nb_layers*(is_directed+1)*sizeof(int));
  foutput.write((char *)(links),  nb_links*sizeof(int)   *2);
  foutput.write((char *)(weights),nb_links*sizeof(double)*2);

  foutput.close();
}
