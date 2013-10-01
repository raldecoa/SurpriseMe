/* slicer.cpp
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

#include "slicer.h"
#include "string.h"

using namespace std;

Slicer::Slicer(char *filename, double w, int is_directed, int is_weighted, int is_single_slice) {
  ifstream finput;
  finput.open(filename,fstream::in);

  this->is_directed     = is_directed;
  this->is_weighted     = is_weighted;
  this->is_single_slice = is_single_slice;

  if (!finput)
    return;

  readNodes(finput);
  cerr << "Done reading " << node_names.size() << " nodes..." << endl;
  readSlices(finput);
  cerr << "Done reading " << slice_names.size() << " slices..." << endl;
  readLinks(finput);
  cerr << "Done reading " << nb_links << " links..." << endl;

  finput.close();

  //clean();
  renumber();
  add_interslice_links(w);

  init_slice_links();
}

Slicer::Slicer(Slicer& s)
{
  node_names            = s.node_names;
  slice_names           = s.slice_names;

  in_links              = s.in_links;
  out_links             = s.out_links;

  neg_in_links          = s.neg_in_links;
  neg_out_links         = s.neg_out_links;

  interslice_in_links   = s.interslice_in_links;
  interslice_out_links  = s.interslice_out_links;

  node_map              = s.node_map;
  slice_map             = s.slice_map;
  slice_rmap            = s.slice_rmap;

  nb_links              = s.nb_links;
  nb_slices             = s.nb_slices;
  nb_nodes              = s.nb_nodes;

  max_node_id           = s.max_node_id;
  max_slice_id          = s.max_slice_id;

  nb_links_per_slice    = new int[nb_slices];
  memcpy(nb_links_per_slice, s.nb_links_per_slice, sizeof(int)*nb_slices);
}

Slicer::~Slicer()
{
  map<int, char*>::iterator it_s;
  //Remove allocated character arrays
  for (it_s = node_names.begin(); it_s != node_names.end(); it_s++)
    delete [] it_s->second;

  for (it_s = slice_names.begin(); it_s != slice_names.end(); it_s++)
    delete [] it_s->second;

  delete [] nb_links_per_slice;
}

void Slicer::readNodes(ifstream& finput)
{
  // move to beginning of nodes;
  int end=0;
  int l=0;
  char* s;
  s = (char*)malloc(256 * sizeof(char));
  while (!finput.eof() && !end)
  {
    l++;
    finput.getline(s, 256);
    if (strcmp(s,">")==0)
      end = 1;
    //cerr << l << ": " << s << endl;
  }

  end=0;
  max_node_id = 0;
  while (!finput.eof() && !end)
  {
    int node;
    int prevg = finput.tellg();
    char* node_name = NULL;

    finput.getline(s, 256);

    //cerr << s << endl;

    if (strcmp(s,">")!=0)
    {
      node = atoi(s);
      if (node > max_node_id)
        max_node_id = node;

      int i = 0;
      int l = strlen(s);
      while (s[i] >= '0' && s[i] <= '9' && i < l)
        i++;

      i++;

      if (i < l)
      {
        node_name = new char[l-i+1];
        strncpy(node_name, s+i, l-i);
      }
      else
      {
        node_name  = new char[1];
        node_name[0] = '\0';
      }

      node_names[node] = node_name;

      //cerr << node_name << endl;
    }
    else
    {
      finput.seekg(prevg);
      end = 1;
    }
  }

  free(s);
}

void Slicer::readSlices(ifstream& finput)
{
  // move to beginning of slices;
  int end=0;
  int l=0;
  char* s;
  s = (char*)malloc(256 * sizeof(char));
  while (!finput.eof() && !end)
  {
    l++;
    finput.getline(s, 256);
    if (strcmp(s,">")==0)
      end = 1;
    //cerr << l << ": " << s << endl;
  }

  end=0;
  int slice_nb = 0;
  max_slice_id = 0;
  while (!finput.eof() && !end)
  {
    int slice;
    int prevg = finput.tellg();
    char* slice_name = NULL;

    finput.getline(s, 256);

    //cerr << s << endl;
    if (strcmp(s,">")!=0)
    {
      slice = atoi(s);
      if (slice > max_slice_id)
        max_slice_id = slice;

      int i = 0;
      int l = strlen(s);
      while (s[i] >= '0' && s[i] <= '9' && i < l)
        i++;

      i++;


      if (i < l)
      {
        slice_name  = new char[l-i+1];
        strncpy(slice_name , s+i, l-i);
      }
      else
      {
        slice_name  = new char[1];
        slice_name[0] = '\0';
      }

      slice_names[slice] = slice_name;
      slice_map[slice] = slice_nb;
      slice_rmap[slice_nb] = slice;
      //cerr << slice_name << " " << slice << " " << slice_nb << endl;
      slice_nb++;

    }
    else
    {
      finput.seekg(prevg);
      end = 1;
    }
  }
  nb_slices = slice_nb;

  if (nb_slices > 1 && is_single_slice)
  {
    cerr << "Conflicting arguments, single slice with multiple slices." << endl;
    free(s);
    exit(-1);
  }

  free(s);
}

void Slicer::readLinks(ifstream& finput)
{
  // move to beginning of slices;
  int end=0;
  int l=0;
  char* s;
  s = (char*)malloc(256 * sizeof(char));
  while (!finput.eof() && !end)
  {
    l++;
    finput.getline(s, 256);
    if (strcmp(s,">")==0)
      end = 1;
    //cerr << l << ": " << s << endl;
  }

  nb_links = 0;

  while (!finput.eof() && finput)
  {
    int src, dest, slice;
    double weight = 1.0; //Default weight for unweighted graphs
    slice = 0;           //Default slice  for single slice graphs
    if (is_single_slice)
    { // No slice indicated on links
      if (is_weighted)
        finput >> src >> dest >> weight;
      else
        finput >> src >> dest;
    }
    else
    { // Slice indicated on links
      if (is_weighted)
        finput >> src >> dest >> weight >> slice;
      else
        finput >> src >> dest >> slice;
    }
    nb_links++;

    int nslice = slice_map[slice];
    if (finput)
    {
      if (weight >= 0)
      {
        out_links[src][nslice].push_back(  LinkInfo(dest,weight) );
        in_links[dest][nslice].push_back(  LinkInfo(src,weight) );
      }
      else
      {
        neg_out_links[src][nslice].push_back( LinkInfo(dest,-weight) );
        neg_in_links[dest][nslice].push_back( LinkInfo(src,-weight) );
      }

      // If not directed, we will add the other link
      if (!is_directed)
      {
        if (weight >= 0)
        {
          out_links[dest][nslice].push_back(  LinkInfo(src,weight) );
          in_links[src][nslice].push_back(  LinkInfo(dest,weight) );
        }
        else
        {
          neg_out_links[dest][nslice].push_back( LinkInfo(src,-weight) );
          neg_in_links[src][nslice].push_back( LinkInfo(dest,-weight) );
        }
        nb_links++;
      }
    }
  }

  /*cerr << "Outgoing links" << endl;
  for (unsigned int i=0 ; i<out_links.size() ; i++)
  {
    for (unsigned int j=0 ; j<out_links[i].size() ; j++)
    {
      cerr << i << "\t" << out_links[i][j].node << "\t" << out_links[i][j].weight << "\t" << out_links[i][j].slice << endl;
    }
  }

  cerr << "Incoming links" << endl;
  for (unsigned int i=0 ; i<in_links.size() ; i++)
  {
    for (unsigned int j=0 ; j<in_links[i].size() ; j++)
    {
      cerr << in_links[i][j].node << "\t" << i << "\t" << in_links[i][j].weight << "\t" << in_links[i][j].slice << endl;
    }
  }*/

   free(s);
}

