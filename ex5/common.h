#ifndef EX5_COMMON_H
#define EX5_COMMON_H

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_CLIENTS 10
#define MAX_MSG_LEN 1024

#define F_INIT 1
#define F_ERROR -1
#define F_SUCCESS 0

#define ESC_SEQ "EXIT"

using namespace std;

void _printError(std::string msg)
{
    std::cerr << "ERROR: " << msg << " " << errno << "." << std::endl;
}

void passingData(int socket, std::string data)
{
    if (send(socket, data.c_str(), data.length(), 0) != (ssize_t) data.length())
    {
        _printError("send");
    }
}


#define DEBUG
#ifdef DEBUG
#define C_BLUE  "\x1B[34m"
#define C_RED   "\x1B[35m"
#define C_RESET "\x1B[0m"

void _printCustomError(std::string msg)
{
    std::cout << C_RED << msg << C_RESET << std::endl;
}

void _printCustomDebug(std::string msg)
{
    std::cout << C_BLUE << msg << C_RESET << std::endl;
}

void _printCustomDebug(ssize_t msg)
{
    std::cout << C_BLUE << msg << C_RESET << std::endl;
}

#else
void _printCustomError(string msg) { }
void _printCustomDebug(string msg) { }
void _printCustomDebug(ssize_t msg) { }
#endif  //DEBUG


#endif //EX5_COMMON_H
