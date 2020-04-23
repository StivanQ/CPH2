#include "helpers.h"

#define MAX_CLIENTS 9999

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
	// conectat. Trebuie sa am corespondenta ID_CLIENT <-> clientfd

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