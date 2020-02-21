#include "rdma.h"
#include <iostream>

int main(){

    std::cout<<"This is the client side"<<std::endl;
    char remote_addr[40] = "tcp://172.23.12.124:8888";
    char msg[40] = "This is the client side";
    class rdma::socket client_sock = rdma::socket(5);
    client_sock.connect(remote_addr);
    // sleep(5);
    int rc = -1;
    while(rc<0) rc = client_sock.send(msg, 40);
    std::cout<<"message send success"<<std::endl;
    while(rc<=0) rc = client_sock.recv(msg, 40);
    std::cout<<"receive remote message success."<<std::endl<<"remote message is "<<std::endl<<msg<<std::endl;
    
    return 0;
}