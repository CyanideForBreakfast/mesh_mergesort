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

typedef struct Message{
    int node;
    char type[100];
    int nums[100];
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

void push(Queue*,Message);
Message pop(Queue*);
#endif