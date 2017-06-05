//
// Created by h3shiri on 6/5/17.
//

#include "Block.h"
#include <sys/stat.h>
#include <stdlib.h>

/**
 * A constructor foer the block object.
 * @param index - the index of the block in the file
 * @param name - actual unique name
 * @param BlockSize  - the actual block size
 * @param realSize - real size of data inside.
 * @return - the relevant block object.
 */
Block::Block(int index, string name, int BlockSize, int realSize)
{
    this->address = aligned_alloc(BlockSize, BlockSize);
    this->realSize = realSize;
    this->refCount++;
    this->name = name;
    this->index = index;
}
/**
 * A simple deconstructor freeing the used memory by this class.
 */
Block::~Block()
{
    delete(this->address);
}
/**
 * A getter function for the relevant index.
 * @return - the index of the block inside the file.
 */
int Block::getIndex()
{
    return index;
}

/**
 * A getter function for the relevant ref count.
 * @return - the current ref count.
 */
int Block::getRefCount()
{
    return refCount;
}

/**
 * Updating hte ref count by one.
 */
void Block::incRef()
{
    refCount++;
}

/**
 * A getter function for the actual size of the block.
 * @return
 */
int Block::getRealSize()
{

}
/**
 * A getter func for the actual file's name.
 * @return - the relevant name.
 */
string Block::getName()
{
    return name;
}