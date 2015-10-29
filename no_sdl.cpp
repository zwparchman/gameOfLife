#include "config.h"
#include "common.cpp"

int main(){
  cout<<"Begin---"<<endl;

  Timer gen_timer, main_timer;
  gen_timer.start();
  gen_map();
  gen_timer.stop();


  cout<<"Generating the grid took "<<gen_timer.getTime() << " seconds."<<endl;
  main_timer.start();
  for( int i=0; i<iterations; i++){
    update( map, back); 
    swap( map, back);
  }
  main_timer.stop();

  cout<<"Performed "<<iterations<<" steps of a grid with a side lenth of "<<side_length<<
    " in "<<main_timer.getTime()<<" seconds."<<endl;

  cout<<"END-----"<<endl;
}
