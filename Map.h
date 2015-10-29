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
  struct cell{
    uint8_t set:1;
    uint8_t count:7;
  };

  std::vector<cell> data;

  Map(){
    data.resize(side_length*side_length);
  }

  bool get( int row, int col ) const {
    return data[row*side_length+col].set;
  }

  void set( const int row, const int col, const bool on){
    cell *where = &data[row*side_length+col];
    bool is_set = where->set;
    if( is_set == on ) return;

    unsigned char direction = ((int)on) * 2 - 1;

    where->set = on;
    //manual unroling of setting neighboring counts
    int rows[3];
    int cols[3];

    rows [0] = row -1;
    rows [1] = row ;
    rows [2] = row + 1;
    if( rows[0] < 0 ) rows[0] += side_length;
    if( rows[2] >= side_length) rows[2] -= side_length;

    rows[0] *= side_length;
    rows[1] *= side_length;
    rows[2] *= side_length;

    cols [0] = col -1;
    cols [1] = col ;
    cols [2] = col +1;
    if( cols[0] < 0 ) cols[0] += side_length;
    if( cols[2] >= side_length) cols[2] -= side_length;

    data[ rows[0] + cols[0] ].count += direction;
    data[ rows[0] + cols[1] ].count += direction;
    data[ rows[0] + cols[2] ].count += direction;

    data[ rows[1] + cols[0] ].count += direction;
    data[ rows[1] + cols[2] ].count += direction;

    data[ rows[2] + cols[0] ].count += direction;
    data[ rows[2] + cols[1] ].count += direction;
    data[ rows[2] + cols[2] ].count += direction;
  }

  static int wrapped( int x) {
    if( x < 0 ) x += side_length;
    if( x >= side_length ) x -= side_length;
    return x;
  }

  void set_wrap( int row, int col, bool on){
    row = wrapped( row );
    col = wrapped( col );
    set( row, col, on);
  }

  bool get_wrap( int row, int col ) const {
    row = wrapped(row); 
    col = wrapped(col);
    return get( row, col );
  }

  size_t size() const {
    return side_length;
  }

  int neighbors( int i, int j) const {
    return data[i*side_length+j].count;
  }
};

#endif   /* ----- #ifndef Map_INC  ----- */
