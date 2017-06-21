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




#define BUFFER_SIZE 256

void passingData(int socket, std::string data);

#endif //EX5_WHATSAPPCLIENT_H
