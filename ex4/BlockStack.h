
#ifndef EX4_BLOCKSTACK_H
#define EX4_BLOCKSTACK_H
#include "Block.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>


class BlockStack {
public:
    BlockStack(int totalNumOfBlocks, int policy, double f_old , double f_new);
    bool belongsToNew(Block * target);
    bool belongsToMiddle(Block * target);
    bool belongsToOld(Block * target);

    int LRU_Algo();
    int LFU_Algo();
    int FBR_Algo();

    void updateAgeStack();

    vector<Block *> getUtilityAgingStack();
    vector<Block *> getstack();


    /**
     * Should actually take into account replacement policy
     * @return - zero upon successful update and -1 upon failure.
     */
    int insertNewBloack(Block * target);

    Block * readBlockFromStack(string target, int index);

    int shuffleStack(Block * target);

    int printLogTofile(ofstream &file);

private:
    int algo_policy;
    int numOfActiveBlock;
    int totalNumOfBlocks;
    /* last index of actual new section */
    int indexOfNew;
    /* first index of old instance */
    int indexOfOld;
    vector<Block *> _utilityAgingstack;
    vector<Block *> _stack;
};


#endif //EX4_BLOCKSTACK_H
