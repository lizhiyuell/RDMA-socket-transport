#include "rdma.h"
/*
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <rdma/rdma_verbs.h>
*/

namespace rdma{

    socket::socket(){
        rrdma = (rdma_m*) malloc( sizeof(rdma_m) );
        rrdma->s_ctx = ( struct connection * )malloc( sizeof( struct connection ) );

        struct ibv_device **dev_list = NULL;
        int num_devices;        
        // struct ibv_qp_init_attr qp_init_attr;
        struct ibv_device *ib_dev = NULL;
        // size_t 	size;
        
        // int mr_flags = 0;
        // int cq_size = 0;
        
        // char *dev_name = NULL;

        dev_list = ibv_get_device_list(&num_devices);
        if (!dev_list) {
            fprintf(stderr, "failed to get IB devices list\n");
            assert(false);
        }

        /* if there isn't any IB device in host */
        if (!num_devices) {
            fprintf(stderr, "found %d device(s)\n", num_devices);
            assert(false);
        }

        fprintf(stdout, "found %d device(s)\n", num_devices);
    
        int device_index;
        for (device_index = 0; device_index < num_devices; device_index ++) {
        ib_dev = dev_list[device_index];

        /* get device handle */
        rrdma->s_ctx->ctx = ibv_open_device(ib_dev);
        if (!rrdma->s_ctx->ctx) {
            fprintf(stderr, "failed to open device %s\n", dev_name);
            continue;
        }

        /* query port properties  */
        if (ibv_query_port(rrdma->s_ctx->ctx, ib_port, &rrdma->s_ctx->port_attr)) {
            fprintf(stderr, "ibv_query_port on port %u failed\n", ib_port);
            continue;
        }
        fprintf(stderr, "%s phy %d\n", ibv_get_device_name(dev_list[i]), rrdma->s_ctx->port_attr.phys_state);
        if( rrdma->s_ctx->port_attr.state == IBV_PORT_ACTIVE ){
            break;
        }
	}


        //	/* We are now done with device list, free it */
        ibv_free_device_list(dev_list);
        dev_list = NULL;
        ib_dev = NULL;

        /* query port properties  */
        if (ibv_query_gid(rrdma->s_ctx->ctx, ib_port, ib_gid, &rrdma->s_ctx->gid)) {
            fprintf(stderr, "ibv_query_gid on port %u gid %d failed\n", ib_port, ib_gid);
            assert(false);
        }
        rrdma->s_ctx->gidIndex = ib_gid;
        
    }

    socket::~socket(){
        
        delete rrdma;


    }

    socket::bind(const char *addr){
        char* ip_addr;
        int port;
        seperate_addr(addr, ip_addr, port);
        // bind TCP port for data exchange
        

    }

    char* socket::seperate_addr(const char *addr,  char* ip_addr, int& port_number){
        int i=0;
        char temp_str[20]

        while(addr[i]!='\0'){
            if(addr[i]=='/'){
                i+=2;
                int j=0;
                while(addr[i+j]!=':'){
                    temp_str[j]=addr[i+j];
                    j++;
                }
                temp_str[j]='\0';
                i=i+j+;
                port_number=atoi(&addr[i]);
                strcpy(ip_addr, temp_str);
                break;
            }
            else i++;
        }
    }
    

}