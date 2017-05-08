CC=g++
flags= -Wall -g -std=c++11
all_headers = Thread.h uthreads.h
all_cpp = Thread.cpp uthreads.cpp

default: clean libuthreads.a

Thread.o: Thread.cpp Thread.h
	$(CC) $(flags) -c Thread.cpp -o Thread.o

uthreads.o: uthreads.cpp Thread.cpp Thread.h uthreads.h
	$(CC) $(flags) -c uthreads.cpp -o uthreads.o

libuthreads.a: uthreads.o Thread.o
	$(AR) rcs $@ $^

tar: Makefile README $(all_cpp) Thread.h 
	tar -cvf ex2.tar Makefile README Thread.h uthreads.h $(all_cpp)

clean:
	rm -f *.o libuthreads.a ex2.tar test1 *~ *core

