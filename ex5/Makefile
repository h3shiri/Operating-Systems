CC=g++
flags= -Wall -g -std=c++11
# all_headers = Block.h BlockStack.h
all_cpp = whatsappClient.cpp whatsappServer.cpp

default: clean whatsappServer whatsappClient

whatsappServer.o: whatsappServer.cpp
	$(CC) $(flags) -c whatsappServer.cpp -o whatsappServer.o

whatsappClient.o: whatsappClient.cpp 
	$(CC) $(flags) -c whatsappClient.cpp -o whatsappClient.o

whatsappServer: whatsappServer.o
	$(CC) $(flags) whatsappServer.o -o whatsappServer

whatsappClient: whatsappClient.o
	$(CC) $(flags) whatsappServer.o -o whatsappClient

tar: Makefile README $(all_cpp)
	tar -cvf ex5.tar Makefile README $(all_cpp)

clean:
	rm -f *.o ex5.tar whatsappServer whatsappClient *~ *core 