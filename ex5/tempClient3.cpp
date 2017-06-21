#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <string>
#include <iostream>


#define MSGMAX 1024
#define MSG 256
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define RST  "\x1B[0m"
#define ERROR -1
#define SUCCESS 0

using namespace std;


void print_error(string Msg);
void print_custom_error(string Msg);
void passingData(int socket, string data);

string name;
bool running = true;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[MSG];
    if (argc != 4) {
        string eMsg = "Usage: whatsappClient clientName serveraddress serverPort";
        cout << eMsg << endl;
        exit(0);
    }
    name = (string) argv[1];
    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        print_error("socket");
    server = gethostbyname(argv[2]);
    if (server == NULL) {
        print_custom_error("didn't find the host!");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        print_error("connect");

    // passingData()
    while(running)
        {
        bzero(buffer,MSG);
        fgets(buffer,MSG-1, stdin);
        
        n = write(sockfd, buffer, strlen(buffer));
        //TODO: set condition for breaking from the session.
        if (n == 0)
        {
            continue;
        }
        if (n < 0) 
             print_error("write");
        bzero(buffer,MSG);
        n = read(sockfd,buffer,MSG-1);
        if (n < 0) 
             print_error("read");

        printf("%s\n",buffer);
        }
    close(sockfd);
    return 0;
}


void passingData(int socket, string data)
{
    if (send(socket, data, data.length(), 0) != data.length())
    {
        print_error("send");
    }
}

void print_custom_error(string Msg)
{
    cout << KMAG << Msg << RST << endl;
}

/**
 * standard error in case of function failure in case of the server.
 */
void print_error(string Msg)
{
    cerr << "ERROR: "<< Msg << " "<< errno << "." << endl;
}