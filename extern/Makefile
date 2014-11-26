LIBNAME = libsimanneal.a

CPPSRC	= $(wildcard *.cpp)   

OBJ	= $(patsubst %.cpp,%.o,$(CPPSRC))

INCLUDE = -Iinclude

CC	= clang
CXX	= clang++

CFLAGS	= $(INCLUDE) -std=c++11 -g -Wall -O2 -march=native -Wno-unused-variable
CXXFLAGS = $(CFLAGS)
LDFLAGS	= -g -Wall -O2 -march=native

all: lib

.PHONY : lib
lib: $(OBJ)
	ar rcs $(LIBNAME) $^


%.o:	%.c Makefile
	$(CC) -c $(CFLAGS) $(@:.o=.c) -o $@

%.o:	%.cpp Makefile
	$(CXX) -c $(CXXFLAGS) $(@:.o=.cpp) -o $@

.PHONY : clean
clean: 
	\rm -f *.o $(LIBNAME)