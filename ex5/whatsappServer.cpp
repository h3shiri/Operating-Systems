#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h> 
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


#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define RST  "\x1B[0m"


#define MAX_CLIENTS 10
#define ERROR -1
#define SUCCESS 0
#define BIG 1000

#define ESC_SEQ "EXIT"


using namespace std; 

void print_error(string Msg);
void startBinding(int sockfd, int* resFlag);
void startListening(int sockfd, int * resFlag);
void startTraffic();

/* data structs for the server side */

// list of the relevant active clients.
list<string> lClientsByName;
vector<int> clients;
vector<int> del_clients;

// actual relevent socket address.
struct sockaddr_in my_addr;
bool break_flag = false;

/* holds the open port of this server */
int openPort;

/* holds the currently highest socket open */
int max_soc;

/* holds our local socket for this server */
int sockfd;

/* A set for the active fds */
fd_set activeFdsSet;


//TODO: make sure errors comply with the expected format

int init_server(const char* port)
{
	char host_name[MAXHOSTNAMELEN+1];
	host_name[MAXHOSTNAMELEN] = '\0';
	struct hostent *host = NULL;
	gethostname(host_name, MAXHOSTNAMELEN );
    host = gethostbyname(host_name);

    my_addr.sin_family = AF_INET;
	sockfd =  socket(AF_INET, SOCK_STREAM, 0);
	openPort = stoi(port);
	my_addr.sin_port = htons(openPort);
	memcpy(&my_addr.sin_addr, host->h_addr, host->h_length);
    
    // legacy code for case of manual address 
	// int check1 = inet_aton(host->h_addr, &(my_addr.sin_addr));
	memset(&(my_addr.sin_zero), '\0', 8);
	int initFlag = 1;
	startBinding(sockfd, &initFlag);
	startListening(sockfd, &initFlag);
	if (initFlag < 0)
	{
		return ERROR;
	}
	max_soc = sockfd;
	startTraffic();

	return SUCCESS;
}

/**
 * the actuall trafic for the sever accepting incoming connections..etc
 */
void startTraffic()
{
    int fresh_sock;
	int sSize = sizeof(my_addr); 
	int toss, readFromSock;
	char input[BIG+1];
    while (!break_flag)
    {
    	int topSocket = sockfd;
    	FD_ZERO(&activeFdsSet);
    	FD_SET(sockfd, &activeFdsSet);
    	FD_SET(0, &activeFdsSet);
        
    	for (int i = 0; i < (int) clients.size(); ++i)
    	{
    		FD_SET(clients[i], &activeFdsSet);
    		topSocket = (clients[i] > topSocket) ? clients[i] : topSocket;
    	}
        toss = select(topSocket + 1, &activeFdsSet, nullptr, nullptr, nullptr);
        if (toss < 0)
        {
        	string eMsg = "local error with select";
        	print_error(eMsg);
        }
        if (FD_ISSET(sockfd, &activeFdsSet))
        {
        	if ((fresh_sock = accept(sockfd, (struct  sockaddr *) &my_addr, (socklen_t *) &sSize)) < 0)
        	{
        		string eMsg = "ERROR accept failing to open new socket";
        		print_error(eMsg);
        	}
        	// TODO: remove test passes.
        	if (send(fresh_sock, "test", 4, 0) != 4)
        	{
        		print_error("issue in establishing communication");
        	}
        	//TODO: check that the format is fine, server side.
        	cout << "client " << fresh_sock << " connected." << endl;
        }
        /* logging activity from the server shell  */
        if (FD_ISSET(0, &activeFdsSet))
        {
        	string clientInitInput;
        	getline(cin, clientInitInput);
            //TODO: check whether u should echo all the terminal input?
        	cout << clientInitInput << endl;
            if (!clientInitInput.compare(ESC_SEQ))
            {
                break_flag = true;
            }
        }

        /* parsing clients for content */
        for (auto fd : clients)
        {
        	if (FD_ISSET(fd, &activeFdsSet))
        	{
        		if ((readFromSock = read(fd, input, BIG)) == 0)
        		{
        			// client has no input
        			//TODO: check he actually disconnected.
        			close(fd);
        			del_clients.push_back(fd);
        		}
        		else if (readFromSock > 0)
        		{
        			char closer = '\0';
        			input[BIG] = closer;
        			//TODO: remove debug values
        			cout << input << "socket passed" << fd << endl;
        			flush(cout);
        			send(fd, input, strlen(input), 0);
        		} else {
        			print_error("read");
        		}
        	}
        }
        /* clearing the closed clients */
        for (auto fd : del_clients)
        {
            clients.erase(remove(clients.begin(), clients.end(), fd), clients.end());
        }
        del_clients.clear();
    }
}


void startBinding(int sockfd, int* resFlag)
{
	if (bind(sockfd, (struct sockaddr *) &my_addr,
	              sizeof(my_addr)) < 0)
		{
			print_error("bind");
	        *resFlag = ERROR;
		}
}

void startListening(int sockfd, int * resFlag)
{
	if(listen(sockfd, MAX_CLIENTS) < 0)
	{
		print_error("listen");
        *resFlag = ERROR;
	}
}

/**
 * standard error in case of function failure.
 */
void print_error(string Msg)
{
	cerr << "ERROR: "<< Msg << " "<< errno << "." << endl;
}

void print_custom_error(string Msg)
{
	cout << KMAG << Msg << RST << endl;
}



/* testing function */
// int main2()
// {
//     const char* lcl_ip = "132.65.125.3";
//     string name = "Cookie";
//     const char* port = "4423";
//     init_server(port);
//     cout << KMAG<< "server is on:)" << endl;
// }

int main(int argc, char* argv[])
{
	if( argc != 2 )
	{
        cout << KBLU << "pray use: Server <portNum>" << endl << RST;
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


//legacy currently redundant function.
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