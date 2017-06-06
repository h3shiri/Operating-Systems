#include <map>
#include "CacheFS.h"
#include "BlockStack.h"

#define SUCCESS 0
#define ERROR -1

using namespace std;

static BlockStack * pointerToStack;
static int cacheHits = 0;
static int cacheMisses = 0;

static map<int, string> idsToFullNames;

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
    pointerToStack = new BlockStack(blocks_num, cache_algo, f_old, f_new);
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



