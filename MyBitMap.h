//
// Created by Kemari Chen Loy on 3/20/22.
//

#ifndef OFFICIALMEMORYMANAGER_MYBITMAP_H
#define OFFICIALMEMORYMANAGER_MYBITMAP_H

#include <string>
#include <iomanip>
#include <math.h>
#include <bitset>
#include <iostream>

using namespace std;

class MyBitMap {
public:
    void clear();
    void setMyBitmap(unsigned n);
    bool set(int n);
    bool unset(int n);
    int get(int n);
    int getRange();
    void append(int length, int offset);
    void release(int length, int offset);
    string getMemmap();
    uint16_t * ToList();
    uint8_t* formatOutput();

private:
    int* memBuf;
    int memR;
};


#endif //OFFICIALMEMORYMANAGER_MYBITMAP_H
