#ifndef COMMON
#define COMMON
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/types.h>
#include <limits.h>
#include <poll.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>

typedef struct Message{
    int node_to;
    int node_from;
    char type[10];
    int action;
    int num_of_nums;
    int nums[20];
} Message;

typedef struct Element{
    Message message;
    struct Element* next;
    struct Element* prev;
} Element;
typedef struct Queue{
    int size;
    Element* head;
    Element* tail;
} Queue;

typedef struct Item{
    int action;
    int num_of_nums;
    int nums[20];
    int filled;
    int node_from;
    struct Item* next;
} Item;
typedef struct List{
    int length;
    Item* head;
} List;

void push(Queue*,Message);
Message pop(Queue*);

void insert(List*,int,int,int);
void delete(List*,int action);//delete based on action
void printList(List*);
#endif