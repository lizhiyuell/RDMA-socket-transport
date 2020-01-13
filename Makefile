CC := g++
FLAG := -g
LDLIBS := ${LDLIBS} -libverbs -lpthread -lrt

.PHONY: clean

all:server client
server:server.o rdma.o sock.o
	$(CC) $(FLAG) -o $@ $^ $(LDLIBS)

client:client.o rdma.o sock.o
	$(CC) $(FLAG) -o $@ $^ $(LDLIBS)

clean:all
	rm -r server client *.o
