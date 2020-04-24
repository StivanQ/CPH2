#include "helpers.h"

#ifndef SERVER_H
#define SERVER_H

#define BIG_NUMBER 10

typedef struct server {

	int i, n, ret;
	struct sockaddr_in serv_addr;

	int tcp_sockfd;
	int udp_sockfd;


	// pe multmea asta ascult. tot aici se afla toti clientii posibili
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds


	// imi mai trebuie o structura sa stiu ce client (pe baza de ID_CLIENT) este
	// conectat. Tr1ebuie sa am corespondenta ID_CLIENT <-> clientfd

	// the list of all subscribers who ever logged-in
	// has elements of TSubscriber
	list subscribers;

	// the list of all topics that have at least one subscriber
	list topics;


	// imi mai trebuie o structura unde sa vad fiecare client la ce topic este
	// abonat; (Bonus: sa vad per topc lista de clienti abonati pt SF)
} TServer;

// main arg checking
void usage(char *file);

// ar trebui sa fie main loop
void loop(TServer* server);

// aloca spatiu pentru server si structuri de stocare
// TODO: structuri de stocare
TServer* init_server();

// deschide socketii 
// TODO: altele
void set_up_server(TServer* server, char* port);

// does everithing he has to do like closing the sockets and w/e
// then stops running
void shutdown_server(TServer* server);

int search_int(void*, void*);
// creates a nea subscriber to be adden in the subscribers list of the server
TSubscriber* newSubscriber(char* ID);
// creates a ne topic which can be user for both topicList in server and in
// topicList in subscriber
TTopic* newTopic(char* name);

// cmp function for searching subs by ID
int ID_Search (void* s1, void* s2);

// cmp function for searching topics by topic name
int topic_Search (void* t1, void* t2);

// cmp function for searching subs by fd
int fd_Search(void* f1, void* f2);

// parses the package recieved from the client
void parse_client_package(TServer* server, int sockfd);

// returs 0 on succesfully accepted clients
// returns -1 on some kind of error idk
int accept_client(TServer* server);

// print function for list of subscribers
void print_sub(void* sub);

// print function for list of subscribers
void print_topic(void* topic);

// creates a package orderint the client to shutdown
TPkg* shutdown_order();

#endif /* SERVER_H */