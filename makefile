OFLAGS = -Ofast -flto -march=native
CC=g++
STD=-std=c++14
CFLAGS=  -g -c -W -Wall -Wextra $(STD) -Wno-missing-field-initializers -Wshadow \
				 -fopenmp \
				$(OFLAGS)



NO_SDL_LFLAGS= -g $(STD) $(OFLAGS)   -pthread -fopenmp
SDL_LFLAGS = `sdl2-config --libs` $(NO_SDL_LFLAGS) 

.PHONY:clean 

Base_Objects = Timer.o Map.o
Objects= $(Base_Objects) 

all : $(Objects) no_sdl with_sdl

with_sdl: $(Base_Objects) with_sdl.o 
	$(CC) $(Std) $(Base_Objects) with_sdl.o $(SDL_LFLAGS) -o with_sdl

no_sdl: $(Base_Objects) no_sdl.o 
	$(CC) $(Std) $(Base_Objects) no_sdl.o $(NO_SDL_LFLAGS) -o no_sdl

$(Objects): %.o: %.cpp config.h
	$(CC) $(CFLAGS) $<

with_sdl.o: with_sdl.cpp common.cpp Map.h config.h
	$(CC) `sdl2-config --cflags`  with_sdl.cpp $(CFLAGS)

no_sdl.o : no_sdl.cpp common.cpp Map.h config.h
	$(CC) $(CFLAGS) no_sdl.cpp

clean:
	rm -f *o 
	rm -f c*grind\.out\.*
	rm -f with_sdl no_sdl
