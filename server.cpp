#include "rdma.h"
#include <iostream>

int main(){

    std::cout<<"This is the server side"<<std::endl;
    char local_addr1[40] = "tcp://172.23.12.124:8888";
    // char local_addr2[40] = "tcp://172.23.12.124:4444";
    char msg1[40] = "This is the server side1";
    char msg2[40] = "This is the server side2";
    char msg3[40] = "This is the server side3";
    char msg4[40] = "This is the server side4";
    char recv_msg1[40];
    class rdma::socket server_sock1 = rdma::socket(3);
    server_sock1.bind(local_addr1);
    std::cout<<"bind port success"<<std::endl;
    int rc = 0;
    rc = -1;
    while(rc<0) rc = server_sock1.send(msg1, 40, 0);
    rc = -1;
    while(rc<0) rc = server_sock1.send(msg2, 40, 0);
    // sleep(10);
    rc = -1;
    while(rc<0) rc = server_sock1.send(msg3, 40, 0);
    rc = -1;
    while(rc<0) rc = server_sock1.send(msg4, 40, 0);
    std::cout<<"message send success"<<std::endl;    
    return 0;
}
