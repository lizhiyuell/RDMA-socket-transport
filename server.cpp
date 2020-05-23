#include"rdma.h"
#include<iostream>
#include<algorithm>
#include<time.h>
#include<string.h>
#include<pthread.h>
#include <sys/stat.h>
#include<fstream>
#include"nn.hpp"

#define msg_size 64*1024
#define test_num 100000
// #define USE_RDMA

int epoch = 11;
long int latency[2][test_num];
long int r_latency[test_num];
char msg_s[msg_size];
char msg_r[BufferSize * MAX_CQ_NUM];


#ifdef USE_RDMA
class rdma::socket sock_send = rdma::socket(3);
class rdma::socket sock_recv = rdma::socket(3);
#else
int timeo = 1000; // timeout in ms
int stimeo = 1000; // timeout in ms
int opt = 0;
class nn::socket sock_send = nn::socket(AF_SP,NN_PAIR);
class nn::socket sock_recv = nn::socket(AF_SP,NN_PAIR);
#endif
long int dur_s, dur_n;

void get_time(long int*sec, long int* nsec){
    struct timespec c_time;
    clock_gettime(CLOCK_REALTIME, &c_time);
    *sec = c_time.tv_sec;
    *nsec = c_time.tv_nsec;
}

void *data_send(void* argv){
    get_time(&dur_s, &dur_n);
    #ifdef USE_RDMA
    class rdma::socket* sock_ptr = (class rdma::socket*) argv;
    #else
    class nn::socket* sock_ptr = (class nn::socket*) argv;
    #endif
    printf("start to execute send thread\n");
    memset(msg_s, 0, msg_size);
    // begin to test
    // int count=0; // test finish count
    int rc;
    for(int count=0;count<test_num;count++){
        memcpy(msg_s, &count, sizeof(int));
        rc = -1;
        // printf("before send function\n");
        usleep(1);
        get_time(&latency[0][count], &latency[1][count]);
        while(rc<0) rc = sock_ptr->send(msg_s, msg_size, 0);
        // if(count%200==0) printf("send %d success\n", count);
        // if(count%200==0) printf("count:%d\n", count);
    }
    long int t1, t2;
    get_time(&t1, &t2);
    dur_s = t1 - dur_s;
    dur_n = t2 - dur_n;
}
void *data_recv(void* argv){
    #ifdef USE_RDMA
    class rdma::socket* sock_ptr = (class rdma::socket*) argv;
    #else
    class nn::socket* sock_ptr = (class nn::socket*) argv;
    #endif
    printf("start to execute recv thread\n");
    int rc;
    int valid_num=0;
    int error_num=0;
    #ifdef USE_RDMA
    for(int count=0;count<test_num;){
        rc=0;
        while(rc<=0) rc = sock_ptr->recv(msg_r, BufferSize, 0);
        int num;
        for(int k=0;k<rc;k++){
        memcpy(&num, msg_r+k*BufferSize, sizeof(int));
        if(num!=valid_num){ error_num++;valid_num = num;}
        // long long int t2 = get_time();
        long int t1, t2;
        get_time(&t1, &t2);
        r_latency[num] = (t1-latency[0][num])*1000000000+t2-latency[1][num];
        // if(num%200==0) printf("finish with num:%d, count:%d\n", num, count);
        valid_num++;
        }
        count+=rc;
    }
    #else
    for(int count=0;count<test_num;count++){
        rc=0;
        while(rc<=0) rc = sock_ptr->recv(msg_r, BufferSize, 0);
        int num;
        memcpy(&num, msg_r, sizeof(int));
        if(num!=valid_num){ error_num++;valid_num = num;}
        // long long int t2 = get_time();
        long int t1, t2;
        get_time(&t1, &t2);
        r_latency[num] = (t1-latency[0][num])*1000000000+t2-latency[1][num];
        // if(num%200==0) printf("finish with num:%d, count:%d\n", num, count);
        valid_num++;
    }
    #endif
    printf("In recv function, %d errors in total\n", error_num);
}

int main(){

    std::cout<<"This is the server side"<<std::endl;
    char local_addr1[40] = "tcp://172.23.12.124:8888";
    char local_addr2[40] = "tcp://172.23.12.124:9999";
    #ifndef USE_RDMA
    sock_send.setsockopt(NN_SOL_SOCKET,NN_RCVTIMEO,&timeo,sizeof(timeo));
    sock_send.setsockopt(NN_SOL_SOCKET,NN_SNDTIMEO,&stimeo,sizeof(stimeo));
    sock_send.setsockopt(NN_SOL_SOCKET,NN_TCP_NODELAY,&opt,sizeof(opt));
    sock_recv.setsockopt(NN_SOL_SOCKET,NN_RCVTIMEO,&timeo,sizeof(timeo));
    sock_recv.setsockopt(NN_SOL_SOCKET,NN_SNDTIMEO,&stimeo,sizeof(stimeo));
    sock_recv.setsockopt(NN_SOL_SOCKET,NN_TCP_NODELAY,&opt,sizeof(opt));
    #endif
    // std::cout<<"before bind port function"<<std::endl;
    sock_send.bind(local_addr1);
    sock_recv.bind(local_addr2);
    // test if connection is build
    printf("bind started\n");
    int rc = -1;
    char msg[5] = "ACK";
    while(rc<0) rc = sock_send.send(msg, 5, 0);
    printf("pre send success\n");
    rc=0;
    while(rc<=0) rc = sock_recv.recv(msg, 5, 0);
    if(msg[0]=='A'&&msg[1]=='C'&&msg[2]=='K') printf("connection built\n");
    else{
        printf("Error when sync\n");
        return 0;
    }
    printf("start to execute threads\n");
    pthread_t send_t, recv_t;
    
    pthread_create( &send_t, NULL, data_send, (void*)&sock_send);
    pthread_create( &recv_t, NULL, data_recv, (void*)&sock_recv);
    // wait for the end
    pthread_join( send_t, NULL );
    pthread_join( recv_t, NULL );
    long int dur = dur_s*1000000000+dur_n;
    double tput = (double)test_num * 1e9 / (double)(dur -  1e3*test_num);
    printf("test finish!\ntput is %f\n", tput);
    // printf("the first ten latency is:\n");
    // for(int i=0;i<10;i++) printf("%lld\n", latency[i]);
    // order the latency
    std::sort(r_latency, r_latency+test_num);
    printf("starting input file\n");
    char filepath[100];
    sprintf(filepath, "result/%d.txt", epoch);
    std::fstream _file;
    _file.open(filepath, std::ios::out);
    if(!_file)
	{
		 mkdir(filepath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
    std::fstream output_file;
    output_file.open(filepath, std::ios::out);
    for(int i=0;i<test_num;i++) output_file<<r_latency[i]<<std::endl;
    output_file.close();
    printf("test finish\n");
    // for(int i=0;i<test_num;i++) printf("%lld\n", latency[i]);
    // printf("p50=%lld, p90")
    return 0;
}