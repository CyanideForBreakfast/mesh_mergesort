#include "common.h"

#define MAX_NODES 512
#define PORT_SM_NAME "/node.c"
#define MUX_NAME "mux_name"

#define MERGE_SORT_TYPE "MERGE_SORT"
#define MERGE_TYPE "SORTED_LIST"
#define PASS_TYPE "PASS_MESSAGE"

int node;
int* nums;
int num_of_nums;
int num_of_nodes;
char buf[1000];
List* merge_list;

sem_t* print_mux;
unsigned short int* port_sm_ptr;

void processMessage(Message,Queue*);
void merge_and_fill(int*,int*,int*,int);
void printMessageInfo(Message*);
void endProg(int);

void endProg(int sig){
    char *args[]={"killall node",NULL}; 
    execvp(args[0],args);
    sleep(1);
    exit(1); 
}

int main(){

    node=0;
    num_of_nums = 0; num_of_nodes = 1;
    nums = (int*)malloc(MAX_NODES*sizeof(int));
    int n;
    printf("Make sure file has 2^n integers.\n");
    printf("Enter n for 2^n nodes (= total number of integers to be sorted): ");
    scanf("%d",&n);
    for(int i=0;i<n;i++) num_of_nodes*=2;
    printf("%d\n",num_of_nodes);
    FILE* fptr;
    if((fptr = fopen("./nums.txt","r"))==NULL) printf("error opening file.\n");
    //fscanf(fptr,"%d",&num_of_nums); //first number denotes number of nums
    num_of_nums=num_of_nodes;
    for(int i=0;i<num_of_nums;i++){fscanf(fptr,"%d",&nums[i]);}
    for(int i=0;i<num_of_nums;i++) printf("%d\t",nums[i]); printf("\n");

    // int n;
    // printf("Enter n for 2^n nodes: ");
    // scanf("%d",&n);
    // for(int i=0;i<n;i++) num_of_nodes*=2;
    // printf("%d\n",num_of_nodes);

    //create shared memory of ports and an semaphore
    int port_sm = shm_open(PORT_SM_NAME, O_CREAT | O_RDWR, 0600);
    if(port_sm<0) printf("shm_open error.\n");
    ftruncate(port_sm,MAX_NODES*sizeof(unsigned short int)); 
    port_sm_ptr = (unsigned short*)mmap(0,MAX_NODES*sizeof(unsigned short*),PROT_WRITE|PROT_READ,MAP_SHARED,port_sm,0);

    sem_t* mux = sem_open(MUX_NAME,O_CREAT,0660,0);
    if(mux==SEM_FAILED) printf("sem_open error.\n"); 
    print_mux = sem_open("PRINT_MUX",O_CREAT,0660,1);
    if(print_mux==SEM_FAILED) printf("sem_open error.\n");  

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
        //printf("Waiting now.\n");
        sem_wait(mux);
    }   

    printf("Node\tPort\n");
    for(int i=0;i<num_of_nodes;i++) printf("%d\t%hu\n",i+1,port_sm_ptr[i]);
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
    printf("Node 0 successfully connected in mesh.\n");

    //List testing.
    merge_list = (List*)malloc(sizeof(List));
    merge_list->length=0;
    merge_list->head=NULL;
    // int test[] = {8,7,9,10};
    // insert(merge_list,1,4,test,0);
    
    // insert(merge_list,7,4,test,0);
    // insert(merge_list,20,4,test,0);
    // insert(merge_list,2,4,test,0);
    // printList(merge_list);
    // delete(merge_list,7);
    // printList(merge_list);
    
    
    Queue* message_q = (Queue*)malloc(sizeof(Queue));
    Queue* to_recv_q = (Queue*)malloc(sizeof(Queue));
    message_q->size = 0; to_recv_q->size = 0;

    Message m;
    m.node_to=0;
    strcpy(m.type,MERGE_SORT_TYPE);
    m.action=0;
    m.node_from=-666;
    m.num_of_nums=num_of_nodes;
    for(int i=0;i<m.num_of_nums;i++) m.nums[i] = nums[i];

    //send(send_sockfd,(char*)&m,sizeof(m),0);
    push(message_q,m);
    //printf("pushed %d %d.\n",node,message_q->size);

    struct pollfd pfds[2];
    pfds[0].fd = recv_datafd;
    pfds[0].events = POLLIN;
    pfds[1].fd = send_sockfd;
    pfds[1].events = POLLOUT;

    //signal(SIGINT,endProg);

    printf("\nMessage_Types:\n");
    printf("MERGE_SORT: Mergesort request sent.\nSORTED_LIST: Sorted List returned.\nPASS_MESSAGE: Message not message for this node hence passed to the next.\n\n");
    printf("Node(Port)\tMessage_Type\tNode_From->Node_To\n");
    while(1){
        int fd_watch_num = message_q->size==0?1:2;
        //printf("fd_watch_num is %d %d\n",fd_watch_num,node);
        int num_events = poll(pfds,fd_watch_num,-1);
        if(num_events<0) printf("poll failed %d.\n",node);
        //printf("passed %d\n",node);
        for(int i=0;i<fd_watch_num;i++){
            //printf("checkpoint.\n");
            if(i==0 && pfds[0].revents != 0){
                int b=-2;
                if((b=recv(pfds[0].fd, (void*)buf,sizeof(Message),0))<(int)sizeof(Message)){
                    if(b<0) printf("%s",strerror(errno));
                    continue;
                };
                //printf("recieved %d vs %d\n",b,(int)sizeof(Message));
                Message m;
                m = *(Message*)buf;
                push(message_q,m);
                //printf("pushed %d %d.\n",node,message_q->size);
                //printf("size now: %d\n",to_send_q->size);
            }
            if(i==1 && pfds[1].revents != 0){
                Message m = pop(message_q);
                if(m.node_to==-666){
                    printf("--*---*---\nSorted List: ");
                    for(int i=0;i<m.num_of_nums;i++) printf("%d ",m.nums[i]);
                    printf("\n---*---*---\n");
                    kill(getpid(),SIGINT);
                }
                printMessageInfo(&m);
                //printf("popped %d %d\n",node,message_q->size);
                if(m.node_to==node){
                    processMessage(m,message_q);
                    break;
                }
                //printMessageInfo(&m);
                if(send(pfds[1].fd,(char*)&m,sizeof(m),0)<0) printf("%s\n",strerror(errno));
            }
        }
    }
    pause();
    return 0;
}

