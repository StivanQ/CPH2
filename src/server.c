#include "server.h"

int main(int argc, char** argv) {
	if(argc < 2) {
		usage(argv[0]);
	}

	TServer* server = init_server();

	set_up_server(server, argv[1]);

	loop(server);

	shutdown_server(server);

	// ceva
}

void loop(TServer* server) {


	// minitest
	char buffer[BUFFER_LEN];
	int i;
	int err;
	int n;
	TSubscriber* sub;
	TTopic* topic;
	TPkg* p;

	while (1) {
		server->tmp_fds = server->read_fds; 
		
		err = select(server->fdmax + 1, &(server->tmp_fds), NULL, NULL, NULL);
		if(err < 0) {
			perror("eroare la select in server\n");
		}

		for(i = 0; i <= server->fdmax; i++) {
			if(FD_ISSET(i, &(server->tmp_fds))) {
				if(i == STDIN_FILENO) {
					memset(buffer, 0, BUFFER_LEN);
					n = read(STDIN_FILENO, buffer, BUFFER_LEN - 1);
					if(n > 0) {
						// TODO: bug-free this
						if(strstr(buffer, "exit")) {
							shutdown_server(server);
							exit(0);
						}
					} else {
						// some kind of error i think
					}
				} else if (i == server->tcp_sockfd) {
					// conexiune noua cient
					err = accept_client(server);
					if(err < 0 ) {
						// not good
					}
				} else if (i == server->udp_sockfd) {
					// inseamna ca e packet de la crient udp
					// TODO

					// aici trebuie parsat 

				} else {
					parse_client_package(server, i);
				}
			}
		}
	}
}

void parse_client_package(TServer* server, int sockfd) {
	// inseamna ca nu e nici stdin, nici client tcp nou, nici
	// packet udp deci e client tcp deja conectat

	// vedem ce zice packetul primit;
	// poate e garbage

	// daca e cerere de login atunci cautam in liste serverului
	// de subs daca exista deja baiatul care se logheaza
	// daca exista si e offline inseamna ca ele 
	// daca exista si e online inseamna ca ii dam eroare
	// daca nu exitsta dupa ID il bagam 

	char buffer[BUFFER_LEN];
	int n;

	TPkg* p;
	TSubscriber* sub;
	TTopic* topic;

	n = recv(sockfd, buffer, sizeof(TPkg), 0);

	if(n < 0) {
		perror("error recv server");
	}

	p = (TPkg*) buffer;

	if(p->packet_type == LIGHT) {
		// daca e package light inseamna ca e de la client tcp

		if(p->op_code == LOG_IN) {
			// daca e opcode de log_in il cautam sa vedem daca il avem in lista 
			// de subscriberi

			sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(p->ID), ID_Search);
			if(sub) {
				// daca e in lista vedem daca e online
				// daca e online nu e bine
				// daca nu e online ii dam switch ca online si ii setam fd
				// cel de unde am primit mesajul

				if(sub->status == ONLINE) {
					printf("boss avem deja userul asta logat\n");
					return;
				} else if (sub->status == OFFLINE) {
					sub->status = ONLINE;
					sub->fd = sockfd;
					//
					//
					//
					//
					// TODO: goleste toata coada de packete spre
					// baiatul asta
					//
					//
					//
					//
					//
				}
			} else {
				// daca nu il gasim dupa nume tre sa il inregistram
				sub = newSubscriber(p->ID);
				if(!sub) {
					printf("well idk sub allocation[%d]\n", __LINE__);
					return;
				}
				sub->status = ONLINE;
				sub->fd = sockfd;
				add_elem(&(server->subscribers), sub);
			}
		} else if(p->op_code == SUB) {
			// daca e op_code de subscribe
			// cautam sa vedem daca e subscribed deja

			sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(p->ID), fd_Search);
			if(!sub) {
				printf("eroare nu-l gasim pe bosseanul deja logaT?\n");
			} else {
				topic = (TTopic*) search_elem((sub->topicList), (void*)p->topic, 
					topic_Search);
				if(!topic) {
					topic = newTopic(p->topic);
					if(!topic) {
						printf("well idk topic allocation[%d]\n", __LINE__);
						return;
					}
					// adaugam topicul in lista de topicuri ale lui sub
					topic->SF = p->SF;
					add_elem(&(sub->topicList), sub);

					// cautam topicul in lista de topicuri si daca il gasim
					// doar il adaugam pe sub in lista de subscriberi la topic
					// daca nu gasim cream topicul repectiv si il adaugam pe
					// subscriber acolo
					topic = (TTopic*) search_elem((server->topics), 
						(void*)p->topic, topic_Search); 
					if(!topic) {
						topic = newTopic(p->topic);
						if(!topic) {
							printf("well idk topic allocation[%d]\n", __LINE__);
							return;
						}
						add_elem(&(server->topics), topic);
						add_elem(&(topic->subs), sub);
					}
				}
			}
		} else if(p->op_code == UNSUB) {
			// daca vrea sa-si dea subscribe 
			// cautam sa vedem daca e abonat la topicul ala
			// daca nu e abonat atunci dam asa un printf
			// daca e abonat ii stergem etry-ul de topic din lista lui de
			// topicuri apoi stergem si subscriberul din lista topicului de 
			// abonati
			// TODO: ^^^^^^^

		} else {
			printf(" nush fra wrong op_code [%d]\n", __LINE__);
		}
	} else if (p->packet_type == HEAVY) {
		printf("n-ai trimis pacetul cumtrebuie boss\n");
	}

	// daca nu e cerere de login si nici garbage inseamna ca e
	// ceva mesaj subscribe/unsubscribe topic
	// vedem ce zice si il rezolvam si pe el


	// sub = search_elem(server->subscribers, (void*)&i, fd_Search);
	// if(sub == NULL) {
	// 	// asta ar fi cazul cand nu exitsa si il bagam i guess?
	// }
}

