/* greedy_louvain.cpp
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

#include "greedy_louvain.h"

static int total_nodes_visited = 0;

// Total number of passes
int             GreedyLouvain::nb_pass                        = 1000;
double          GreedyLouvain::min_modularity                 = 10e-6;
double          GreedyLouvain::min_prop_changes               = 10e-4;
int             GreedyLouvain::iterate_randomly               = 0;
int             GreedyLouvain::move_individual                = 0;
int             GreedyLouvain::max_nb_threads                 = 1;
#ifdef THREAD_SUPPORT
pthread_mutex_t GreedyLouvain::rand_lock                      = PTHREAD_MUTEX_INITIALIZER;
#endif

long waiting_time = 0;
long skipped_nodes = 0;
//detect_communities(Community* c)
//c - The community on which to apply the algorithm
//
// This function sets up the actual detection of the communities
// which basically amounts to dealing with the several hierarchical
// levels. The actual implementation per level is given in the one_level
// function (threaded or non-threaded). Some of the helper functions
// are maintained in the Community class, but some aspects of the
// algorithm, specifically the hierarchical part and the threaded part
// needs to be separately implemented.
void GreedyLouvain::detect_communities(Community* c)
{
  if (c->g->nb_nodes == 0)
    return;

  int level   = 0;
  double diff = 1; //Increase in modularity
  //double mod  = 0.0; //Most recent modularity

  //We want stochastic algorithm with lambda parameters
  //clock_t t1, t2;
  time_t tt1, tt2;
  //t1 = clock();
  time(&tt1);

  diff = one_level(c);

  Community* cc = c;
  Graph* g      = NULL;
  //First try to merge communities
  while(diff>min_modularity)
  {
    Graph* g_temp = g; // Temporary pointer, so that we can delete it later
    g = cc->partition2graph();
    if (g_temp != NULL)  // Delete the temp pointer
      delete g_temp;
    if (cc != c)
      delete cc;

    cc = new Community(g, c->null_model_per_layer, c->sign_per_layer, c->lambda_per_layer);

    // First try to merge communities
    diff = one_level(cc);

    // Reinitialize the community assignments
    c->reinit_communities(cc);

    if (move_individual)
    {
      //then do one level again possibly
      diff += one_level(c);
    }
    //mod = c->modularity();

    level++;
  }

  if (g != NULL)
    delete g;

  if (cc != c)
    delete cc;

  //t2 = clock();
  time(&tt2);

  /*cerr << "Visited " << total_nodes_visited << " nodes in " << ((double)(t2 - t1)/CLOCKS_PER_SEC) << " CPU s. "
       << (double)total_nodes_visited/((double)(t2 - t1)/CLOCKS_PER_SEC) << " nodes/CPU s. " << endl
       << "Spend " << (double)waiting_time/CLOCKS_PER_SEC << " CPU s. waiting. "
       << "Skipped " << skipped_nodes << " nodes. "  << endl
       << "Real time spend: " << tt2 - tt1 << " s. "
       << (double)total_nodes_visited/((double)(tt2 - tt1)) << " nodes/s. " << endl;*/

}

double GreedyLouvain::one_level(Community* cc)
{
  double mod1   = cc->modularity(); //Start modularity
  int nb_nodes  = cc->g->nb_nodes;

#ifdef THREAD_SUPPORT
  int nb_threads    = (int)(double)nb_nodes/NODES_PER_THREAD;

  if (nb_threads > max_nb_threads) // Never use more than NUM_THREADS
    nb_threads = max_nb_threads;

  if (nb_threads < 1) // Use (obviously) at least one
    nb_threads = 1;
  
  ThreadSync* ts = new ThreadSync(nb_nodes, nb_threads);

  if (nb_threads == 1)
  { // Use single thread (more efficient routine, when only using one thread)
    one_level_single(cc, ts);
  }
  else
  { // Use multiple threads
    int rc, i;

    // Create all threads
    pthread_t* threads = new pthread_t[nb_threads];

    level_thread_data* ltd = new level_thread_data[nb_threads];
    pthread_mutex_lock( &(ts->sync_lock) ); // Make sure the threads only start when all have been created
    for (i=0; i<nb_threads; ++i)
    {
      ltd[i].c = cc;
      ltd[i].ts = ts;
      ltd[i].thread_id = i;
      rc = pthread_create(&threads[i], NULL, one_level_thread, (void *) &ltd[i]);
      assert(0 == rc);
    }
    pthread_mutex_unlock( &(ts->sync_lock) ); // We're finished creating threads, so let's start the threads.

    /* wait for all threads to complete */
    for (i=0; i<nb_threads; ++i) {
      rc = pthread_join(threads[i], NULL);
      assert(0 == rc);
    }

    delete [] threads;

  }
#else //If no thread support
  ThreadSync* ts = new ThreadSync(nb_nodes, 1);
  one_level_single(cc, ts);
#endif  
  delete ts;
  cc->renumber_communities();

  double mod2 = cc->modularity();
  return mod2 - mod1;
}

