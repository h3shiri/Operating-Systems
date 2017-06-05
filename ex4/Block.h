//
// Created by h3shiri on 6/5/17.
//

#ifndef EX4_BLOCK_H
#define EX4_BLOCK_H

#include <sys/stat.h>
#include <string>

using namespace std;


class Block {
public:
    Block(int index, string name, int BlockSize,int realSize);
    ~Block();
    int getIndex();
    int getRefCount();
    string getName();
    int getRealSize();
    void incRef();

private:
    void * address = nullptr;
    string name;
    int index;
    int realSize;
    int refCount = 0;
};



#endif //EX4_BLOCK_H
