//
// Created by h3shiri on 6/6/17.
//
#include "CacheFS.h"
#include "Block.h"
#include <vector>


static vector<Block *> blockStack = vector<Block *>();
int MaxNumOfBlocks;
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
    MaxNumOfBlocks = blocks_num;

}