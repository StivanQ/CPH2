#include "helpers.h"

#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

typedef struct client {
	char ID[ID_MAX_LEN];
	int server_sockfd;
	struct sockaddr_in serv_addr;
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;
} TClient;

void usage(char *file);

// this is where all the things happen
void loop(TClient* client);

// starts the Client a.k.a. subscriber
// connects the client to the server
void start_subscriber();

// allocates a TClient
TClient* init_client(char* ID, char* ip, char* port);
#endif /* SUBSCRIBER_H */