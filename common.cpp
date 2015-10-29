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

inline bool newState( const Map &_map, int row, int col ){
  return game_of_life( _map, row, col );
}

void update( Map &_map, Map &_back ){
#pragma omp parallel for
  for( int i=0; i<side_length; i++){
    for( int j=0; j<side_length; j++){
      _back.set(i,j, newState(_map, i, j));
    }
  }
}

inline void rand_row( Map &_map,  int row, int seed ){
  mt19937 rand;
  rand.seed( seed );
  for( int i=0; i<side_length; i++){
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
