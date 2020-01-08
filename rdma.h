#ifndef _RDMA_H_
#define _RDMA_H_

#include <rdma/rdma_verbs.h>
#include "sock.h"
#include <stdio.h>


#include <pthread.h>

namespace rdma{

	struct ScatterList
	{
		struct ScatterList *next;  //链表的形式实现
		void *address;             //数据存放区域的页内地址
		int length;                //数据存放区域的长度
	};

	struct connection{
		struct ibv_context *ctx;
		struct ibv_pd *pd;
		struct ibv_cq **cq_data, **cq_ctrl, **cq_mem;
		struct ibv_comp_channel *comp_channel, *mem_channel;
		struct ibv_port_attr		port_attr;	
		int gidIndex;
		union ibv_gid gid;
	};

	struct memory_management{
	// 自己写的rdma内存管理控制结构体
	int number;
	int memory_reback_flag;
	
	struct ScatterList application; // 数据的结构体，存储要收发的数据的信息，包括起始地址与长度
	
	struct ibv_mr *rdma_send_mr; // 内存登记  这里为发的数据以及将要接收的返回数据都登记了一下
	struct ibv_mr *rdma_recv_mr;
	// 与上面的区别是少了rdma。 上面带有rdma的recv是在初次建立链接时发送给对方的内存
	struct ibv_mr *send_mr;
	struct ibv_mr *recv_mr;
	// 存储对面的mr。 是为了能够直接访问对方的内存
	struct ibv_mr peer_mr;
	
	// 这两个只是在给对方传输这边的内存信息时用到了
	char *recv_buffer;
	char *send_buffer;
	
	char *rdma_send_region;
	char *rdma_recv_region;	
	
	struct bitmap *send, *peer[10], *peer_using[10], *peer_used[10]; // bitmap数据结构待定
	void **mapping_table[10];
	
	pthread_mutex_t mapping_mutex[10], flag_mutex;
	};

	struct qp_management{
	// 管理待处理的请求的结构体
	int data_num;
	int ctrl_num;
	struct ibv_qp *qp[128]; // 链接对的数组，应该是为了并行处理请求所以才放了这么多
							// 后面的代码显示只用了第一个，应该只是为了好的扩展性才这么写的
	};

	typedef struct rdma_management
	{
		struct connection *s_ctx;
		struct memory_management *memgt;
		struct qp_management *qpmgt;
	}rdma_m;
	
	class Node{
	public:
		Node();
		Node(){delete rrdma;}

	private:
			rdma_m* rrdma;
	};


	class socket{
	public:
		// initialize socket
		socket();
		
		~socket();

		int bind(const char *addr);

		int connect(const char *addr);

		int send (const void *buf, size_t len);

		int recv (void *buf, size_t len);

	
	private:
		 


		 
	
	};




}


#endif
