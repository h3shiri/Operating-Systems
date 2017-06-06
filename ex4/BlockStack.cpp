
#define SUCCESS 0
#define ERROR -1

#include "BlockStack.h"
#include <math.h>

BlockStack::BlockStack(int totalNumOfBlocks, int policy, double f_old , double f_new)
{
	algo_policy = policy;
    numOfActiveBlock = 0;
    this->totalNumOfBlocks = totalNumOfBlocks;
    indexOfNew = (int) floor(totalNumOfBlocks * f_new) - 1;
    indexOfOld = totalNumOfBlocks - (int)floor(totalNumOfBlocks * f_old);
}

/**
 * checking whether a block is in the new section.
 * @param trBlock - the target block.
 * @return true - iff the block is in the new section.
 */
bool BlockStack::belongsToNew(Block * trBlock)
{
    int i = 0;
    for(auto * bl : _stack)
    {
        if((bl == trBlock) && (i <= indexOfNew))
        {
            return true;
        }
        ++i;
    }
    return false;
}

/**
 * A function checking whether a blocks beloings to the old section.
 * @param target - the relevant block.
 * @return true - iff the block belongs to the old section.
 */
bool BlockStack::belongsToOld(Block *target)
{
    int i = 0;
    for(auto * bl : _stack)
    {
        if((bl == target) && (i >= indexOfOld))
        {
            return true;
        }
        ++i;
    }
    return false;
}

/**
 * checking whether the block belongs to the middle section.
 * @param target
 * @return - true iff the block isn't part of the new or the old sectoin.
 */
bool BlockStack::belongsToMiddle(Block * target)
{
    return !(belongsToNew(target) || belongsToOld(target));
}

/**
 * quite an important function managing the insertion properly
 * @param target - the relevant block that shall be inserted.
 * @return zero upon successful completion and -1 in case of error.
 */
int BlockStack::insertNewBloack(Block *target)
{
    // In case the stack isn't full yet.
    if(numOfActiveBlock < totalNumOfBlocks)
    {
        _stack.insert(_stack.begin(), target);
        numOfActiveBlock++;
    }
    // In this case we shall use the external algo for updating the stack.
    else
    {
        /* pass to relevant algorithm the relevant info aka our stack Based Object */
        BlockStack * currentStack = this;
        selectedAlgo(currentStack);
    }

}