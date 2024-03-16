TARGETS = tcp_client tcp_server

# Targets to build when running `make`
all: $(TARGETS)

# Phony targets. Executed always.
.PHONY: all clean

# Targets to build `project` binary
tcp_client: tcp_client.o
	gcc tcp_client.o -o tcp_client
tcp_client.o: tcp_client.c
	gcc tcp_client.c -c -o tcp_client.o

tcp_server: tcp_server.o
	gcc tcp_server.o -o tcp_server
tcp_server.o: tcp_server.c
	gcc tcp_server.c -c -o tcp_server.o

# Run `make clean` to delete binaries and object files.
clean:
	rm -rf $(TARGETS) *.o

