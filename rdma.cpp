#include "rdma.h"
/*
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <rdma/rdma_verbs.h>
*/

using namespace rdma;

    socket::socket(int gid){

        connect_flag = 0;
        fprintf(stdout, "starting a new socket ...\n");
        rrdma = (rdma_m*) malloc( sizeof(rdma_m) );
        rrdma->s_ctx = ( struct connection * )malloc( sizeof( struct connection ) );
        rrdma->memgt = (struct memory_management *) malloc(sizeof( struct memory_management ));

        struct ibv_device **dev_list = NULL;
        int num_devices;        
        // struct ibv_qp_init_attr qp_init_attr;
        struct ibv_device *ib_dev = NULL;
        // size_t 	size;
        
        // int mr_flags = 0;
        // int cq_size = 0;
        
        char *dev_name = NULL;

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
            fprintf(stderr, "%s phy %d\n", ibv_get_device_name(dev_list[device_index]), rrdma->s_ctx->port_attr.phys_state);
            if( rrdma->s_ctx->port_attr.state == IBV_PORT_ACTIVE ){
                break;
            }
        }


        //	/* We are now done with device list, free it */
        // ibv_free_device_list(dev_list);
        // dev_list = NULL;
        // ib_dev = NULL;

        /* query port properties  */
        if (ibv_query_gid(rrdma->s_ctx->ctx, ib_port, gid, &rrdma->s_ctx->gid)) {
            fprintf(stderr, "ibv_query_gid on port %u gid %d failed\n", ib_port, gid);
            assert(false);
        }
        rrdma->s_ctx->gidIndex = gid;
    }


    socket::~socket(){

        // join the threads
        pthread_join( bind_thread, NULL );
        pthread_join( connect_thread, NULL );

        fprintf(stdout, "destroying current socket ...\n");
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
        TEST_NZ(ibv_destroy_cq(rrdma->s_ctx->send_cq));
        TEST_NZ(ibv_destroy_comp_channel(rrdma->s_ctx->comp_channel));
        ibv_dealloc_pd(rrdma->s_ctx->pd);
	    ibv_close_device(rrdma->s_ctx->ctx);
        free(rrdma->s_ctx);
        free(rrdma);

    }

    void* rdma::bind_thread_func(void *args){
        param_t *param = (param_t *) args;
        param->sock_ptr->inner_bind(param->addr);
    }

    int socket::bind( const char *addr ){

        fprintf(stdout, "running bind function in the background\n");
        this->param_bind.addr = addr;
        this->param_bind.sock_ptr = this;
        pthread_create( &bind_thread, NULL, rdma::bind_thread_func, (void *) &this->param_bind);
        return 0;
    }

    void socket::inner_bind( const char *addr ){

        fprintf(stdout, "starting binding port on server side ...\n");
        // bind TCP port for data exchange
        char* ip_addr;
        int bind_port;

        // before seperate addr
        seperate_addr(addr, ip_addr, bind_port);
        fprintf(stdout, "[Debug] After seperate function\n");
        free(ip_addr);
        ip_addr = NULL;

        // fprintf(stdout, "seperate port number is %d\n", bind_port);
        sock = sock_daemon_connect(bind_port);
        // fprintf(stdout, "sock number is %d\n", sock);

        if (sock < 0) {
            fprintf(stderr, "failed to establish TCP connection with client on port %d\n", bind_port);
            assert(false);
        }
        fprintf(stdout, "TCP connection was established\n");

        qp_connection(1);

        struct ibv_wc wc;
        memcpy(rrdma->memgt->rdma_send_region, rrdma->memgt->rdma_recv_mr, sizeof(struct ibv_mr));
        post_send( 50, sizeof(struct ibv_mr), 0 );  
        int ss = get_wc( &wc, 0 );
        post_recv( 20, sizeof(struct ibv_mr));
        ss = get_wc( &wc, 1 );
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

        connect_flag = 1;
}

    void* rdma::connect_thread_func(void *args){
        param_t *param = (param_t *) args;
        param->sock_ptr->inner_connect(param->addr);
}

    int socket::connect( const char *addr ){

        fprintf(stdout, "running connect function in the background\n");
        this->param_connect.addr = addr;
        this->param_connect.sock_ptr = this;
        pthread_create( &connect_thread, NULL, rdma::connect_thread_func, (void *) &this->param_connect);
        return 0;
}

    void socket::inner_connect( const char *addr ){

        fprintf(stdout, "starting connecting to the remote side ...\n");
        char* ip_addr;
        int connect_port;
        seperate_addr(addr, ip_addr, connect_port);
        int connect_count = 0;
        sock = -1;
        while (1) {
            sock = sock_client_connect(ip_addr, connect_port);
            if (sock>=0) break;
            fprintf(stdout, "failed to establish TCP connection to server %s, port %d. Try another time ...\n", ip_addr, connect_port);
            sleep(3);
            connect_count ++;
            if(connect_count >= 10){
                fprintf(stderr, "failed to establish TCP connection with client on port %d\n", connect_port);
                assert(false);
            }
        }
        fprintf(stdout, "TCP connection was established\n");
        free(ip_addr);
        ip_addr = NULL;

        qp_connection(0);

        struct ibv_wc wc;
        post_recv( 20, sizeof(struct ibv_mr)); 
        int tmp = get_wc( &wc, 1 ); 
        memcpy( &rrdma->memgt->peer_mr, rrdma->memgt->rdma_recv_region, sizeof(struct ibv_mr) );
        memcpy( rrdma->memgt->rdma_send_region, rrdma->memgt->rdma_recv_mr, sizeof(struct ibv_mr) );
        post_send( 50, sizeof(struct ibv_mr), 0 );
        int ss = get_wc( &wc, 0 );  // problem exists here.
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

        connect_flag = 1;
    }

    void socket::qp_connection(int is_server){

        fprintf(stdout, "starting qp connection ...\n");
        // build_context
        rrdma->s_ctx->pd = ibv_alloc_pd(rrdma->s_ctx->ctx);
        TEST_Z(rrdma->s_ctx->pd);

        rrdma->s_ctx->comp_channel = ibv_create_comp_channel(rrdma->s_ctx->ctx);
        TEST_Z(rrdma->s_ctx->comp_channel);
        // TEST_Z(rrdma->s_ctx->mem_channel = ibv_create_comp_channel(rrdma->s_ctx->ctx));
        /* pay attention to size of CQ */
        // rrdma->s_ctx->cq_mem = (struct ibv_cq **)malloc(sizeof(struct ibv_cq *)*2);
        
        // TEST_Z(rrdma->s_ctx->cq_data = ibv_create_cq(rrdma->s_ctx->ctx, cq_size, NULL, rrdma->s_ctx->comp_channel, 0)); 
        rrdma->s_ctx->send_cq = ibv_create_cq(rrdma->s_ctx->ctx, cq_size, NULL, rrdma->s_ctx->comp_channel, 0);
        TEST_Z(rrdma->s_ctx->send_cq); 
        rrdma->s_ctx->recv_cq = ibv_create_cq(rrdma->s_ctx->ctx, cq_size, NULL, rrdma->s_ctx->comp_channel, 0); 
        TEST_Z(rrdma->s_ctx->recv_cq); 
        #ifndef __polling			
                TEST_NZ(ibv_req_notify_cq(s_ctx->send_cq, 0));
                TEST_NZ(ibv_req_notify_cq(s_ctx->recv_cq, 0));
        #endif
        // allocate memory
        rrdma->memgt->rdma_recv_region = (char *) malloc(BufferSize);
        rrdma->memgt->rdma_send_region = (char *) malloc(BufferSize);
        // register memory for RDMA
        rrdma->memgt->rdma_recv_mr = ibv_reg_mr( rrdma->s_ctx->pd, rrdma->memgt->rdma_recv_region, BufferSize, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
        TEST_Z( rrdma->memgt->rdma_recv_mr );
        rrdma->memgt->rdma_send_mr = ibv_reg_mr( rrdma->s_ctx->pd, rrdma->memgt->rdma_send_region, BufferSize, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);

        // set qp attribution
        struct ibv_qp_init_attr *qp_attr;
        qp_attr = ( struct ibv_qp_init_attr* )malloc( sizeof( struct ibv_qp_init_attr ) );
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
        // connect qp
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
            assert(false);
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

    int socket::modify_qp_to_rtr(struct ibv_qp *qp,	uint32_t remote_qpn, uint16_t dlid, uint8_t *remoteGid, struct connection *s_ctx){
        
        struct ibv_qp_attr 	attr;
	    int 			flags;
        int 			rc;

        /* do the following QP transition: INIT -> RTR */
        memset(&attr, 0, sizeof(attr));

        attr.qp_state 			= IBV_QPS_RTR;
        attr.path_mtu 			= IBV_MTU_256;
        attr.dest_qp_num 		= remote_qpn;
        attr.rq_psn 			= 0;
        attr.max_dest_rd_atomic 	= 1;
        attr.min_rnr_timer 		= 0x12;
        
        this->fillAhAttr(&attr.ah_attr, dlid, remoteGid, s_ctx);

        flags = IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN | 
            IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER;

        rc = ibv_modify_qp(qp, &attr, flags);
        if (rc) {
            fprintf(stderr, "failed to modify QP state to RTR %d \n", rc);
            printf("%s\n", strerror(rc));
            return rc;
        }

        return 0;
    }

    int socket::modify_qp_to_rts(struct ibv_qp *qp){

        struct ibv_qp_attr 	attr;
        int 			flags;
        int 			rc;


        /* do the following QP transition: RTR -> RTS */
        memset(&attr, 0, sizeof(attr));

        attr.qp_state 		= IBV_QPS_RTS;
        attr.timeout 		= 0x12;
        attr.retry_cnt 		= 7;
        attr.rnr_retry 		= 7;
        attr.sq_psn 		= 0;
        attr.max_rd_atomic 	= 1;

        flags = IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT | 
            IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC;

        rc = ibv_modify_qp(qp, &attr, flags);
        if (rc) {
            fprintf(stderr, "failed to modify QP state to RTS\n");
            return rc;
        }

        return 0;

    }

    void socket::fillAhAttr(ibv_ah_attr *attr, uint32_t remoteLid, uint8_t *remoteGid, struct connection *s_ctx){

        memset(attr, 0, sizeof(ibv_ah_attr));
        attr->dlid = remoteLid;
        attr->sl = 0;
        attr->src_path_bits = 0;
        attr->port_num = ib_port;

        //attr->is_global = 0;

        // fill ah_attr with GRH
        
        attr->is_global = 1;
        memcpy(&attr->grh.dgid, remoteGid, 16);
        attr->grh.flow_label = 0;
        attr->grh.hop_limit = 1;
        attr->grh.sgid_index = s_ctx->gidIndex;
        attr->grh.traffic_class = 0;
    }

    void socket::post_recv( ull tid, int recv_size) // ok
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
  }

    void socket::post_send( ull tid, int send_size, int imm_data )
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

    int socket::send(const void *buf, size_t len){  // ok

        if(connect_flag == 0) return -1;
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
        sge.addr = (uintptr_t)rrdma->memgt->rdma_send_region;
		sge.length = len;
		sge.lkey = rrdma->memgt->rdma_send_mr->lkey;

        TEST_NZ(ibv_post_send(rrdma->qp, &swr, &sbad_wr));

        // test if rejected
        // if(sbad_wr != NULL){
        //     fprintf(stdout, "[Error] The wr is rejected!\n");
        // }

        return len;
    }

    int socket::recv(void *buf,  size_t len){  // ok
        struct ibv_wc* wc;
        struct ibv_wc* wc_array;
        struct ibv_cq *cq;

        wc_array = ( struct ibv_wc* ) malloc( sizeof(struct ibv_wc) * 20 );
        cq = rrdma->s_ctx->recv_cq;

        if(connect_flag == 0) return 0;

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

        return len;
    }

    void socket::seperate_addr(const char *addr,  char* &ip_addr, int& port_number){
        int i=0;
        char temp_str[20];
        // initialize
        fprintf(stdout, "[func] db 1\n");
        // for(int i=0;i<20;i++) temp_str[i]='\0';
        // fprintf(stdout, "[func] db 2\n");
        ip_addr = (char*) malloc(20);
        fprintf(stdout, "[func] db 3\n");
        while(addr[i]!='\0'){
            if(addr[i]=='/'){
                i+=2;
                int j=0;
                while(addr[i+j]!=':'){
                    temp_str[j]=addr[i+j];
                    j++;
                }
                temp_str[j]='\0';
                i=i+j+1;
                port_number=atoi(&addr[i]);
                strcpy(ip_addr, temp_str);
                break;
            }
            else i++;
        }
    }

    int socket::get_wc(struct ibv_wc *wc, int is_recv){
    
    // void *ctx;
	// struct ibv_cq *cq;
	int ret;
	while(1){
		if( is_recv ) ret = ibv_poll_cq(rrdma->s_ctx->recv_cq, 1, wc); //找cq里是否有完成的任务，有的话返回1
		else ret = ibv_poll_cq(rrdma->s_ctx->send_cq, 1, wc);
        if( ret > 0 ) break;
	}
	if( ret <= 0 || wc->status != IBV_WC_SUCCESS ){
		printf("get CQE fail: %d wr_id: %d\n", wc->status, (int)wc->wr_id);
		return -1;
	}
    #ifdef _TEST_SYN
        return wc->wr_id;
    #else
        return 0;
    #endif
    }
    

