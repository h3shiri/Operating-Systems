#include <map>
#include <fcntl.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include "CacheFS.h"
#include "BlockStack.h"
#include <list>
#include <cstring>
#include <cmath>

#define SUCCESS 0
#define ERROR -1
#define LOCATION "/tmp"


using namespace std;

static BlockStack * pointerToStack;
static int cacheHits = 0;
static int cacheMisses = 0;
static int blockSize;

// note it doesn't seclude several copies of the same file.
static int numOfOpenFiles = 0;
static map<int, string> idsToFullNames;
static map<string, std::list<int>> stringToOpenFds;

/**
 * Init func
 * @param blocks_num - the number of blocks in the buffer cache.
 * @param cache_algo -  the cache algorithm that will be used.
 * @param f_old - the percentage of blocks in the old partition (rounding down).
 *                  relevant in FBR algorithm only.
 * @param f_new - the percentage of blocks in the new partition (rounding down)
				   relevant in FBR algorithm only.
 * @return
 */
int CacheFS_init(int blocks_num, cache_algo_t cache_algo,
                 double f_old , double f_new  )
{
    struct stat fi;
    stat("/tmp", &fi);
    blockSize = (int) fi.st_blksize;
    pointerToStack = new BlockStack(blocks_num, cache_algo, f_old, f_new);
    return SUCCESS;
}

/**
 * destroying the current cache and freeing all associated memory.
 * it is th euser responsibility to close the files.
 * @return 0 upon success -1 in case of failure.
 */
int CacheFS_destroy()
{
    for (auto bl : pointerToStack->getstack())
    {
        delete(bl);
    }
    for (auto bl : pointerToStack->getUtilityAgingStack())
    {
        delete(bl);
    }
    delete(pointerToStack);
    cacheHits = 0;
    cacheMisses = 0;
    return SUCCESS;
}


/**
 * printing the cache situation
 * LRU and FBR use the stack order to print out.
 * LFU uses the eveiction order aka frequency from old to young.
 * @param log_path - the relevant file to log into
 * @return 0 upon success and -1 upon failure.
 */
int CacheFS_print_cache (const char *log_path)
{
    int res;
    ofstream ofs;
    ofs.open(log_path, ofstream::out | ofstream::app);
    if (!ofs.is_open())
    {
        return ERROR;
    }
    res = pointerToStack->printLogTofile(ofs);
    ofs.close();
    return res;
}

/**
 * printing stats for the stack, (appending in case of existing).
 * @param log_path - the path for the file to write into.
 * @return 0 in case of success and -1 in case of error.
 */
int CacheFS_print_stat (const char *log_path)
{
    ofstream ofs;
    ofs.open(log_path, ofstream::out | ofstream::app);
    if (!ofs.is_open())
    {
        return ERROR;
    }
    //TODO: check '.' dots and endl in case of last line
    ofs << "Hits number: "<< to_string(cacheHits) << "." << endl;
    ofs << "Misses number: "<< to_string(cacheMisses) << "." << endl;
    return SUCCESS;
}

/**
 * Openning a file requires us to check whether we access a file or block from
 * the cache actually.
 * @param pathname - a relevant pathname to the file, may be relative
 * @return reltive index fd upon successfully openning a file -1 in case of an error.
 */
int CacheFS_open(const char *pathname)
{
    /* in case file isn't open yet or in cache */
    int fd = open(pathname, O_RDONLY | O_DIRECT | O_SYNC);
    if (fd < 0)
    {
        return ERROR;
    }
    numOfOpenFiles++;
    char *full_path = realpath(pathname, NULL);
    string PathId(full_path);
    string::size_type trace = PathId.find(LOCATION);
    if (trace == string::npos)
    {
        return ERROR;
    }
    idsToFullNames[fd] = PathId;
    stringToOpenFds[PathId].push_back(fd);
    return fd;
}


/**
 * closing a file.
 * @param file_id
 * @return 0 upon successful implementation, -1 in case of error.
 */
