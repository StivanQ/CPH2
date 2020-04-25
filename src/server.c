#include "server.h"
#include <math.h>

int main(int argc, char** argv) {
	if(argc < 2) {
		usage(argv[0]);
	}

	TServer* server = init_server();
	set_up_server(server, argv[1]);
	loop(server);
}

void loop(TServer* server) {
	char buffer[BUFFER_LEN];
	int i;
	int err;
	int n;
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
						if(strstr(buffer, "exit")) {
							shutdown_clients(server);
							shutdown_server(server);
							return;
						}
					}
				} else if (i == server->tcp_sockfd) {
					err = accept_client(server);
				} else if (i == server->udp_sockfd) {
					parse_udp_package(server);
				} else {
					parse_client_package(server, i);
				}
			}
		}
	}
}

void parse_udp_package(TServer* server) {
	char buffer[BUFFER_LEN];
	char topic[TOPIC_MAX_LEN + 1];
	char message[BUFFER_LEN];
	memset(buffer, 0, BUFFER_LEN);
	memset(topic, 0, TOPIC_MAX_LEN + 1);
	memset(message, 0, BUFFER_LEN);
	char data_type[20];
	char char_sign[2];
	TUDP* p;
	int n;
	uint8_t sign;
	uint8_t power;
	uint32_t number;
	uint16_t short_number;
	float float_number;

	char payload[MAX_LEN + 1];

	struct sockaddr_in fromAddr;
	socklen_t fromAddrLen = sizeof(fromAddr);

	n = recvfrom(server->udp_sockfd, buffer, BUFFER_LEN, 0, 
		(struct sockaddr *)&fromAddr, &fromAddrLen);
	if(n < 0) {
		printf("[%d]\n", __LINE__);
		perror("\tsome kind of errr when recieving from UDP client");
	}

	char display[16] = {0};
	inet_ntop(AF_INET, &fromAddr.sin_addr.s_addr, display, sizeof display);
	p = (TUDP*) buffer;

	strncpy(topic, p->topic, TOPIC_MAX_LEN);
	topic[TOPIC_MAX_LEN] = '\0';

	if(p->data_type == 0) {
		sprintf(data_type, "INT");
		sign =  *((uint8_t*)(p->payload));
		if(sign == 0) {
			char_sign[0] = '\0';
		} else {
			char_sign[0] = '-';
			char_sign[1] = '\0';
		}
		number = *((uint32_t*)(p->payload + 1));
		number = ntohl(number);
		sprintf(payload, "%s%d", char_sign, number);
	} else if (p->data_type == 1) {
		sprintf(data_type, "SHORT_REAL");
		short_number = *((uint16_t*)(p->payload));
		short_number = ntohs(short_number);
		float_number = ((float) short_number) / 100;
		sprintf(payload, "%f", float_number);
	} else if (p->data_type == 2) {
		sprintf(data_type, "FLOAT");
		sign =  *((uint8_t*)(p->payload));
		if(sign == 0) {
			char_sign[0] = '\0';
		} else {
			char_sign[0] = '-';
			char_sign[1] = '\0';
		}
		number = *((uint32_t*)(p->payload + 1));
		number = ntohl(number);
		power = *((uint8_t*)(p->payload + 5));
		float_number = ((float) number) / pow((double)10, (double)power);
		sprintf(payload, "%s%f", char_sign, ((float) float_number));
	} else if (p->data_type == 3) {
		sprintf(data_type, "STRING");
		strncpy(payload, p->payload, MAX_LEN);
		payload[MAX_LEN] = '\0';
	}
	sprintf(message, "%s:%d - %s - %s - %s", display, 
			ntohs(fromAddr.sin_port), topic, data_type, payload);

	TSmall small;
	memset(&small, 0, sizeof(TSmall));
	small.package_type = SMALL;
	strncpy(small.payload, message, BUFFER_LEN);
	small.payload[BUFFER_LEN] = '\0';

	TTopic topicS;
	strncpy(topicS.name, topic, TOPIC_MAX_LEN);

	send_packages_to_subscribers(server, &small, &topicS);
}

