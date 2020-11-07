#include "common.h"

#define MAX_NODES 512
#define PORT_SM_NAME "/node.c"
#define MUX_NAME "mux_name"

int node;
int num_of_nodes;
char buf[1000];

int main(int argc, char* argv[]){
    node=0;
    node=atoi(argv[1]);
    num_of_nodes = 1;
    num_of_nodes = atoi(argv[2]);
    printf("started node %d.\n",node);

    int port_sm = shm_open(PORT_SM_NAME, O_CREAT | O_RDWR, 0600);
    if(port_sm<0) printf("shm_open error.\n"); 
    unsigned short* port_sm_ptr = (unsigned short*)mmap(0,MAX_NODES*sizeof(unsigned short*),PROT_WRITE|PROT_READ,MAP_SHARED,port_sm,0);

    sem_t* mux = sem_open(MUX_NAME,O_CREAT,0660,0);
    if(mux==SEM_FAILED) printf("sem_open error %d.\n",node);

    //opening recieving socket
    //checking all free ports possible
    int recv_sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(recv_sockfd<0) printf("socket error.\n");
    struct sockaddr_in recv_sockaddr;
    memset(&recv_sockaddr,0,sizeof(recv_sockaddr));
    recv_sockaddr.sin_family = AF_INET;
    recv_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    for(unsigned short i=1;i<=USHRT_MAX;i++){
        recv_sockaddr.sin_port = htons(i);
        if(bind(recv_sockfd, (struct sockaddr*)&recv_sockaddr,sizeof(recv_sockaddr))==0){
            port_sm_ptr[node] = i;
            break;
        }
    }
    if(listen(recv_sockfd,100)<0) printf("listen error %d.\n",node);

    printf("opening semaphore in %d.\n",node);
    sem_post(mux);

    //accept data connection
    struct sockaddr_in recv_dataaddr;
    int recv_dataaddr_len = sizeof(recv_dataaddr);
    int recv_datafd = accept(recv_sockfd,(struct sockaddr*)&recv_dataaddr,&recv_dataaddr_len); 
    printf("successfully connected %d.\n",node);

    //make connection to next node
    //opening sending socket
    int send_sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(send_sockfd<0) printf("socket error send.\n");
    struct sockaddr_in send_sockaddr;
    memset(&send_sockaddr,0,sizeof(send_sockaddr));
    send_sockaddr.sin_family = AF_INET;
    send_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    send_sockaddr.sin_port = htons(port_sm_ptr[(node+1)%num_of_nodes]);
    while(1){
        if(connect(send_sockfd,(struct sockaddr*)&send_sockaddr,sizeof(send_sockaddr))==0) break;
        printf("connect error %d\n",node); 
    }

    Queue* message_q = (Queue*)malloc(sizeof(Queue));
    Queue* to_recv_q = (Queue*)malloc(sizeof(Queue));
    message_q->size = 0; to_recv_q->size = 0;

    //push pop testing
    // Message m;
    // m.node = 1;
    // for(int i=0;i<8;i++) {push(to_send_q,m);m.node+=3;}
    // for(int i=0;i<to_send_q->size;i++) {m=pop(to_send_q);printf("%d ",m.node);}
    // printf("\n");

    //creating polling mech
    struct pollfd pfds[2];
    pfds[0].fd = recv_datafd;
    pfds[0].events = POLLIN;
    pfds[1].fd = send_sockfd;
    pfds[1].events = POLLOUT;

    while(1){
        int fd_watch_num = message_q->size==0?1:2;
        //printf("fd_watch_num is %d %d\n",fd_watch_num,node);
        int num_events = poll(pfds,fd_watch_num,-1);
        if(num_events<0) printf("poll failed %d.\n",node);
        //printf("passed %d\n",node);
        for(int i=0;i<fd_watch_num;i++){
            if(i==0 && pfds[0].revents != 0){
                //printf("wating at recieve %d.\n",node);
                recv(pfds[0].fd, (void*)buf,sizeof(Message),0);
                printf("recieved message, it is %s\n", ((Message*)&buf)->type);
                Message m;
                m = *(Message*)buf;
                push(message_q,m);
                //printf("size now: %d\n",to_send_q->size);
            }
            if(i==1 && pfds[1].revents != 0){
                Message m = pop(message_q);
                send(pfds[1].fd,(char*)&m,sizeof(m),0);
            }
        }
    }

    pause();
    return 0;
}

void processMessage(Message m,Queue* message_q){
    if(m.node!=node) {
        push(message_q,m);
        return;
    }
    //mergesort
}