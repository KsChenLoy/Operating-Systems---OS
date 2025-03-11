//
// Created by Kemari Chen Loy on 3/20/22.
//

#include "MyBitMap.h"

//deleted the occupied memory in area of use by the buffer
void MyBitMap::clear() {
    if (memBuf)
    {
        delete[] memBuf;
    }
}

void MyBitMap::setMyBitmap(unsigned n){
    memBuf = new int[n];
    unsigned i = 0;
    while(i < n){
        memBuf[i] = 0x00;
        i++;
    }
    memR = n;
}

//Boolen to check if the memory in buffer is correctly allocated and then sets it
bool MyBitMap::set(int n)
{
    if (n >= 0 && n < memR) {
        memBuf[n] = 1;
        return true;
    } else {
        return false;
    }
}

//Boolen to check if the memory in buffer is correctly allocated and then unsets it
bool MyBitMap::unset(int n)
{
    if (n >= 0 && n < memR) {
        memBuf[n] = 0;
        return true;
    } else {
        return false;
    }
}

//determines if memory is in use and then returns that value
int MyBitMap::get(int n)
{
    if (n >= 0 && n < memR)
        return memBuf[n];
    return false;
}

//returns the range of size of memory block currently allocated
int MyBitMap::getRange() {
    return memR;
}

//keeps track of holes
void MyBitMap::append(int length, int offset) {
    int check = offset + length;
    int i = offset;
    while(i < check){
        set(i);
        i++;
    }
}

//Memory that is in used is freed here
void MyBitMap::release(int length, int offset) {
    int check = offset + length;
    int i = offset;
    while(i < check){
        unset(i);
        i++;
    }
}

//return the correct output of the string text
string MyBitMap::getMemmap() {
    string output;
    int begin = 0;
    int range;
    int i = 0;
    while (begin + i < memR) {
        if (get(begin) != 0)
            begin++;
        else {
            i = 0;
            range = 0;
            while (!get(begin + i) && (begin + i < memR)) {
                range++;
                i++;
            }
            output += (string) "[" + to_string(begin) + ", " + to_string(range) + "] - ";
            begin += i;
            i = 0;
        }
    }
    //extra strings erased
   // char temp = output.size()-3;

    return output.substr(0, output.size()-3);
}

//create an array of holes
uint16_t *MyBitMap::ToList() {
    int holes = 0;
    int begin = 0;
    while (begin < memR) {
        if (get(begin) != 0)
            begin++;
        else {
            holes++;
            while (!get(begin) && begin < memR)
                begin++;
        }
    }
    auto * myArray = new uint16_t[2* holes + 1];
    myArray[0] = holes;

    begin = 0;
    int length;
    int i = 0;
    int atArray = 1;
    while (begin + i < memR) {
        if (get(begin) != 0)
            begin++;
        else {
            i = 0;
            length = 0;
            while (!get(begin + i) && (begin + i < memR)) {
                length++;
                i++;
            }
            myArray[atArray] = (uint16_t) begin;
            myArray[atArray + 1] = (uint16_t) length;
            begin += i;
            i = 0;
            atArray += 2;
        }
    }
    return myArray;
}

//creates the format for the hex values needed
uint8_t *MyBitMap::formatOutput() {
    auto length = (uint16_t) ceil((double) memR / 8);
    auto *myArray = new uint8_t[(int) ceil((double) memR / 8)+2];
    for (int i = 0; i < (int) ceil((double) memR / 8) + 2; i++) {
        myArray[i] = 0;
    }

    length = static_cast<uint16_t>((length << 8) | ((length >> 8) & 0x00ff));
    myArray[0] = length >> 8;
    myArray[1] = length & 0x0FF;

    uint8_t count = 0;
    int i = 0;
    while(i < memR) {
        if (get(i)) {
            count += (uint8_t) pow(2, i % 8);
        }
        if (((i + 1) % 8 == 0 && i != 0) || i == memR - 1) {
            myArray[(i + 1) / 8 + 1] = myArray[(i + 1) / 8 + 1] ^ count;
            count = 0;
        }
        i++;
    }
    return myArray;
}