void send_packages_to_subscribers(TServer* server, TSmall *p, TTopic* topicS) {
	TTopic* topic;

	topic = search_elem(server->topics, (void*)topicS, topic_Search);

	TPair* pair;
	TSubscriber* sub;
	list iterator;

	int n;
	int SF;

	if(!topic) {
	} else {
		iterator = topic->pair_of_subs;
		while(iterator) {
			pair = iterator->element;
			if(pair) {
				sub = pair->sub;
				SF = pair->SF;
				if(sub->status == ONLINE) {
					n = send(sub->fd, p, sizeof(TSmall), 0);
					if(n < 0) {
						perror("");
						printf("err of some king[%d]\n", __LINE__);
					}
				} else {
					if(SF == 0) {
						iterator = iterator->next;
						continue;
					}
					TSmall* pk = (TSmall*) calloc(1, sizeof(TSmall));
					
					memcpy(pk, p, sizeof(TSmall));
					queue_enq(sub->storage, pk);
				}
			}
			iterator = iterator->next;
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
	int err;

	TPkg* p;
	TSubscriber* sub;
	TSubscriber subAux;
	TTopic* topic;
	TTopic topicAux;

	struct sockaddr_in fromAddr;
	socklen_t fromAddrLen = sizeof(fromAddr);

	n = recvfrom(sockfd, buffer, BUFFER_LEN, 0, 
		(struct sockaddr *)&fromAddr, &fromAddrLen);

	char display[16] = {0};
	inet_ntop(AF_INET, &fromAddr.sin_addr.s_addr, display, sizeof (display));


	if(n == 0) {
		subAux.fd = sockfd; 
		sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(&subAux), fd_Search);

		if(sub) {
			sub->status = OFFLINE;
		}

		char ID[ID_MAX_LEN + 1];
		strncpy(ID, sub->ID, ID_MAX_LEN);

		printf("Client %s discconected.\n", ID);
		FD_CLR(sockfd, &(server->read_fds));
		return;
	}

	if(n < 0) {
		perror("error recv server");
		return;
	}

	p = (TPkg*) buffer;
	char localID[ID_MAX_LEN + 1];
	if(p->package_type == LIGHT) {
		// daca e package light inseamna ca e de la client tcp
		if(p->op_code == LOG_IN) {
			// daca e opcode de log_in il cautam sa vedem daca il avem in lista 
			// de subscriberi
			strncpy(subAux.ID, p->ID, ID_MAX_LEN);
			sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(&subAux), ID_Search);
			if(sub) {
				// daca e in lista vedem daca e online
				// daca e online nu e bine
				// daca nu e online ii dam switch ca online si ii setam fd
				// cel de unde am primit mesajul

				if(sub->status == ONLINE) {
					p = shutdown_order();
					if(!p) {
						printf("err malloc[%d]\n", __LINE__);
						return;
					}
					n = send(sockfd, p, sizeof(TPkg), 0);
					FD_CLR(sockfd, &(server->read_fds));


					if(n < 0) {
						perror("eroare");
						printf("[%d]\n", __LINE__);
						return;
					}
					return;
				} else if (sub->status == OFFLINE) {
					sub->status = ONLINE;
					sub->fd = sockfd;
					strncpy(localID, p->ID, ID_MAX_LEN);
					localID[ID_MAX_LEN] = '\0';

					if(sub->storage) {
						TSmall* sendme;
						while(!queue_empty(sub->storage)) {
							sendme = queue_deq(sub->storage);
							n = send(sub->fd, sendme, sizeof(TSmall), 0);
							if(n <= 0) {
								perror("");
								printf("[%d]\n", __LINE__);
							}
							free(sendme);
						}
					}
				}
			} else {
				sub = newSubscriber(p->ID);
				if(!sub) {
					printf("well idk sub allocation[%d]\n", __LINE__);
					return;
				}
				sub->status = ONLINE;
				sub->fd = sockfd;
				add_elem(&(server->subscribers), sub);
			}
			inet_ntop(AF_INET, &(server->clientInfo[sockfd].sin_addr.s_addr), 
				display, sizeof (display));
			printf("New client %s connected from %s:%d\n", p->ID, display, 
			ntohs(server->clientInfo[sockfd].sin_port));
		} else if(p->op_code == SUB) {
			// daca e op_code de subscribe
			// cautam sa vedem daca e subscribed deja

			strncpy(subAux.ID, p->ID, ID_MAX_LEN);
			sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(&subAux), ID_Search);
			if(!sub) {
				return;
			}

			strncpy(topicAux.name, p->topic, TOPIC_MAX_LEN);
			topic = (TTopic*) search_elem((sub->topicList), (void*)(&topicAux), 
				topic_Search);
			if(!topic) {
				topic = newTopic(p->topic);
				if(!topic) {
					printf("well idk topic allocation[%d]\n", __LINE__);
					return;
				}
				// adaugam topicul in lista de topicuri ale lui sub
				add_elem(&(sub->topicList), topic);

				// cautam topicul in lista de topicuri si daca il gasim
				// doar il adaugam pe sub in lista de subscriberi la topic
				// daca nu gasim cream topicul repectiv si il adaugam pe
				// subscriber acolo
				strncpy(topicAux.name, p->topic, TOPIC_MAX_LEN);
				topic = (TTopic*) search_elem((server->topics), 
					(void*)(&topicAux), topic_Search); 
				if(!topic) {
					topic = newTopic(p->topic);
					if(!topic) {
						printf("well idk topic allocation[%d]\n", __LINE__);
						return;
					}
					add_elem(&(server->topics), topic);

					TPair* pair = newPair(sub, p);

					add_elem(&(topic->pair_of_subs), pair);
				}
			}

			strncpy(topicAux.name, p->topic, TOPIC_MAX_LEN);
			topic = search_elem(server->topics, (void*) &topicAux, topic_Search);
			if(!topic) {
				return;
			}

			TPair* pair = newPair(sub, p);

			TPair* pair2;
			pair2 = search_elem(topic->pair_of_subs, (void*)pair, ID_Search_pair);
			if(!pair2) {
				add_elem(&(topic->pair_of_subs), pair);
			} else {
				free(pair);
			}
		} else if(p->op_code == UNSUB) {

			// daca vrea sa-si dea subscribe 
			// cautam sa vedem daca e abonat la topicul ala
			// daca nu e abonat atunci dam asa un printf
			// daca e abonat ii stergem etry-ul de topic din lista lui de
			// topicuri apoi stergem si subscriberul din lista topicului de 
			// abonati

			strncpy(subAux.ID, p->ID, ID_MAX_LEN);

			subAux.fd = sockfd;
			sub = search_elem(server->subscribers, (void*)(&subAux), ID_Search);
			if(!sub) {
				return;
			}
			strncpy(topicAux.name, p->topic, TOPIC_MAX_LEN);
			err = rm_elem(&(sub->topicList), (void*)(&topicAux), 
				topic_Search, 0);
			sub->fd = sockfd;
			if(err == -1) {
				return;
			}
			topic = search_elem(server->topics, (void*)(&topicAux), 
				topic_Search);

			if(!topic) {
				return;
			}
			TPair* pair;
			pair = newPair(sub, p);

			err = rm_elem(&(topic->pair_of_subs), (void*)(pair), 
				ID_Search_pair, 1);
			free(pair);
			if(err == -1){
				return;
			}
		} else if(p->op_code == LOG_OUT) {
			strncpy(subAux.ID, p->ID, ID_MAX_LEN);
			sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(&subAux), ID_Search);
			if(!sub) {
				return;
			}
			sub->status = OFFLINE;
		}
	}
}

