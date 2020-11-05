#include "common.h"

#define MAX_NODES 512
#define PORT_SM_NAME "/node.c"
#define MUX_NAME "mux_name"

int* nums;
int num_of_nums;
int num_of_nodes;

int main(){
    num_of_nums = 0; num_of_nodes = 1;
    nums = (int*)malloc(MAX_NODES*sizeof(int));
    FILE* fptr;
    if((fptr = fopen("./nums.txt","r"))==NULL) printf("error opening file.\n");
    fscanf(fptr,"%d",&num_of_nums); //first number denotes number of nums
    for(int i=0;i<num_of_nums;i++){fscanf(fptr,"%d",&nums[i]);}
    for(int i=0;i<num_of_nums;i++) printf("%d\t",nums[i]); printf("\n");

    int n;
    printf("Enter n for 2^n nodes: ");
    scanf("%d",&n);
    for(int i=0;i<n;i++) num_of_nodes*=2;
    printf("%d\n",num_of_nodes);

    //create shared memory of ports and an semaphore
    int port_sm = shm_open(PORT_SM_NAME, O_CREAT | O_RDWR, 0600);
    if(port_sm<0) printf("shm_open error.\n");
    ftruncate(port_sm,MAX_NODES*sizeof(unsigned short int)); 
    unsigned short* port_sm_ptr = (unsigned short*)mmap(0,MAX_NODES*sizeof(unsigned short*),PROT_WRITE|PROT_READ,MAP_SHARED,port_sm,0);

    sem_t* mux = sem_open(MUX_NAME,O_CREAT,0660,0);
    if(mux==SEM_FAILED) printf("sem_open error.\n");  

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
            port_sm_ptr[0] = i;
            break;
        }
    }
    if(listen(recv_sockfd,100)<0) printf("listen error.\n");

    //creating num_of_nodes nodes
    for(int i=1;i<num_of_nodes;i++){
        if(fork()==0){
            char value[10];
            sprintf(value,"%d",i);
            char total_nodes_string[10];
            sprintf(total_nodes_string,"%d",num_of_nodes);
            char *args[]={"./node",value,total_nodes_string,NULL}; 
            execvp(args[0],args); 
        }
        printf("Waiting now.\n");
        sem_wait(mux);
    }   

    for(int i=0;i<num_of_nodes;i++) printf("%hu ",port_sm_ptr[i]);
    printf("\n");

    struct sockaddr_in recv_dataaddr;
    int recv_dataaddr_len = sizeof(recv_dataaddr_len);

    //opening sending socket
    int send_sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(send_sockfd<0) printf("socket error send.\n");
    struct sockaddr_in send_sockaddr;
    memset(&send_sockaddr,0,sizeof(send_sockaddr));
    send_sockaddr.sin_family = AF_INET;
    send_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    send_sockaddr.sin_port = htons(port_sm_ptr[1]);
    if(connect(send_sockfd,(struct sockaddr*)&send_sockaddr,sizeof(send_sockaddr))<0) printf("connect error.\n"); 

    int recv_datafd = accept(recv_sockfd,(struct sockaddr*)&recv_dataaddr,&recv_dataaddr_len); 
    printf("successfully connected.\n");

    pause();
    return 0;
}