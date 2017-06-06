

#ifndef EX4_BLOCK_H
#define EX4_BLOCK_H

#include <sys/stat.h>
#include <string>

using namespace std;


class Block {
public:
    Block(int index, string name, int BlockSize,int realSize);
    ~Block();
    int getIndex() const;
    int getRefCount();
    string getName() const;
    int getRealSize();
    void incRef();

    bool operator==(const Block& rhs);

private:
    void * address = nullptr;
    string name;
    int index;
    int realSize;
    int refCount = 0;
};



#endif //EX4_BLOCK_H
