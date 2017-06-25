#include <cstring>
#include "whatsappServer.h"
#include "common.h"
#include <algorithm>


using namespace std;

/* data structs for the server side */

// list of the relevant active clients.

vector<int> gClients;
vector<string> gClientNames;
vector<int> gDelClients;


/* the groups online in this server */
map<string, vector<string>> gGroups;

/* clieantName :groups*/
map<string, vector<string>> clientNameToGroups;

map<string, int> nameToSocket;

/*<socketId, clientName*/
map<int,string> idToClient;


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
        //TODO: check does enter more then once per user.
        // registering new users mostly
        if (FD_ISSET(gSockfd, &gActiveFdsSet))
        {
            if ((fresh_sock = accept(gSockfd, (struct sockaddr *) &gMyAddr, (socklen_t *) &sSize)) < 0)
            {
                string eMsg = "accept";
                _printError(eMsg);
            }
            // initial pass letting the socket now, session is established.
            //TO DO add check of duplicate name!
            if (send(fresh_sock, "ALIVE", 5, 0) != 5)
            {
                _printCustomError("issue in establishing communication");
            }
            bool found = (std::find(gClients.begin(), gClients.end(), fresh_sock) != gClients.end());
            if (!found)
            {
                // registering a new user
                bzero(inputBuffer, MAX_MSG_LEN);
                readFromSock = read(fresh_sock, inputBuffer, MAX_MSG_LEN);
                registerUser(inputBuffer, fresh_sock);

            }
            //TODO: check that the format is fine, server side.

//            _printCustomDebug(("client " + idToClient[fresh_sock] + " connected."));
            string connectMsg = idToClient[fresh_sock] + " connected.";
            cout << connectMsg << endl;
        }
        /* logging activity from the server shell  */
        if (FD_ISSET(0, &gActiveFdsSet))
        {
            string terminalInput;
            getline(cin, terminalInput);
            
            //TODO: check whether u should echo all the terminal input?
//            cout << terminalInput << endl;
            if (!terminalInput.compare(ESC_SEQ))
            {
                gBreakFlag = true;
            }
            _debugMaster(terminalInput);
        }

        /* parsing clients for content */
        for (auto fd : gClients)
        {
            if (FD_ISSET(fd, &gActiveFdsSet))
            {
                // cleaning the buffer prior to reading
                bzero(inputBuffer, MAX_MSG_LEN);
                readFromSock = read(fd, inputBuffer, MAX_MSG_LEN);
//                _printCustomDebug(readFromSock);
                //TODO: migrate input to processor.
                if (readFromSock == 0)
                {
                    // client has no input
                    //TODO: check he actually disconnected.
                    close(fd);
                    unRegister(idToClient[fd]);
                    gDelClients.push_back(fd);
                }
                if (readFromSock > 0)
                {
                    char closer = '\0';
                    inputBuffer[MAX_MSG_LEN] = closer;
                    //TODO: remove debug values
//                    _printCustomDebug((string(inputBuffer) + "socket num: " + to_string(fd) + "\n"));

                    // casting the buffer perhaps to string ?
                    processRequest(inputBuffer, fd);
                    //send(fd, inputBuffer, strlen(inputBuffer), 0);
                }
                else
                {
                    _printError("read");
                }
            }
        }
        for(auto fd:gDelClients)
        {
            gClients.erase(remove(gClients.begin(),gClients.end(),fd),gClients.end());
        }
        gDelClients.clear();
    }
    shutDown();
}


void shutDown()
{
    //TO DO end only current task
    cout<<"EXIT command is typed: server is shutting down" << endl;
    exit(0);
}

