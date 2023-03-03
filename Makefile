CXX = g++ -O3 -Wall
CXXFLAGS = -I /usr/include/SDL2
LDLIBS = -lm -lSDL2
INCLUDES = /usr/include/SDL2

sdltest: sdltest.o devrand.o frametimer.o sdllibraryhelper.o lifescalar.o
	${CXX} $^ -o sdltest ${LDLIBS}

sdltest.o: sdltest.c
	${CXX} -c $^ ${LDLIBS} -I${INCLUDES}
devrand.o: devrand.cc
	${CXX} -c $^ ${LDLIBS} -I${INCLUDES}
frametimer.o: frametimer.cc
	${CXX} -c $^ ${LDLIBS} -I${INCLUDES}
sdllibraryhelper.o: sdllibraryhelper.cc
	${CXX} -c $^ ${LDLIBS} -I${INCLUDES}
lifescalar.o: lifescalar.cc
	${CXX} -c $^ ${LDLIBS} -I${INCLUDES}

clean:
	rm -f sdltest *.o
