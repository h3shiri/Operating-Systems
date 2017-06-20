#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <list>
#include <arpa/inet.h>
#include <cstring>

#include <map>


#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"

#define MAX_CLIENTS 10
#define ERROR -1

using namespace std; 

void print_error(string Msg);

/* data structs for the server side */

// list of the relevant active clients.
list<string> lClients;


// actual relevent socket address.
struct sockaddr_in my_addr;

int init_server(string name, const char* address, const char* port)
{
    my_addr.sin_family = AF_INET;
    int sockfd, portN;
	sockfd =  socket(AF_INET, SOCK_STREAM, 0);
	portN = stoi(port);
	my_addr.sin_port = htons(portN);
	int check1 = inet_aton(address, &(my_addr.sin_addr));
    if (check1 <= 0)
    {
        string eMsg = "error resolving around finding address";
        print_error(eMsg);
        return ERROR;
    }
	memset(&(my_addr.sin_zero), '\0', 8);

	if (bind(sockfd, (struct sockaddr *) &my_addr,
              sizeof(my_addr)) < 0)
	{
		string eMsg = "failure in binding process";
		print_error(eMsg);
        return ERROR;
	}

	listen(sockfd, MAX_CLIENTS);
}


void print_error(string Msg)
{
	cerr << Msg << endl;
}



/* testing function */
int main()
{
    const char* lcl_ip = "132.65.125.3";
    string name = "Cookie";
    const char* port = "4423";
    init_server(name, lcl_ip, port);
    cout << KMAG<< "server is on:)" << endl;
}