// buffer contains the init data.
void registerUser(char buffer[], int sockId)
{
    vector<string> commands;
    istringstream iss(buffer);
    vector<string> tokens{istream_iterator<string>{iss},
                          istream_iterator<string>{}};
    string check = tokens[0];
    string name = tokens[1];
    if (check == "CLIENT")
    {
        nameToSocket[name] = sockId;
        gClients.push_back(sockId);
        gClientNames.push_back(name);
        idToClient[sockId] = name;
    }
    // In case of malformed content from the user
    else {
        _printCustomError("User sent an illegal");
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
        _printCustomDebug((groupName + "," + rawListOfUsers + "," + clientName));
        createGroupRoutine(groupName, rawListOfUsers, clientSocket, clientName);
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
                        int clientSocketId, string clientName)
{
    // check group name isn't used
    bool success = false;
    bool checkName = true;
    if((std::find(gClientNames.begin(), gClientNames.end(), groupName)
     != gClientNames.end()) || gGroups.count(groupName))
    {
        checkName = false;
    }
    if (checkName)
    {
        vector<string> names;
        string delim = ",";
        parseStringWithDelim(rawListOfUsers, delim, &names);
        names.push_back(clientName);
        //Temove possible duplicates from names, plus user name.
        sort(names.begin(), names.end());
        names.erase(std::unique(names.begin(), names.end()), names.end());
        //check that clients in group are registered
        int registers = 0;
        int i = 0;
        for ( auto  name : names)
        {
            if(registers != i)
            {
                break;
            }
            for(auto client : gClientNames)
            {
                if(!client.compare(name))
                {
                    registers++;
                    break;
                }
            }
            i++;
        }
        if(registers == (int) names.size())
        {
            success = true;
        }
        if(success)
        {
            gGroups[groupName] = names;
            for ( auto  name : names)
            {
                auto it = clientNameToGroups.find(clientName);
                if(it != clientNameToGroups.end())
                {
                    (it->second).push_back(groupName);
                } else{
                    clientNameToGroups[name] = vector<string>(1,groupName);
                }
            }
            string clientMsgSuccess = (RESPONSE_SING + "Group " + groupName +
                                       " was created successfully.");
            passingData(clientSocketId, clientMsgSuccess);
            string serverMsgSuccess = (clientName + ": Group " + groupName +
                                       " was created successfully.");
            cout << serverMsgSuccess << endl;
        }

    }
    if(!success)
    {
        string clientMsgError = RESPONSE_SING + "ERROR: failed to create group ";
        clientMsgError += (groupName + ".");
        passingData(clientSocketId, clientMsgError);
        string serverErrorMsg = (clientName +
                                 " : ERROR: failed to create group " + groupName);
        cerr << serverErrorMsg << endl;
    }
}

void whoRoutine(string clientName, int clientSocketId)
{
    sort(gClientNames.begin(),gClientNames.end());
    cout<< clientName <<": Requests the currently connected client names."<<endl;
    string clientsList = RESPONSE_SING;
    // to debudg
    if(gClientNames.size() == 0 )
    {
        cerr << "empty clients list, something wrong" <<endl;
    }
    for( auto str : gClientNames)
    {
        clientsList += str;
        clientsList += ",";
    }
    clientsList[clientsList.length() -1]  = '.';
    passingData(clientSocketId, clientsList);

}

void sendRoutine(string targetName, string message,
                 string clientName, int clientSocketId)
{
    cout <<"send routine" <<endl;
    bool found  = false;
    string toSend = clientName + ": " +message;
    for(auto group :gGroups)
    {
        if(!group.first.compare(targetName))
        {
            vector<string> cGroups = clientNameToGroups[clientName];
            for(auto g:cGroups)
            {
                if(!g.compare(targetName))
                {
                    found = true;
                }
            }
            if(found) {
                for (auto member : group.second) {

                    if (member.compare(clientName)) {
                        int socketId = nameToSocket[member];
                        passingData(socketId, toSend);
                    }
                }
            }
            break;
        }
    }
    if(!found)
    {
        for(auto name : gClientNames)
        {
            if(!name.compare(targetName))
            {
                found = true;
                int socketId = nameToSocket[name];
                passingData(socketId,toSend);
                break;
            }
        }
    }
    string clientMsg = RESPONSE_SING;
    unsigned char a = '"';
    if(found)
    {
        clientMsg+=string("Send successfully.");
        cout << clientName << ": " << a << message << a << " was send "
                "successfully to " << targetName << "." <<endl;
    } else
    {
        string msg = string("ERROR: failed to send.");
        clientMsg += msg;
        cerr << clientName << ": " << msg << a <<message << a <<" to " <<targetName
             <<"." <<endl;
    }
    passingData(clientSocketId, clientMsg);


}

void exitRoutine(string clientName, int clientSocketId) {
    unRegister(clientName);
    gDelClients.push_back(clientSocketId);
    string msg = string("Unregistered successfully.");
    string clientMsg = RESPONSE_SING + EXIT0_SIGN + msg;
    passingData(clientSocketId, clientMsg);
    close(clientSocketId);
    cout << clientName << ": " << msg << endl;
}



void unRegister(string clientName)
{
    auto it = gClientNames.begin();
    while((*it).compare(clientName))
    {
        ++it;
    }
    gClientNames.erase(it);
    int fd = nameToSocket[clientName];
    auto it2 = idToClient.find(fd);
    idToClient.erase(it2);
    auto it3 = nameToSocket.find(clientName);
    nameToSocket.erase(it3);
    vector<string>& clientGroups = clientNameToGroups[clientName];
    for( auto group: clientGroups)
    {
        auto members = gGroups[group];
        auto iter = members.begin();
        int i = 0;
        while((*iter).compare(clientName))
        {
            i++;
            iter++;
        }
        vector<string> fresh = gGroups[group];
        fresh.erase(fresh.begin() +i);
        gGroups[group] = fresh;
    }
    _debugGroupsPrint();
    _debugUserPrint();

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


void _debugMaster(string terminalInput)
{
    if (!terminalInput.compare(GROUPPRINT))
    {
        _debugGroupsPrint();
    }
    else if (!terminalInput.compare(USERPRINT))
    {
        _debugUserPrint();
    }
}

void _debugUserPrint()
{
    for (auto const &G : nameToSocket)
    {
        string temp = "";
        temp += (G.first + " : " + to_string(G.second));
        _printCustomDebug(temp);
    }
}

/** use to print all the current groups in memory */
void _debugGroupsPrint()
{
    for (auto const &G : gGroups)
    {
        string temp = "";
        temp += (G.first + " : ");
        for(auto const &M : G.second)
        {
            temp += (M + ",");
        }
        _printCustomDebug(temp);
    }
}
//TODO: test this function for proper parsing
void parseStringWithDelim(string raw, string delim, vector<string>* res)
{
    size_t pos = 0;
    string token;
    while ((pos = raw.find(delim)) != string::npos) {
        token = raw.substr(0, pos);
        res->insert(res->begin(), token);
        raw.erase(0, pos + delim.length());
    }
    if(raw.length() !=0)
    {
        res->insert(res->begin(),raw);
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