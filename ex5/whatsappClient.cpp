#include <string.h>
#include "whatsappClient.h"
#include "common.h"

using namespace std;

string gName;
bool gRunning = true;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno;
    ssize_t n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_SIZE];
    if (argc != 4)
    {
        cout << "Usage: whatsappClient clientName serveraddress serverPort" << endl;
        exit(0);
    }
    gName = (string) argv[1];
    portno = stoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        _printError("socket");
    }
    server = gethostbyname(argv[2]);
    if (server == NULL)
    {
        _printCustomError("didn't find the host!");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr,
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        _printError("connect");
    }
    passingData(sockfd, ("CLIENT " + gName));
    // passingData()
    while (gRunning)
    {
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE - 1, stdin);

        n = write(sockfd, buffer, strlen(buffer));
        //TODO: set condition for breaking from the session.
        if (n == 0)
        {
            continue;
        }
        if (n < 0)
        {
            _printError("write");
        }
        bzero(buffer, BUFFER_SIZE);
        n = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (n < 0)
        {
            _printError("read");
        }

        printf("%s\n", buffer);
    }
    close(sockfd);
    return 0;
}