void Slicer::init_slice_links()
{
  nb_links_per_slice = new int[nb_slices];

  for (int slice=0; slice < nb_slices; slice++)
    nb_links_per_slice[slice] = 0;

  for (int i=0 ; i<nb_nodes ; i++)
  {
    for (int slice=0; slice < nb_slices; slice++)
    {
      nb_links_per_slice[slice]+= (out_links[i][slice].size() + neg_out_links[i][slice].size());
    }
  }
}

long Slicer::getNodeSliceId(int node, int slice)
{
  map<int, int>::iterator it = slice_map.find(slice);
  if (it != slice_map.end())
    return node*(nb_slices+1) + it->second;
  else
    return -1;
}

void Slicer::setId(long nodeSliceId, int &node, int &slice)
{
  map<int, int>::iterator it = slice_rmap.find((int)(nodeSliceId / (max_node_id+1)));
  if (it != slice_map.end())
  {
    node  = (int)(nodeSliceId / (nb_slices+1));
    slice = slice_rmap[(int)(nodeSliceId % (nb_slices+1))];
  }
  else
  {
    slice = -1;
    node  = -1;
  }
}

void
Slicer::renumber() {
  // mapping the combination of
  // current node number and slice number
  // to a new number.

  nb_nodes=0;
  map< int, map< int, vector< Slicer::LinkInfo > > >::iterator it_node;
  for (it_node = out_links.begin();  it_node != out_links.end(); it_node++)
  {
    map< int, vector< Slicer::LinkInfo > >::iterator it_slice;
    for (it_slice = it_node->second.begin() ; it_slice != it_node->second.end(); it_slice++)
    {
      for (unsigned int j=0 ; j< it_slice->second.size() ; j++)
      {
        long node_to   = getNodeSliceId(it_slice->second[j].node, slice_rmap[it_slice->first]);
        long node_from = getNodeSliceId(it_node->first,slice_rmap[it_slice->first]);
        node_map[node_from] = 0;
        node_map[node_to]   = 0;
      }
    }
  }

  //Negative links

  for (it_node = neg_out_links.begin();  it_node != neg_out_links.end(); it_node++)
  {
    map< int, vector< Slicer::LinkInfo > >::iterator it_slice;
    for (it_slice = it_node->second.begin() ; it_slice != it_node->second.end(); it_slice++)
    {
      for (unsigned int j=0 ; j< it_slice->second.size() ; j++)
      {
        long node_to   = getNodeSliceId(it_slice->second[j].node, slice_rmap[it_slice->first]);
        long node_from = getNodeSliceId(it_node->first,slice_rmap[it_slice->first]);
        node_map[node_from] = 0;
        node_map[node_to]   = 0;
      }
    }
  }

  cerr << "Mapping to new nodes..." << endl;
  map<long, int>::iterator it;
  for (it = node_map.begin(); it != node_map.end(); it++)
  {
    it->second = nb_nodes++;

    int node;
    int slice;
    setId(it->first, node, slice);

    //cerr << it->first << "(" << node << ", " << slice << ") " << " ->\t" << it->second << endl;
  }

  //cerr << "Second try:" << endl;

  /*it;
  for (it = node_map.begin(); it != node_map.end(); it++)
  {
    int node;
    int slice;
    setId(it->first, node, slice);

    //cerr << it->first << "(" << node << ", " << slice << ") " << " ->\t" << it->second << endl;
  }*/

  /* We have to rebuild to links completely
   * since the new node numbers are different from before.
   * The is a node for various slices is split into several
   * nodes, hence, this needs to be taken into account.
   */

  map< int, map< int, vector< Slicer::LinkInfo > > >* tmp_links = NULL;

  for (int k=0; k < 4; k++)
  {
    switch(k)
    {
      case 0:
        tmp_links = &out_links; break;
      case 1:
        tmp_links = &in_links; break;
      case 2:
        tmp_links = &neg_out_links; break;
      case 3:
        tmp_links = &neg_in_links; break;
      default:
        tmp_links = NULL;
    }

    //make a copy
    map< int, map< int, vector< Slicer::LinkInfo > > > tmp_cp_links(*tmp_links);

    //and empty original
    tmp_links->clear();

    for (it_node = tmp_cp_links.begin();  it_node != tmp_cp_links.end(); it_node++)
    {
      map< int, vector< Slicer::LinkInfo > >::iterator it_slice;
      for (it_slice = it_node->second.begin() ; it_slice != it_node->second.end(); it_slice++)
      {
        unsigned int nn =it_slice->second.size();
        long node_from = getNodeSliceId(it_node->first, slice_rmap[it_slice->first]);

        for (unsigned int j=0 ; j <  nn; j++)
        {
          long node_to   = getNodeSliceId(it_slice->second[j].node, slice_rmap[it_slice->first]);

          (*tmp_links)[node_map[node_from]][it_slice->first].push_back( LinkInfo(node_map[node_to], it_slice->second[j].weight) );
        }
      }
    }

  /*cerr << "Outgoing links" << endl;
  for (unsigned int i=0 ; i<out_links.size() ; i++)
  {
    for (unsigned int j=0 ; j<out_links[i].size() ; j++)
    {
      cerr << i << "\t" << out_links[i][j].node << "\t" << out_links[i][j].weight << "\t" << out_links[i][j].slice << endl;
    }
  }

  cerr << "Incoming links" << endl;
  for (unsigned int i=0 ; i<in_links.size() ; i++)
  {
    for (unsigned int j=0 ; j<in_links[i].size() ; j++)
    {
      cerr << in_links[i][j].node << "\t" << i << "\t" << in_links[i][j].weight << "\t" << in_links[i][j].slice << endl;
    }
  }*/

  /*in_links.resize(nb);
  out_links.resize(nb);
  neg_in_links.resize(nb);
  neg_out_links.resize(nb);
  interslice_in_links.resize(nb);
  interslice_out_links.resize(nb);
  */

}
}

