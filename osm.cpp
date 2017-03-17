//
// Created by Shiri on 17/03/2017.
//


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
    return (double) (totalTime / iterations) * 1000;
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
    return (double) (totalTime / iterations) * 1000;
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
    return (double) (totalTime / iterations) * 1000;
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
    return (double) (totalTime / iterations) * 1000;
}

