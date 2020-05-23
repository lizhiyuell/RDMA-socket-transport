#include"rdma.h"
#include<iostream>
#include<algorithm>
#include<time.h>
#include<pthread.h>
#include"nn.hpp"

#define msg_size 32*1024
#define test_num 10000
// #define USE_RDMA

char msg_s[msg_size];
char msg_r[BufferSize * MAX_CQ_NUM];
#ifdef USE_RDMA
class rdma::socket sock_send = rdma::socket(5);
class rdma::socket sock_recv = rdma::socket(5);
#else
int timeo = 1000; // timeout in ms
int stimeo = 1000; // timeout in ms
int opt = 0;
class nn::socket sock_send = nn::socket(AF_SP,NN_PAIR);
class nn::socket sock_recv = nn::socket(AF_SP,NN_PAIR);
#endif

struct param_t{
    #ifdef USE_RDMA
    class rdma::socket* s1;
    class rdma::socket* s2;
    #else
    class nn::socket* s1;
    class nn::socket* s2;
    #endif
};


void *data_recv(void* argv){
    struct param_t* myp = (struct param_t*) argv;
    
    // printf("start to execute recv thread\n");
    int rc;
    int rc2;
    #ifdef USE_RDMA
    for(int count=0;count<test_num;){
        rc=0;
        while(rc<=0) rc = myp->s2->recv(msg_r, BufferSize, 0);
        for(int k=0;k<rc;k++){
            memcpy(msg_s, msg_r+k*BufferSize, msg_size);
            //
            int num;
            memcpy(&num, msg_r+k*BufferSize, sizeof(int));
            //
            rc2=-1;
            while(rc2<0)  rc2 = myp->s1->send(msg_s, msg_size, 0);
            // if(num%200==0) printf("finish with num:%d, count:%d\n", num, count); 
        }
        count+=rc;
    }
    #else
        for(int count=0;count<test_num;count++){
        rc=0;
        while(rc<=0) rc = myp->s2->recv(msg_r, BufferSize, 0);
        memcpy(msg_s, msg_r, msg_size);
        rc2=-1;
        while(rc2<0)  rc2 = myp->s1->send(msg_s, msg_size, 0);
    }
    #endif
}

int main(){

    std::cout<<"This is the client side"<<std::endl;
    char remote_addr1[40] = "tcp://172.23.12.124:8888";
    char remote_addr2[40] = "tcp://172.23.12.124:9999";
    char msg1[BufferSize * MAX_CQ_NUM] = "This is the client side1";
    #ifndef USE_RDMA
    sock_send.setsockopt(NN_SOL_SOCKET,NN_RCVTIMEO,&timeo,sizeof(timeo));
    sock_send.setsockopt(NN_SOL_SOCKET,NN_SNDTIMEO,&stimeo,sizeof(stimeo));
    sock_send.setsockopt(NN_SOL_SOCKET,NN_TCP_NODELAY,&opt,sizeof(opt));
    sock_recv.setsockopt(NN_SOL_SOCKET,NN_RCVTIMEO,&timeo,sizeof(timeo));
    sock_recv.setsockopt(NN_SOL_SOCKET,NN_SNDTIMEO,&stimeo,sizeof(stimeo));
    sock_recv.setsockopt(NN_SOL_SOCKET,NN_TCP_NODELAY,&opt,sizeof(opt));
    #endif
    sock_recv.connect(remote_addr1);
    sock_send.connect(remote_addr2);
    printf("connect started\n");
    int rc = 0;
    while(rc<=0) rc = sock_recv.recv(msg_r, 5, 0);
    // printf("pre recv\n");
    if(msg_r[0]=='A'&&msg_r[1]=='C'&&msg_r[2]=='K'){
        printf("connection built\n");
        rc=-1;
        while(rc<0) rc = sock_send.send(msg_r, 5, 0);
    }
    else{
        printf("Error when sync\n");
        return 0;
    }
    printf("start to execute threads\n");
    pthread_t recv_t;
    struct param_t myp;
    myp.s1 = &sock_send;
    myp.s2 = &sock_recv;
    pthread_create( &recv_t, NULL, data_recv, (void*)&myp);
    pthread_join( recv_t, NULL );
    printf("test finish\n");
    return 0;
}
