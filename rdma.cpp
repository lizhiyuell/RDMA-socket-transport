#include "rdma.h"
/*
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <rdma/rdma_verbs.h>
*/

namespace rdma{

    socket::socket(int gid){
        ib_gid = gid;
        is_server = -1;
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

        if(is_server==-1){
            delete rrdma;
            return;
        }

        // destroy qp management
        ibv_destroy_qp(rrdma->qp);
        
        // destroy memory management

        TEST_NZ(ibv_dereg_mr(rrdma->memgt->rdma_send_mr));
        free(rrdma->memgt->rdma_send_region);  rrdma->memgt->rdma_send_region = NULL;

        TEST_NZ(ibv_dereg_mr(rrdma->memgt->rdma_recv_mr));
        free(rrdma->memgt->rdma_recv_region);  rrdma->memgt->rdma_recv_region = NULL;
        free(rrdma->memgt); rrdma->memgt = NULL;

        // destory connection
        TEST_NZ(ibv_destroy_cq(rrdma->s_ctx->recv_cq));
        TEST_NZ(ibv_destroy_comp_channel(rrdma->s_ctx->comp_channel));

        ibv_dealloc_pd(rrdma->s_ctx->pd);
	    ibv_close_device(rrdma->s_ctx->ctx);

        free(rrdma->s_ctx);
        delete rrdma;
    }

    int socket::bind(const char *addr){

        // bind TCP port for data exchange
        is_server = 1;
        char* ip_addr;
        int bind_port;
        seperate_addr(addr, ip_addr, bind_port);
        sock = sock_daemon_connect(bind_port);
        if (sock < 0) {
            fprintf(stderr, "failed to establish TCP connection with client on port %d\n", bind_port);
            return -1;
        }
        fprintf(stdout, "TCP connection was established\n");

        qp_connection(1)

        post_send( 50, sizeof(struct ibv_mr), 0 );  
        int ss = get_wc( rrdma, &wc );
        
        // printf("add: %p length: %d\n", rrdma->memgt->rdma_recv_mr->addr, rrdma->memgt->rdma_recv_mr->length);
        
        post_recv( 20, sizeof(struct ibv_mr));
        ss = get_wc( rrdma, &wc );
        memcpy( &rrdma->memgt->peer_mr, rrdma->memgt->rdma_recv_region, sizeof(struct ibv_mr) );
        // printf("peer add: %p length: %d\n", rrdma->memgt->peer_mr.addr, rrdma->memgt->peer_mr.length);

        printf("bind port success with remote side\n");
        // add additional recv
        for( int i = 0; i < 10; i ++ ){
            struct ibv_recv_wr wr, *bad_wr = NULL;
            struct ibv_sge sge;
            wr.wr_id = i;
            wr.next = NULL;
            wr.sg_list = &sge;
            wr.num_sge = 1;

            sge.addr = (uintptr_t)rrdma->memgt->rdma_recv_region;		
            sge.length = BufferSize;
            sge.lkey = rrdma->memgt->rdma_recv_mr->lkey;

            TEST_NZ(ibv_post_recv(rrdma->qp, &wr, &bad_wr));
        }

        return 1;
}
    int socket::connect(const char* addr){
        is_server = 0;
        char* ip_addr;
        int connect_port;
        seperate_addr(addr, ip_addr, connect_port);
        int connect_count = 0;
        while (1) {
            if (sock>=0) break;
            sock = sock_client_connect(ip_addr, connect_port);
            fprintf(stdout, "failed to establish TCP connection to server %s, port %d. Try another time ...\n", ip_addr, connect_port);
            sleep(3);
            connect_count ++;
            if(connect_count >= 5){
                fprintf(stderr, "failed to establish TCP connection with client on port %d\n", bind_port);
                return -1;
            }
        }
        fprintf(stdout, "TCP connection was established\n");

        qp_connection(0);

        post_recv( 20, sizeof(struct ibv_mr)); 
        
        int tmp = get_wc( rrdma, &wc ); 
        memcpy( &rrdma->memgt->peer_mr, rrdma->memgt->rdma_recv_region, sizeof(struct ibv_mr) );
        
        memcpy( rrdma->memgt->rdma_send_region, rrdma->memgt->rdma_recv_mr, sizeof(struct ibv_mr) );
        post_send( 50, sizeof(struct ibv_mr), 0 );
        
        int ss = get_wc( rrdma, &wc );
        
        printf("connect port success with remote side\n");
        // add additional recv
        for( int i = 0; i < 10; i ++ ){
            struct ibv_recv_wr wr, *bad_wr = NULL;
            struct ibv_sge sge;
            wr.wr_id = i;
            wr.next = NULL;
            wr.sg_list = &sge;
            wr.num_sge = 1;

            sge.addr = (uintptr_t)rrdma->memgt->rdma_recv_region;		
            sge.length = BufferSize;
            sge.lkey = rrdma->memgt->rdma_recv_mr->lkey;

            TEST_NZ(ibv_post_recv(rrdma->qp, &wr, &bad_wr));
        }

        return 1;
    }

