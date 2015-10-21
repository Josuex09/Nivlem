//
// Created by josue on 10/18/15.
//

#include <stdlib.h>
#include "list.h"


void init(list_t* llist){
    llist->head = NULL;
    llist->tail = NULL;
    llist->size = 0;
}

void add(list_t* llist,double val){
    struct node* nod = malloc(sizeof(*nod)) ;
    nod->value = val;
    nod->next = NULL;
    if(llist->size == 0){
        llist->head = nod;
        llist->tail = nod;
    }
    else{
        llist->tail->next = nod;
        llist->tail = nod;
    }
    llist->size++;
}

double get(list_t* llist,int i){
    if(i == 0 ) return llist->head->value;
    else {
        int index = 0;
        struct node * nod = llist->head;
        while(i>index){
            index++;
            nod = nod->next;
        }
        return nod->value;
    }
}
