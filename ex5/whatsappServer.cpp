#include <stdio.h>
#include <stdlib.h>
#include <string>
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

#include <sstream>
#include <iterator>


#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define RST  "\x1B[0m"


#define MAX_CLIENTS 10
#define ERROR -1
#define SUCCESS 0
#define MSGMAX 1024

#define ESC_SEQ "EXIT"


using namespace std; 

void print_error(string Msg);
void print_custom_error(string Msg);
void startBinding(int sockfd, int* resFlag);
void startListening(int sockfd, int * resFlag);
void startTraffic();
void passingData(int socket, string data);


/* data structs for the server side */

// list of the relevant active clients.
list<string> lClientsByName;

vector<int> clients;
vector<int> del_clients;

/* the groups online in this server */
map<string, vector<int>> groups;

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
    //TODO: remeber to remove hardcoding address.
    const char hostM[10] = "127.0.0.1"; 
	char host_name[MAXHOSTNAMELEN+1];
	host_name[MAXHOSTNAMELEN] = '\0';
	struct hostent *host = NULL;
	gethostname(host_name, MAXHOSTNAMELEN );
    //TODO:  mend and test on server.
    host = gethostbyname(hostM);

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
	char inputBuffer[MSGMAX+1];
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
        	string eMsg = "select";
        	print_error(eMsg);
        }
        if (FD_ISSET(sockfd, &activeFdsSet))
        {
        	if ((fresh_sock = accept(sockfd, (struct  sockaddr *) &my_addr, (socklen_t *) &sSize)) < 0)
        	{
        		string eMsg = "accept";
        		print_error(eMsg);
        	}
        	// initial pass letting the coket now, session is established.
        	if (send(fresh_sock, "ALIVE", 5, 0) != 5)
        	{
        		print_custom_error("issue in establishing communication");
        	}
            bool found = (std::find(clients.begin(), clients.end(), fresh_sock) != clients.end());
            if (!found)
            {
                clients.push_back(fresh_sock);
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
                // cleaning the buffer prior to reading
                bzero(inputBuffer,MSGMAX);
                readFromSock = read(fd, inputBuffer, MSGMAX);
                // TODO: remove length debug
                cout << KBLU << readFromSock << RST << endl;
        		//TODO: migrate to input processor.
                if (readFromSock == 0)
        		{
        			// client has no input
        			//TODO: check he actually disconnected.
        			close(fd);
        			del_clients.push_back(fd);
        		}
        		else if (readFromSock > 0)
        		{
        			char closer = '\0';
        			inputBuffer[MSGMAX] = closer;
        			//TODO: remove debug values
        			cout << inputBuffer << "socket num:" << fd << endl;
        			flush(cout);
        			send(fd, inputBuffer, strlen(inputBuffer), 0);
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

/**
 * the driver function for the various commands processing the clients
 * requests. 
 * assuming valid input from the client.
 */
void processRequest(string rawCommand)
{
    istringstream iss(rawCommand);
    vector<string> tokens{istream_iterator<string>{iss}, 
                            istream_iterator<string>{}};

    string command = tokens[0];

    if (/* condition */)
    {
        /* code */
    }
}


void passingData(int socket, string data)
{
    if (send(socket, data.c_str(), data.length(), 0) != data.length())
    {
        print_error("send");
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
 * standard error in case of function failure in case of the server.
 */
void print_error(string Msg)
{
	cerr << "ERROR: "<< Msg << " "<< errno << "." << endl;
}

void print_custom_error(string Msg)
{
	cout << KMAG << Msg << RST << endl;
}


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