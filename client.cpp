#include "rdma.h"
#include <iostream>

int main(){

    std::cout<<"This is the client side"<<std::endl;
    char remote_addr1[40] = "tcp://172.23.12.124:8888";
    char msg1[BufferSize * 5] = "This is the client side1";
    class rdma::socket client_sock1 = rdma::socket(5);
    client_sock1.connect(remote_addr1);
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
    // int temp=0;
    while(true){
        rc = 0;
        while(rc<=0) rc = client_sock1.recv(msg1, BufferSize, 0);
        for(int k=0;k<rc;k++){
            memcpy(&count, msg1+k*BufferSize, sizeof(int));
            // if(count==(100000-1)) break;
            fprintf(stdout, "success with %d\n", count++);
            // if(count==temp){
            //     temp++;
            //     continue;
            // }
            // fprintf(stdout, "error with %d, should be %d\n", count,temp);
        }
    }
    return 0;
}
