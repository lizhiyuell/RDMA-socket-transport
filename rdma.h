#ifndef _RDMA_H_
#define _RDMA_H_

#include <rdma/rdma_verbs.h>
#include "sock.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

// RDMA definition
#define ib_port 1
// #define ib_gid 3
#define qp_size 500 // maximum of outstanding send/recv requests
#define cq_size 500
#define TEST_Z(x) assert(!x)
#define TEST_NZ(x) assert(x)
#define __polling
#define BufferSize 2048  // send/recv buffer size for each node
// RDMA definition

namespace rdma{

	/*
	struct ScatterList
	{
		struct ScatterList *next;  //链表的形式实现
		void *address;             //数据存放区域的页内地址
		int length;                //数据存放区域的长度
	};
	*/

	struct connection{
		struct ibv_context *ctx;
		struct ibv_pd *pd;
		// struct ibv_cq *cq_data, *cq_ctrl, *cq_mem;
		// struct ibv_cq send_cq;
		struct ibv_cq recv_cq;
		struct ibv_comp_channel *comp_channel, *mem_channel;
		struct ibv_port_attr		port_attr;	
		int gidIndex;
		union ibv_gid gid;
	};

	struct memory_management{
	// 自己写的rdma内存管理控制结构体
	// only one of them is useful for each QP
	struct ibv_mr *rdma_send_mr;
	struct ibv_mr *rdma_recv_mr;

	struct ibv_mr peer_mr;
	
	char *rdma_send_region;
	char *rdma_recv_region;	
	};

	/*
	struct qp_management{
	// 管理待处理的请求的结构体
	int data_num;
	int ctrl_num;
	struct ibv_qp *qp; // 链接对的数组，应该是为了并行处理请求所以才放了这么多。由于我们的qp管理不在这一层，就不在这里写了
							// 后面的代码显示只用了第一个，应该只是为了好的扩展性才这么写的
	};
	*/

	typedef struct rdma_management
	{
		struct connection *s_ctx;
		struct memory_management *memgt;
		// struct qp_management *qpmgt;
		struct ibv_qp *qp;
	}rdma_m;
	
	struct cm_con_data_t {
		uint32_t 			qp_num;		/* QP number */
		uint16_t 			lid;		/* LID of the IB port */
		uint8_t     		remoteGid[16];  /* GID  */
	};


	class socket{
	public:
		// initialize socket
		socket(int gid);// finish local configuration
		
		~socket();

		int bind(const char *addr); // bind TCP listening port, and finish the remaining parts

		int connect(const char *addr); // connect to remote port, and finish the remaining part

		int send (const void *buf, size_t len);

		int recv (void *buf, size_t len);

		

	private:
		rdma_m* rrdma;
		int sock;  // sock to exchange data with the remote side
		int ib_gid;

		void seperate_addr(const char *addr, char *ip_addr, int& port_number);

		void qp_connection(int is_server);

		int modify_qp_to_init(struct  ibv_qp* qp);

		int modify_qp_to_rtr(struct ibv_qp *qp,	uint32_t remote_qpn, uint16_t dlid, uint8_t *remoteGid, struct connection *s_ctx);
		
		int modify_qp_to_rts(struct ibv_qp *qp);
		
		void post_recv( ull tid, int recv_size);
		
		void post_send( ull tid, int send_size, int imm_data );

	};

}


#endif
