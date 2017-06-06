//
// Created by h3shiri on 6/6/17.
//

#ifndef EX4_BLOCKSTACK_H
#define EX4_BLOCKSTACK_H
#include "Block.h"
#include <vector>

class BlockStack {
public:
    BlockStack(int totalNumOfBlocks, int policy, double f_old , double f_new);
    bool belongsToNew(Block *);
    bool belongsToMiddle(Block *);
    bool belongsToOld(Block *);



    /**
     * Should actually take into account replacement policy
     * @return - zero upon successful update and -1 upon failure.
     */
    int insertNewBloack(Block *);



private:
    int algo_policy;
    int numOfActiveBlock;
    int totalNumOfBlocks;
    int indexOfNew;
    int indexOfOld;
    static vector<Block *> blockStack;
};


#endif //EX4_BLOCKSTACK_H