double GreedyLouvain::one_level_single(Community* cc, ThreadSync* ts)
{
#ifdef THREAD_SUPPORT
  pthread_mutex_lock(&rand_lock);
  MTRand* r = new MTRand();
  //cerr << "MTRand initialized with seed " << s << endl;
  pthread_mutex_unlock(&rand_lock);
#else
  MTRand* r = new MTRand();
#endif  

  int nb_pass_done      = 0;
  double m              = cc->g->total_weight();

  int nb_nodes_visited  = 0;

  // All the nodes have been claimed now, so we can start iterating over them
  deque<int> nodes(cc->g->nb_nodes, 1);
  for (int i = 0; i < cc->g->nb_nodes; i++)
    nodes[i] = i;

  if (GreedyLouvain::iterate_randomly)
  {
    random_shuffle(nodes, r); // Reshuffle nodes
  }

  //cerr << "Single thread claimed " << nodes.size() << " nodes." << endl;

  do // while there is any improvement
  {
    ts->reset_sync();

    nb_pass_done++;

    deque<int>::iterator it; // Iterator for the set of nodes
    for ( it=nodes.begin() ; it != nodes.end(); it++)
    {
      nb_nodes_visited++;
      total_nodes_visited++;
      int node = *it;

      // We were able to obtain locks for all the nodes, so examine this node further
      int node_comm     = cc->n2c[node];

      // computation of all neighboring communities of current node
      map<int,map<int, double> >* weight_to_comm   = new map<int,map<int, double> >();
      map<int,map<int, double> >* weight_from_comm = new map<int,map<int, double> >();
      set<int>* comm = new set<int>();
      cc->total_weight_node_comm(node, weight_to_comm, comm);
      cc->total_weight_comm_node(node, weight_from_comm, comm);

      // compute the nearest community for node
      // default choice for future insertion is the former community
      // remove node from its current community
      cc->remove(node, node_comm, (*weight_to_comm)[node_comm], (*weight_from_comm)[node_comm]);

      // Check out the current community of the node
      int best_comm        = node_comm;
      double base_increase = cc->modularity_gain(node, node_comm, (*weight_to_comm)[node_comm], (*weight_from_comm)[node_comm]);
      double best_increase = base_increase;

      // Check out all the other communities
      for (set<int>::iterator it=comm->begin() ; it!=comm->end() ; it++)
      {
        if (*it != node_comm && *it >= 0)
        {
          double increase = cc->modularity_gain(node, *it, (*weight_to_comm)[*it], (*weight_from_comm)[*it]);
          if (increase>best_increase) // If the community is better, remember it.
          {
            best_comm     = *it;
            best_increase = increase;
          }
        }
      }

      // If we actually made a change
      if (best_comm != node_comm)
      {
        ts->diff += (best_increase - base_increase)/m;          // Record the change made by the improvement
        ts->nb_improvements++;                                  // Increase the total number of changes
      }

      // insert node in the nearest community
      cc->insert(node, best_comm, (*weight_to_comm)[best_comm], (*weight_from_comm)[best_comm]);

      //delete the weights
      delete weight_to_comm;
      delete weight_from_comm;
      delete comm;
    } // End of loop on all nodes
  } while ((double)ts->nb_improvements/(double)cc->g->nb_nodes > min_prop_changes && ts->diff > min_modularity);
  //cerr << "Improvements: " << (double)ts->nb_improvements/(double)cc->g->nb_nodes << ", diff: " << ts->diff << endl;
  delete r;

  return ts->diff;
}

