#include "rdma.h"
#include <iostream>

int main(){

    std::cout<<"This is the server side"<<std::endl;
<<<<<<< HEAD
    char local_addr1[40] = "tcp://172.23.12.124:8888";
    // char local_addr2[40] = "tcp://172.23.12.124:4444";
||||||| merged common ancestors
    char local_addr1[40] = "tcp://172.23.12.124:8888";
    char local_addr2[40] = "tcp://172.23.12.124:4444";
=======
    char local_addr1[40] = "tcp://172.23.12.127:17002";
    char local_addr2[40] = "tcp://172.23.12.127:17006";
>>>>>>> e108c134006f857b22feeefae97f4ca47893b5b5
    char msg1[40] = "This is the server side1";
    // char msg2[40] = "This is the server side2";
    char recv_msg1[40];
<<<<<<< HEAD
    // char recv_msg2[40];
    class rdma::socket server_sock1 = rdma::socket(3);
    // class rdma::socket server_sock2 = rdma::socket(3);
||||||| merged common ancestors
    char recv_msg2[40];
    class rdma::socket server_sock1 = rdma::socket(3);
    class rdma::socket server_sock2 = rdma::socket(3);
=======
    char recv_msg2[40];
    class rdma::socket server_sock1 = rdma::socket(5);
    class rdma::socket server_sock2 = rdma::socket(5);
>>>>>>> e108c134006f857b22feeefae97f4ca47893b5b5
    server_sock1.bind(local_addr1);
    // server_sock2.bind(local_addr2);
    std::cout<<"bind port success"<<std::endl;
    sleep(10);
    int rc = 0;
    while(rc<=0) rc = server_sock1.recv(recv_msg1, 40);
    // rc = 0;
    // while(rc<=0) rc = server_sock2.recv(recv_msg2, 40);
    std::cout<<"receive remote message success."<<std::endl<<"remote message is "<<std::endl<<recv_msg1<<std::endl;
    // std::cout<<"receive remote message success."<<std::endl<<"remote message is "<<std::endl<<recv_msg2<<std::endl;
    rc = -1;
    while(rc<0) rc = server_sock1.send(msg1, 40);
    // rc = -1;
    // while(rc<0) rc = server_sock2.send(msg2, 40);
    std::cout<<"message send success"<<std::endl;    
    return 0;
}
