#include "common.h"

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <map>
#include <iterator>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <asm/param.h>


#ifndef EX5_WHATSAPPSERVER_H
#define EX5_WHATSAPPSERVER_H

#define MAX_CLIENTS 10
#define MAX_MSG_LEN 1024

#define F_INIT 1
#define F_ERROR -1
#define F_SUCCESS 0


void startBinding(int sockfd, int *resFlag);

void startListening(int sockfd, int *resFlag);

/**
 * the actual traffic for the sever accepting incoming connections..etc
 */
void startTraffic();

void passingData(int socket, std::string data);

/**
 * the driver function for the various commands processing the clients
 * requests.
 * assuming valid input from the client.
 */
void processRequest(std::string rawCommand, int clientSocket);

void createGroupRoutine(std::string groupName, std::string rawListOfUsers,
                        int clientSocketId);

void whoRoutine(std::string clientName, int clientSocketId);

void sendRoutine(std::string targetName, std::string message,
                 std::string clientName, int clientSocketId);

void sendRoutine(std::string targetName, std::string message,
                 std::string clientName, int clientSocketId);

void exitRoutine(std::string clientName, int clientSocketId);


#endif //EX5_WHATSAPPSERVER_H