#ifdef THREAD_SUPPORT
void* GreedyLouvain::one_level_thread(void* data)
{
  level_thread_data* ltd = (GreedyLouvain::level_thread_data*) data;
  Community* cc = ltd->c;
  ThreadSync* ts = ltd->ts;
  int thread_id = ltd->thread_id;

  pthread_mutex_lock(&rand_lock);
  MTRand* r = new MTRand();
  pthread_mutex_unlock(&rand_lock);

  int nb_pass_done      = 0;
  double m              = cc->g->total_weight();

  int nb_nodes_visited  = 0;
  // First wait for lock to unleas (otherwise, more uneven distribution of nodes)
  pthread_mutex_lock( &(ts->sync_lock) );
  pthread_mutex_unlock( &(ts->sync_lock) );

  // Now we will start doing a breadth-first search and claiming the neighbours
  // from the initial node specified when calling this function.
  int nb_seed_nodes = 0;
  deque<int> seed_nodes;
  set<int> nodes_set;
  queue<int> nodes_to_visit;
  int s = ts->unclaimed_nodes.size();                           // Make sure there are still elements
  while (s > 0)                                                 // Make sure all nodes are claimed by a thread (e.g. also small components)
  {
    pthread_mutex_lock( &(ts->sync_lock) );                     // Lock
    s = ts->unclaimed_nodes.size();                             // Make sure there are still elements
    if (s <= 0)                                                 // If not
    {
      pthread_mutex_unlock( &(ts->sync_lock) );                 //  Unlock and
      break;                                                    //  get the hell out of here
    }
                                                                // If so
    int v = r->randInt(s-1);                                    //  Pick a random element
    set<int>::const_iterator it(ts->unclaimed_nodes.begin());   //  Set iterator to first element
    advance(it,v);                                              //  Advance the appropriate number of steps
    int node = *it;                                             //  Get the actual node number
    ts->unclaimed_nodes.erase(it);                              //  Remove this element from the set
    pthread_mutex_unlock( &(ts->sync_lock) );                   // Unlock

    nodes_to_visit.push(node);                                  // Add node to the queue

    pthread_mutex_lock( &(ts->node_locks[node]) );              // Lock node
    if (ts->thread_id_per_node[node] == -1)                     // Check if already claimed
    {
      ts->thread_id_per_node[node] = thread_id;                 // Claim the node for this thread
      nodes_set.insert(node);                                   // and add it to our list
      nb_seed_nodes++;
      seed_nodes.push_back(node);
    }
    pthread_mutex_unlock( &(ts->node_locks[node]) );            // Unlock node

    while (!nodes_to_visit.empty())                             // While there are still nodes to visit (i.e. neighbours)
    {
        int cn = nodes_to_visit.front();                        // Pick the first
        nodes_to_visit.pop();                                   // and pop it

        // Queue neighbours
        int nb_neigh = cc->g->nb_neighbors(cn);
        pair<int *,double *> neigh_pair = cc->g->neighbors(cn);

        for (int i = 0; i < nb_neigh; i++)
        {
          int neigh = neigh_pair.first[i];
          if (ts->thread_id_per_node[neigh] == -1)
          {
            pthread_mutex_lock(&(ts->node_locks[neigh]) );      // Lock the node, to make sure *we* claim it, not some other thread
            ts->thread_id_per_node[neigh] = thread_id;          // Claim the node for this thread
            nodes_set.insert(neigh);                            // Insert node to our set (per thread)
            nodes_to_visit.push(neigh);                         // Insert the node to the list of nodes we'll have to visit
            pthread_mutex_unlock( &(ts->node_locks[neigh]) );   // Unlock the node

            pthread_mutex_lock( &(ts->sync_lock) );
            ts->unclaimed_nodes.erase(neigh);                   // Make sure it is not in the unclaimed set
            pthread_mutex_unlock( &(ts->sync_lock) );
          }
        }
    }
  }
  //cerr << "Thread " << thread_id << " claimed " << nodes_set.size() << " nodes, reached through " << nb_seed_nodes << " seed nodes." << endl;
  //for (deque<int>::iterator it = seed_nodes.begin(); it != seed_nodes.end(); it++)
  //  cerr << thread_id << ") Used seed node " << *it << endl;
  // All the nodes have been claimed now, so we can start iterating over them
  deque<int> all_nodes(nodes_set.begin(), nodes_set.end());


  random_shuffle(all_nodes, r); // Reshuffle nodes

  do // while there is any improvement
  {

    nb_pass_done++;

    deque<int>::iterator it; // Iterator for the set of nodes
    queue<int> nodes(all_nodes);
    int node;
    while (!nodes.empty())
    {
      node=nodes.front();
      nodes.pop();
      nb_nodes_visited++;
      total_nodes_visited++;

      // Get all neighbours
      int nb_neigh = cc->g->nb_neighbors(node);
      pair<int *,double *> neigh_pair = cc->g->neighbors(node);

      int lock_success = true;
      // We lock before we go lock the nodes, in order to prevent a possible race-condition/deadlock situation
      // The following situation is possible. Consider node 1 who has neighbours 2 and 3 and node 4 who also
      // has neighbours 2 and 3. The when the following happens we obtain a race condition (who ever can claim
      // the complete neighbourhood first gets it)
      //       Thread A                 Thread B
      //------------------------------------------------------------
      //       trylock node 2           trylock node 3
      //       trylock node 3           trylock node 2
      //       fail, unlock all         fail, unlock all
      //
      // Although this might seem unlikely, for larger and large neighbourhoods, the probability of getting such a
      // race condition increases. Deadlocks will actually never happen using the trylocks, but when using ordinary locks
      // we obtain deadlock situations.
      pthread_mutex_lock( &(ts->lock) );
      clock_t t1 = clock();
      int cn = 0;
      for (cn = 0; cn < nb_neigh; cn++)                          // For each neighbour
      {
        // Because we use trylock, and not the usual lock method, we are able to
        // to go on with the next node (and put it back to the end of the queue) whenever
        // we are unable to deal with it immediately.
        int lock_err = pthread_mutex_trylock( &(ts->node_locks[neigh_pair.first[cn]]) );

        if (lock_err)     //  Lock neighbour
        {
          /*switch (lock_err)
          {
            case EBUSY:
              cerr << thread_id << ") Couldn't get a lock on node " << node << " because it was already locked." << endl; break;
            case EINVAL:
              cerr << thread_id << ") Couldn't get a lock on node " << node << " because it was not properly initialized." << endl; break;
            case EDEADLK:
              cerr << thread_id << ") Couldn't get a lock on node " << node << " because it was already called by the calling thread." << endl; break;
            default:
              cerr << thread_id << ") Couldn't get a lock on node " << node << " (unknown error " << lock_err << ")" << endl;  break;
          }*/
          // there was some problem with the lock, so unlock everything up until this points, and go to the next node
          lock_success = false;
          skipped_nodes++;
          nodes.push(node);
          break;
        }
      }
      if (!lock_success)
      {
        for (int i = cn - 1; i >= 0; i--)
          pthread_mutex_unlock( &(ts->node_locks[neigh_pair.first[i]]) );
      }
      //We want to know how much time threads spend waiting
      clock_t t2 = clock();
      waiting_time += (t2 - t1);
      pthread_mutex_unlock( &(ts->lock) );

      if (lock_success)
      {
        // We were able to obtain locks for all the nodes, so examine this node further
        int node_comm     = cc->n2c[node];

        // computation of all neighboring communities of current node
        map<int,map<int, double> >* weight_to_comm   = new map<int,map<int, double> >();
        map<int,map<int, double> >* weight_from_comm = new map<int,map<int, double> >();
        set<int>* comm = new set<int>();
        cc->total_weight_node_comm(node, weight_to_comm, comm);
        cc->total_weight_comm_node(node, weight_from_comm, comm);

        // compute the nearest community for node
        // default choice for future insertion is the former community
        // remove node from its current community
        pthread_mutex_lock( &(ts->comm_locks[node_comm]) );
        clock_t t1 = clock();
        cc->remove(node, node_comm, (*weight_to_comm)[node_comm], (*weight_from_comm)[node_comm]);
        clock_t t2 = clock();
        waiting_time += (t2 - t1);
        pthread_mutex_unlock( &(ts->comm_locks[node_comm]) );

        // Check out the current community of the node
        int best_comm        = node_comm;
        double base_increase = cc->modularity_gain(node, node_comm, (*weight_to_comm)[node_comm], (*weight_from_comm)[node_comm]);
        double best_increase = base_increase;

        // Check out all the other communities
        for (set<int>::iterator it=comm->begin() ; it!=comm->end() ; it++)
        {
          if (*it != node_comm && *it >= 0)
          {
            double increase = cc->modularity_gain(node, *it, (*weight_to_comm)[*it], (*weight_from_comm)[*it]);
            if (increase>best_increase) // If the community is better, remember it.
            {
              best_comm     = *it;
              best_increase = increase;
            }
          }
        }

        // If we actually made a change
        if (best_comm != node_comm)
        {
          pthread_mutex_lock( &(ts->bookkeeping_lock) );          // Make sure we are the only ones making the change
          ts->diff += (best_increase - base_increase)/m;          // Record the change made by the improvement
          ts->nb_improvements++;                                  // Increase the total number of changes
          pthread_mutex_unlock( &(ts->bookkeeping_lock) );        // Unlock
        }

        // insert node in the nearest community
        pthread_mutex_lock( &(ts->comm_locks[best_comm]) );
        t1 = clock();
        cc->insert(node, best_comm, (*weight_to_comm)[best_comm], (*weight_from_comm)[best_comm]);
        t2 = clock();
        waiting_time += (t2 - t1);
        pthread_mutex_unlock( &(ts->comm_locks[best_comm]) );

        // Unlock neighbourhood
        for (int i = 0; i < nb_neigh; i++)                          // For each neighbour
          pthread_mutex_unlock(&(ts->node_locks[neigh_pair.first[i]]));     //  Lock neighbour
        /*for ( it=neigh_set.begin() ; it !=neigh_set.end(); it++ )
          pthread_mutex_unlock(&GreedyLouvain::node_locks[*it]);*/

        //delete the weights
        delete weight_to_comm;
        delete weight_from_comm;
        delete comm;
      } // End if succesfully locked
    } // End of loop on all nodes

    // Synchronize the different threads
    pthread_mutex_lock( &(ts->sync_lock) );
    ts->nb_threads_finished_pass++;
    if (ts->nb_threads_finished_pass == ts->nb_threads)
    {
      // We are the last thread to have finished this pass, so we should check
      // whether we should do another pass or not
      if ((double)ts->nb_improvements/(double)cc->g->nb_nodes > min_prop_changes && ts->diff > min_modularity)
        ts->reset_sync(); // We should not stop yet, so reset the counters to determine whether we should stop or not.
      else
        ts->stop = true;

      // Signal the other thread to continue (i.e. do another pass, or stop here)
      pthread_cond_broadcast( &(ts->sync_signal) );
    }
    else // We were not the last thread, so we will have to wait for the signal to come
    {
      pthread_cond_wait( &(ts->sync_signal), &(ts->sync_lock) ); // Wait for signal of thread to have decided whether we should stop or not
    }
    pthread_mutex_unlock( &(ts->sync_lock) );

  } while (!ts->stop);
  //cerr << "Improvements: " << (double)ts->nb_improvements/(double)cc->g->nb_nodes << ", diff: " << ts->diff << endl;

  delete r;
  return NULL;
}
#endif

