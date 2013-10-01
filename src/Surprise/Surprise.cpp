/****************************************************************************
 * Copyright (C) 2012 Rodrigo Aldecoa and Ignacio Marín                     *
 *                                                                          *
 *  This program is free software: you can redistribute it and/or modify    *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, either version 3 of the License, or       *
 *  (at your option) any later version.                                     *
 *                                                                          *
 *  This program is distributed in the hope that it will be useful,         *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *                                                                          *
 * Contact info: Rodrigo Aldecoa <raldecoa@ibv.csic.es>                     *
 ****************************************************************************/


/*
 If you use this program, please cite:
       Aldecoa R, Marín I (2011)
       Deciphering network community structure by Surprise
       PLoS ONE 6(9): e24195
*/


#include <cmath>

#include "Surprise.h"


double computeSurprise(const double& F, const double& M, 
		       const double& n, const double& p)
{ 
  double j = p;
  double logP = logHyperProbability(F, M, n, j);
 
  double min = M;
  if(n < M) 
    min = n;
  
  bool isEnough = false;
  while(!isEnough && j < min){
    j++;
    double nextLogP = logHyperProbability(F, M, n, j);
    isEnough = sumLogProbabilities(nextLogP, logP);
  }
  if(logP == 0)
    logP *= -1;
  return -logP;
}

double logHyperProbability(const double& F, const double& M, 
			   const double& n, const double& j)
{
  double logH = logC(M, j) + logC(F - M, n - j) - logC(F, n);
  return logH / log(10);
}

double logC(const double& n, const double& k)
{
  if(k == n || !k)
    return 0;
  
  else if(n > 1000 && k > 1000) // Stirling's binomial coeff approximation
    {
    return n * log(n) - (n-k) * log(n-k) - k * log(k);
    }
  else
    {
      double t = n - k;
      if(t < k) t = k;
      
      double logC = sumRange(t + 1, n) - sumFactorial(n - t);
      return logC;
    }
}

double sumRange(const double& min, const double& max)
{
  double sum = 0.0;
  for(double i = min; i <= max; ++i)
    sum += log(i);
  
  return sum;
}

double sumFactorial(const double& n)
{
  if(n > 1000)
    {
      return n * log(n) - n;
    }
  else
    {
      double sum = 0.0;
      for(int i = 2; i <= n; ++i)
	sum += log(i);
      
      return sum;
    }
}


bool sumLogProbabilities(const double& nextLogP, double& logP)
{
  
  if(nextLogP == 0) 
    return true;
  
  // Several optimizations to avoid over/underflow problems
  double common, diffExponent;
  if(nextLogP > logP){
    common = nextLogP;
    diffExponent = logP - common;
  }
  else{
    common = logP;
    diffExponent = nextLogP - common;
  }
  logP = common + ( (log(1 + pow(10, diffExponent))) / log(10) );
  
  // The cumulative summation stops when the increasing is less than 10e-4
  if(nextLogP - logP > -4)
    return true;
  
  return false;
}
