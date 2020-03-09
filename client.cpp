#include "rdma.h"
#include <iostream>

int main(){

    std::cout<<"This is the client side"<<std::endl;
    char remote_addr1[40] = "tcp://172.23.12.124:8888";
    char msg1[40] = "This is the client side1";
    class rdma::socket client_sock1 = rdma::socket(5);
    client_sock1.connect(remote_addr1);
    int rc;
    // rc = -1;
    // while(rc<0){
    //     rc = client_sock1.send(msg1, 40);
    // }
    // std::cout<<"send success!"<<std::endl;
    // std::cout<<"message send success"<<std::endl;
    rc = 0;
    sleep(5);
    while(rc<=0) rc = client_sock1.recv(msg1, 40);
    // rc = 0;
    // while(rc<=0) rc = client_sock2.recv(msg2, 40);
    std::cout<<"receive remote message success."<<std::endl<<"remote message is "<<std::endl<<msg1<<std::endl;
    // std::cout<<"receive remote message success."<<std::endl<<"remote message is "<<std::endl<<msg2<<std::endl;
    rc = 0;
    while(rc<=0) rc = client_sock1.recv(msg1, 40);
    std::cout<<"receive remote message success."<<std::endl<<"remote message is "<<std::endl<<msg1<<std::endl;
    rc = 0;
    while(rc<=0) rc = client_sock1.recv(msg1, 40);
    std::cout<<"receive remote message success."<<std::endl<<"remote message is "<<std::endl<<msg1<<std::endl;
    rc = 0;
    while(rc<=0) rc = client_sock1.recv(msg1, 40);
    std::cout<<"receive remote message success."<<std::endl<<"remote message is "<<std::endl<<msg1<<std::endl;

    
    
    return 0;
}
