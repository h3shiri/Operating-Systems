CC=g++
flags= -Wall -g -std=c++11
all_headers = Block.h BlockStack.h
all_cpp = Block.cpp BlockStack.cpp CacheFS.cpp CacheFS.cpp

default: clean CacheFS.a

Block.o: Block.cpp Block.h
	$(CC) $(flags) -c Block.cpp -o Block.o

BlockStack.o: BlockStack.cpp Block.cpp Block.h BlockStack.h
	$(CC) $(flags) -c BlockStack.cpp -o BlockStack.o

CacheFS.o: $(all_cpp) $(all_headers) CacheFS.h
	$(CC) $(flags) -c CacheFS.cpp -o CacheFS.o

CacheFS.a: CacheFS.o BlockStack.o Block.o
	$(AR) rcs $@ $^

tar: Makefile README $(all_cpp) $(all_headers) 
	tar -cvf ex4.tar Makefile README Answers.pdf $(all_headers) $(all_cpp)

clean:
	rm -f *.o CacheFS.a ex4.tar *~ *core 