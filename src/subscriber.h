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

// sends to the server the log in informations
void send_log_in_info(TClient* client);

// closes the socket then shuts down and tells the server we logged-out
void shutdown_client(TClient* client);

// closes the socket then shuts down
void exit_shutdown_client(TClient* client);

// parses the input from stdin and sends the package to the server accordingly 
int my_parse_stdin(TClient* client);
#endif /* SUBSCRIBER_H */