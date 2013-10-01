/* main_slicer.cpp
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
#include <stdlib.h>

using namespace std;

#define NO_NULL    1
#define ER_NULL    2
#define CONF_NULL  3
#define FIXED_NULL 4

char *infile          = NULL;
char *outfile         = NULL;
char *node_map_file   = NULL;
char *conf_file       = NULL;
int   modelType       = CONF_NULL;
int   is_weighted     = 1;
int   is_directed     = 1;
int   is_single_slice = 0;

double interslice_weight = 1.0;

void
usage(const char *prog_name, const char *more) {
  cerr << more;
  cerr << "usage: " << prog_name << " -i input_file -o outfile -n node_map_file -c conf_file -w interslice_weight" << endl << endl;
  cerr << "-C set the configuration file to the Constant Potts model" << endl;
  cerr << "-u undirected graph" << endl;
  cerr << "-U unweighted graph" << endl;
  cerr << "-s single slice (no slice indicated in links)" << endl;
  cerr << "read the graph and convert it to binary format." << endl;
  cerr << "The output file is flattened (all slices contained" << endl;
  cerr << "in one global network, with interslice links added)." << endl;
  cerr << "Therefore, the node map file can be used to look up what" << endl;
  cerr << "the mapping between the 'new nodes' and the original nodes are." << endl;
  cerr << endl;
  cerr << "The input format is: " << endl;
  cerr << ">" << endl;
  cerr << "<node_id> <node_name>" << endl;
  cerr << ">" << endl;
  cerr << "<slice_id> <slice_name>" << endl;
  cerr << ">" << endl;
  cerr << "<from_node> <to_node> <weight> <slice_id>" << endl;
  exit(0);
}

void
parse_args(int argc, char **argv) {
  for (int i = 1; i < argc; i++)
  {
    if(argv[i][0] == '-')
    {
      switch(argv[i][1])
      {
        case 'i':
          if (i==argc-1)
            usage(argv[0], "Infile missing\n");
          infile = argv[i+1];
          i++;
          break;
        case 'o':
          if (i==argc-1)
            usage(argv[0], "Outfile missing\n");
          outfile = argv[i+1];
          i++;
          break;
        case 'n':
          if (i==argc-1)
            usage(argv[0], "Node map file missing\n");
          node_map_file = argv[i+1];
          i++;
          break;
        case 'c':
          if (i==argc-1)
            usage(argv[0], "Config file missing\n");
          conf_file = argv[i+1];
          i++;
          break;
        case 'w':
          if (i==argc-1)
              usage(argv[0], "Interslice weight missing\n");
        	interslice_weight = atof(argv[i+1]);
          i++;
          break;
        case 'C':
          modelType = FIXED_NULL;
          break;
        case 'u':
          is_directed = 0;
          break;
        case 'U':
          is_weighted = 0;
          break;
        case 's':
          is_single_slice = 1;
          break;
        default:
          usage(argv[0], "Unknown option\n");
      }
    }
    else
    {
      usage(argv[0], "More than one filename\n");
    }
  }
  if (infile==NULL || outfile==NULL || node_map_file==NULL || conf_file==NULL)
    usage(argv[0], "File missing\n");
}

int
main(int argc, char **argv) {

  parse_args(argc, argv);

  /*infile            = "/media/TRAVELLER/src/test_networks/weighted_directed_nets/network.txt";
  outfile           = "/media/TRAVELLER/src/test_networks/weighted_directed_nets/network.bin";
  //char* outfile_alt = "/media/TRAVELLER/src/Louvain_Slice/sample_networks/example_alt.txt";
  node_map_file     = "/media/TRAVELLER/src/test_networks/weighted_directed_nets/network_node_map.txt";
  conf_file         = "/media/TRAVELLER/src/test_networks/weighted_directed_nets/network.conf";
  interslice_weight = 1.0;
  is_directed       = 1;*/

  cerr << "Converting " << infile << " to " << outfile << endl;
  cerr << "Interslice weight: " << interslice_weight << endl;

  Slicer s(infile, (double)interslice_weight, is_directed, is_weighted, is_single_slice);

  //s.display_binary(outfile);
  Graph* g = s.get_graph();
  g->display_binary(outfile);
  //g->display(outfile_alt);
  delete g;

  s.display_node_mapping(node_map_file);
  s.display_conf(conf_file, modelType);

}
