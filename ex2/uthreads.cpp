using namespace std;
#include <stdio.h>
#include <vector>
#include <list>
#include <queue>
#include "Thread.h"
#include "setjmp.h"
#include "signal.h"
#include <string>
#include <iostream>
#include <stdlib.h>


list<Thread> readyThreads;
// TODO: change to dict, or perhaps not due to order.
list<Thread> blockedThreads;

Thread search_ready_threads(int tid);
Thread search_blocked_threads(int tid);
Thread search_thread(int tid);
void schedulingDes();
void syncThread();


#define SUCCESS 0
#define ERROR -1
#define MIC_TO_SEC 1000000

#define SYS_ERROR "system error: "

/**
 * id of runnig thread
 */
int curThreadId;

/**
 * running thread
 */
Thread currThread(0);

/**
 * number of toal quantums untill now
 */
int totalQuantumRunning = 0;

/**
 * que to the ids
 */
std::priority_queue<int, std::vector<int>, std::greater<int> > idsQ;

// Storing the various stacks, statically.
sigjmp_buf env[MAX_THREAD_NUM];
/**
 * dealing with the timer and the vt alarm
 */
struct itimerval _clock;
struct sigaction _signalSet;
sigset_t _signalsToBlock, _savedAlarmsDuringBlock;


#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
            "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#endif

/**
 * block SIGVTALRM and save in _savedAlarmsDuringBlock
 * the blocked signals
 */
static void block_SIGVT_alarm() {
    sigprocmask(SIG_BLOCK, &_signalsToBlock, NULL);
    sigemptyset(&_savedAlarmsDuringBlock);
    //save in _savedAlarms.. the blocked signals
    sigpending(&_savedAlarmsDuringBlock);
}

/**
 * unblock SIGVTALRM, without ignoring the blocked signals
 */
static void unblock_not_ignoring_blocked_signals() {
    sigprocmask(SIG_UNBLOCK, &_signalsToBlock, NULL);
}


/**
 * A function for retriving the next vriable id.
 * @return - the relevant id.
 */
int get_next_available_id()
{
    if (idsQ.size() > 0)
    {
        int ret = idsQ.top();
        idsQ.pop();
        return ret;
    }
    return -1;
}

/**
 * The handler for timer actions
 * @param sig - the relevant signals
 */
void timer_handler(int sig)
{   
    block_SIGVT_alarm();
    if (sig != SIGVTALRM)
    {
        eMsg = "non-familiar signal";
        errorMsg(eMsg);
        unblock_not_ignoring_blocked_signals();
        return; // should actually break our code.
    }
    // Make sure u use the appropriate one 
    totalQuantumRunning++;

    int jump_val = sigsetjmp(env[curThreadId], 1);
    if (jump_val == 1) // longjump
    {
        return;
    }
    // standrd behaviour
    if (currThread.getState() == RUNNING)
    {
        curThread.setState(READY);
        readyThreads.push_back(currThread);
        syncThread();

    }
    else
    // we use this to force a manual change between threads.
    {
        setitimer(ITIMER_VIRTUAL, &_clock, NULL);
    }
    // Round Robin in action.
    currThread = readyThreads.front();
    readyThreads.pop_front();
    currThreadId = currThread.get_id();
    currThread.setState(RUNNING);
    currThread.update_quantum_counter();
    siglongjmp(env[currThreadId], 0);

    unblock_not_ignoring_blocked_signals();
}

void errorMsg(string msg)
{
    cerr << SYS_ERROR << msg << endl;
}