    socket::qp_connection(int is_server){
        // initialization
        struct ibv_qp_init_attr *qp_attr;
        qp_attr = ( struct ibv_qp_init_attr* )malloc( sizeof( struct ibv_qp_init_attr ) );

        // build_context
        TEST_Z(rrdma->s_ctx->pd = ibv_alloc_pd(rrdma->s_ctx->ctx));
        TEST_Z(rrdma->s_ctx->comp_channel = ibv_create_comp_channel(rrdma->s_ctx->ctx));
        // TEST_Z(rrdma->s_ctx->mem_channel = ibv_create_comp_channel(rrdma->s_ctx->ctx));
        /* pay attention to size of CQ */
        // rrdma->s_ctx->cq_mem = (struct ibv_cq **)malloc(sizeof(struct ibv_cq *)*2);
        
        // TEST_Z(rrdma->s_ctx->cq_data = ibv_create_cq(rrdma->s_ctx->ctx, cq_size, NULL, rrdma->s_ctx->comp_channel, 0)); 
        TEST_Z(rrdma->s_ctx->send_cq = ibv_create_cq(rrdma->s_ctx->ctx, cq_size, NULL, rrdma->s_ctx->comp_channel, 0)); 
        TEST_Z(rrdma->s_ctx->recv_cq = ibv_create_cq(rrdma->s_ctx->ctx, cq_size, NULL, rrdma->s_ctx->comp_channel, 0)); 

        #ifndef __polling			
                TEST_NZ(ibv_req_notify_cq(s_ctx->send_cq, 0));
                TEST_NZ(ibv_req_notify_cq(s_ctx->recv_cq, 0));
        #endif

        // allocate memory
        rrdma->memgt->rdma_recv_region = (char *) malloc(BufferSize);
        rrdma->memgt->rdma_send_region = (char *) malloc(BufferSize);
        
        // register memory for RDMA
        TEST_Z( rrdma->memgt->rdma_recv_mr = ibv_reg_mr( rrdma->s_ctx->pd, rrdma->memgt->rdma_recv_region, BufferSize, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ) );
        TEST_Z( rrdma->memgt->rdma_send_mr = ibv_reg_mr( rrdma->s_ctx->pd, rrdma->memgt->rdma_send_region, BufferSize, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ) );

        // rdma->qpmgt->data_num = connect_number-ctrl_number;
        // rdma->qpmgt->ctrl_num = ctrl_number;
        memset(qp_attr, 0, sizeof(*qp_attr));
        qp_attr->qp_type = IBV_QPT_RC;
        qp_attr->send_cq = rrdma->s_ctx->send_cq;
        qp_attr->recv_cq = rrdma->s_ctx->recv_cq;

        qp_attr->cap.max_send_wr = qp_size;
        qp_attr->cap.max_recv_wr = qp_size;
        qp_attr->cap.max_send_sge = 20;
        qp_attr->cap.max_recv_sge = 20;
        qp_attr->cap.max_inline_data = 200; // max size in byte of inline data on the send queue
        
        qp_attr->sq_sig_all = 1; // set as 1 to generate CQE from all WQ
        
        struct ibv_qp *myqp = ibv_create_qp( rrdma->s_ctx->pd, qp_attr );
        rrdma->qp = myqp;
        // ----------------------------
        // connect_qp( rrdma, myqp, tid );
        struct cm_con_data_t 	local_con_data;
	    struct cm_con_data_t 	remote_con_data;
	    struct cm_con_data_t 	tmp_con_data;
	    int 			rc;

        rc = modify_qp_to_init(myqp);
        if (rc) {
            fprintf(stderr, "change QP state to INIT failed\n");
            assert(false);
        }

        local_con_data.qp_num = myqp->qp_num;
        local_con_data.lid    = rrdma->s_ctx->port_attr.lid;
        memcpy( local_con_data.remoteGid, rrdma->s_ctx->gid.raw, 16*sizeof(uint8_t) );

        // exchange local data with remote side
        if (sock_sync_data(sock, is_server, sizeof(struct cm_con_data_t), &local_con_data, &tmp_con_data) < 0) {
            fprintf(stderr, "failed to exchange connection data between sides\n");
            assert(false);
        }

        remote_con_data.qp_num = tmp_con_data.qp_num;
        remote_con_data.lid    = tmp_con_data.lid;
        memcpy( remote_con_data.remoteGid, tmp_con_data.remoteGid, 16*sizeof(uint8_t) );

        rc = modify_qp_to_rtr(myqp, remote_con_data.qp_num, remote_con_data.lid, remote_con_data.remoteGid, rrdma->s_ctx);
        if (rc) {
            fprintf(stderr, "failed to modify QP state from RESET to RTS\n");
            assert(false);
        }

        /* only the daemon post SR, so only he should be in RTS
        (the client can be moved to RTS as well)
        */
        if (0)
            fprintf(stdout, "QP state was change to RTR\n");
        else {
            rc = modify_qp_to_rts(myqp);
            if (rc) {
                fprintf(stderr, "failed to modify QP state from RESET to RTS\n");
                assert(false);
            }

            fprintf(stdout, "QP state was change to RTS\n");
        }

        // -----------------------------------
        /* sync to make sure that both sides are in states that they can connect to prevent packet loose */
        if (sock_sync_ready(sock, is_server)) {
            fprintf(stderr, "sync after QPs are were moved to RTS\n");
            assert(false)
        }
        // -----------------------------------
    }


