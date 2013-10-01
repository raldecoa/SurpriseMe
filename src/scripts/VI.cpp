#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
using namespace std;


int main( int argc, char* argv[] )
{

  ifstream file1( argv[1] );
  ifstream file2( argv[2] );
  if ( !file1 )
    {
      std::cerr << "Unable to open file: " << file1 << std::endl;
      exit( 1 );
    }
  if ( !file2 )
    {
      std::cerr << "Unable to open file: " << file2 << std::endl;
      exit( 1 );
    }

  //Headers
  char trash[256];
  file1.getline(trash, 256);
  file2.getline(trash, 256);

  map<int,int> mappingA, mappingB;

  vector<int> x, y;
  int tmp;
  while( file1 >> trash >> tmp )
    x.push_back( tmp );
  while( file2 >> trash >> tmp )
    y.push_back( tmp );
     
  if( x.size() != y.size() )
    { 
      cerr << "ERROR: Different length: " 
	   << x.size() << " vs. " << y.size() << endl;
      exit( 1 );
    }


  int nNodes = x.size();

  //Create mapping
  int index = 0;
  for( int i = 0; i < nNodes; ++i )
      if( mappingA.find(x[i]) == mappingA.end() )
	{
	  mappingA.insert( pair<int,int>( x[i], index ) );
	  index++;
	}
  index = 0;
  for( int i = 0; i < nNodes; ++i )
    if( mappingB.find(y[i]) == mappingB.end() )
      {
	mappingB.insert( pair<int,int>( y[i], index ) );
	index++;
      }

  int Ca = (int) mappingA.size();
  int Cb = (int) mappingB.size();

  //Create confusion matrix
  vector<vector<int> > confusion_matrix (Ca, vector<int>(Cb));
  for( int i = 0; i < Ca; ++i )
    for( int j = 0; j < Cb; ++j )
      confusion_matrix[i][j] = 0;


  int i,j;
  for( int l = 0; l < nNodes; ++l )
    {
      i = mappingA[x[l]];
      j = mappingB[y[l]];
      confusion_matrix[i][j]++;
    }

  /*
  for( int i = 0; i < Ca; ++i ){
    for( int j = 0; j < Cb; ++j )
      cout << confusion_matrix[i][j] << "\t";
    cout << endl;
  }
  */

  //Compute Ni
  int Ni[Ca];
  for( int i = 0; i < Ca; ++i )
    {
      Ni[i] = 0;
      for( int j = 0; j < Cb; ++j )
	Ni[i] += confusion_matrix[i][j];
    }

  //Compute Nj
  int Nj[Cb];
  for( int j = 0; j < Cb; ++j )
    {
      Nj[j] = 0;
      for( int i = 0; i < Ca; ++i )
	Nj[j] += confusion_matrix[i][j];
    }

  //Compute numerator
  double num = 0;
  for( int i = 0; i < Ca; ++i )
    for( int j = 0; j < Cb; ++j )
      {
	if( confusion_matrix[i][j] == 0 ) num += 0;
	else num += confusion_matrix[i][j] 
	       * log( (double)(confusion_matrix[i][j] * nNodes) / (Ni[i] * Nj[j]) );
      }

  //Compute denominator
  double d1 = 0, d2 = 0;
  for( int i = 0; i < Ca; ++i )
    d1 += Ni[i] * log( (double)Ni[i] / nNodes );
  for( int j = 0; j < Cb; ++j )
    d2 += Nj[j] * log( (double)Nj[j] / nNodes );

  double nmi = -2 * num / ( d1 + d2 );

  double Ha = 0, Hb = 0;
  for(int i = 0; i < Ca; i++){
    double p = (double) Ni[i] / nNodes;
    Ha -= p * log(p);
  }
  for(int i = 0; i < Cb; i++){
    double p = (double) Nj[i] / nNodes;
    Hb -=  p * log(p);
  }

  double vi = (1 - nmi) * (Ha + Hb);

  // In case both partitions are composed of only one community
  if(Ca == 1 && Cb == 1){
    vi = 0;
    nmi = 1;
  }

  double oneMinusNMI = 1 - nmi;
  cout << vi << " - " << oneMinusNMI << endl;
}
