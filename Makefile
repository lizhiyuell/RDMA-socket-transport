CC := g++
FLAG := -g
.PHONY: clean

all:server client
server:server.o rdma.o
    $(CC) $(FLAG) -o $@ $^
client:client.o rdma.o
    $(CC) $(FLAG) -o $@ $^

clean:all
    rm -r server client *.o