void Slicer::add_interslice_links(double w)
{
  map<long,int>::iterator it;
  int nb_slice_links = 0;

  for (it = node_map.begin(); it != node_map.end(); it++)
  {
    int node;
    int slice;
    setId(it->first, node, slice);
    int src = it->second;


    map<long,int>::iterator it_next;
    int node_id = getNodeSliceId(node, slice+1);
    if (node_id >= 0)//not sure if slice exist at all
    {
      it_next = node_map.find(node_id);
      if (it_next != node_map.end())
      { // we have found the node in the next slice.
        //so add an interslice link.
        int dest = it_next->second;

        //cerr << src << "\t" << dest << endl;

        interslice_out_links[src].push_back( LinkInfo( dest, w) );
        interslice_in_links[dest].push_back( LinkInfo( src, w) );
        //interslice_out_links[dest].push_back( LinkInfo( src, w) );
        //interslice_in_links[src].push_back( LinkInfo( dest, w) );

        nb_slice_links++;
      }
    }
  }
  cerr << "Adding interslice " << nb_slice_links << " links..." << endl;
}

void
Slicer::clean() {
  /*for (unsigned int i=0 ; i<in_links.size() ; i++)
  {
    if (i%10000000==0) fprintf(stderr,".");fflush(stderr);

    map<int,int> m;
    map<int,int>::iterator it;

    for (unsigned int j=0 ; j<in_links[i].size() ; j++)
    {
      it = m.find(in_links[i][j].node);
      if (it==m.end())
      	m.insert(make_pair(in_links[i][j].node, in_links[i][j].weight));
    }

    vector<pair<int,int> > v;
    for (it = m.begin() ; it!=m.end() ; it++)
      v.push_back(*it);
    in_links[i].clear();
    in_links[i]=v;
  }

  for (unsigned int i=0 ; i<out_links.size() ; i++)
  {
    if (i%10000000==0) fprintf(stderr,".");fflush(stderr);

    map<int,int> m;
    map<int,int>::iterator it;

    for (unsigned int j=0 ; j<out_links[i].size() ; j++)
    {
      it = m.find(out_links[i][j].first);
      if (it==m.end())
      	m.insert(make_pair(out_links[i][j].first, out_links[i][j].second));
      else if (layer==WEIGHTED)
      	it->second+=out_links[i][j].second;
    }

    vector<pair<int,int> > v;
    for (it = m.begin() ; it!=m.end() ; it++)
      v.push_back(*it);
    out_links[i].clear();
    out_links[i]=v;
  }
  */
}

