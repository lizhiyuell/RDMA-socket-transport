#include"rdma.h"
#include<iostream>
#include<algorithm>
#include<time.h>
#include<pthread.h>

#define msg_size 4*1024
#define test_num 10

char msg_s[msg_size];
char msg_r[BufferSize * MAX_CQ_NUM];
class rdma::socket sock_send;
class rdma::socket sock_recv;

long int get_time(){
    struct timespec c_time;
    clock_gettime(CLOCK_REALTIME, &c_time);
    long int t1, t2;
    t1 = c_time.tv_sec;
    t2 = c_time.tv_nsec;
    return t2 + t1*1000000000;
}

void *data_recv(void* argv){
    printf("start to execute recv thread\n");
    int rc;
    int rc2;
    for(int count=0;count<test_num;){
        rc=0;
        while(rc<=0) rc = sock_recv.recv(msg_r, BufferSize, 0);
        for(int k=0;k<rc;k++){
            memcpy(msg_s, msg_r+k*BufferSize, msg_size);
            //
            int num;
            memcpy(&num, msg_r+k*BufferSize, sizeof(int));
            //
            rc2=0;
            while(rc2<0)  rc2 = sock_send.send(msg_s, msg_size, 0);
            printf("finish with %d\n", num);
        }
        count+=rc;
    }
}

int main(){

    std::cout<<"This is the client side"<<std::endl;
    char remote_addr1[40] = "tcp://172.23.12.124:8888";
    char remote_addr2[40] = "tcp://172.23.12.124:9999";
    char msg1[BufferSize * MAX_CQ_NUM] = "This is the client side1";
    sock_send = rdma::socket(3);
    sock_recv = rdma::socket(3);
    sock_recv.connect(remote_addr1);
    sock_send.connect(remote_addr2);
    printf("connect started\n");
    int rc = 0;
    while(rc<=0) rc = sock_recv.recv(msg_r, 5, 0);
    printf("pre recv success\n");
    if(msg_r[0]=='A'&&msg_r[1]=='C'&&msg_r[2]=='K'){
        printf("connection built\n");
        rc=0;
        while(rc<0) rc = sock_send.send(msg_r, 5, 0);
    }
    else{
        printf("Error when sync\n");
        return 0;
    }
    printf("start to execute threads\n");
    pthread_t recv_t;
    pthread_create( &recv_t, NULL, data_recv, NULL);
    pthread_join( recv_t, NULL );
    printf("test finish");
    return 0;
}
