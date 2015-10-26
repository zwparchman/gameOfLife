OFLAGS = -Ofast -flto -march=native
CC=g++
STD=-std=c++14
CFLAGS=  -g -c -W -Wall -Wextra $(STD) -Wno-missing-field-initializers -Wshadow \
				 `sdl2-config --cflags` \
				 -fopenmp \
				$(OFLAGS)
LFLAGS= -g $(STD) $(OFLAGS)  `sdl2-config --libs` -pthread -fopenmp

.PHONY:clean 

Objects= main.o Timer.o

all : $(Objects) program  

program : $(Objects)
	$(CC) $(Std) $(Objects) $(LFLAGS)  -o program

$(Objects): %.o: %.cpp
	$(CC) $(CFLAGS) $<

dbg: program
	gdb program

run: program
	./program

time: program
	time ./program

cache: program
	rm c*grind* -f
	valgrind --tool=cachegrind ./program

call: program
	rm c*grind* -f
	valgrind --tool=callgrind ./program

inspect: 
	kcachegrind c*grind\.out\.*

clean:
	rm -f *o 
	rm -f program
	rm -f c*grind\.out\.*
