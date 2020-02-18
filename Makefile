CC := g++
CPPFLAGS := -g
LDLIBS := ${LDLIBS} -libverbs -lpthread -lrt

.PHONY: clean

all:server client
server:server.o rdma.o sock.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

client:client.o rdma.o sock.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

clean:all
	rm -r server client *.o
