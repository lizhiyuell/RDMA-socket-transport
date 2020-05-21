#include"rdma.h"
#include<iostream>
#include<algorithm>
#include<time.h>
#include<string.h>
#include<pthread.h>
#include <sys/stat.h>
#include<fstream>

#define msg_size 4*1024
#define test_num 10000
int epoch = 1;
long long int latency[test_num];
char msg_s[msg_size];
char msg_r[BufferSize * MAX_CQ_NUM];
class rdma::socket sock_send = rdma::socket(3);
class rdma::socket sock_recv = rdma::socket(3);

long int get_time(){
    struct timespec c_time;
    clock_gettime(CLOCK_REALTIME, &c_time);
    long int t1, t2;
    t1 = c_time.tv_sec;
    t2 = c_time.tv_nsec;
    return t2 + t1*1000000000;
}

void *data_send(void* argv){
    class rdma::socket* sock_ptr = (class rdma::socket*) argv;
    printf("start to execute send thread\n");
    memset(msg_s, 0, msg_size);
    // begin to test
    // int count=0; // test finish count
    int rc;
    for(int count=0;count<test_num;count++){
        memcpy(msg_s, &count, sizeof(int));
        rc = -1;
        // printf("before send function\n");
        while(rc<0) rc = sock_ptr->send(msg_s, msg_size, 0);
        latency[count] = get_time();
        if(count%200==0) printf("send %d success\n", count);
    }
}
void *data_recv(void* argv){
    class rdma::socket* sock_ptr = (class rdma::socket*) argv;
    printf("start to execute recv thread\n");
    int rc;
    for(int count=0;count<test_num;){
        rc=0;
        while(rc<=0) rc = sock_ptr->recv(msg_r, BufferSize, 0);
        int num;
        for(int k=0;k<rc;k++){
        memcpy(&num, msg_r+k*BufferSize, sizeof(int));
        long long int t2 = get_time();
        latency[num] = t2 - latency[num];
        printf("finish with %d\n", num); 
        }
        count+=rc;
    }
}

int main(){

    std::cout<<"This is the server side"<<std::endl;
    char local_addr1[40] = "tcp://172.23.12.124:8888";
    char local_addr2[40] = "tcp://172.23.12.124:9999";
    // std::cout<<"before bind port function"<<std::endl;
    class rdma::socket sock_send = rdma::socket(3);
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
    long int dur = get_time();
    pthread_create( &send_t, NULL, data_send, (void*)&sock_send);
    pthread_create( &recv_t, NULL, data_recv, (void*)&sock_recv);
    // wait for the end
    pthread_join( send_t, NULL );
    pthread_join( recv_t, NULL );
    dur = get_time() - dur;
    double tput = (double)test_num * 1e9 / (double)dur;
    printf("test finish!\ntput is %f\n", tput);
    // printf("the first ten latency is:\n");
    // for(int i=0;i<10;i++) printf("%lld\n", latency[i]);
    // order the latency
    std::sort(latency, latency+test_num);
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
    for(int i=0;i<test_num;i++) output_file<<latency[i]<<std::endl;
    output_file.close();
    printf("test finish\n");
    // for(int i=0;i<test_num;i++) printf("%lld\n", latency[i]);
    // printf("p50=%lld, p90")
    return 0;
}