    int socket::modify_qp_to_init(struct  ibv_qp* qp){
	
    struct ibv_qp_attr 	attr;
	int 			flags;
	int 			rc;


	/* do the following QP transition: RESET -> INIT */
	memset(&attr, 0, sizeof(attr));

	attr.qp_state 	= IBV_QPS_INIT;
	attr.port_num 	= ib_port;
	attr.pkey_index = 0;

	/* we don't do any RDMA operation, so remote operation is not permitted */
	attr.qp_access_flags = IBV_ACCESS_REMOTE_WRITE  | IBV_ACCESS_REMOTE_READ  | IBV_ACCESS_REMOTE_ATOMIC ;

	flags = IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS;

	rc = ibv_modify_qp(qp, &attr, flags);
	if (rc) {
		fprintf(stderr, "failed to modify QP state to INIT\n");
		return rc;
	}

	return 0;

    }

    socket::post_recv( ull tid, int recv_size)
    {
        //ibv_recv_wr 用来布置receiver request（RR）。RR定义了非RDMA操作的buffer
        struct ibv_recv_wr wr, *bad_wr = NULL;
        struct ibv_sge sge;
        
        //wr 是放到qp里的，这里是新建了一个work request
        wr.wr_id = tid;
        wr.next = NULL;
        wr.sg_list = &sge;
        wr.num_sge = 1;
        // 用来存放数据有关的信息，如地址、长度以及钥匙
        // 这样来看这个rec_buffer应该是本地用来存储对方内存位置信息的内存
        sge.addr = (uintptr_t)rrdma->memgt->rdma_recv_region;
        sge.length = recv_size;
        sge.lkey = rrdma->memgt->rdma_recv_mr->lkey;
        
        TEST_NZ(ibv_post_recv(rrdma->qp, &wr, &bad_wr));
        //TEST_NZ是一个定义的函数，如果内部不为0，则返回错误。
        //ibv_post_recv返回0时表示成功，返回-1时表示错误。好像是用来将一个工作请求（work request）放到队列对（QP）的，大概的用处应该就是添加一个传送任务进去
    }

