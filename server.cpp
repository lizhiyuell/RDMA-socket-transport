#include "rdma.h"
#include <iostream>

int main(){

    std::cout<<"This is the server side"<<std::endl;
    char local_addr1[40] = "tcp://172.23.12.124:8888";
    class rdma::socket server_sock1 = rdma::socket(3);
    server_sock1.bind(local_addr1);
    std::cout<<"bind port success"<<std::endl;
    int rc = 0;
    int count = 0;
    char msg[5];
    // struct timespec time1 = {0, 0};
    // clock_gettime(CLOCK_REALTIME, &time1);
    // long int start = time1.tv_sec*(int)1e9+time1.tv_nsec;
    for(int i=0;i<100000;i++){
        rc = -1;
        memcpy(msg, &count, sizeof(int));
        while(rc<0) rc = server_sock1.send(msg, 5, 0);
        fprintf(stdout, "send %d success\n", count);
        count++;
        // usleep(10);
    }
    // clock_gettime(CLOCK_REALTIME, &time1);
    // long int end = time1.tv_sec*(int)1e9+time1.tv_nsec;
    // long int dur = end - start;
    // double tput = 100000.0/(double)dur;
    // printf("duration: %d, tput: %f\n", dur, tput);
    // rc = -1;
    // while(rc<0) rc = server_sock1.send(msg1, 5, 0);
    // sleep(2);
    // rc = -1;
    // while(rc<0) rc = server_sock1.send(msg2, 5, 0);
    // // sleep(10);
    // sleep(2);
    // rc = -1;
    // while(rc<0) rc = server_sock1.send(msg3, 5, 0);
    // rc = -1;
    // sleep(2);
    // while(rc<0) rc = server_sock1.send(msg4, 5, 0);
    std::cout<<"message send success"<<std::endl;    
    return 0;
}
