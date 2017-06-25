#include <string.h>
#include "whatsappClient.h"
#include "common.h"
#include <regex>
#include <boost/algorithm/string.hpp>


using namespace std;

string gName;
bool gRunning = true;

/* holds our local socket for this client */
// shall be updated for the seever socket and then stay stable.
int gClientSockfd;

int serverSocketFd ;
int terminalSocketFd = 0;
/* A set for the active fds */
fd_set gClientActiveFdsSet;

vector<int> gSockets;


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
        exit(1);
    }
    server = gethostbyname(argv[2]);
    if (server == NULL)
    {
        _printCustomError("gethostbyname");
        exit(1);
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
        exit(1);
    }
    serverSocketFd = sockfd;
    gSockets.push_back(sockfd);
    passingData(sockfd, ("CLIENT " + gName));
    //server tells if succes
    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE - 1);
    if (n < 0)
    {
        _printError("read");
    }
    else
    {
        //TO DO: Add name checking!!
        buffer[strlen(buffer)] = 0;
        if(!string(buffer).compare(string("ALIVE")))
        {
            cout<<"Connected Successfully.\n";
        }
        else
        {
            cout<< buffer <<endl;
            cerr<<"Failed to connect the server"<< endl;
            exit(1);
        }
    }
    //TODO:register user, get message from sever.
//    int valid = ERROR;
    while (gRunning) 
    {
        int topSocket = gClientSockfd;
        FD_ZERO(&gClientActiveFdsSet);
        FD_SET(gClientSockfd, &gClientActiveFdsSet);
        FD_SET(0, &gClientActiveFdsSet);

        for (int i = 0; i < (int) gSockets.size(); ++i)
        {
            FD_SET(gSockets[i], &gClientActiveFdsSet);
            topSocket = (gSockets[i] > topSocket) ? gSockets[i] : topSocket;
        }
        int toss = select(topSocket + 1, &gClientActiveFdsSet, nullptr, nullptr, nullptr);
        if (toss < 0)
        {
            string eMsg = "select";
            _printError(eMsg);
        }


        /* message from server */
        if (FD_ISSET(serverSocketFd, &gClientActiveFdsSet))
        {
            readingFromServer(buffer, sockfd);
        }

        /* client inputs from terminal */
        // check u actually get here.
        if (FD_ISSET(0, &gClientActiveFdsSet))
        {
            readingFromTerminal(buffer, sockfd);
        }
    }
    close(sockfd);
    return 0;
}

/**
 * treating input from server.
 * @param buffer - the relevant buffer we work with.
 * @param sockfd - the sockfd we currently read from aka the server.
 */
void readingFromServer(char * buffer, int sockfd)
{
    //input from server
    bzero(buffer, BUFFER_SIZE);
//    _printCustomError("reading from server" + to_string(sockfd));
    int n = (int) read(sockfd, buffer, BUFFER_SIZE - 1);
//    _printCustomError(("finished reading from server"));
    //server shut down functions,
    if(n == 0)
    {
        exit(1);
    }
    if (n < 0)
    {
        _printError("read");
    }
    //TODO: process data after request.
    //printf("%s\n", buffer);
    if(n > 0)
    {
        buffer[strlen(buffer)] = 0;
        serverMsgTreat(string(buffer));
    }
}

/**
 * reading from terminal user input.
 * @param buffer - the relevant buffer for handeling the input.
 * @param sockfd - the relevant socket file descriptor.
 */
void readingFromTerminal(char * buffer, int sockfd)
{
    bzero(buffer, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE - 1, stdin);
    buffer[strcspn(buffer,"\n")] = 0;
    // checks validity, plus adds name.
    int valid = parssingInput(buffer);
    if (!valid)
    {
        // prior to sending data to the server.
        int n = (int) write(sockfd, buffer, strlen(buffer));
        //TODO: set condition for breaking from the session.
        if (n == 0)
        {
            gRunning = false;
        }
        if (n < 0) {
            _printError("write");
        }
    }
}

