
#define SUCCESS 0
#define ERROR -1

#include "BlockStack.h"
#include "CacheFS.h"
#include <math.h>
#include <algorithm>

// #TODO: check name is a quality comparison between blocks.

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
    for(auto bl : _stack)
    {
        if((bl->getName() == trBlock->getName()) && (i <= indexOfNew))
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
        if((bl->getName() == target->getName()) && (i >= indexOfOld))
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
        int result;
        if (algo_policy == LRU)
        {
            result = LRU_Algo();
            _stack.insert(_stack.begin(), target);
        }
        else if (algo_policy == LFU)
        {
            result = LFU_Algo();
            _stack.insert(_stack.begin(), target);
        }
        else if (algo_policy == FBR)
        {
            result = FBR_Algo();
            _stack.insert(_stack.begin(), target);
        }
        else
        {
            result = ERROR;
        }
        return result;
    }
}

/**
 * simple comparator for checking the blocks age.
 * note this leads for the oldest block to be in the front.
 * @param left - the left argument aka a block.
 * @param right - the right argument aka a block.
 * @return - true if left is younger then right
 */
bool compareAge(Block * left,  Block * right)
{
    return left->getRefCount() > right->getRefCount();
}

/**
 * This function clears the aged stack and updated elements.
 * plus it additionaly sorts them by age ref count
 */
void BlockStack::updateAgeStack()
{
    _utitlityAgingstack.clear(); // doesn't erase data thankfully
    _utitlityAgingstack.assign(_stack.begin(), _stack.end());
    std::sort(_utitlityAgingstack.begin(), _utitlityAgingstack.end(), compareAge);
}



/**
 * The last element in the stack shall be kicked out
 * @return - zero in case of a mooth operation -1 in case of an error.
 */
int BlockStack::LRU_Algo()
{
    //TODO: check memory is actually freed by the default destructor.
    if (numOfActiveBlock < totalNumOfBlocks)
    {
        return ERROR;
    }
    _stack.pop_back();
    return SUCCESS;
}

/**
 * kicking out he least frequently used by age.
 * aka the yougest one without any forced order.
 * @return - zero in case of a mooth operation -1 in case of an error.
 */
int BlockStack::LFU_Algo()
{
    int result = ERROR;
    // updating the age block.
    updateAgeStack();
    Block * toRemove = _utitlityAgingstack.back();
    /* note here we need a quality comparison. */
    int index = 0;
    for (auto bl : _stack)
    {
        if (bl->getName() == toRemove->getName())
        {
            _stack.erase(_stack.begin() + index);
            delete(bl);
            result = SUCCESS;
            break;
        }
        ++index;
    }
    return result;
}
/**
 * maintaing the LRU order in the cache aka order of arrival.
 * primary decision is based upon ref count and secondary is on LRU order.
 * @return zero in case of successful call and -1 in case of an error.
 */
int BlockStack::FBR_Algo()
{
    //TODO: check secondary order is actually preserved.
    LFU_Algo();
}

/**
 * A function for getting a relevant block from memmory.
 * @param target - the relevant file full path aka an ID.
 * @param index - the block index.
 * @return - upon finding return pointer to the relevant block
 * in case of failure return the nullptr.
 */
Block * BlockStack::readBlockFromStack(string target, int index)
{
    Block * res = nullptr;
    for (auto bl : _stack)
    {
        if ((bl->getName() == target) && (bl->getIndex() == index))
        {
            res = bl;
        }
    }
    return res;
}
