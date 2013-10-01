/* main_community.cpp
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

#define NDEBUG

#include <sys/stat.h>

#include <stdio.h>
#include <string.h>

#include "../Base/greedy_louvain.h"

#include <unistd.h>
#define GetCurrentDir getcwd

  // number of pass for one level computation
  // if -1, compute as many pass as needed to increase modularity
  int nb_pass;

  // a new pass is computed if the last one has generated an increase
  // greater than min_modularity
  // if 0. even a minor increase is enough to go for one more pass
  double min_modularity;

  // Use random iteration over the nodes?
  int iterate_randomly;

  // Move individual nodes in between levels?
  int move_individual;

using namespace std;

char *filename        = NULL;
char *conf_filename   = NULL;
char *lambda_filename = NULL;
char *out_filename    = NULL;
int type              = UNWEIGHTED;
//int nb_pass           = 1000;
double precision      = 0.00000001;
int display_level     = -2;
int k1                = 16;
//int move_individual   = 0;
int stochastic        = 0;
int only_output_mod   = 0;
int nb_threads        = 1;

int* conf = NULL;
int* sign = NULL;
double* lambda = NULL;

// If no specific file specified, use these
// lambdas
double lambda_pos = 1.0;
double lambda_neg = 1.0;

const char* doc_string = "\
Usage: community [options] input_file conf_file\n\
Arguments:\n\
<input_file>      - Read the graph to partition from input_file.\n\
<conf_file>       - Read the configuration (null-model and sign) from <conf_file>\n\
\n\
Options:\n\
-c <conf_file>    - You may alternatively specify the configuration file here.\n\
-o <file>         - Output the communities to <file>.\n\
-q <epsilon>      - a given pass stops when the modularity is increased by less than <epsilon>.\n\
-pf <file>        - uses the resolution parameters for the layer in <file>.\n\
-pp <lambda+>     - Using the same positive <lambda+> for resolution for all positive layers.\n\
-pn <lambda->     - Using the same negative <lambda-> for resolution for all negative layers.\n\
-b <nb_pass>      - maximum number of passes for one level.\n\
-i                - do individual moves between hierarchical steps.\n\
-r                - use randomized node order.\n\
-m                - only output modularity.\n\
-h                - show this usage message.\n\
-t <#threads>     - indicate the number of threads to use (currently not supported).\n";

void
usage(const char *prog_name, const char *more) {
  cerr << more << endl;
  cerr << doc_string;
  exit(0);
}

void check_settings()
{
  struct stat stFileInfo;


  char cCurrentPath[1000];

  if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
  {
    usage("", "Couldn't get current path");
  }
  cerr << cCurrentPath << endl;

  //Check filename
  if (filename != NULL)
  {
    if(stat(filename,&stFileInfo))
    {
      //File does not exist
      usage("", "Can not find binary graph file.");
    }
  }
  else
  {
    usage("", "No binary graph file specified.");
  }

  //Check filename
  if (conf_filename != NULL)
  {
    if(stat(conf_filename,&stFileInfo))
      //File does not exist
      usage("", "Can not find configuration file");
  }
  else
  {
    usage("", "No configuration file specified.");
  }
}

void
parse_args(int argc, char **argv) {
  if (argc<2)
    usage(argv[0], "Bad arguments number\n");

  for (int i = 1; i < argc; i++)
  {
    if(argv[i][0] == '-')
    {
      switch(argv[i][1])
      {
      case 'c':
        if (i==argc-1)
          usage(argv[0], "Config file missing.\n");
        conf_filename = argv[i+1];
        i++;
        break;
      case 'q':
        precision = atof(argv[i+1]);
        i++;
        break;
      case 'l':
        display_level = atoi(argv[i+1]);
        i++;
        break;
      case 'k':
        k1 = atoi(argv[i+1]);
        i++;
        break;
      case 'i':
        move_individual = 1;
        break;
      case 'r':
        stochastic = 1;
        break;
      case 'm':
        only_output_mod = 1;
        break;
      case 'b':
        nb_pass = atoi(argv[i+1]);
        i++;
        break;
      case 'o':
        out_filename = argv[i+1];
        i++;
        break;        
      case 't':
        if (i==argc-1)
          usage(argv[0], "Number of threads missing\n");
        nb_threads = atoi(argv[i+1]);
        i++;
        break;
      case 'p':
        if (strcmp(argv[i],"-pf") == 0)
        {
          if (i==argc-1)
            usage(argv[0], "Lambda file missing\n");
          lambda_filename = argv[i+1];
          i++;
        }
        else if (strcmp(argv[i],"-pp") == 0)
        {
          if (i==argc-1)
            usage(argv[0], "Lambda parameter missing\n");
          lambda_pos = atof(argv[i+1]);
          i++;
        }
        else if (strcmp(argv[i], "-pn") == 0)
        {
          if (i==argc-1)
            usage(argv[0], "Lambda parameter missing\n");
          lambda_neg = atof(argv[i+1]);
          i++;
        }
        else
          usage(argv[0], "Unknown option\n");
        break;
      default:
        usage(argv[0], "Unknown option\n");
      }
    }
    else
    {
      if (filename==NULL)
        filename = argv[i];
      else if (conf_filename==NULL)
        conf_filename = argv[i];
      else
        usage(argv[0], "Only the binary graph file and the configuration file should be submitted\n");
    }
  }
}

void
display_time(const char *str) {
  time_t rawtime;
  time ( &rawtime );
  cerr << str << " : " << ctime (&rawtime);
}

int read_conf(char* filename, int* &conf, int* &sign)
{
  ifstream finput;
  finput.open(filename,fstream::in | fstream::binary);
  if (finput.fail())
  {
    cerr << "Couldn't read config file " << filename << endl;
    exit(-1);
  }

  int nb_layers;
  //read number of layers
  finput.read((char *)&nb_layers, sizeof(int));

  conf = (int*)malloc((long)nb_layers*sizeof(int));
  sign = (int*)malloc((long)nb_layers*sizeof(int));

  // read conf
  finput.read((char *)conf, nb_layers*sizeof(int));
  if (finput.fail() || finput.eof())
  {
    cerr << "Error";
    exit(-1);
    //throw new exception();
  }
  finput.read((char *)sign, nb_layers*sizeof(int));

  return nb_layers;
}

int read_lambda(char* filename, double* &lambda_per_layer, int nb_layers)
{
  ifstream finput;
  finput.open(filename,fstream::in);

  lambda_per_layer = new double[nb_layers];

  cerr << nb_layers << "\n";

  int i = 0;
  while (!finput.eof() && i < nb_layers)
  {
    double lambda;
    finput >> lambda;

    cerr << lambda << "\t";

    lambda_per_layer[i] = lambda;

    i++;
  }
  cerr << "\n";

  return 0;
}

void create_resolution(double lp, double ln, int nb_layers)
{
  for (int i = 0; i < nb_layers; i++)
  {
    if (sign[i] == POSITIVE)
      lambda[i] = lp;
    else if (sign[i] == NEGATIVE)
      lambda[i] = ln;
    else
      cerr << "Error, layer not recognized" << endl;
  }
}

int main(int argc, char **argv)
{
  srand(time(NULL));

  parse_args(argc, argv);
  /*only_output_mod = 1;
  filename = "/Volumes/TRAVELLER/src/Louvain_Slice_parallel/sample_networks/network.bin";
  conf_filename = "/Volumes/TRAVELLER/src/Louvain_Slice_parallel/sample_networks/network.conf";*/

  check_settings();

  time_t time_begin, time_end;
  time(&time_begin);
  display_time("start");

  int layers = read_conf(conf_filename, conf, sign);
  lambda = new double[layers];

  // use the resolution parameters file if present
  if (lambda_filename != NULL)
  {
    cerr << "Using lambda file " << lambda_filename << endl;
    read_lambda(lambda_filename, lambda, layers);
  }
  else
  {
    cerr << "Using lambda+ " << lambda_pos << ", lambda- " << lambda_neg << endl;
    create_resolution(lambda_pos, lambda_neg, layers);
  }

  Community* co        = new Community(filename, conf, sign, lambda);

  GreedyLouvain::iterate_randomly = stochastic;
  GreedyLouvain::move_individual  = move_individual;
  GreedyLouvain::max_nb_threads   = nb_threads;

  if (nb_threads > 1)
    cerr << "Using multithreading with at most " << nb_threads << " threads." << endl;

  if (move_individual)
    cerr << "Moving individual nodes between hierarchical levels." << endl;

  if (stochastic)
    cerr << "Using random node order." << endl;

  display_time("file read");

  //cerr << "Hierarchical level 0"<< endl;

  cerr << "network : "
       << co->g->nb_nodes << " nodes, "
       << co->g->nb_links << " links, " << endl;

  GreedyLouvain::detect_communities(co);
  double mod = co->modularity();
  if (!only_output_mod)
  {
    ostream* comm_output = &cout;
    ofstream* foutput = new ofstream();

    if (out_filename != NULL)
    {
      foutput->open(out_filename,fstream::out);
      comm_output = foutput;
    }

    co->display_partition(*comm_output);

    if (out_filename != NULL)
    {
      foutput->close();
    }

    delete foutput;
  }
  else
    cout << "'" << filename << "'\t" << mod << "\t" << co->nb_comm <<  endl;

  time(&time_end);

  cerr << mod << " " << co->nb_comm << " in " << (double)(time_end-time_begin) << "s." << endl;

  free(conf);
  free(sign);
  if (lambda != NULL)
    delete [] lambda;

  delete co; co = NULL;
}
