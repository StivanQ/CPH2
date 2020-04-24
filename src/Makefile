PORT = 9856
IP = "127.0.0.1"

all: server subscriber

run_server: server
	./server ${PORT}

run_client: subscriber
	./subscriber "marius" ${IP} ${PORT}

server: server.c list.c queue.c
	gcc -Wall server.c list.c queue.c -o server

subscriber: subscriber.c
	gcc -Wall subscriber.c -o subscriber

server.o:
	gcc -Wall -c -g server.c -o server.o

subscriber.o:
	gcc -Wall -c -g subscriber.c -o subscriber.o 

clean:
	rm -rf *.o server subscriber