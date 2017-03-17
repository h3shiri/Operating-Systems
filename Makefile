CC=g++
flags= -Wall -g -std=c++11
all_headers = osm.h
all_cpp = osm.cpp

default: clean libosm.a

osm.o: osm.cpp osm.h
	$(CC) $(flags) -c osm.cpp -o osm.o

libosm.a: osm.o
	$(AR) $(ARFLAGS) $@ $^

tar: Makefile README $(all_headers) $(all_cpp)
	tar -cvf ex1.tar Makefile README $(all_headers) $(all_cpp)

clean:
	rm -f *.o libosm.a ex1.tar test1 *~ *core

simpletest.o: simpletest.cpp osm.h
	$(CC) $(flags) -c simpletest.cpp -o simpletest.o

test1: simpletest.o osm.o
	$(CC) $(flags) simpletest.o osm.o -o test1
