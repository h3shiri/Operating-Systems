//
// Authors: H.b & S.H
//

#include "Thread.h"
#include <stdio.h>
#include <iostream>

/**
 * constructor
 * @param id
 * @return
 */
Thread::Thread(int id)
{
    set_id(id);
    this->state = READY;
    syncThreads.clear();
}

// getters and setters
int Thread::get_id() const
{
    return id;
}

int Thread::get_quantum_counter()
{
    return quantumQounter;
}

char* Thread::getStackAddress()
{
    return stack1;
}

void Thread::set_id(int id)
{
    this->id = id;
}

void Thread::update_quantum_counter()
{
    quantumQounter++;
}

void Thread::setState(int FLAG)
{
    if (FLAG != READY && FLAG!=RUNNING && FLAG != WAITING && FLAG!=TERMINATED && FLAG != BLOCKED)
    {
        std::cerr << "error: %d" << FLAG << std::endl;

    }
    state = FLAG;
}

int Thread::getState()
{
    return state;
}

// overloading comparison op.
bool Thread::operator==(const Thread &other)
{
    return (id == other.get_id());
}

/* dummy main func
int main(){
    return 0;
}
*/