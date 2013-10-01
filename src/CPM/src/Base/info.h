/* info.h
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

#ifndef INFO_H
#define INFO_H


#include <deque>
#include <map>
#include <math.h>
#include <iostream>

int* get_array(std::deque<int> m);
std::deque<int> get_deque(int n, int* m);

double entropy(std::deque<int> m);
double entropy(int n, int* m);

double entropy_un(std::deque<int> m);
double entropy_un(int n, int* m);

double joint_entropy(std::deque<int> m1, std::deque<int> m2);
double joint_entropy(int n, int* m1, int* m2);

// Unnormalized, calculated in terms of frequencies (to prevent
// some round off error to accumulate
double joint_entropy_un(std::deque<int> m1, std::deque<int> m2);
double joint_entropy_un(int n, int* m1, int* m2);

double calc_vi(std::deque<int> m1, std::deque<int> m2);
double calc_vi(int n, int* m1, int* m2);

double calc_nvoi(std::deque<int> m1, std::deque<int> m2);
double calc_nvoi(int n, int* m1, int* m2);

double calc_nmi(std::deque<int> m1, std::deque<int> m2);
double calc_nmi(int n, int* m1, int* m2);

#endif
