CXX             := g++
OPTS            := -Ofast -march=native -DNDEBUG
LANG            := -std=c++14
PICKY           := -Wall
INCLUDES	:= -Isrc
LIBS		+= -L/usr/local/lib
XDEFS		+= -fopenmp

CXXFLAGS	+= $(DEFS) $(XDEFS) $(OPTS) $(DEBUG) $(PROFILE) $(LANG) $(PICKY) $(INCLUDES) $(DIAG)

all		: hopfield

hopfield: hopfield.o matrix.o vector.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

hopfield.o: src/hopfield.cpp src/matrix.hpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

matrix.o: src/matrix.cpp src/matrix.hpp src/CImg.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

vector.o: src/vector.cpp src/vector.hpp src/CImg.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean		:
		  /bin/rm -f hopfield.o hopfield matrix.o vector.o