/*
 * Description: This function initializes the thread library.
 * You may assume that this function is called before any other thread library
 * function, and that it is called exactly once. The input to the function is
 * the length of a quantum in micro-seconds. It is an error to call this
 * function with non-positive quantum_usecs.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_init(int quantum_usecs)
{
    //initialize blockSignals set
    sigemptyset(&_signalsToBlock);
    sigaddset(&_signalsToBlock, SIGVTALRM);


    // Install timer_handler as the signal handler for SIGVTALRM.
    _signalSet.sa_handler = &timer_handler;

    // fixing the timer
    _clock.it_value.tv_sec = (int)(quantum_usecs/MIC_TO_SEC);
    _clock.it_value.tv_usec = quantum_usecs % MIC_TO_SEC ;
    _clock.it_interval.tv_sec = (int)(quantum_usecs/MIC_TO_SEC);
    _clock.it_interval.tv_usec = quantum_usecs % MIC_TO_SEC;

    // inserting all ID's for available ID's que.
    for (int i = 1; i < MAX_THREAD_NUM; ++i)
    {
        idsQ.push(i);
    }

    // set timer to initial conditions
    if (setitimer(ITIMER_VIRTUAL, &_clock, NULL) == ERROR)
    {
        string eMsg = "issue with setting our timer";
        errorMsg(eMsg);
    }
    return SUCCESS;
}

/*
 * Description: This function creates a new thread, whose entry point is the
 * function f with the signature void f(void). The thread is added to the end
 * of the READY threads list. The uthread_spawn function should fail if it
 * would cause the number of concurrent threads to exceed the limit
 * (MAX_THREAD_NUM). Each thread should be allocated with a stack of size
 * STACK_SIZE bytes.
 * Return value: On success, return the ID of the created thread.
 * On failure, return -1.
*/
int uthread_spawn(void (*f)(void))
{
    int newId = get_next_available_id();
    if(newId  < 0)
    {
        return -1;
    }
    Thread newT = Thread(newId);

    block_SIGVT_alarm();
    //save thread
    address_t sp, pc;
    sp = (address_t)newT.getStackAddress() + STACK_SIZE - sizeof(address_t);
    pc = (address_t)f;
    sigsetjmp(env[newId], 1);
    (env[newId]->__jmpbuf)[JB_SP] = translate_address(sp);
    (env[newId]->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&env[newId]->__saved_mask);
    readyThreads.push_back(newT);
    unblock_not_ignoring_blocked_signals();
    return newId;

}


/*
 * Description: This function terminates the thread with ID tid and deletes
 * it from all relevant control structures. All the resources allocated by
 * the library for this thread should be released. If no thread with ID tid
 * exists it is considered as an error. Terminating the main thread
 * (tid == 0) will result in the termination of the entire process using
 * exit(0) [after releasing the assigned library memory].
 * Return value: The function returns 0 if the thread was successfully
 * terminated and -1 otherwise. If a thread terminates itself or the main
 * thread is terminated, the function does not return.
*/
int uthread_terminate(int tid)
{

    // TODO: check condition for self termination, whether we should exit the whole programme.
    if (tid == 0)
    {
        // TODO: Release all lib data structures and memory allocated.
        exit(0);
    }
    block_SIGVT_alarm();
    if (tid == curThreadId)
    {
        currThread.setState(TERMINATED);
        idsQ.push(tid);
        syncThread(currThread);
        // TODO: Should actually exhaust the timer
        unblock_not_ignoring_blocked_signals();
        raise(SIGVTALRM);
        schedulingDes();
    } else

    {
        Thread targetThread = search_blocked_threads(tid);
        if (targetThread.get_id() == ERROR)
        {
            targetThread = search_ready_threads(tid);
            if (targetThread.get_id() == ERROR)
            {
                // Non existing thread
                unblock_not_ignoring_blocked_signals();
                return ERROR;
            } else
            {
                readyThreads.remove(targetThread);
            }
        } else
        {
            blockedThreads.remove(targetThread);
        }
        targetThread.setState(TERMINATED);
        idsQ.push(tid);
        syncThread(targetThread);
        unblock_not_ignoring_blocked_signals();
        return SUCCESS;
    }

}

/**
 * dealing with the synced threads and moving it to front of
 * the ready que.
 * @param targetThread - that is being called and damping the dependency list.
 */
void syncThread(Thread targetThread)
{
    //TODO: check this code is protected from signal interuptions.
    Thread toSync(-1);
    list<int> synctIds = targetThread.syncThreads;
    if(synctIds.size() > 0)
    {
        // TODO: check order of iteration here.
        for( int id : synctIds) 
        {
            toSync = search_blocked_threads(id);
            toSync.syncFlag = false;
            if (!toSync.blockedFlag)
            {
                blockedThreads.remove(toSync);
                toSync.setState(READY);
                readyThreads.push_front(toSync);
            }
        }
        targetThread.syncThreads.clear();
    }
}


