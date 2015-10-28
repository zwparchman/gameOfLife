
#ifndef  Map_INC
#define  Map_INC

#include <vector>

#include <algorithm>
#include <array>
#include <bitset>
#include <future>
#include <iostream>
#include <limits.h>
#include <random>
#include "Timer.h"
#include <vector>

#include "config.h"

struct Map {
  std::vector<bool> data;

  Map(){
    data.resize(side_length*side_length);
  }

  bool get( int row, int col ) const {
    return data[row*side_length+col];
  }
  void set( int row, int col, bool on){
    data[row*side_length+col] = on;
  }

  void set_wrap( int row, int col, bool on){
    if( row < 0 ) row += side_length;
    if( col < 0 ) col += side_length;
    set( row % side_length, col % side_length, on);
  }

  bool get_wrap( int row, int col ) const {
    row = row % side_length;
    col = col % side_length;
    if( row < 0 ) row += side_length;
    if( col < 0 ) col += side_length;
    return get( row, col );
  }

  size_t size() const {
    return side_length;
  }
};

#endif   /* ----- #ifndef Map_INC  ----- */
