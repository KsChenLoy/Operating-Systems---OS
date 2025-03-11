//
// Created by Kemari Chen Loy on 3/20/22.
//

#ifndef OFFICIALMEMORYMANAGER_LINKEDLIST_H
#define OFFICIALMEMORYMANAGER_LINKEDLIST_H

#include <cstdlib>
#include <cstdio>

#include <iostream>
class LinkedList{
public:
    struct Node{
        int length, offset;
        Node *next;

        Node(){
            length = 0;
            offset = 0;
            next = nullptr;
        }
    };

public:
    LinkedList();
    ~LinkedList();
    void clear();
    int getSizeOffset(int wordOffset);
    void addList(size_t length, int offset);
    void deleteList(int offset);

private:
    Node *head;

};



#endif //OFFICIALMEMORYMANAGER_LINKEDLIST_H
