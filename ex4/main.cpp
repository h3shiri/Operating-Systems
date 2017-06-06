#include <iostream>
#include <sys/stat.h>
#include <stdlib.h>
#include "BlockStack.h"

using namespace std;

int main() {

    BlockStack * ptr = new BlockStack(100, 0, 0.22, 0.6);
    struct stat fi;
    stat("/tmp", &fi);
    int blksize=fi.st_blksize;

    cout << realpath(".", NULL) << endl;
    cout << (2 % 5) << endl;
    cout << "Hello, World!" << to_string(blksize) << std::endl;
    return 0;
}