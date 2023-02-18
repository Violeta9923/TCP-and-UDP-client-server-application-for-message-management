CC=g++ -std=c++11 -Wall -Wextra

all: build

build: server subscriber

server: utils.o server.o
	$(CC) $^ -o $@

utils.o: utils.c
	$(CC) $^ -c
server.o: server.c
	$(CC) $^ -c
	
subscriber: TCPclient.o
	$(CC) $^ -o $@

subscriber.o: TCPclient.c
	$(CC) $^ -c
clean:
	rm *.o server subscriber