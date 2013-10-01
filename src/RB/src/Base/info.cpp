/* info.cpp
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

#include "info.h"

using namespace std;

int* get_array(deque<int> m)
{
  int* a = new int[m.size()];
  int i = 0;
  for(deque<int>::iterator it=m.begin(); it != m.end(); it++)
    a[i++] = (*it);
  return a;
}

std::deque<int> get_deque(int n, int* m)
{
  std::deque<int> d(n);
  for (int i = 0; i < n; i++)
    d[i] = m[i];
  return d;
}


double entropy(deque<int> m)
{
  double e = entropy_un(m);
  return (e/m.size()) + log((double)m.size());
}

double entropy_un(deque<int> m)
{
  int* ma = get_array(m);
  double e = entropy_un(m.size(), ma);
  delete [] ma;
  return e;
}

double entropy(int n, int* m)
{
  double e = entropy_un(n, m);
  return (e/n) + log((double)n);
}

double entropy_un(int n, int* m)
{
  double entrpy = 0.0;

  //Create probabilities
  map<int, double> table;
  for ( int i = 0; i < n; i++)
  {
    table[m[i]] += 1;
  }

  //Calc entropy
  for ( map<int, double>::iterator e=table.begin(); e != table.end(); e++)
  {
    if (e->second > 0)
      entrpy -= (e->second) * log(e->second);
  }
  return entrpy;
}

double joint_entropy(deque<int> m1, deque<int> m2)
{
  double je = joint_entropy_un(m1, m2);
  return (je/m1.size()) + log((double)m1.size());
}

double joint_entropy_un(deque<int> m1, deque<int> m2)
{
  int* ma1 = get_array(m1);
  int* ma2 = get_array(m2);
  double e = joint_entropy_un(m1.size(), ma1, ma2);
  delete [] ma1;
  delete [] ma2;
  return e;
}

double joint_entropy(int n, int* m1, int* m2)
{
  double je = joint_entropy_un(n, m1, m2);
  return (je/n) + log((double)n);
}

double joint_entropy_un(int n, int* m1, int* m2)
{
  //Create probabilities
  map< int, map<int, double> > table;
  for (int i = 0; i < n; i++)
  {
    table[m1[i]][m2[i]] += 1;
  }

  //Calc joing entropy
  double joint_e = 0.0;
  for ( map< int, map<int, double> >::iterator ex=table.begin(); ex != table.end(); ex++)
  {
    for (map<int, double>::iterator ey=(ex->second).begin(); ey != (ex->second).end(); ey++)
    {
      if (ey->second > 0)
        joint_e -= (ey->second) * log(ey->second);
    }
  }
  return joint_e;
}

double calc_vi(deque<int> m1, deque<int> m2)
{
  int* ma1 = get_array(m1);
  int* ma2 = get_array(m1);
  double e = calc_nvoi(m1.size(), ma1, ma2);
  delete ma1;
  delete ma2;
  return e;
}

double calc_vi(int n, int* m1, int* m2)
{
  double e1       = entropy_un(n, m1);
  double e2       = entropy_un(n, m2);
  double joint_e  = joint_entropy_un(n, m1, m2);

  double nvoi = 0.0;

  nvoi = (2*joint_e - (e1 + e2))/n;
  if (nvoi < 0) { nvoi *= -1; }
  return nvoi;
}

double calc_nvoi(deque<int> m1, deque<int> m2)
{
  int* ma1 = get_array(m1);
  int* ma2 = get_array(m1);
  double e = calc_nvoi(m1.size(), ma1, ma2);
  delete ma1;
  delete ma2;
  return e;
}

double calc_nvoi(int n, int* m1, int* m2)
{
  double e1       = entropy_un(n, m1);
  double e2       = entropy_un(n, m2);
  double joint_e  = joint_entropy_un(n, m1, m2);

  double nvoi = 0.0;
  double nlogn = (double)n*log((double)n);

  nvoi = 2 - (e1 + e2 + 2*nlogn)/(joint_e + nlogn);
  if (nvoi < 0) { nvoi *= -1; }
  return nvoi;
}

double calc_nmi(deque<int> m1, deque<int> m2)
{
  int* ma1 = get_array(m1);
  int* ma2 = get_array(m1);
  double e = calc_nmi(m1.size(), ma1, ma2);
  delete [] ma1;
  delete [] ma2;
  return e;
}

double calc_nmi(int n, int* m1, int* m2)
{
  //Create probabilities 1
  map<int, double> p1;
  for ( int i = 0; i < n; i++)
  {
    p1[m1[i]] += 1;
  }
  //Create probabilities 2
  map<int, double> p2;
  for ( int i = 0; i < n; i++)
  {
    p2[m2[i]] += 1;
  }

  //Create joint probabilities
  map< int, map<int, double> > table;
  for (int i = 0; i < n; i++)
  {
    table[m1[i]][m2[i]] += 1;
  }

  //Calc mutual inf
  double mutual_inf = 0.0;
  for ( map< int, map<int, double> >::iterator ex=table.begin(); ex != table.end(); ex++)
  {
    for (map<int, double>::iterator ey=(ex->second).begin(); ey != (ex->second).end(); ey++)
    {
      if ( p1[ex->first] > 0 && p2[ey->first] > 0)
      {
        if (ey->second > 0)
          mutual_inf += (ey->second) * log((ey->second) / (p1[ex->first] * p2[ey->first]) );
      }
    }
  }

  double e1 = entropy_un(n, m1);
  double e2 = entropy_un(n, m2);
  double nd = (double)n;
  double nmi = 2*(mutual_inf + nd*log(nd))/(e1 + e2 + 2*nd*log(nd));
  if (nmi < 0) { nmi *= -1; }
  return nmi;
}