/*
 * Description: This function blocks the thread with ID tid. The thread may
 * be resumed later using uthread_resume. If no thread with ID tid exists it
 * is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision
 * should be made. Blocking a thread in BLOCKED state has no
 * effect and is not considered as an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_block(int tid)
{
    block_SIGVT_alarm();
    if (tid == 0)
    {
        eMsg = "attempting to lock the main thread";
        errorMsg(errorMsg)
        return ERROR;
    }


    if (tid == curThreadId)
    {
        currThread.setState(BLOCKED);
        currThread.blockedFlag = true;
        blockedThreads.push_back(currThread);
        syncThread(tid);
        // clear the previous timer
        unblock_not_ignoring_blocked_signals();
        raise(SIGVTALRM);
        return SUCCESS;
    }
    Thread th = search_ready_threads(tid);
    if (th.get_id() == ERROR)
    {
        th = search_blocked_threads(tid);
        if (th.get_id() == ERROR)
        {
            eMsg = "no thread with relevant ID exists";
            errorMsg(eMsg);
            unblock_not_ignoring_blocked_signals();
            return ERROR;
        }   
    }
    else
    {
        readyThreads.remove(th);
        blockedThreads.push_back(th);
    }
    th.setState(BLOCKED);
    th.blockedFlag = true;
    unblock_not_ignoring_blocked_signals();
    return SUCCESS;

}


/*
 * Description: This function resumes a blocked thread with ID tid and moves
 * it to the READY state. Resuming a thread in a RUNNING or READY state
 * has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered as an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_resume(int tid)
{
    block_SIGVT_alarm();
    if ((tid == curThreadId) || (search_ready_threads(tid).get_id() >= 0))) 
    {
        unblock_not_ignoring_blocked_signals();
        return SUCCESS;
    }
    Thread target = search_blocked_threads(tid);
    if (target.get_id < 0)
    {
        eMsg = "thread does not exists"
        errorMsg(eMsg);
        unblock_not_ignoring_blocked_signals();
        return ERROR;
    }
    target.blockedFlag = false;
    if (!target.syncFlag)
    {
        target.setState(READY);
        blockedThreads.remove(target);
        readyThreads.push_back(target);
        unblock_not_ignoring_blocked_signals();
        return SUCCESS;
    }
    unblock_not_ignoring_blocked_signals();
    return SUCCESS;
}


/*
 * Description: This function blocks the RUNNING thread until thread with
 * ID tid will move to RUNNING state (i.e.right after the next time that
 * thread tid will stop running, the calling thread will be resumed
 * automatically). If thread with ID tid will be terminated before RUNNING
 * again, the calling thread should move to READY state right after thread
 * tid is terminated (i.e. it won’t be blocked forever). It is considered
 * as an error if no thread with ID tid exists or if the main thread (tid==0)
 * calls this function. Immediately after the RUNNING thread transitions to
 * the BLOCKED state a scheduling decision should be made.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_sync(int tid)
{
    block_SIGVT_alarm();
    //TODO: check edge cases for main thread syncing.
    if (curThreadId == 0)
    {
        eMsg = "main thread attempting to sync";
        errorMsg(eMsg);
        unblock_not_ignoring_blocked_signals();
        return ERROR;
    }
    Thread targetThread = search_thread(tid);
    int tempId = targetThread.get_id();
    if (tempId == ERROR)
    {
        eMsg = "thread not found issue";
        errorMsg(eMsg);
        unblock_not_ignoring_blocked_signals();
        return ERROR;
    }
    currThread.syncFlag = true;
    targetThread.syncThreads.push_back(curThreadId);
    uthread_block(curThreadId);
    // force the timer issue.
    unblock_not_ignoring_blocked_signals();
    return SUCCESS;
}

Thread search_thread(int tid)
{
    Thread targetThread = search_ready_threads(tid);
    int tempId = targetThread.get_id();
    if (tempId == ERROR)
    {
        targetThread = search_blocked_threads(tid);
        return targetThread;
    }
    return  targetThread;
}


Thread search_ready_threads(int tid)
{
    Thread th(-1);
    for (th : readyThreads)
    {
        if (th.get_id() == tid)
        {
            return th;
        }
    }
    return Thread(-1);
}

Thread search_blocked_threads(int tid)
{
    Thread th(-1);
    for (th : blockedThreads)
    {
        if (th.get_id() == tid)
        {
            return th;
        }
    }
    return Thread(-1);

}


/*
 * Description: This function returns the thread ID of the calling thread.
 * Return value: The ID of the calling thread.
*/
int uthread_get_tid()
{
    return curThreadId;
}


/*
 * Description: This function returns the total number of quantums that were
 * started since the library was initialized, including the current quantum.
 * Right after the call to uthread_init, the value should be 1.
 * Each time a new quantum starts, regardless of the reason, this number
 * should be increased by 1.
 * Return value: The total number of quantums.
*/
int uthread_get_total_quantums()
{
    return totalQuantumRunning;
}


/*
 * Description: This function returns the number of quantums the thread with
 * ID tid was in RUNNING state. On the first time a thread runs, the function
 * should return 1. Every additional quantum that the thread starts should
 * increase this value by 1 (so if the thread with ID tid is in RUNNING state
 * when this function is called, include also the current quantum). If no
 * thread with ID tid exists it is considered as an error.
 * Return value: On success, return the number of quantums of the thread with ID tid. On failure, return -1.
*/
int uthread_get_quantums(int tid)
{
    if( tid == curThreadId)
    {
        return currThread.get_quantum_counter();
    }
    Thread targThread = search_thread(tid);
    if (targThread.get_id() == ERROR)
    {
        return ERROR;
    }
    return targThread.get_quantum_counter();

}