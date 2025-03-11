//
// Created by Kemari Chen Loy on 3/20/22.
//

#ifndef OFFICIALMEMORYMANAGER_MEMORYMANAGER_H
#define OFFICIALMEMORYMANAGER_MEMORYMANAGER_H

#include <iostream>
#include <stdlib.h>
#include <functional>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <cmath>
#include <string.h>
#include "LinkedList.h"
#include "MyBitMap.h"

using namespace std;

class MemoryManager {

private:

    size_t wSize;
    std::function<int(int, void *)> alloc;
    char* memoryChunk;
    int memoryChunkCap;
    bool valid;
    MyBitMap *bMap;
    LinkedList *memLinkedlist;

public:

    MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator);
    ~MemoryManager();
    void initialize(size_t sizeInWords);
    void shutdown();
    void *allocate(size_t sizeInBytes);
    void free(void *address);
    void setAllocator(std::function<int(int, void *)> allocator);
    int dumpMemoryMap(char *filename);
    void *getList();
    void *getBitmap();
    unsigned getWordSize();
    void *getMemoryStart();
    unsigned getMemoryLimit();

};

//Algorithms
int bestFit(int sizeInWords, void *list);
int worstFit(int sizeInWords, void *list);

#endif //OFFICIALMEMORYMANAGER_MEMORYMANAGER_H
