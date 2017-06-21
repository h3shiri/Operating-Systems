#ifndef EX5_WHATSAPPSERVER_H
#define EX5_WHATSAPPSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <list>
#include <vector>
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <algorithm>

#include <map>
#include <asm/param.h>

#include <sstream>
#include <iterator>


#define MAX_CLIENTS 10
#define MAX_MSG_LEN 1024

#define F_INIT 1
#define F_ERROR -1
#define F_SUCCESS 0

#define ESC_SEQ "EXIT"

void startBinding(int sockfd, int *resFlag);

void startListening(int sockfd, int *resFlag);

/**
 * the actual traffic for the sever accepting incoming connections..etc
 */
void startTraffic();

void passingData(int socket, string data);

/**
 * the driver function for the various commands processing the clients
 * requests.
 * assuming valid input from the client.
 */
void processRequest(string rawCommand, int clientSocket);

void createGroupRoutine(string groupName, string rawListOfUsers,
                        int clientSocketId);

void whoRoutine(string clientName, int clientSocketId);

void sendRoutine(string targetName, string message,
                 string clientName, int clientSocketId);

void sendRoutine(string targetName, string message,
                 string clientName, int clientSocketId);

void exitRoutine(string clientName, int clientSocketId);


#endif //EX5_WHATSAPPSERVER_H
