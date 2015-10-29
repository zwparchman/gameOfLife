/*  common functions to both version */
#include "Map.h"
#include "config.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <future>
#include <iostream>
#include <limits.h>
#include <random>
#include "Timer.h"
#include <vector>

using namespace std;

typedef random_device rand_dev;
//typedef mt19937 rand_dev;

std::uniform_int_distribution<int> idistro( INT_MIN, INT_MAX);
std::bernoulli_distribution distro(live_chance / 100.0f);

void swap( Map &a, Map&b ){
  swap(a.data, b.data);
}

Map map;
Map back;

inline bool game_of_life( const Map &_map, int row, int col ){
  switch( _map.neighbors(row, col)){
    case 0: case 1:
      return false;
    case 2:
      return _map.get(row,col);
    case 3:
      return true;
    default:
      return false;
  }
}

#if  0     /* ----- #if 0 : If0Label_1 ----- */
inline bool day_night( const Map &_map, int row, int col){
  const int n = neighbors( _map, row, col);
  const bool live = _map.get(row,col);

  vector<int> come_alive = {3,6,8};
  //vector<int> stay_alive = {3,4,6,8};

  if( std::any_of( come_alive.begin(), come_alive.end(), [=](int x){return n==x;}))
    return true;

  if( live && n == 4 )
    return true;

  return false;
}

inline bool seeds( const Map &_map, int row, int col){
  int n = neighbors( _map, row, col);
  if( n == 2){
    return true;
  }
  return false;
}


inline bool no_death( const Map &_map, int row, int col){
  int n = neighbors( _map, row, col);
  int ret=_map.get(row,col);
  if( n == 3){
    ret= true;
  }
  return ret;
}


#endif     /* ----- #if 0 : If0Label_1 ----- */

inline bool newState( const Map &_map, int row, int col ){
  return game_of_life( _map, row, col );
}

void update( Map &_map, Map &_back ){
#pragma omp parallel for
  for( size_t i=0; i<side_length; i++){
    for( size_t j=0; j<side_length; j++){
      _back.set(i,j, newState(_map, i, j));
    }
  }
}

inline void rand_row( Map &_map,  int row, int seed ){
  mt19937 rand;
  rand.seed( seed );
  for( size_t i=0; i<side_length; i++){
    bool be_alive = distro(rand) ;
    if( be_alive){
      _map.set(row, i, be_alive);
    }
  }
}

void gen_map(){
  rand_dev mt;
    #pragma omp parallel for
    for( size_t i=0; i< map.size(); i++){
      int seed; 
      #pragma omp critical
      seed = idistro(mt);

      rand_row( map, i, seed);
    }
}
