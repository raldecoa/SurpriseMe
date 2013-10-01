/* greedy_louvain.h
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


#ifndef GREEDY_LOUVAIN_H
#define GREEDY_LOUVAIN_H

//#define THREAD_SUPPORT

#define NODES_PER_THREAD 10000

#include "community.h"
#include <set>
#include <queue>
#include <algorithm>

#ifdef THREAD_SUPPORT
#include <pthread.h>
#include <errno.h>
#endif

#include "../MTRand/MersenneTwister.h"

class GreedyLouvain
{
  public:
  static void detect_communities(Community* c); // Detect communities and return community vector

  // number of pass for one level computation
  // if -1, compute as many pass as needed to increase modularity
  static int nb_pass;

  // a new pass is computed if the last one has generated an increase
  // greater than min_modularity
  // if 0. even a minor increase is enough to go for one more pass
  static double min_modularity;

  // Minimum proportion of changes (below this percentage of changes)
  // we will not make another pass
  static double min_prop_changes;

  // Use random iteration over the nodes?
  static int iterate_randomly;

  // Move individual nodes in between levels?
  static int move_individual;

  // Maximum number of threads to use
  static int max_nb_threads;

private:

  //////////////////////////////////////////////////////////////////////////////
  // Syncing variables
  //////////////////////////////////////////////////////////////////////////////
  // These variables allow the threads to synchronize at the end of a pass
  // and to hence determine whether another general pass should be made. Since
  // each thread will only examine a smaller portion of it's nodes, it is not
  // necessarily the case that if that part is optimized, it won't be affected by
  // optimizations in another part of the network. Hence, the decision to do
  // another pass, must be made in synchrony.

  class ThreadSync
  {
    public:
    // Variables required for threading
    // If these variables would be static, it would interfere when doing
    // multiple communities at the same time.

    // Keep track of the number of improvements in each pass
    int nb_improvements;

    // Keep track of the increase in modularity
    double diff;

#ifdef THREAD_SUPPORT
    // Keep track of the number of threads that have finished their part of the pass
    int nb_threads_finished_pass;

    // This is the total number of threads. If this contains *ANY* error, there will either
    // be some deadlock situation, or there will be unsynchronized threads.
    int nb_threads;

    // This is the shared condition upon which all threads base themselves in order to
    // determine whether they should stop or not.
    int stop;

    //////////////////////////////////////////////////////////////////////////////
    // Locking variables
    //////////////////////////////////////////////////////////////////////////////
    // These variables are used to enable multi threaded community detection
    //
    // General lock, used for the node locking section. This is to
    // prevent deadlock situations in which two different nodes have
    // shared neighbours and are hence weighting for each other to unlock
    // that neighbour.
    pthread_mutex_t lock;

    // Bookkeeping lock, for if any changes are being made to the bookkeeping of
    // the algorithm. That is, changes to community assignments, et cetera.
    pthread_mutex_t bookkeeping_lock;

    // Node lock, per node. Each node can be locked, so that other thread can't
    // simultaneously examine it and make changes. Also the neighbours are locked
    // so as to minimize the affects of interfering changes from other threads. These
    // effects are still present in the expected values, but should be relatively
    // minor in terms of optimization. The locking of the neighbours however does ensure
    // that the bookkeeping remains correct. That is, the total weight from and to a
    // community *cannot* change when examining a node, ensuring correct bookkeeping.
    pthread_mutex_t* node_locks; // Need to dynamically allocate this one

    // Comm lock, per community. This way, changes can be made for different communities
    // at the same time, without affecting each other. As is the case with the node lock
    // it is best to have as much of the nodes of the same community in the same thread
    // so that they interfere as little as possible.
    pthread_mutex_t* comm_locks; // Need to dynamically allocate this one

    // The lock to enable synchronization signals
    pthread_mutex_t sync_lock;

    // The synchronization signals. All threads wait at the end of the pass, to let the
    // last thread decide whether they should stop or not. After deciding whether to stop
    // or not, it broadcasts to all other thread, signaling it may continue.
    pthread_cond_t  sync_signal;

    pthread_mutexattr_t lock_t; // Lock type for the recursive locks


    int* thread_id_per_node;
    set<int>unclaimed_nodes;

#endif

    // Functions
    void reset_sync();
    ThreadSync(int nb_nodes, int nb_threads);   // Constructor
    ~ThreadSync();                              // Destructuror
  };

  // compute communities of the graph for one level
  // returns the modularity
  static double one_level(Community* c);
#ifdef THREAD_SUPPORT  
  static void* one_level_thread(void* cc); //Do threaded community detection (one level)
#endif  
  static double one_level_single(Community* cc, ThreadSync* ts); // Run a single level (without multiple threads)

  static void random_shuffle(deque<int> &v, MTRand* r);

  // Use this PRNG to initialize the seed (to avoid multhread problems when detecting communities
  // repeatedly, i.e. when probing resolution parameters)
  //static MTRand& mt_rand();
#ifdef THREAD_SUPPORT  
  static pthread_mutex_t rand_lock;

  struct level_thread_data
  {
    Community* c;
    ThreadSync* ts;
    int thread_id;
  };
#endif
  
};
#endif
