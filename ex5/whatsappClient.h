#ifndef EX5_WHATSAPPCLIENT_H
#define EX5_WHATSAPPCLIENT_H

#include "common.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define ERROR -1
//enum request{ "who", "create_group", "exit", "send"};



//TO DO check if enough
#define BUFFER_SIZE 1024


void serverMsgTreat(string msg);

void exitClients();


int parssingInput(char* msg);

void passingData(int socket, std::string data);


bool membersCheck(string groupMmbers);

/* reading from server */
void readingFromServer(char * buffer, int sockfd);

/* reading from terminal */
void readingFromTerminal(char * buffer, int sockfd);

/*treat message that comes as a response to the client request*/
void treatResponse(string msg);














#endif //EX5_WHATSAPPCLIENT_H