TServer* init_server() {
	TServer* server = (TServer* ) calloc(1, sizeof(TServer));
	DIE(server == NULL, "init_server");
	server->subscribers = NULL;
	server->topics = NULL;

	return server;
}

void set_up_server(TServer* server, char* port) {
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

	// udp socket
	server->udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(server->udp_sockfd < 0, "udp socket");

	// disabling Nagle's algorithm
	int state = 1;
	setsockopt(server->tcp_sockfd, IPPROTO_TCP, TCP_NODELAY | SO_REUSEADDR, 
				&state, sizeof(state));

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

	err = listen(server->tcp_sockfd, BIG_NUMBER);
	DIE(err < 0, "listen");
}

int accept_client(TServer* server) {
	socklen_t clilen;
	struct sockaddr_in cli_addr;

	int newsockfd = accept(server->tcp_sockfd, (struct sockaddr *) &cli_addr, 
						   &clilen);
	if(newsockfd < 0) {
		// some kind of error
		printf("error when accepting new clien[%d]\n", 
				__LINE__);
		perror("");
		printf("errcode=[%d]\n", errno);
		return -1;
	}

	if(newsockfd > server->fdmax) {
		server->fdmax = newsockfd;
	}

	// we add the client to read fds
	FD_SET(newsockfd, &(server->read_fds));

	memcpy(&(server->clientInfo[newsockfd]), &cli_addr, 
			sizeof(struct sockaddr_in));
	return 0;
}

