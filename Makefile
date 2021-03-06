::Makefile for Assignment 2

# Update following lines as needed.
# You may change this file as you wish, as long as your Makefile can generate three binaries (server, client, and client_multi).


SERVER_FORK_SRC = src/webserver_fork.c
SERVER_FORK_HDR = 

SERVER_THREAD_SRC = src/webserver_thread.c
SERVER_THREAD_HDR =

SERVER_LIBEVENT_SRC = src/webserver_libevent.c
SERVER_LIBEVENT_HDR =

##############################

CC=gcc
CFLAGS=-I. -g
LDFLAGS= -lpthread -levent -levent_core
LIBS_PATH= -L/usr/local/lib

build: bin/webserver_fork bin/webserver_thread bin/webserver_libevent

bin/webserver_fork: $(SERVER_FORK_SRC) $(SERVER_FORK_HDR) bin
	$(CC) $(CFLAGS) -o $@ $(SERVER_FORK_SRC) $(LIBS_PATH) $(LDFLAGS)

bin/webserver_thread: $(SERVER_THREAD_SRC) $(SERVER_THREAD_HDR) bin
	$(CC) $(CFLAGS) -o $@ $(SERVER_THREAD_SRC) $(LIBS_PATH) $(LDFLAGS)

bin/webserver_libevent: $(SERVER_LIBEVENT_SRC) $(SERVER_LIBEVENT_HDR) bin
	$(CC) $(CFLAGS) -o $@ $(SERVER_LIBEVENT_SRC) $(LIBS_PATH) $(LDFLAGS)

.PHONY: clean build

bin:
	mkdir -p bin

clean:
	rm -rf bin