void Slicer::display(char* filename)
{
  ofstream foutput;
  foutput.open(filename, fstream::out);
  cout << "Slicer: " << endl;

  //write out per slice, which becomes a layer in the binary def.
  //pos_out
  //pos_in
  //neg_out
  //neg_in
  //then after all slices are done, at the interslice links for the node.

  for (int i=0 ; i< nb_nodes; i++)
  {
    map< int, map< int, vector< Slicer::LinkInfo > > >* tmp_links = NULL;
    int layer = 0;
    int direction = 0;
    for (int slice=0; slice<nb_slices; slice++) //we'll make a layer for each slice.
    {
      for (int k=0; k < 4; k++)
      {
        layer = slice*2 + (k/2);
        direction = k%2;
        switch(k)
        {
          case 0:
            tmp_links = &out_links; break;
          case 1:
            tmp_links = &in_links; break;
          case 2:
            tmp_links = &neg_out_links; break;
          case 3:
            tmp_links = &neg_in_links; break;
          default:
            tmp_links = NULL;
        }

        unsigned int s = (*tmp_links)[i][slice].size();

        for (unsigned int j=0 ; j < s ; j++)
        {
          int src       = i;
          int dest      = (*tmp_links)[i][slice][j].node;
          double weight = (*tmp_links)[i][slice][j].weight;

          foutput << src << "\t" << dest << "\t" << weight << "\t" << layer << "\t" << direction << endl;
        }
      }//for k (pos_out,pos_in,neg_out,neg_in)
    } //for layer (per slice)

    layer = nb_slices*2; //layer for the interslice links is the number of slices.
    //interslice out
    for (unsigned int j=0 ; j<interslice_out_links[i].size() ; j++)
    {
      int src       = i;
      int dest      = interslice_out_links[i][j].node;
      double weight = interslice_out_links[i][j].weight;
	    foutput << src << "\t" << dest << "\t" << weight << "\t" << layer << endl;
    }

    //interslice in
    for (unsigned int j=0 ; j<interslice_in_links[i].size() ; j++)
    {
      int src       = i;
      int dest      = interslice_in_links[i][j].node;
      double weight = interslice_in_links[i][j].weight;
	    foutput << src << "\t" << dest << "\t" << weight << "\t" << layer << endl;
    }
  }
  foutput.close();
}

