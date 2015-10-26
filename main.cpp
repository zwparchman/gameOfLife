#include <limits.h>
#include <future>
#include <algorithm>
#include <iostream>
#include <vector>
#include <SDL.h>
#include <random>
#include <array>
#include <bitset>
#include "Timer.h"


#define COUT(x) " "#x": "<<(x)
using namespace std;

const int side_length=400;

typedef random_device rand_dev;
//typedef mt19937 rand_dev;

struct Map {
  vector<bool> data;

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

void swap( Map &a, Map&b ){
  swap(a.data, b.data);
}


std::uniform_int_distribution<int> distro(0,100);

#define ERR_PRINT(x) if(x) { cout << SDL_GetError() << endl; SDL_ClearError();  }


Map map;
Map back;



int pixel_shift = 3;
unsigned int frame_time=10; //unsigned since <0 is an error
int xoff=0;
int yoff=0;
int square_size=2;
bool paused=false;
bool working = false;
bool single_step = false;
bool need_new_surface=false;
bool fullscreen = false;
SDL_GameController * controler = NULL;

//starting window size is set here, this is runtime resizable
int window_x = 1200;
int window_y = 720;


template <typename T>
T clamp( T cur , T low, T high){
  if( cur < low ) return low;
  if( cur > high ) return high;
  return cur;
}

template<typename R>
  bool is_ready(std::future<R> const& f)
  { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }

void display_wraped( SDL_Surface *root, const Map &_map){
  for( int i=-yoff; i< -yoff + window_y / square_size + 1 ; i++){
    if( (yoff+i)*square_size > window_y ) break;
    for( int j=-xoff; j< -xoff + window_x / square_size + 1 ; j++){
      if( (xoff+j)*square_size > window_x ) break;
      SDL_Rect r; 
      r.x = (xoff+j)*square_size;
      r.y = (yoff+i)*square_size;
      r.h = square_size;
      r.w = square_size;
      
      if( _map.get_wrap(i,j) ){
        ERR_PRINT(SDL_FillRect( root, &r , 0x00000000 ));
      }
    }
  }
}

void display( SDL_Surface *root, const Map &_map){
  for( int i=max(0,-yoff); i< (int)_map.size()  ; i++){
    if( (yoff+i)*square_size > window_y ) break;
    for( int j= max(0, -xoff) ; j< (int)_map.size(); j++){
      if( (xoff+j)*square_size > window_x ) break;
      SDL_Rect r; 
      r.x = (xoff+j)*square_size;
      r.y = (yoff+i)*square_size;
      r.h = square_size;
      r.w = square_size;
      
      if( _map.get(i,j) ){
        ERR_PRINT(SDL_FillRect( root, &r , 0x00000000 ));
      }
    }
  }
}

int neighbors( const Map &_map, int i, int j){
  int ret=0;
  for( int r=-1; r<=1; r++){
    int modr = (i+r)%side_length;
    if( modr < 0 ){
      modr+= side_length;
    }
    for( int c=-1; c<=1; c++){
      if( r==0 && c==0 ) continue;

      int modc = (j+c)%side_length;
      if( modc < 0 ){
        modc+= side_length;
      }
      if( _map.get(modr, modc)){
        ret++;
      }
    }
  }
  return ret;
}

inline bool game_of_life( const Map &_map, int row, int col ){
  int n = neighbors( _map, row, col);

  switch( n ){
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

std::pair<int,int> mouse_square( int mousex, int mousey ){
  float x_per_side = window_x / (float) square_size;
  float y_per_side = window_y / (float) square_size;
  float x_percent = (float) mousex / window_x ;
  float y_percent = (float) mousey / window_y ;

  int wherex = x_percent * x_per_side - xoff;
  int wherey = y_percent * y_per_side - yoff;

  return make_pair( wherex, wherey);
}

bool input(){
  SDL_Event e;
  while( SDL_PollEvent( &e ) ){
    if( e.type == SDL_KEYDOWN ){

      switch( e.key.keysym.scancode ){
        case SDL_SCANCODE_DELETE:
          return true;

        case SDL_SCANCODE_A:  case SDL_SCANCODE_LEFT:
          xoff += pixel_shift; break;
        case SDL_SCANCODE_D:  case SDL_SCANCODE_RIGHT:
          xoff -= pixel_shift; break;
        case SDL_SCANCODE_W:  case SDL_SCANCODE_UP:
          yoff += pixel_shift; break;
        case SDL_SCANCODE_S: case SDL_SCANCODE_DOWN:
          yoff -= pixel_shift; break;

        case SDL_SCANCODE_Z:
          pixel_shift = clamp( --pixel_shift, 1, INT_MAX); break;
        case SDL_SCANCODE_X:
          pixel_shift = clamp( ++pixel_shift, 1, INT_MAX); break;


        case SDL_SCANCODE_F11:
          fullscreen = !fullscreen;
          need_new_surface = true;
          break;


        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:
          single_step=true;
          break;

        case SDL_SCANCODE_EQUALS:
          frame_time = clamp( frame_time*4, (unsigned)2, UINT_MAX);

        case SDL_SCANCODE_MINUS:
          frame_time /= 2;
          cout<<"Frame_time: "<<frame_time<<endl;
          break;

        case SDL_SCANCODE_SPACE:
          paused = !paused; break;

        case SDL_SCANCODE_Q:
          square_size -= 1; 
          if( square_size == 0 ) square_size = 1;
          break;
        case SDL_SCANCODE_E:
          square_size += 1; 
          break;

        default:
          break;
      }
    } else if( e.type == SDL_MOUSEBUTTONDOWN ){
      int mousex, mousey;
      int buttons = SDL_GetMouseState(&mousex, &mousey);

      auto pair = mouse_square( mousex, mousey );
      
      int wherex = pair.first;
      int wherey = pair.second;

      bool set_value = buttons & 1;

      back.set_wrap(wherey, wherex, set_value);
      map.set_wrap(wherey, wherex, set_value);
    } else if( e.type == SDL_WINDOWEVENT ){
      switch( e.window.event ){
        case SDL_WINDOWEVENT_CLOSE:
          return false;

        case SDL_WINDOWEVENT_RESIZED:
          window_x = e.window.data1;
          window_y = e.window.data2;
          need_new_surface = true;
          break;

        default:
          break;
      }
    }
  }

  return false;
}

const int live_chance = 5;

void rand_row( Map &_map,  int row, int seed ){
  mt19937 rand;
  rand.seed( seed );
  for( size_t i=0; i<side_length; i++){
    _map.set(row, i, distro(rand) < live_chance );
  }
}

uint32_t gray_by_value( float in){
  uint8_t val = in*255;
  uint32_t ret = val;
  ret = ret << 8 | ret;
  ret = ret << 16 | ret;
  return ret;
}



int main(){
  cout<<"Begin-----------"<<endl;

  rand_dev mt;
#pragma omp parallel for
  for( size_t i=0; i< map.size(); i++){
    int seed; 
#pragma omp critical
    seed = distro(mt);

    rand_row( map, i, seed);
  }

  if( SDL_Init( SDL_INIT_EVERYTHING ) ){ cout << SDL_GetError() << endl; SDL_ClearError(); }

  SDL_Window * window = SDL_CreateWindow("Game Of Life", 0,0, \
      window_x, window_y, SDL_SWSURFACE | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
  SDL_Surface * root = SDL_GetWindowSurface( window );

  float up_to_date_gray = 0.5f;
  float oldest_gray = 0.25f;
  float update_grayness = up_to_date_gray;
  uint32_t current_color = gray_by_value( update_grayness );

  ERR_PRINT( SDL_FillRect( root, NULL, current_color ) );
  ERR_PRINT( SDL_UpdateWindowSurface( window ) );
  SDL_Delay( frame_time ) ;

  display(root,map);
  ERR_PRINT(SDL_UpdateWindowSurface( window ));

  int i=0;

  auto do_step = [&](){
      int start_at = SDL_GetTicks() + frame_time;
      //cout<<"Step "<<i<<endl;
      i++;
      working = true;
      update(map, back);
      working = false;

      int delta = 0;
      while( (delta = SDL_GetTicks() - (start_at + frame_time)) < 0 ){
        SDL_Delay( min( -delta, 10) );
      }
      return 0;
  };


  future<int> fut = std::async(std::launch::async, do_step);

  while(true){
    if( need_new_surface ){
      need_new_surface = false;

      if( fullscreen ){
        ERR_PRINT(SDL_SetWindowFullscreen( window, SDL_WINDOW_FULLSCREEN));
      } else {
        ERR_PRINT(SDL_SetWindowFullscreen( window, 0));
      }
      root = SDL_GetWindowSurface( window );
      if( ! root ){
        ERR_PRINT(0);
        cerr<<"failed to get window surface"<<endl;
        return 1;
      }
    }
    if ( ! paused || single_step ){
      single_step = false;
      if( is_ready(fut) ){
        fut.get();
        swap(map,back);
        fut=std::async(std::launch::async, do_step);
        update_grayness = 0x88;
      }
  }


    current_color = gray_by_value( clamp(update_grayness , oldest_gray, up_to_date_gray));
    ERR_PRINT( SDL_FillRect( root, NULL, current_color ) );

    if( ! paused ){
      update_grayness =  clamp( update_grayness*0.999f, oldest_gray, up_to_date_gray);
    }

    display( root, map);
    ERR_PRINT(SDL_UpdateWindowSurface( window ));

    if( input() ){
      break;
    }

    if( frame_time ){
      SDL_Delay(min((unsigned int)16, frame_time) );
    }

  }

  SDL_Quit();

  cout<<"END-----------"<<endl;
  return 0;
}
