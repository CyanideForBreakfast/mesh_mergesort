#include "common.h"

#define MAX_NODES 512
#define PORT_SM_NAME "/node.c"
#define MUX_NAME "mux_name"

int node;
int num_of_nodes;

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

    pause();
    return 0;
}