TServer* init_server() {
	TServer* server = (TServer* ) calloc(1, sizeof(TServer));
	DIE(server == NULL, "init_server");

	// TODO: stuff gen 
	// gen liste pt clienti
	// si liste pt topicuri

	printf("Buna dimineata\n");

	server->subscribers = NULL;
	server->topics = NULL;

	return server;
}

// sa spunem ca asta a fost ce-a fost
void set_up_server(TServer* server, char* port) {

	printf("Buna ziua!\n");

	// pasring the port
	int portno;
	portno = atoi(port);
	DIE(portno == 0, "atoi");

	// clearing the fd set
	FD_ZERO(&(server->read_fds));
	FD_ZERO(&(server->tmp_fds));

	// tcp socket
	server->tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(server->tcp_sockfd < 0, "tcp socket");
	printf("TCP sockfd=[%d]\n", server->tcp_sockfd);

	// udp socket
	server->udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(server->udp_sockfd < 0, "udp socket");
	printf("UDP sockfd=[%d]\n", server->udp_sockfd);

	// disabling Nagle's algorithm
	int state = 1;
	setsockopt(server->tcp_sockfd, IPPROTO_TCP, TCP_NODELAY, &state, 
			   sizeof(state));

	int err;
	struct sockaddr_in serv_addr;

	// binding the sockets to 
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	err = bind(server->tcp_sockfd, (struct sockaddr *) &serv_addr,
			   sizeof(struct sockaddr));
	DIE(err < 0, " tcp socket bind");

	err = bind(server->udp_sockfd, (struct sockaddr *) &serv_addr,
			   sizeof(struct sockaddr));
	DIE(err < 0, " udp socket bind");

	FD_SET(STDIN_FILENO, &(server->read_fds));
	FD_SET(server->tcp_sockfd, &(server->read_fds));
	FD_SET(server->udp_sockfd, &(server->read_fds));

	server->fdmax = server->udp_sockfd;

	//printf(" cel mai mare sockfd este [%d]\n", server->fdmax);

	err = listen(server->tcp_sockfd, BIG_NUMBER);
	DIE(err < 0, "listen");

}

// returs 0 on succesfully accepted clients
// returns -1 on some kind of error idk
int accept_client(TServer* server) {
	socklen_t clilen;
	struct sockaddr_in cli_addr;

	int newsockfd = accept(server->tcp_sockfd, (struct sockaddr *) &cli_addr, 
						   &clilen);
	if(newsockfd < 0) {
		// some kind of error
		printf("<<<<YOU SHOULD NOT SEE THIS at [%d] in server{%d}>>>>\n", 
				__LINE__, errno);
		return -1;
	}

	if(newsockfd > server->fdmax) {
		server->fdmax = newsockfd;
	}

	// we add the client to read fds
	FD_SET(newsockfd, &(server->read_fds));
	return 0;
}

void usage(char *file) {
	fprintf(stderr, "Usage: ./%s <PORT_DORIT>\n", file);
	exit(0);
}



void shutdown_server(TServer* server) {

	// de-alloc w/e was malloced
	// close every client socket
	printf("Noapte buna!\n");
	close(server->tcp_sockfd);
	close(server->udp_sockfd);
}


TSubscriber* newSubscriber(char* ID) {
	TSubscriber* newSub = (TSubscriber*) calloc(1, sizeof(TSubscriber));

	if(newSub == NULL) {
		return NULL;
	}

	strncpy(newSub->ID, ID, ID_MAX_LEN);
	newSub->status = 0;
	newSub->fd = -1;
	newSub->topicList = NULL;
	return newSub;
}

TTopic* newTopic(char* name) {
		TTopic* newTopic = (TTopic*) calloc(1, sizeof(TTopic));

	if(newTopic == NULL) {
		return NULL;
	}

	strncpy(newTopic->name, name, ID_MAX_LEN);
	newTopic->SF = 0;
	newTopic->subs = NULL;
	return newTopic;
}

int ID_Search (void* s1, void* s2) {
	if(strncmp(((TSubscriber*)s1)->ID, ((TSubscriber*)s2)->ID, ID_MAX_LEN) == 0) {
		return 0;
	}
	return 1;
}

int topic_Search (void* t1, void* t2) {
	if(strncmp(((TTopic*)t1)->name, ((TTopic*)t2)->name, TOPIC_MAX_LEN) == 0) {
		return 0;
	}
	return 1;
}

int fd_Search(void* f1, void* f2) {
	if(((TSubscriber*)f1)->fd == ((TSubscriber*)f2)->fd) {
		return 0;
	}
	return 1;
}

int search_int(void* i1, void *i2) {
	if((int*)i1 == (int*) i2) {
		return 0;
	}
	return -1;
}