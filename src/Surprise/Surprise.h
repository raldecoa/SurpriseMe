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


#ifndef SURPRISE_H
#define SURPRISE_H

// Global function. Receives the four parameters F, M, n and p 
// and returns the value of Surprise
double computeSurprise(const double& F, const double& M, 
		       const double& n, const double& p);

// Computes one term of the summation 
// (a single hypergeometric probability)
double logHyperProbability(const double& F, const double& M, 
			   const double& n, const double& j);


/*** Other functions ***/

// Computes log(n k) --logarithm of a binomial coefficient
double logC(const double& n, const double& k);

// Function needed to simplify the division of factorials
double sumRange(const double& min, const double& max);

// Computes log(n!)
double sumFactorial(const double& n);

// Computes the sum of the past and current terms of the cumulative summation
bool sumLogProbabilities(const double& nextLogP, double& logP);

#endif
