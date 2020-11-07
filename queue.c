#include "common.h"

void push(Queue*,Message);
Message pop(Queue*);

void push(Queue* q,Message m){
    Element* ele = malloc(sizeof(Element));
    if(q->size==0){
        ele->message = m;
        ele->next = NULL;
        ele->prev = NULL;
        q->size++;
        q->head = ele;
        q->tail = ele;
        return;    
    }
    ele->message = m;
    ele->next = q->tail;
    ele->prev = NULL;
    q->tail->prev = ele;
    q->tail = ele;
    q->size++;
}

Message pop(Queue* q){
    q->size--;
    Message temp = q->head->message;
    Element* temp_ptr = q->head;
    q->head = q->head->prev;
    free(temp_ptr);
    return temp;
}