# 
# This make file builds the target server and client  
# uses : make all; make;  make server;  make client 
# To clean the targets use : make clean
#
#
CC = gcc
CFLAGS  = -g -Wall -ljansson

FLAG = -DDEBUG_FLAG 

CFLAGS += $(FLAG)

all: server client

server:  epoll_server.o server_utils.o server_socket.o list.o common.o
	$(CC) $(CFLAGS) -o server epoll_server.o server_utils.o server_socket.o list.o common.o -lpthread

epoll_server.o:  epoll_server.c server.h common.h
	$(CC) $(CFLAGS) -c epoll_server.c

server_socket.o: server_socket.c server.h
	$(CC) $(CFLAGS) -c server_socket.c

server_utils.o: server_utils.c server.h common.h
	$(CC) $(CFLAGS) -c server_utils.c

list.o: list.c common.h
	$(CC) $(CFLAGS) -c list.c


client:  client.o common.o list.o
	$(CC) $(CFLAGS) -o client client.o common.o list.o -lpthread

client.o: client.c client.h common.h
	$(CC) $(CFLAGS) -c client.c

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c

# This make clean will 
# removes the executable file, as well as old .o object
#
clean: 
	-rm -f *.o *.core core.* *.h.gch server client
