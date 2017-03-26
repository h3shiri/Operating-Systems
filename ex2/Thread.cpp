//
// Created by hadasba on 3/26/17.
//

#include "Thread.h"
#include <stdio.h>

/**
 * constructor
 * @param id
 * @return
 */
Thread::Thread(int id)
{
    set_id(id);
}

// getters and setters
int Thread::get_id()
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

void Thread::set_reminingTime(struct timeval *internalTime)
{
    this->remaingTime = *internalTime;
}

void Thread::update_quantum_counter()
{
    quantumQounter++;
}

void Thread::setState(int FLAG)
{
    if (FLAG != READY && FLAG!=RUNNING && FLAG != WAITING && FLAG!=TERMINATED)
    {
        printf("error: %d", FLAG);
    }
    state = FLAG;
}

int Thread::getState()
{
    return state;
}