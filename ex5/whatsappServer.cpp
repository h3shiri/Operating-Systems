#include <cstring>
#include <sstream>
#include "whatsappServer.h"

using namespace std;

/* data structs for the server side */

// list of the relevant active clients.
list <string> glClientsByName;

vector<int> gClients;
vector<int> gDelClients;

/* the groups online in this server */
map<string, vector<int>> gGroups;

// actual relevant socket address.
struct sockaddr_in gMyAddr;
bool gBreakFlag = false;

string gCommands[] = {"create_group", "send", "who", "exit"};

/* holds the open port of this server */
int gOpenPort;

/* holds the currently highest socket open */
int gMaxSocket;

/* holds our local socket for this server */
int gSockfd;

/* A set for the active fds */
fd_set gActiveFdsSet;


// TODO: make sure errors comply with the expected format

int initServer(const char *port)
{
    const char hostM[10] = "127.0.0.1"; // TODO: remember to remove hard coding address.
    char host_name[MAXHOSTNAMELEN + 1];
    host_name[MAXHOSTNAMELEN] = '\0';
    struct hostent *host = NULL;
    gethostname(host_name, MAXHOSTNAMELEN);
    host = gethostbyname(hostM); // TODO:  mend and test on server.

    gMyAddr.sin_family = AF_INET;
    gSockfd = socket(AF_INET, SOCK_STREAM, 0);
    gOpenPort = stoi(port);
    gMyAddr.sin_port = htons((uint16_t) gOpenPort);
    memcpy(&gMyAddr.sin_addr, host->h_addr, host->h_length);

    // legacy code for case of manual address 
    // int check1 = inet_aton(host->h_addr, &(gMyAddr.sin_addr));
    memset(&(gMyAddr.sin_zero), '\0', 8);
    int initFlag = F_INIT;
    startBinding(gSockfd, &initFlag);
    startListening(gSockfd, &initFlag);
    if (initFlag == F_ERROR)
    {
        return F_ERROR;
    }
    gMaxSocket = gSockfd;
    startTraffic();

    return F_SUCCESS;
}


void startTraffic()
{
    int fresh_sock;
    int sSize = sizeof(gMyAddr);
    int toss;
    ssize_t readFromSock;
    char inputBuffer[MAX_MSG_LEN + 1];

    while (!gBreakFlag)
    {
        int topSocket = gSockfd;
        FD_ZERO(&gActiveFdsSet);
        FD_SET(gSockfd, &gActiveFdsSet);
        FD_SET(0, &gActiveFdsSet);

        for (int i = 0; i < (int) gClients.size(); ++i)
        {
            FD_SET(gClients[i], &gActiveFdsSet);
            topSocket = (gClients[i] > topSocket) ? gClients[i] : topSocket;
        }
        toss = select(topSocket + 1, &gActiveFdsSet, nullptr, nullptr, nullptr);
        if (toss < 0)
        {
            string eMsg = "select";
            _printError(eMsg);
        }
        if (FD_ISSET(gSockfd, &gActiveFdsSet))
        {
            if ((fresh_sock = accept(gSockfd, (struct sockaddr *) &gMyAddr, (socklen_t *) &sSize)) < 0)
            {
                string eMsg = "accept";
                _printError(eMsg);
            }
            // initial pass letting the socket now, session is established.
            if (send(fresh_sock, "ALIVE", 5, 0) != 5)
            {
                _printCustomError("issue in establishing communication");
            }
            bool found = (std::find(gClients.begin(), gClients.end(), fresh_sock) != gClients.end());
            if (!found)
            {
                gClients.push_back(fresh_sock);
            }
            //TODO: check that the format is fine, server side.
            cout << "client " << fresh_sock << " connected." << endl;
        }
        /* logging activity from the server shell  */
        if (FD_ISSET(0, &gActiveFdsSet))
        {
            string clientInitInput;
            getline(cin, clientInitInput);
            //TODO: check whether u should echo all the terminal input?
            cout << clientInitInput << endl;
            if (!clientInitInput.compare(ESC_SEQ))
            {
                gBreakFlag = true;
            }
        }

        /* parsing clients for content */
        for (auto fd : gClients)
        {
            if (FD_ISSET(fd, &gActiveFdsSet))
            {
                // cleaning the buffer prior to reading
                bzero(inputBuffer, MAX_MSG_LEN);
                readFromSock = read(fd, inputBuffer, MAX_MSG_LEN);
                // TODO: remove length debug
                _printCustomDebug(readFromSock);
                //TODO: migrate to input processor.
                if (readFromSock == 0)
                {
                    // client has no input
                    //TODO: check he actually disconnected.
                    close(fd);
                    gDelClients.push_back(fd);
                }
                else if (readFromSock > 0)
                {
                    char closer = '\0';
                    inputBuffer[MAX_MSG_LEN] = closer;
                    //TODO: remove debug values
                    cout << inputBuffer << "socket num:" << fd << endl;
                    flush(cout);
                    send(fd, inputBuffer, strlen(inputBuffer), 0);
                }
                else
                {
                    _printError("read");
                }
            }
        }
        /* clearing the closed clients */
        for (auto fd : gDelClients)
        {
            gClients.erase(remove(gClients.begin(), gClients.end(), fd), gClients.end());
        }
        gDelClients.clear();
    }
}


