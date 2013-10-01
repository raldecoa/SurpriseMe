#ifndef UPGMAC_H
#define UPGMAC_H

#include <vector>

class UpgmaCluster
{
public:
  UpgmaCluster(){ 
    left = 0;
    right = 0;
    //height = 0;
  }
  
  ~UpgmaCluster(){
    delete left;
    delete right;
  }

  int index;
  UpgmaCluster* left;
  UpgmaCluster* right;
  //  double height;
};
#endif
