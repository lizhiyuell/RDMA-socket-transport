#include "rdma.h"
#include <iostream>

int main(){

    std::cout<<"This is the client side"<<std::endl;
    char remote_addr[40] = "tcp://172.23.12.124:8888";
    char msg[40] = "This is the client side";
    class rdma::socket client_sock = rdma::socket();
    client_sock.connect(remote_addr);
    client_sock.send(msg, 40);
    std::cout<<"message send success"<<std::endl;
    client_sock.recv(msg, 40);
    std::cout<<"receive remote message success."<<std::endl<<"emote message is "<<std::endl<<msg<<std::endl;
    
    return 0;
}