void processRequest(string rawCommand, int clientSocket)
{
    vector<string> commands;
    istringstream iss(rawCommand);
    vector<string> tokens{istream_iterator<string>{iss},
                          istream_iterator<string>{}};

    string command = tokens[0];

    if (command == "create_group")
    {
        string groupName = tokens[1];
        string rawListOfUsers = tokens[2];
        // Assume clients adds this arg.
        string clientName = tokens[3];
        createGroupRoutine(groupName, rawListOfUsers, clientSocket);
    }
    else if (command == "who")
    {
        // Assuming client feeds his name to request.
        string clientName = tokens[1];
        whoRoutine(clientName, clientSocket);
    }
    else if (command == "send")
    {
        string targetName = tokens[1];
        string message = tokens[2];
        string clientName = tokens[3];
        sendRoutine(targetName, message, clientName, clientSocket);
    }
    else if (command == "exit")
    {
        string clientName = tokens[1];
        exitRoutine(clientName, clientSocket);
    }
        // invalid command
    else
    {
        _printCustomError("invalid command from the user");
    }
}

void createGroupRoutine(string groupName, string rawListOfUsers,
                        int clientSocketId)
{
    // check group name isn't used
    if (!gGroups.count(groupName))
    {

    }
    else
    {
        string sMsgError = "ERROR:failed to create group ";
        sMsgError += groupName + "\n";
    }
}

void whoRoutine(string clientName, int clientSocketId)
{
    /* code */
}

void sendRoutine(string targetName, string message,
                 string clientName, int clientSocketId)
{
    /* code */
}

void exitRoutine(string clientName, int clinetSocketId)
{
    /* code */
}


void passingData(int socket, string data)
{
    if (send(socket, data.c_str(), data.length(), 0) != data.length())
    {
        _printError("send");
    }
}


void startBinding(int sockfd, int *resFlag)
{
    if (bind(sockfd, (struct sockaddr *) &gMyAddr, sizeof(gMyAddr)) < 0)
    {
        _printError("bind");
        *resFlag = F_ERROR;
    }
}

void startListening(int sockfd, int *resFlag)
{
    if (listen(sockfd, MAX_CLIENTS) < 0)
    {
        _printError("listen");
        *resFlag = F_ERROR;
    }
}


/**
 * standard error in case of function failure in case of the server.
 */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        _printCustomDebug("pray use: Server <portNum>");
        exit(F_ERROR);
    }

    gOpenPort = stoi(argv[1]);
    int check1 = initServer(argv[1]);
    if (check1 == F_ERROR)
    {
        string eMsg = "failure in opening the server";
        _printError(eMsg);
        return F_ERROR;
    }
    return F_SUCCESS;
}