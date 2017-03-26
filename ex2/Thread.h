//
// Created by hadasba on 3/26/17.
//

#ifndef EX2_THREAD_H
#define EX2_THREAD_H
#include <list>
#include <sys/time.h>
#include "uthreads.h"
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define WAITING 3
#define TERMINATED 4


class Thread
{
public:

    Thread(int id);
    // getters and setters
    int get_id() ;
    void set_id(int id);

    std::list<int> syncThreads;
    int get_quantum_counter() ;
    void update_quantum_counter();
    void set_reminingTime(struct timeval * internalTime);
    char *  getStackAddress() ;
    void setState( int FLAG);
    int getState();
private:
    int id;
    char stack1[STACK_SIZE];
    int state;
    struct timeval remaingTime;
    int quantumQounter;

};



#endif //EX2_THREAD_H
