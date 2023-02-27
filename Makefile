CXX = g++ -O3 -Wall
CXXFLAGS = -I /usr/include/SDL2
LDLIBS = -lm -lSDL2
INCLUDES = /usr/include/SDL2

gltest: sdltest.o
	${CXX} $^ -o sdltest ${LDLIBS}

sdltest.o: sdltest.c
	${CXX} -c $^ ${LDLIBS} -I${INCLUDES}

clean:
	rm -f sdltest *.o
