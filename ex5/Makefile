CC=g++
flags= -Wall -g -std=c++11
# all_headers = Block.h BlockStack.h
all_cpp = whatsappClient.cpp whatsappServer.cpp
all_headers = whatsappServer.h whatsappClient.h common.h 

default: clean whatsappServer whatsappClient

whatsappServer.o: whatsappServer.cpp
	$(CC) $(flags) -c whatsappServer.cpp -o whatsappServer.o

whatsappClient.o: whatsappClient.cpp 
	$(CC) $(flags) -c whatsappClient.cpp -o whatsappClient.o

whatsappServer: whatsappServer.o
	$(CC) $(flags) whatsappServer.o -o whatsappServer

whatsappClient: whatsappClient.o
	$(CC) $(flags) whatsappClient.o -o whatsappClient

tar: Makefile README $(all_cpp) $(all_headers)
	tar -cvf ex5.tar Makefile README $(all_cpp) $(all_headers)

clean:
	rm -f *.o ex5.tar whatsappServer whatsappClient whatsappClientTemp *~ *core 

whatsappClientTemp.o: tempClient3.cpp
	$(CC) $(flags) -c tempClient3.cpp -o whatsappClientTemp.o


whatsappClientTemp: whatsappClientTemp.o
	$(CC) $(flags) whatsappClientTemp.o -o whatsappClientTemp


test: clean whatsappServer whatsappClientTemp

