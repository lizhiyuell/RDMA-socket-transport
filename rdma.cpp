#include "rdma.h"

namespace rdma{

    socket::socket(int a, int b){
        
        this->rrdma = ( rdma_m* ) malloc( sizeof( rdma_m ) );

        this->rrdma->s_ctx = ( struct connection * )malloc( sizeof( struct connection ) );
    
        struct ibv_device  **dev_list = NULL;
        struct ibv_qp_init_attr qp_init_attr;
        struct ibv_device 	*ib_dev = NULL;
        size_t 			size;
        int 			i;
        int 			mr_flags = 0;
        int 			cq_size = 0;
        int 			num_devices;
        char *dev_name = NULL;
        dev_list = ibv_get_device_list(&num_devices);
        if (!dev_list) {
            fprintf(stderr, "failed to get IB devices list\n");
            return ;
        }

        /* if there isn't any IB device in host */
        if (!num_devices) {
            fprintf(stderr, "found %d device(s)\n", num_devices);
            return ;
        }

        fprintf(stdout, "found %d device(s)\n", num_devices);

        }

    socket::~socket(){
        delete(this->rrdma);
    }



}