#include "rdma.h"
#include <iostream>

int main(){

    std::cout<<"This is the client side"<<std::endl;
    char remote_addr1[40] = "tcp://172.23.12.124:8888";
    char msg1[BufferSize * MAX_CQ_NUM] = "This is the client side1";
    class rdma::socket client_sock1 = rdma::socket(5);
    client_sock1.connect(remote_addr1);
    // sleep(100);
    int rc;
    // rc = -1;
    // while(rc<0){
    //     rc = client_sock1.send(msg1, 40);
    // }
    // std::cout<<"send success!"<<std::endl;
    // std::cout<<"message send success"<<std::endl;
    char recv_buf[BufferSize];
    // sleep(5);
    // for(int ii=0;ii<3000;ii++){
    // rc = 0;
    // while(rc<=0) rc = client_sock1.recv(msg1, BufferSize, 0);
    // for(int i=0;i<rc;i++){
    //     fprintf(stdout, "The %d th:\n", i);
    //     memcpy(recv_buf, msg1 + i*BufferSize, BufferSize);
    //     std::cout<<"receive remote message success."<<std::endl<<"remote message is "<<std::endl<<recv_buf<<std::endl;
    // }

    // fprintf(stdout, "The msg is:\n");
    // for(int i=0;i<4;i++){
    //     for(int j=0;j<BufferSize;j++) fprintf(stdout, "%c", *(msg1+i*BufferSize +j));
    //     fprintf(stdout, "\n");
    // }
    // }
    int count=0;
    int temp=0;
    // struct timespec time1 = {0, 0};
    // clock_gettime(CLOCK_REALTIME, &time1);
    // long int start = time1.tv_sec*(int)1e9+time1.tv_nsec;
    while(true){
        rc = 0;
        while(rc<=0) rc = client_sock1.recv(msg1, BufferSize, 0);
        for(int k=0;k<rc;k++){
            memcpy(&count, msg1+k*BufferSize, sizeof(int));
            if(count==(10000-1)) return 0;
            // if(count%1000==0) 
            if(count%10000==0) 
            // fprintf(stdout, "success with %d\n", count);
            // if(count==temp){
            //     temp++;
            //     continue;
            // }
            // fprintf(stdout, "error with %d, should be %d\n", count,temp);
            // return 0;
        }
    }
    // clock_gettime(CLOCK_REALTIME, &time1);
    // long int end = time1.tv_sec*(int)1e9+time1.tv_nsec;
    // long int dur = end - start;
    // double tput = 100000.0/(double)dur;
    // printf("duration: %d, tput: %f\n", dur, tput);
    return 0;
}
