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
#include <netdb.h>

#include <map>
#include <asm/param.h>


#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define RST  "\x1B[0m"


#define MAX_CLIENTS 10
#define ERROR -1
#define SUCCESS 0

#define ESC_SEQ "EXIT"


using namespace std; 

void print_error(string Msg);

/* data structs for the server side */

// list of the relevant active clients.
list<string> lClients;


// actual relevent socket address.
struct sockaddr_in my_addr;
bool break_flag = false;

int openPort;

int init_server(const char* port)
{
	char host_name[MAXHOSTNAMELEN+1];
	host_name[MAXHOSTNAMELEN] = '\0';
	struct hostent *host = NULL;
	gethostname(host_name, MAXHOSTNAMELEN );
    host = gethostbyname(host_name);


    my_addr.sin_family = AF_INET;
    int sockfd, portN;
	sockfd =  socket(AF_INET, SOCK_STREAM, 0);
	portN = stoi(port);
	my_addr.sin_port = htons(portN);
	memcpy(&my_addr.sin_addr, host->h_addr, host->h_length);
    // legacy code for case of manual address 
	// int check1 = inet_aton(host->h_addr, &(my_addr.sin_addr));
	memset(&(my_addr.sin_zero), '\0', 8);

	if (bind(sockfd, (struct sockaddr *) &my_addr,
              sizeof(my_addr)) < 0)
	{
		string eMsg = "failure in binding process";
		print_error(eMsg);
        return ERROR;
	}

	if(listen(sockfd, MAX_CLIENTS) < 0)
	{
		string eMsg = "failure in listening process";
		print_error(eMsg);
        return ERROR;
	}
	return SUCCESS;
}


void print_error(string Msg)
{
	cerr << Msg << endl;
}



/* testing function */
int main2()
{
    const char* lcl_ip = "132.65.125.3";
    string name = "Cookie";
    const char* port = "4423";
    init_server(port);
    cout << KMAG<< "server is on:)" << endl;
}

int main(int argc, char* argv[])
{
	if( argc != 2 )
	{
        cout << KBLU << "pray use: emServer <portNum>" << endl << RST;
        exit(ERROR);
    }
    openPort = stoi(argv[1]);
    int check1 = init_server(argv[1]);
    if (check1 < 0)
    {
        string eMsg = "failure in opening the server";
        print_error(eMsg);
        return ERROR;
    }
    return SUCCESS;
}


/* fetching input for the server */
void* server_input(void* index)
{
    string command_buf;
    getline(cin, command_buf);
    while (command_buf.compare(ESC_SEQ))
    {
        getline(cin, command_buf);
    } 
    break_flag = true;
    return nullptr;
}