void Slicer::display_node_mapping(char* filename)
{
  cerr << "Outputting node mapping..." << endl;
  ofstream foutput;
  foutput.open(filename, fstream::out);
  map<long, int>::iterator it;
  for (it = node_map.begin(); it != node_map.end(); it++)
  {
    int node;
    int slice;
    setId(it->first, node, slice);

    foutput << node << "\t" << node_names[node] << "\t" << slice << "\t" << slice_names[slice] << "\t" << "\t" << it->second << endl;
  }
}

void Slicer::display_conf(char* filename, int modelType)
{
  cerr << "Outputting layer configuration..." << endl;
  ofstream foutput;
  foutput.open(filename, fstream::out | fstream::binary);
  int c_s = modelType;
  int c_i = NO_NULL;
  int c_p = POSITIVE;
  int c_n = NEGATIVE;

  int n = nb_slices*2 + 1;

  foutput.write((char*)(&n), sizeof(int));
  //Configuration
  for (int slice = 0; slice < nb_slices; slice++)
  {
    foutput.write((char*)(&c_s), sizeof(int)); //Positive
    foutput.write((char*)(&c_s), sizeof(int)); //Negative
  }
  foutput.write((char*)(&c_i), sizeof(int)); //Interslice
  //Weighting (sign)
  for (int slice = 0; slice < nb_slices; slice++)
  {
    foutput.write((char*)(&c_p), sizeof(int)); //Positive
    foutput.write((char*)(&c_n), sizeof(int)); //Negative
  }
  foutput.write((char*)(&c_p), sizeof(int)); //Interslice
}

