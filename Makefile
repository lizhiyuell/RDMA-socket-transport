CC := g++
JEMALLOC:=/root/jty/jemalloc-4.5.0
NNMSG := ./nanomsg-1.0.0
LDLIBS := ${LDLIBS} -libverbs -lpthread -lrt -lnanomsg -ljemalloc
LDFLAGS := -L$(NNMSG) -I$(JEMALLOC)/include
CPPFLAGS := -g -std=c++11 $(LDFLAGS)


.PHONY: clean

all:server client
server:server.o rdma.o sock.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

client:client.o rdma.o sock.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

clean:all
	rm -r server client *.o
