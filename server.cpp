#include "rdma.h"
#include <iostream>

int main(){

    std::cout<<"This is the server side"<<std::endl;
    char local_addr[40] = "tcp://172.23.12.124:8888";
    char msg[40] = "This is the server side";
    char recv_msg[40];
    class rdma::socket server_sock = rdma::socket(3);
    server_sock.bind(local_addr);
    std::cout<<"bind port success"<<std::endl;
    server_sock.recv(recv_msg, 40);
    std::cout<<"receive remote message success."<<std::endl<<"emote message is "<<std::endl<<recv_msg<<std::endl;
    server_sock.send(msg, 40);
    std::cout<<"message send success"<<std::endl;    
    return 0;
}