void GreedyLouvain::random_shuffle(deque<int> &v, MTRand* r)
{
  // Make a random shuffle (i.e. permutation) of the vector, using the Knuth shuffle
  int size = v.size();
  for (int i=size-1 ; i>1 ; i--)
  {
    int rand_pos = r->randInt(i-1);

    int tmp      = v[i];
    v[i] = v[rand_pos];
    v[rand_pos] = tmp;
  }
}

GreedyLouvain::ThreadSync::ThreadSync(int nb_nodes, int nb_threads)
{
#ifdef THREAD_SUPPORT
  if (nb_threads > 1)
  {
    thread_id_per_node        = new int[nb_nodes];
    node_locks                = new pthread_mutex_t[nb_nodes];
    comm_locks                = new pthread_mutex_t[nb_nodes]; // Initially there are as many communities as nodes

    //init mutexes, thread_ids and unclaimed nodes for all nodes
    unclaimed_nodes.clear();

    pthread_mutexattr_init(&lock_t);
    pthread_mutexattr_settype(&lock_t, PTHREAD_MUTEX_RECURSIVE); // Do recursive locking

    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&sync_lock, NULL);
    pthread_mutex_init(&bookkeeping_lock, NULL);

    pthread_cond_init(&sync_signal, NULL);

    for (int i = 0; i < nb_nodes; i++)
    {
      pthread_mutex_init(&node_locks[i], &lock_t);  // Init node lock
      pthread_mutex_init(&comm_locks[i], NULL);     // Init comm lock
      unclaimed_nodes.insert(i);                    // Push a node on the unclaimed nodes queue
      thread_id_per_node[i] = -1;                   // Init thread id
    }
  }
  this->nb_threads = nb_threads;
#endif

  reset_sync();
}

GreedyLouvain::ThreadSync::~ThreadSync()
{
#ifdef THREAD_SUPPORT
  if (nb_threads > 1)
  {
    delete [] thread_id_per_node;
    delete [] node_locks;
    delete [] comm_locks;
  }
#endif
}

void GreedyLouvain::ThreadSync::reset_sync()
{
#ifdef THREAD_SUPPORT
  stop                      = false;
  nb_threads_finished_pass  = 0;  
#endif
  nb_improvements           = 0;
  diff                      = 0.0;
}