void processMessage(Message m,Queue* message_q){
    //printf("processing %d, %s,%d,%d\n",node,m.type,m.action,m.num_of_nums);
    //printf("recieved %d: ",node);
    //for(int i=0;i<m.num_of_nums;i++) printf("%d ",m.nums[i]);
    //printf("\n");
    if(strcmp(m.type,MERGE_SORT_TYPE)==0){
        if(m.num_of_nums==1) {
            //printf("---------- forever %d %d.\n",node,m.nums[0]);
            Message merge;
            strcpy(merge.type,MERGE_TYPE);
            merge.node_to = m.node_from;
            merge.node_from = node;
            merge.num_of_nums=1;
            merge.action=m.action;
            merge.nums[0]=m.nums[0];
            push(message_q,merge);
            //printf("pushed %d 1.\n",node);
            return;
        }

        Message m1;
        strcpy(m1.type,MERGE_SORT_TYPE);
        m1.node_from=node;
        m1.node_to=node+m.num_of_nums/2;
        m1.num_of_nums=m.num_of_nums/2;
        m1.action=m.action+1;
        for(int i=0;i<m1.num_of_nums;i++) m1.nums[i] = m.nums[i];
        push(message_q,m1);
        //printf("pushed %d 2.\n",node);
        //printf("pushed %d %d.\n",node,message_q->size);

        Message m2;
        strcpy(m2.type,MERGE_SORT_TYPE);
        m2.node_from=node;
        m2.node_to=node;
        m2.num_of_nums=m.num_of_nums/2;
        m2.action=m.action+1;
        for(int i=0;i<m2.num_of_nums;i++) m2.nums[i] = m.nums[i+m2.num_of_nums];
        push(message_q,m2);
        //printf("pushed %d 2.\n",node);
        //printf("pushed %d %d.\n",node,message_q->size);

        insert(merge_list,m.action,m.num_of_nums,m.node_from);
        return;
    }
    //mergesort
    if(strcmp(m.type,MERGE_TYPE)==0){
        //printf("merge request recieved %d.\n",node);

        //find required pointer
        Item* merge_item = merge_list->head;
        bool found=false;

        for(int i=0;i<merge_list->length;i++){
            if(merge_item->action==m.action-1){
                //printf("holla found %d.\n",node);
                found=true;
                break;
            }
            merge_item=merge_item->next;
        }
        if(!found) printf("jeez not found.\n");
        if(m.node_from==node){
            for(int i=0;i<m.num_of_nums;i++){
                merge_item->nums[i] = m.nums[i];
            }
            (merge_item->filled)+=m.num_of_nums;
        }
        else{
            for(int i=0;i<m.num_of_nums;i++){
                merge_item->nums[i+m.num_of_nums] =  m.nums[i];
            }
            (merge_item->filled)+=m.num_of_nums;
        }
        

        //printf("%d filled %d num_of_nums %d\n",node,merge_item->filled,merge_item->num_of_nums);
        
        if(merge_item->filled==merge_item->num_of_nums){
            //printf("%d entered here.\n",node);
            Message merge;
            //printf("%d before merge: ",node);
            //for(int i=0;i<merge_item->num_of_nums;i++) printf("%d ",merge_item->nums[i]);
            //printf("\n");
            merge_and_fill(merge_item->nums,merge_item->nums+merge_item->num_of_nums/2,merge.nums,merge_item->num_of_nums/2);
            //printf("%d after: ",node);
            //for(int i=0;i<merge_item->num_of_nums;i++) printf("%d ",m.nums[i]);
            //printf("\n");
            strcpy(merge.type,MERGE_TYPE);
            merge.node_from=node;
            merge.node_to=merge_item->node_from;
            merge.action=merge_item->action;
            merge.num_of_nums=merge_item->num_of_nums;
            push(message_q,merge);
            //printf("pushed %d 3.\n",node);
            delete(merge_list,m.action-1);
        }


    }
}