    socket::post_send( ull tid, int send_size, int imm_data )
    {
        struct ibv_send_wr wr, *bad_wr = NULL;
        struct ibv_sge sge;

        memset(&wr, 0, sizeof(wr));
        
        wr.wr_id = tid;
        wr.opcode = IBV_WR_SEND_WITH_IMM;  // 写操作里开始有opcode了   ---这里用了有立即数的写，后面需要对其作修改
        wr.sg_list = &sge;
        wr.send_flags = IBV_SEND_SIGNALED; // 给WR的发送完成flag
        if( imm_data != 0 )// 应该是发送可以选择的立即数项
            wr.imm_data = imm_data;
        wr.num_sge = 1;
        // 这里为什么又是send_buffer了  这只是个名字好看，但是发的的确是本地存储数据的位置
        sge.addr = (uintptr_t)rrdma->memgt->rdma_send_region;
        sge.length = send_size;
        sge.lkey = rrdma->memgt->rdma_send_mr->lkey;
        
        TEST_NZ(ibv_post_send(rrdma->qp, &wr, &bad_wr));
    }

    int socket::send(const void *buf, size_t len){
        struct ibv_send_wr swr, *sbad_wr = NULL;
        struct ibv_sge sge;
        struct ibv_cq *cq;

        memset(&swr, 0, sizeof(swr));
        swr.wr_id = 0;
		swr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
		swr.sg_list = &sge;
		swr.send_flags = IBV_SEND_SIGNALED;
		swr.num_sge = 1;
		swr.wr.rdma.remote_addr = (uintptr_t)rrdma->memgt->peer_mr.addr;
		swr.wr.rdma.rkey = rrdma->memgt->peer_mr.rkey;

        memcpy(rrdma->memgt->rdma_send_region, buf, len);
        sge.addr = (uintptr_t)buf;
		sge.length = len;
		sge.lkey = rrdma->memgt->rdma_send_mr->lkey;

        TEST_NZ(ibv_post_send(rrdma->qp, &swr, &sbad_wr));

        return 1;
    }

    int socket::recv(void *buf,  size_t len){
        struct ibv_wc *wc, *wc_array;
        struct ibv_cq *cq;

        wc_array = ( struct ibv* ) malloc( sizeof(struct ibv_wc) * 20 );
        cq = rrdma->s_ctx->recv_cq;

        int flag=1;
        while(flag){
            int num = ibv_poll_cq(cq, 10, wc_array);
            if( num<0 ) continue;
            for( int k = 0; k < num; k ++ ){
				wc = &wc_array[k];
				if( wc->opcode == IBV_WC_RECV || wc->opcode == IBV_WC_RECV_RDMA_WITH_IMM ){
					if( wc->status != IBV_WC_SUCCESS ){
						printf("recv error %d!\n", 0);

					}				
					// printf("back message received\n");
					flag = 0;
					struct ibv_recv_wr wr, *bad_wr = NULL;
					struct ibv_sge sge;
					wr.wr_id = wc->wr_id;
					wr.next = NULL;
					wr.sg_list = &sge;
					wr.num_sge = 1;

					sge.addr = (uintptr_t)rrdma->memgt->rdma_recv_region;
					sge.length = BufferSize;
					sge.lkey = rrdma->memgt->rdma_recv_mr->lkey;

					TEST_NZ(ibv_post_recv(rrdma->qp, &wr, &bad_wr));
					break;
				}
			}
        }

        memcpy(buf, rrdma->memgt->rdma_recv_region, len);

        return 1;
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