Graph* Slicer::get_graph()
{
  Graph* g = new Graph();

  int s = nb_nodes;
  int w = 1;
  int t = nb_slices*2 + 1;

  g->is_weighted = w;
  g->is_directed = w;

  g->nb_nodes    = s;
  g->nb_layers    = t;

  g->degrees = (int *)malloc((long)g->nb_nodes*g->nb_layers*(g->is_directed+1)*sizeof(int));
  //g->nonnull_layers_per_node = NULL;

  int tot=0;
  int ind=0;
  for (int i=0 ; i<s ; i++)
  {
    for (int slice=0; slice < nb_slices; slice++)
    {
      tot+=out_links[i][slice].size();
      g->degrees[ind++] = tot;

      tot+=in_links[i][slice].size();
      g->degrees[ind++] = tot;

      tot+=neg_out_links[i][slice].size();
      g->degrees[ind++] = tot;

      tot+=neg_in_links[i][slice].size();
      g->degrees[ind++] = tot;
    }

    tot+=interslice_out_links[i].size();
    g->degrees[ind++] = tot;

    tot+=interslice_in_links[i].size();
    g->degrees[ind++] = tot;
  }

  g->nb_links=g->degrees[g->nb_nodes*g->nb_layers*(g->is_directed+1)-1]/2;
  g->links = (int *)malloc((long)g->nb_links*sizeof(int)*2);
  //links
  ind=0;
  for (int i=0 ; i<s ; i++)
  {
    for (int slice=0; slice < nb_slices; slice++)
    {
      for (int k = 0; k < 4; k++)
      {
        map< int, map< int, vector< Slicer::LinkInfo > > >* tmp_links;

        switch(k)
        {
          case 0:
            tmp_links = &out_links; break;
          case 1:
            tmp_links = &in_links; break;
          case 2:
            tmp_links = &neg_out_links; break;
          case 3:
            tmp_links = &neg_in_links; break;
          default:
            tmp_links = NULL;
        }

        vector< Slicer::LinkInfo >* tmp = &((*tmp_links)[i][slice]);

        unsigned int size = tmp->size();
        for (unsigned int j=0 ; j<size ; j++)
          g->links[ind++] = (*tmp)[j].node;
      }
    }


    vector< Slicer::LinkInfo >* tmp = &(interslice_out_links[i]);

    unsigned int size = tmp->size();
    for (unsigned int j=0 ; j<size ; j++)
      g->links[ind++] = (*tmp)[j].node;

    tmp = &(interslice_in_links[i]);

    size = tmp->size();
    for (unsigned int j=0 ; j<size ; j++)
      g->links[ind++] = (*tmp)[j].node;
  }

  if (g->is_weighted)
  {
    g->weights = (double *)malloc((long)g->nb_links*sizeof(double)*2);
  }
  //weights
  ind=0;
  for (int i=0 ; i<s ; i++)
  {
    for (int slice=0; slice < nb_slices; slice++)
    {
      for (int k = 0; k < 4; k++)
      {
        map< int, map< int, vector< Slicer::LinkInfo > > >* tmp_links;

        switch(k)
        {
          case 0:
            tmp_links = &out_links; break;
          case 1:
            tmp_links = &in_links; break;
          case 2:
            tmp_links = &neg_out_links; break;
          case 3:
            tmp_links = &neg_in_links; break;
          default:
            tmp_links = NULL;
        }

        vector< Slicer::LinkInfo >* tmp = &((*tmp_links)[i][slice]);

        unsigned int size = tmp->size();
        for (unsigned int j=0 ; j<size ; j++)
          g->weights[ind++] = (*tmp)[j].weight;
      }
    }

    vector< Slicer::LinkInfo >* tmp = &(interslice_out_links[i]);

    unsigned int size = tmp->size();
    for (unsigned int j=0 ; j<size ; j++)
      g->weights[ind++] = (*tmp)[j].weight;

    tmp = &(interslice_in_links[i]);

    size = tmp->size();
    for (unsigned int j=0 ; j<size ; j++)
      g->weights[ind++] = (*tmp)[j].weight;
  }
  g->init();
  g->init_self_weights();
  g->init_layers_per_node();

  return g;
}

