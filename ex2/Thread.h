//
// Authors H.B & S.H
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
    int get_id() const;
    void set_id(int id);

    std::list<int> syncThreads;
    int get_quantum_counter() ;
    void update_quantum_counter();
    char *  getStackAddress() ;
    void setState( int FLAG);
    int getState();

    //overloading possible operators.
    bool operator==(const Thread &other);

    bool syncFlag = false;
    bool blockedFlag = false;



private:
    int id;
    char stack1[STACK_SIZE];
    int state;
    int syncTo;
    int quantumQounter;

};



#endif //EX2_THREAD_H