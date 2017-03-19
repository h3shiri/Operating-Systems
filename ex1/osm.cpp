//
// Created by Shiri on 17/03/2017.
//

#include <iostream>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <tgmath.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "osm.h"

/* Time measurement function for a simple arithmetic operation.
   returns time in nano-seconds upon success,
   and -1 upon failure.
   */
double osm_operation_time(unsigned int iterations)
{
    iterations += iterations % 5;
    int j;
    timeval sT, eT, delta;
    double totalTime = 0;
    for (unsigned int i = 0; i < iterations; i += 5)
    {
        if (gettimeofday(&sT, NULL) == -1)
        {
            return -1;
        }
        j = 1 + 1;
        j = 1 + 1;
        j = 1 + 1;
        j = 1 + 1;
        j = 1 + 1;
        if (gettimeofday(&eT, NULL) == -1)
        {
            return -1;
        }
        timersub(&sT, &eT, &delta);
        totalTime += delta.tv_usec + delta.tv_sec * pow(10, 6); // time in usec
    }
    // std::cout << delta.tv_usec << std::endl;
    return (double) (totalTime / iterations) *-1000;
}

void dummyFoo()
{
}

/* Time measurement function for an empty function call.
   returns time in nano-seconds upon success,
   and -1 upon failure.
   */
double osm_function_time(unsigned int iterations)
{
    iterations += iterations % 5;
    timeval sT, eT, delta;
    double totalTime = 0;
    for (unsigned int i = 0; i < iterations; i += 5)
    {
        if (gettimeofday(&sT, NULL) == -1)
        {
            return -1;
        }
        dummyFoo();
        dummyFoo();
        dummyFoo();
        dummyFoo();
        dummyFoo();
        if (gettimeofday(&eT, NULL) == -1)
        {
            return -1;
        }
        timersub(&sT, &eT, &delta);
        totalTime += delta.tv_usec + delta.tv_sec * pow(10, 6); // time in usec
    }
    return (double) (totalTime / iterations) * -1000;
}


/* Time measurement function for an empty trap into the operating system.
   returns time in nano-seconds upon success,
   and -1 upon failure.
   */
double osm_syscall_time(unsigned int iterations)
{
    iterations += iterations % 5;
    timeval sT, eT, delta;
    double totalTime = 0;
    for (unsigned int i = 0; i < iterations; i += 5)
    {
        if (gettimeofday(&sT, NULL) == -1)
        {
            return -1;
        }
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        if (gettimeofday(&eT, NULL) == -1)
        {
            return -1;
        }
        timersub(&sT, &eT, &delta);
        totalTime += delta.tv_usec + delta.tv_sec * pow(10, 6); // time in usec
    }
    return (double) (totalTime / iterations) * -1000;
}

size_t currBlockSize(){
    struct stat st;
    stat("/tmp",&st);
    return st.st_blksize;
}
/* Time measurement function for accessing the disk.
   returns time in nano-seconds upon success,
   and -1 upon failure.
   */
double osm_disk_time(unsigned int iterations)
{
    iterations += iterations % 5;
    int f;
    timeval sT, eT, delta;
    double totalTime = 0;
    size_t blksize = currBlockSize();
    mode_t m = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXG;
    char *buff = (char*)aligned_alloc(blksize, sizeof(char)*blksize);
    f = open("WhatIDo", O_SYNC | O_DIRECT | O_CREAT, m);
    for (unsigned int i = 0; i < iterations; i += 5)
    {
        if (gettimeofday(&sT, NULL) == -1)
        {
            return -1;
        }
        pread(f,buff,blksize,0);
        pread(f,buff,blksize,0);
        pread(f,buff,blksize,0);
        pread(f,buff,blksize,0);
        pread(f,buff,blksize,0);
        if (gettimeofday(&eT, NULL) == -1)
        {
            return -1;
        }
        timersub(&sT, &eT, &delta);
        totalTime += delta.tv_usec + delta.tv_sec * pow(10, 6); // time in usec
    }
    return (double) (totalTime / iterations) * -1000;
}

timeMeasurmentStructure measureTimes (unsigned int operation_iterations,
                                      unsigned int function_iterations,
                                      unsigned int syscall_iterations,
                                      unsigned int disk_iterations)
{
    timeMeasurmentStructure * timeMeasures = new timeMeasurmentStructure;
    int NAMEBUFF = 300;
    timeMeasures->machineName = new char[NAMEBUFF];
    gethostname(timeMeasures->machineName, sizeof(char)*NAMEBUFF);
    timeMeasures->instructionTimeNanoSecond = osm_operation_time(operation_iterations);
    timeMeasures->functionTimeNanoSecond = osm_function_time(function_iterations);
    timeMeasures->trapTimeNanoSecond = osm_syscall_time(syscall_iterations);
    timeMeasures->diskTimeNanoSecond = osm_disk_time(disk_iterations);
    if (timeMeasures->instructionTimeNanoSecond != 0)
    {
        //Function/instruction ratio - the respective times divided.
        timeMeasures->functionInstructionRatio = timeMeasures->functionTimeNanoSecond/timeMeasures->instructionTimeNanoSecond; 
        //Trap/instruction ratio - the respective times divided.
        timeMeasures->trapInstructionRatio = timeMeasures->trapTimeNanoSecond/timeMeasures->instructionTimeNanoSecond;
        // Disk/instruction ratio - the respective times divided.
        timeMeasures->diskInstructionRatio = timeMeasures->diskTimeNanoSecond/timeMeasures->instructionTimeNanoSecond;
    }
    else
    {
        // An error due to the instruction time being zero.
        int ERROR = -1;
        timeMeasures->functionInstructionRatio = ERROR;
        timeMeasures->trapInstructionRatio = ERROR;
        timeMeasures->diskInstructionRatio = ERROR;
    }
    return *timeMeasures;
}
