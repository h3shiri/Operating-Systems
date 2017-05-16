CC=g++
flags= -g -std=c++11 -Wextra -Wall -lpthread
all_headers = defs1.h
all_cpp = MapReduceFramework.cpp Search.cpp
delete_files = *.o Search MapReduceFramework.a .MapReduceFramework.log ex3.tar
tar_files = $(all_headers) $(all_cpp) Makefile README

default: clean MapReduceFramework.a Search

Search.o: MapReduceFramework.h Search.cpp
	$(CC) -c $(flags) Search.cpp -o Search.o

Search: MapReduceFramework.a Search.o
	$(CC) $(flags) Search.o -L. MapReduceFramework.a -o Search

MapReduceFramework.o: MapReduceFramework.h MapReduceClient.h defs1.h MapReduceFramework.cpp
	$(CC) -c $(flags) MapReduceFramework.cpp -o MapReduceFramework.o

MapReduceFramework.a: MapReduceFramework.o
	$(AR) rcs $@ $^

tar: $(tar_files) 
	tar -cvf ex3.tar $(tar_files)

clean:
	rm -f $(delete_files) *~ *core