void
Slicer::display_binary(char *filename) {
  cerr << "Outputting binary network to " << filename << "..." << endl;
  ofstream foutput;
  foutput.open(filename,fstream::out | fstream::binary);

  int s = nb_nodes;
  int w = 1;
  int t = nb_slices*2 + 1;

  foutput.write((char *)(&w), sizeof(int)); //weighted
  foutput.write((char *)(&w), sizeof(int)); //directed

  foutput.write((char *)(&s),sizeof(int)); //number of nodes
  foutput.write((char *)(&t),sizeof(int)) ; //number of layers (positive, negative, interslice)

  int tot=0;
  for (int i=0 ; i<s ; i++)
  {
    for (int slice=0; slice < nb_slices; slice++)
    {
      tot+=out_links[i][slice].size();
      foutput.write((char *)(&tot),sizeof(int));

      tot+=in_links[i][slice].size();
      foutput.write((char *)(&tot),sizeof(int));

      tot+=neg_out_links[i][slice].size();
      foutput.write((char *)(&tot),sizeof(int));

      tot+=neg_in_links[i][slice].size();
      foutput.write((char *)(&tot),sizeof(int));
    }

    tot+=interslice_out_links[i].size();
    foutput.write((char *)(&tot),sizeof(int));

    tot+=interslice_in_links[i].size();
    foutput.write((char *)(&tot),sizeof(int));
  }

  //links
  for (int i=0 ; i<s ; i++)
  {
    for (int slice=0; slice < nb_slices; slice++)
    {
      for (unsigned int j=0 ; j<in_links[i][slice].size() ; j++)
      {
        int dest = in_links[i][slice][j].node;
        foutput.write((char *)(&dest),sizeof(int));
      }
      for (unsigned int j=0 ; j<out_links[i][slice].size() ; j++)
      {
        int dest = out_links[i][slice][j].node;
        foutput.write((char *)(&dest),sizeof(int));
      }
      for (unsigned int j=0 ; j<neg_in_links[i][slice].size() ; j++)
      {
        int dest = neg_in_links[i][slice][j].node;
        foutput.write((char *)(&dest),sizeof(int));
      }
      for (unsigned int j=0 ; j<neg_out_links[i][slice].size() ; j++)
      {
        int dest = neg_out_links[i][slice][j].node;
        foutput.write((char *)(&dest),sizeof(int));
      }
    }

    for (unsigned int j=0 ; j<interslice_out_links[i].size() ; j++)
    {
      int dest = interslice_out_links[i][j].node;
      foutput.write((char *)(&dest),sizeof(int));
    }
    for (unsigned int j=0 ; j<interslice_in_links[i].size() ; j++)
    {
      int dest = interslice_in_links[i][j].node;
      foutput.write((char *)(&dest),sizeof(int));
    }
  }

  //weights
  for (int i=0 ; i<s ; i++)
  {
    for (int slice=0; slice < nb_slices; slice++)
    {
      for (unsigned int j=0 ; j<in_links[i][slice].size() ; j++)
      {
        double weight = in_links[i][slice][j].weight;
        foutput.write((char *)(&weight),sizeof(double));
      }
      for (unsigned int j=0 ; j<out_links[i][slice].size() ; j++)
      {
        double weight = out_links[i][slice][j].weight;
        foutput.write((char *)(&weight),sizeof(double));
      }
      for (unsigned int j=0 ; j<neg_in_links[i][slice].size() ; j++)
      {
        double weight = neg_in_links[i][slice][j].weight;
        foutput.write((char *)(&weight),sizeof(double));
      }
      for (unsigned int j=0 ; j<neg_out_links[i][slice].size() ; j++)
      {
        double weight = neg_out_links[i][slice][j].weight;
        foutput.write((char *)(&weight),sizeof(double));
      }
    }

    for (unsigned int j=0 ; j<interslice_out_links[i].size() ; j++)
    {
      double weight = interslice_out_links[i][j].weight;
      foutput.write((char *)(&weight),sizeof(double));
    }
    for (unsigned int j=0 ; j<interslice_in_links[i].size() ; j++)
    {
      double weight = interslice_in_links[i][j].weight;
      foutput.write((char *)(&weight),sizeof(double));
    }
  }

  foutput.close();

}
