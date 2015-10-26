OFLAGS = -Ofast -flto -march=native
CC=g++
STD=-std=c++14
CFLAGS=  -g -c -W -Wall -Wextra $(STD) -Wno-missing-field-initializers -Wshadow \
				 -I/usr/include/SDL2 \
				 -fopenmp \
				$(OFLAGS)
LFLAGS= -g $(STD) $(OFLAGS)  -L/usr/lib/x86_64-linux-gnu/ -lSDL2 -pthread -fopenmp

.PHONY:clean 

Objects= main.o Timer.o

all : $(Objects) program  

program : $(Objects)
	$(CC) $(Std) $(Objects) $(LFLAGS)  -o program

$(Objects): %.o: %.cpp
	$(CC) $(CFLAGS) $<

dbg: program
	gdb program


Timer.o:Timer.cpp
	$(CC) $(CFLAGS) Timer.cpp

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
