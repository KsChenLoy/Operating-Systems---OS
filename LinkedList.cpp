//
// Created by Kemari Chen Loy on 3/20/22.
//

#include "LinkedList.h"

//Constructor for singly linked list
LinkedList::LinkedList() {
    head = nullptr;
}

//Destructor for singly linked list
LinkedList::~LinkedList() {
    clear();
}
//This ensures the singly linked list is deleted/cleared when ready
void LinkedList::clear(){
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
}

//returns the how long our singly linked list is
int LinkedList::getSizeOffset(int wordOffset) {
    Node *n = head;
    while (n) {
        if (n->offset != wordOffset) {
            n = n->next;
        } else {
            return n->length;
        }
    }
    return -1;
}

//adds new node to the singly linked list which makes new blocks in memory
void LinkedList::addList(size_t length, int offset) {
    auto *n = new Node;
    n->offset = offset;
    n->length = length;
    n->next = nullptr;
    if (!head) {
        head = n;
    } else {
        Node *curr = head;
        while (curr->next != nullptr)
            curr = curr->next;
        curr->next = n;
    }
}

//deletes a node in singly linked list
void LinkedList::deleteList(int wordOffset) {
    Node *curr = head;
    Node *prev = head;

    if (curr != nullptr && curr->offset == wordOffset) {
        head = curr->next;
        delete (curr);
        return;
    }

        while (curr != nullptr && curr->offset != wordOffset) {
            prev = curr;
            curr = curr->next;
        }

    if (curr == nullptr)
        return;

    prev->next = curr->next;
    delete (curr);

}