int CacheFS_close(int file_id)
{

    string fileFullPath = idsToFullNames[file_id];
    stringToOpenFds[fileFullPath].remove(file_id);
    numOfOpenFiles--;
    return SUCCESS;
}


/**
 * This function is essential for managing retrivals from the stuck and the actual disk.
 * reading data from open file similar to POSIX.
 * @param file_id - the relevant file_id.
 * @param buf - copying the data into the buffer.
 * @param count - how many bytes to read.
 * @param offset - the offset in the file.
 * @return number of bytes read upon success -1 in case of failure
 * failure may occur in case of a closed file
 * offset + count > end of file is not an error simply copy the relevant data.
 */
int CacheFS_pread(int file_id, void *buf, size_t count, off_t offset)
{
    /* by the end of the run should be equal to count unless it's an error */
    int totalnumberOfbytesRead = 0;
    char * bufToCopyInto = (char*) (buf);
    //offset to copy into the buffer
    int indexBuffer = 0;
    map<int, string>::iterator it = idsToFullNames.find(file_id);
    if(offset < 0 || (bufToCopyInto == nullptr) || (it == idsToFullNames.end()))
    {
        return ERROR;
    }
    string path = it->second;

    int firstBlockIndex = (int) floor((int)offset/blockSize);
    int lastBlockIndex = (int) floor(((int)offset + count)/blockSize);
    /* how many bits from the last block */
    int sizefromLast = (int) ((int) offset + count - (lastBlockIndex * blockSize));
    int num = firstBlockIndex;
    int readed = 0;
    void * des;
    while(num <= lastBlockIndex)
    {
        char * tempBuf = (char *) aligned_alloc(blockSize,blockSize);
        if(!tempBuf)
        {
            return ERROR;
        }

        Block * block = pointerToStack->readBlockFromStack(path, num);
        // Only one block
        if(lastBlockIndex == firstBlockIndex)
        {
            sizefromLast = sizefromLast - (offset % blockSize);
            /* complete */
        }
        int toCopy = (num == lastBlockIndex) ? sizefromLast : blockSize;
        /* In case we found it in the cache */
        if(block)
        {
            cacheHits++;
            if(num == firstBlockIndex)
            {
                des = memcpy(bufToCopyInto + (indexBuffer * blockSize), block->getAddress() +
                                                      (offset % blockSize), toCopy);
                totalnumberOfbytesRead += toCopy;
                indexBuffer++;
            }
            else{
                des = memcpy(bufToCopyInto + (indexBuffer * blockSize),
                             block->getAddress(), toCopy);
                totalnumberOfbytesRead += toCopy;
                indexBuffer++;
            }
            if(!des)
            {
                return ERROR;
            }
            /* shuffling the stack */
            pointerToStack->shuffleStack(block);
        }
        // fetching it from the the main memory.
        else
        {
            cacheMisses++;
            /* number of bytes read */
            readed = 0;
            int readBytes;
            // TODO : add case of reaching end of file.
            while(readed < blockSize)
            {
                // toCopy - readed
                readBytes = pread(file_id, tempBuf + readed, blockSize,
                                  (num * blockSize) + readed);
                if(readBytes < 0)
                {
                    delete(tempBuf);
                    return ERROR;
                }
                readed +=readBytes;
            }
            Block* newBlock  = new Block(num, path, tempBuf, toCopy);
            pointerToStack->insertNewBloack(newBlock);
            //fixing the offset mistake, not copying the relevant data.
            int dynamicOffset = (int) ((num == firstBlockIndex) ? (offset % blockSize) : 0);
            des = memcpy(bufToCopyInto + (indexBuffer * blockSize), (newBlock->getAddress() + dynamicOffset), toCopy);
            totalnumberOfbytesRead += toCopy;
            indexBuffer++;
            if(!des)
            {
                return ERROR;
            }
        }
        num++;
    }
    return totalnumberOfbytesRead;
}