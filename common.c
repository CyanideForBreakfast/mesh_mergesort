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
    Message temp;
    if(q->size==0) return temp;
    q->size--;
    temp = q->head->message;
    Element* temp_ptr = q->head;
    q->head = q->head->prev;
    free(temp_ptr);
    return temp;
}

void insert(List* l,int action,int num_of_nums,int* nums,int node_from){
    Item* i=(Item*)malloc(sizeof(Item));
    i->action = action;
    i->num_of_nums = num_of_nums;
    i->node_from = node_from;
    i->filled = 0;
    for(int j=0;j<num_of_nums;j++) i->nums[j] = nums[j];

    if(l->length==0){
        l->head = i;
        i->next = NULL;
        l->length = 1;
        return;
    }
    i->next=l->head;
    l->head = i;
    l->length++;
}
void delete(List* l, int action){
    Item* prev = NULL;
    Item* ptr = l->head;
    while(ptr!=NULL){
        if(ptr->action==action){
            prev->next = ptr->next;
            free(ptr);
            break;
        }
        prev=ptr;
        ptr=ptr->next;
    }
}

void printList(List* l){
    Item* ptr = l->head;
    while(ptr!=NULL){
        printf("%d: ",ptr->action);
        for(int i=0;i<ptr->num_of_nums;i++) printf("%d ",ptr->nums[i]);
        printf("\n");
        ptr=ptr->next;
    }
    printf("/////\n");
}