void usage(char *file) {
	fprintf(stderr, "Usage: ./%s <PORT_DORIT>\n", file);
	exit(0);
}

void shutdown_server(TServer* server) {
	// close every client socket
	int i;
	for(i = 0; i <= server->fdmax; i++) {
		if(FD_ISSET(i, &(server->read_fds))){
			FD_CLR(i, &(server->read_fds));
		}
	}
	shutdown(server->tcp_sockfd, SHUT_RDWR);
	shutdown(server->udp_sockfd, SHUT_RDWR);
	close(server->tcp_sockfd);
	close(server->udp_sockfd);
	free_list(&(server->subscribers), free_subs);
	free_list(&(server->topics), free_topic);
	free(server);
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
	newSub->storage = queue_create();
	return newSub;
}

TTopic* newTopic(char* name) {

	TTopic* newTopic = (TTopic*) calloc(1, sizeof(TTopic));

	if(newTopic == NULL) {
		return NULL;
	}

	strncpy(newTopic->name, name, TOPIC_MAX_LEN);
	newTopic->SF = 0;
	newTopic->pair_of_subs = NULL;
	return newTopic;
}

int ID_Search_pair (void* s1, void* s2) {
	TPair* p1 = (TPair*)s1;
	TPair* p2 = (TPair*)s2;
	if(strncmp((p1->sub)->ID, (p2->sub)->ID, ID_MAX_LEN) == 0) {
		return 0;
	}
	return 1;
}

int ID_Search (void* s1, void* s2) {
	if(strncmp(((TSubscriber*)s1)->ID, ((TSubscriber*)s2)->ID, 
		ID_MAX_LEN) == 0) {
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

void print_sub(void* sub) {
	TSubscriber* s = (TSubscriber*) sub;
	char ID[ID_MAX_LEN + 1];
	strncpy(ID, s->ID, ID_MAX_LEN);
	ID[ID_MAX_LEN] = '\0';
	printf("ID[%s]status[%d]sockfd[%d]\n", ID, s->status, s->fd);
}

void print_topic(void* topic) {
	TTopic* t = (TTopic*) topic;
	char name[TOPIC_MAX_LEN + 1];
	strncpy(name, t->name, TOPIC_MAX_LEN);
	name[TOPIC_MAX_LEN] = '\0';
	printf("topic:name[%s]\n", name);
}

TPkg* shutdown_order(){
	TPkg* p = (TPkg *) calloc(1, sizeof(TPkg));
	if(!p) {
		printf("err la malloc [%d]\n", __LINE__);
		return NULL;
	}
	p->package_type = LIGHT;
	return p;
}

void shutdown_clients(TServer* server) {
	TPkg* order = shutdown_order();
	TSubscriber* sub;
	list p = server->subscribers;
	int sockfd;
	while(p) {
		sub = (TSubscriber*)(p->element);
		if(sub->status == ONLINE) {
			sockfd = sub->fd;
			send(sockfd, order, sizeof(TPkg), 0);
			FD_CLR(sockfd, &(server->read_fds));
		}
		p = p->next;
	}
	free(order);
}

TPair* newPair(TSubscriber* sub, TPkg* p) {
	TPair* pair = (TPair*) calloc(1, sizeof(TPair));

	if(!pair) {
		return NULL;
	}

	pair->sub = sub;
	pair->SF = p->SF;

	return pair;
}

void free_topic(void** topic) {
	TTopic* aux = (TTopic*) *topic;

	list l = aux->pair_of_subs;

	free_list(&l, free_pair);
	if(*topic) {
		free(*topic);
	}
}

void free_subs(void** sub) {
	TSubscriber* aux = (TSubscriber*) *sub;

	TSmall* small;

	while(!queue_empty(aux->storage)){
		small = queue_deq(aux->storage);
		free(small);
	}
	if(aux->storage) {
		free(aux->storage);
	}
	if(*sub) {
		free(*sub);
	}
	list l = aux->topicList;

	free_list(&l, free_topic);
}

void free_pair(void** pair) {
	TPair* p = (TPair*) pair;

	if(p->sub) {
		free(p->sub);
	}

	if(*pair) {
		free(*pair);
	}
}