void merge_and_fill(int* nums1,int* nums2,int* final,int each_nums_size){
    int i1=0,i2=0,i=0;
    while(i1!=each_nums_size || i2!=each_nums_size){
        if(i1!=each_nums_size && i2!=each_nums_size){
            if(nums1[i1]<nums2[i2]){
                final[i++] = nums1[i1++];
            }
            else{
                final[i++] = nums2[i2++];
            }
            continue;
        }
        if(i1!=each_nums_size){
            final[i++] = nums1[i1++];
            continue;
        }
        if(i2!=each_nums_size){
            final[i++] = nums2[i2++];
        }
    }
}

void printMessageInfo(Message* m){
    //printf("waiting here.\n");
    //sem_wait(print_mux);
    //printf("wait ended.\n");
    // printf("---%d---\n",node);
    // printf("%s\n",m->type);
    // printf("%d %d\n",m->node_from,m->node_to);
    // for(int i=0;i<m->num_of_nums;i++) printf("%d ",m->nums[i]);
    // printf("\n");

    if(m->node_from!=node){
        char pass_msg[20];
        strcpy(pass_msg,PASS_TYPE);
        printf("%d(%hu)\t\t%s\t%d -> %d\n",node,port_sm_ptr[node],pass_msg,m->node_from==-666?-1:m->node_from,m->node_to);
        return;
    }
    printf("%d(%hu)\t\t%s\t%d -> %d\n",node,port_sm_ptr[node],m->type,m->node_from,m->node_to);
    //sem_post(print_mux);
}