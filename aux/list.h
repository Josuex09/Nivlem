//
// Created by josue on 10/18/15.
//

#ifndef NIVLEM_LIST_H
#define NIVLEM_LIST_H

#include "stdio.h"

struct node{
    double value;
    struct node* next;
};

typedef struct list{
    struct node * head ;
    struct node * tail ;
    int size;
} list_t;


void init(list_t* llist);
void add(list_t* llist,double val);
double get(list_t* llist,int i);


#endif //NIVLEM_LIST_H
