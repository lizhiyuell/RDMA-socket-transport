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
#include <functional>
#include <stack>
#include <map>
#include "semaphore.h"
#include <queue>


// RDMA definition
#define ib_port 1
// #define ib_gid 3
#define qp_size 500 // maximum of outstanding send/recv requests
#define cq_size 500
#define TEST_Z(x) assert(x)
#define TEST_NZ(x) assert(!x)
#define __polling
#define BufferSize MSG_SIZE_MAX  // send/recv  size for each node
#define POLL_SIZE 50
#define MAX_CQ_NUM 100
#define MSG_SIZE_MAX 4096



int recv_poll_size = POLL_SIZE;
int send_poll_size = POLL_SIZE;
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

	typedef unsigned long long ull;

	struct connection{
		struct ibv_context *ctx;
		struct ibv_pd *pd;
		// struct ibv_cq *cq_data, *cq_ctrl, *cq_mem;
		struct ibv_cq* send_cq;
		struct ibv_cq* recv_cq;
		struct ibv_comp_channel *comp_channel;
		struct ibv_port_attr		port_attr;	
		int gidIndex;
		union ibv_gid gid;
	};

	struct memory_management{
	// 自 己写的rdma内存管理控制结构体
	// only one of them is useful for each QP
	struct ibv_mr *rdma_send_mr;
	struct ibv_mr *rdma_recv_mr;

	struct ibv_mr peer_mr;
	
	char *rdma_send_region; 
	char *rdma_recv_region; // per-allocate multiple region( the number is  RECV_BUF_NUM) for recv

	// memory poll related
	sem_t mutex_send;

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
		int send_count; // count for send request to pull send cq
	}rdma_m;
	
	struct cm_con_data_t {
		uint32_t 			qp_num;		/* QP number */
		uint16_t 			lid;		/* LID of the IB port */
		uint8_t     		remoteGid[16];  /* GID  */
	};

	// build for multi-thread coding
	typedef struct thread_param {
		const char * addr;
		class socket * sock_ptr;
	}param_t;
	// ---

	void* bind_thread_func(void * args);

	void* connect_thread_func(void * args);

	class socket{
	public:
		// initialize socket
		socket(uint64_t gid);// finish local configuration
		
		~socket();

		int bind(const char *addr); // bind TCP listening port, and finish the remaining parts

		int connect(const char *addr); // connect to remote port, and finish the remaining part

		int send (const void *buf, size_t len, int _flag);

		int recv (void *buf, size_t len, int _flag); // <=0 for failure. result >=0 means the number of cq polled

		void inner_bind( const char *addr ); // bind function called by a new thread

		void inner_connect( const char *addr );
		

	private:
		rdma_m* rrdma;
		pthread_t bind_thread, connect_thread;
		int connect_flag; // a flag to show whether connection has been built. 0 for no, 1 for bind and 2 for connect
		param_t param_bind, param_connect;
		char ip_addr_temp[50];
		// socket infomation
		int sock_port;
		char sock_addr[50];
		// memory poll index stack
		std::queue<int> send_poll_queue;

		int send_flow_control;
		int recv_flow_control;

		int sock;  // sock to exchange data with the remote side
		int ib_gid;

		void seperate_addr(const char *addr, char* &ip_addr, int& port_number);

		void qp_connection(int is_server);

		int modify_qp_to_init(struct  ibv_qp* qp);

		int modify_qp_to_rtr(struct ibv_qp *qp,	uint32_t remote_qpn, uint16_t dlid, uint8_t *remoteGid, struct connection *s_ctx);
		
		int modify_qp_to_rts(struct ibv_qp *qp);

		void fillAhAttr(ibv_ah_attr *attr, uint32_t remoteLid, uint8_t *remoteGid, struct connection *s_ctx);
		
		void post_recv( ull tid, int recv_size);
		
		void post_send( ull tid, int send_size, int imm_data );

		int get_wc ( struct ibv_wc *wc, int is_recv );

	};




}


#endif
