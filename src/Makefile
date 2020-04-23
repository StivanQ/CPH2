PORT = 4920

all: server subscriber

run_server: server
	./server ${PORT}

run_subscriber: subscriber
	./subscriber 

server: server.c
	gcc -Wall server.c -o server

subscriber: subscriber.c
	gcc -Wall subscriber.c -o subscriber

server.o:
	gcc -Wall -c -g server.c -o server.o

subscriber.o:
	gcc -Wall -c -g subscriber.c -o subscriber.o 

clean:
	rm -rf *.o server subscriber