/*Treat msg from server that is a response to the client request -
 * for example the list of clients in case of "who" request*/
void treatResponse(string msg)
{
    regex error_reg("ERROR");
    if(!msg.substr(0,1).compare(EXIT0_SIGN))
    {
        cout << msg.substr(1,msg.length()-1) << endl;
        exit(0);
    }
    if(regex_search(msg, error_reg, regex_constants::match_continuous))
    {
        cerr<< msg << endl;
    } else{
        cout<< msg << endl;
    }

//    regex groupFail("ERROR: failed to create group [0-9a-zA-Z]+\\.");
//    regex groupSucces("Group [0-9a-zA-Z]+ was created successfully.");
//    if(regex_match(msg,groupFail))
//    {
//        cerr<< msg << endl;
//        return;
//    }
//    if(regex_match(msg,groupSucces))
//    {
//       cout <<msg << endl;
//        return;
//    }
}


/*treat any msg that was read from the server*/
void serverMsgTreat(string msg)
{
    if(msg.length() ==0)
    {
        cout << "empty message reads, someting wrong here" <<endl;
        return;
    }

    if(!msg.substr(0,1).compare(RESPONSE_SING))
    {
        treatResponse(msg.substr(1,msg.length() -1));
    } else{
        cout << msg <<endl;
    }
}

/* check if there are at list 2 members including the client itself*/
bool membersCheck(string groupMmbers)
{
    string delim = ",";
    size_t pos = 0;
    string token;
    if(groupMmbers.find(delim) == string::npos)
    {
        if(groupMmbers.compare(gName))
       {
           return true;
       }
        return false;
    }
    while ((pos = groupMmbers.find(delim)) != string::npos) {
        token = groupMmbers.substr(0, pos);
        if(token.compare(gName))
        {
            return true;
        }
        groupMmbers.erase(0, pos + delim.length());
    }
    return false;
}



int parssingInput(char* buffer)
{
    std::vector<string> tokens;
    boost::split(tokens, buffer, boost::is_any_of("\t "));
    bool match = false;
    string toSend(buffer);
    regex create("()*create_group( )*");
    regex send("()*send ");
    regex create_group_reg("create_group [a-zA-Z0-9]+ ([a-zA-Z0-9]+,)*[a-zA-Z0-9]+");
    regex who_reg("( )*who( )*");
    regex send_reg("send [a-zA-Z0-9]+ .+");
    regex exit_reg("( )*exit( )*");
    if(regex_search(buffer, create, regex_constants::match_continuous))
    {
        if(regex_match(buffer,create_group_reg))
        {
            if(membersCheck(tokens[2]))
            {
                match = true;
            } else
            {
                cerr<<"ERROR: failed to create group "+ tokens[1] +".\n";
                return ERROR;

            }
        }else{
            if(tokens.size() > 1)
            {
                cerr<<"ERROR: failed to create group "+ tokens[1] +".\n";
            }
            else
            {
                cerr<<"ERROR: failed to create group .\n"; }
            return ERROR;
        }
    }
    else {
        if (regex_match(buffer, who_reg) || regex_match(buffer, exit_reg)) {
            match = true;
        } else {
            if (regex_search(buffer, send, regex_constants::match_continuous)) {
                // bool s = tokens[1].compare(gName);
                if (regex_match(buffer, send_reg) && (tokens[1].compare(gName))) {
                    match = true;
                } else {
                    cerr << "ERROR: failed to send.\n";
                    return  ERROR;
                }
            }
        }
    }
    if(!match)
    {
        cerr << "ERROR: Invalid input.\n";
        return ERROR;
    }else{
        toSend = toSend + " " + gName;
        bzero(buffer, BUFFER_SIZE);
        const char * str = toSend.c_str();
        bcopy(str,buffer,strlen(str));
        return 0;
    }

}
