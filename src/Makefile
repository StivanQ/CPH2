PORT = 22000
IP = "127.0.0.1"

all: server subscriber

udp:
	python3 udp_client.py --input_file three_topics_payloads.json --mode random --count 500000 ${IP} ${PORT}

run_server: server
	./server ${PORT}

run_client1: subscriber
	./subscriber "marius" ${IP} ${PORT}

run_client2: subscriber
	./subscriber "ionut" ${IP} ${PORT}

server: server.c list.c queue.c
	gcc -Wall -g server.c list.c queue.c -o server -lm

subscriber: subscriber.c
	gcc -Wall -g subscriber.c -o subscriber

server.o:
	gcc -Wall -c -g server.c -o server.o

subscriber.o:
	gcc -Wall -c -g subscriber.c -o subscriber.o 

clean:
	rm -rf *.o server subscriber