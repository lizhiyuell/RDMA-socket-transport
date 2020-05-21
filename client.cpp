#include"rdma.h"
#include<iostream>
#include<algorithm>
#include<time.h>
#include<pthread.h>

#define msg_size 4*1024
#define test_num 100000

char msg_s[msg_size];
char msg_r[BufferSize * MAX_CQ_NUM];
class rdma::socket sock_send = rdma::socket(5);
class rdma::socket sock_recv = rdma::socket(5);

struct param_t{
    class rdma::socket* s1;
    class rdma::socket* s2;
};


void *data_recv(void* argv){
    struct param_t* myp = (struct param_t*) argv;
    
    // printf("start to execute recv thread\n");
    int rc;
    int rc2;
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
}

int main(){

    std::cout<<"This is the client side"<<std::endl;
    char remote_addr1[40] = "tcp://172.23.12.124:8888";
    char remote_addr2[40] = "tcp://172.23.12.124:9999";
    char msg1[BufferSize * MAX_CQ_NUM] = "This is the client side1";
    class rdma::socket sock_send = rdma::socket(5);
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
