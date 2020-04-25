#include "server.h"
#include <math.h>

int main(int argc, char** argv) {
	if(argc < 2) {
		usage(argv[0]);
	}

	// test
	/*
	char id1[] = "mariusionut";
	char id2[] = {'m', 'a', 'r', 'i', 'u', 's'};

	int res = strncmp(id1, id2, 7);

	printf("res comp = [%d]\n", res);*/

	TServer* server = init_server();

	set_up_server(server, argv[1]);

	loop(server);

	//shutdown_server(server);

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
				// printf("macar vede ca se prmeste input?\n");
				if(i == STDIN_FILENO) {
					printf("~~~~<<<<INPUT TASTATURA>>>>~~~~\n");
					memset(buffer, 0, BUFFER_LEN);
					n = read(STDIN_FILENO, buffer, BUFFER_LEN - 1);
					if(n > 0) {
						// TODO: bug-free this
						if(strstr(buffer, "exit")) {
							shutdown_clients(server);
							shutdown_server(server);
							//exit(0);
							return;
						}
					} else {
						// some kind of error i think
					}
					printf("~~~~<<<<END OF INPUT TASTATURA>>>>~~~~\n");
				} else if (i == server->tcp_sockfd) {
					// conexiune noua cient
					printf("~~~~<<<<INPUT TCP CLIENT[%d]>>>>~~~~\n", __LINE__);
					err = accept_client(server);
					if(err < 0 ) {
						// not good
					}
					printf("~~~~<<<<END OF INPUT TCP CLIENT[%d]>>>>~~~~\n", __LINE__);
				} else if (i == server->udp_sockfd) {
					// inseamna ca e packet de la crient udp
					// TODO
					printf("~~~~<<<<INPUT UDP CLIENT[%d]>>>>~~~~\n", __LINE__);

					// aici trebuie parsat 
					parse_udp_package(server);

				} else {
					printf("~~~~<<<<PARSARE PACKET TCP CLIENT[%d] pe port[%d]>>>>~~~~\n", 
							__LINE__, i);
					parse_client_package(server, i);
					printf("~~~~<<<<END OF PARSARE PACKET TCP CLIENT[%d] pe port[%d]>>>>~~~~\n", 
							__LINE__, i);
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

	struct sockaddr_in listenAddr;
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

	// printf("lungime packet = [%d]IP_SURSA=[%s]PORT[%d]\n", n, display, 
	// 		ntohs(fromAddr.sin_port));


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
	// sprintf(message, "topic: {%s} data_type: {%s} payload{%s}", topic, data_type, payload);
	// printf("mesaj{%s}\n", message);

	sprintf(message, "%s:%d - %s - %s - %s", display, 
			ntohs(fromAddr.sin_port), topic, data_type, payload);
	printf("{%s}\n", message);

	TPkg package;
	memset(&package, 0, sizeof(TPkg));
	package.package_type = HEAVY;
	strncpy(package.topic, topic, TOPIC_MAX_LEN);
	strncpy(package.payload, message, BUFFER_LEN);
	package.payload[MAX_LEN] = '\0';

	send_packages_to_subscribers(server, &package);

}


void send_packages_to_subscribers(TServer* server, TPkg *p) {

	TTopic* topic;

	TTopic aux;
	strncpy(aux.name, p->topic, TOPIC_MAX_LEN);
	topic = search_elem(server->topics, (void*)&aux, topic_Search);

	TSubscriber* sub;
	list iterator;

	int n;

	//printf("\t\t\ttopic_cautat[%s]\n", aux.name);
	//printf("\t\t\ttopic_wanna_find[%s]\n", p->topic);

	if(!topic) {
		printf("nush daca ar trb sa fie eroare sau nu[%d]\n", __LINE__);
	} else {
		iterator = topic->subs;
		while(iterator) {
			sub = iterator->element;
			if(sub) {
				if(sub->status == ONLINE) {
					n = send(sub->fd, p, sizeof(TPkg), 0);
					if(n < 0) {
						perror("");
						printf("eroare of some king[%d]\n", __LINE__);
					}
					printf("am trimis[%d]bytes\n", n);
				} else {
					// TODO: baga in SF

				}
			} else {
				// idk eroare de vreun fel
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
	TSubscriber* sub2;
	TSubscriber subAux;
	TTopic* topic;
	TTopic topicAux;

	struct sockaddr_in listenAddr;
	struct sockaddr_in fromAddr;
	socklen_t fromAddrLen = sizeof(fromAddr);

	n = recvfrom(sockfd, buffer, BUFFER_LEN, 0, 
		(struct sockaddr *)&fromAddr, &fromAddrLen);

	char display[16] = {0};
	inet_ntop(AF_INET, &fromAddr.sin_addr.s_addr, display, sizeof (display));

	printf("lungime packet = [%d]IP_SURSA=[%s]\n", n, display);

	if(n == 0) {
		//subAux = newSubscriber("");
		subAux.fd = sockfd; 
		sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(&subAux), fd_Search);

		if(!sub) {
			printf("boss trebuia sa-l gasim [%d]\n", __LINE__);
		} else {
			sub->status = OFFLINE;
		}

		printf("clientul de pe socketul [%d] left the room\n", sockfd);
		//shutdown(sockfd, SHUT_RDWR);
		FD_CLR(sockfd, &(server->read_fds));

		//print_list((server->subscribers), print_sub);
		printf("~~~EO PARSE PACKAGE~~~[%d]\n", __LINE__);
		return;
	}

	if(n < 0) {
		perror("error recv server");
		return;
	}

	p = (TPkg*) buffer;
	char localID[ID_MAX_LEN + 1];
	// char localID[ID_MAX_LEN + 1];



	if(p->package_type == LIGHT) {
		// daca e package light inseamna ca e de la client tcp
		// printf("am primit packet light\n");

		if(p->op_code == LOG_IN) {
			// daca e opcode de log_in il cautam sa vedem daca il avem in lista 
			// de subscriberi
			// printf("cu credentiale de log-in\n");
			// subAux = newSubscriber(p->ID);
			strncpy(subAux.ID, p->ID, ID_MAX_LEN);
			sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(&subAux), ID_Search);
			//free(subAux);
			if(sub) {
				// daca e in lista vedem daca e online
				// daca e online nu e bine
				// daca nu e online ii dam switch ca online si ii setam fd
				// cel de unde am primit mesajul

				if(sub->status == ONLINE) {
					printf("boss avem deja userul asta logat\n");

					p = shutdown_order();
					if(!p) {
						printf("err malloc[%d]\n", __LINE__);
						return;
					}
					n = send(sockfd, p, sizeof(TPkg), 0);
					FD_CLR(sockfd, &(server->read_fds));

					if(n < 0) {
						perror("eroare fratelemeleu[]");
						printf("[%d]\n", __LINE__);
						return;
					}
					
					print_list((server->subscribers), print_sub);
					printf("~~~EO PARSE PACKAGE~~~[%d]\n", __LINE__);
					return;
				} else if (sub->status == OFFLINE) {
					sub->status = ONLINE;
					sub->fd = sockfd;
					strncpy(localID, p->ID, ID_MAX_LEN);
					localID[ID_MAX_LEN] = '\0';
					// printf("Client[%s] connected\n", localID);

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
				// printf("te-am bagat in lista de subscriberi, boss\n");

				// print_list((server->subscribers), print_sub);
			}
			inet_ntop(AF_INET, &(server->clientInfo[sockfd].sin_addr.s_addr), 
				display, sizeof (display));
			printf("New cient %s connected from %s:%d\n", p->ID, display, 
			ntohs(server->clientInfo[sockfd].sin_port));
		} else if(p->op_code == SUB) {
			// daca e op_code de subscribe
			// cautam sa vedem daca e subscribed deja

			// TODO: cauta sa nu fie abonat deja la topicul ala

			// printf("cu comanda de subscribe\n");
			strncpy(subAux.ID, p->ID, ID_MAX_LEN);
			sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(&subAux), ID_Search);
			if(!sub) {
				printf("eroare nu-l gasim pe bosseanul deja logaT?[%d]\n", __LINE__);
				return;
			}
				printf("[%d]", __LINE__);

			strncpy(topicAux.name, p->topic, TOPIC_MAX_LEN);
			topic = (TTopic*) search_elem((sub->topicList), (void*)(&topicAux), 
				topic_Search);
			if(!topic) {
				// printf("[%d]", __LINE__);
				topic = newTopic(p->topic);
				// printf("[%d]", __LINE__);
				if(!topic) {
					printf("well idk topic allocation[%d]\n", __LINE__);
					return;
				}
				// adaugam topicul in lista de topicuri ale lui sub
				// printf("[%d]", __LINE__);
				topic->SF = p->SF;
				// printf("[%d]", __LINE__);
				add_elem(&(sub->topicList), topic);

				// printf("[%d]", __LINE__);
				// print_list(sub->topicList, print_topic);
				// printf("[%d]", __LINE__);

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
					add_elem(&(topic->subs), sub);
					// printf("[%d]", __LINE__);
				}
				// printf("[%d]", __LINE__);
				// print_list(server->topics, print_topic);
			} else {
				printf("\t\t\t\tavem deja topicul asta[%d]\n", __LINE__);
			}

			strncpy(topicAux.name, p->topic, TOPIC_MAX_LEN);
			topic = search_elem(server->topics, (void*) &topicAux, topic_Search);
			if(!topic) {
				printf("boss ar trebui sa avem topicul asta[%d]\n", __LINE__);
				return;
			}

			sub2 = search_elem(topic->subs, (void*)sub, ID_Search);
			if(!sub2) {
				add_elem(&(topic->subs), sub);
			} else {
				//printf("avem deja topicul asta[%d]\n", __LINE__);
			}
		} else if(p->op_code == UNSUB) {
			// printf("cu comanda de unsubscribe\n");

			// daca vrea sa-si dea subscribe 
			// cautam sa vedem daca e abonat la topicul ala
			// daca nu e abonat atunci dam asa un printf
			// daca e abonat ii stergem etry-ul de topic din lista lui de
			// topicuri apoi stergem si subscriberul din lista topicului de 
			// abonati
			// TODO: ^^^^^^^

			strncpy(subAux.ID, p->ID, ID_MAX_LEN);

			subAux.fd = sockfd;
			// printf("[%d]", __LINE__);
			sub = search_elem(server->subscribers, (void*)(&subAux), ID_Search);
			// printf("[%d]", __LINE__);
			if(!sub) {
				printf("nu gasesc clientul boss[%d]\n", __LINE__);
				return;
			}
			// printf("[%d]", __LINE__);
			// printf("lista de topicuri ale lui[%s]cu socket[%d]\n", sub->ID, sub->fd);
			// print_list(sub->topicList, print_topic);
			// printf("[%d]", __LINE__);
			strncpy(topicAux.name, p->topic, TOPIC_MAX_LEN);
			err = rm_elem(&(sub->topicList), (void*)(&topicAux), topic_Search, 0);
			// printf("lista de topicuri ale lui[%s]cu socket[%d]\n", sub->ID, sub->fd);
			sub->fd = sockfd;
			// print_list(sub->topicList, print_topic);
			if(err == -1) {
				printf("nu gasesc topic [%s] la cientul[%d][%d]\n", topicAux.name, sockfd,
					__LINE__);
				return;
			}
			topic = search_elem(server->topics, (void*)(&topicAux), topic_Search);

			if(!topic) {
				printf("nu gasesc topic [%s] in lista serverlui[%d][%d]\n", topicAux.name, sockfd,
					__LINE__);
				return;
			}
			err = rm_elem(&(topic->subs), (void*)(sub), ID_Search, 0);
			if(err == -1){
				printf("nu gasesc clientul boss[%d]\n", __LINE__);
				return;
			}
		} else if(p->op_code == LOG_OUT) {
			strncpy(subAux.ID, p->ID, ID_MAX_LEN);
			sub = (TSubscriber*) search_elem(server->subscribers,
				(void*)(&subAux), ID_Search);
			if(!sub) {
				printf("eroare nu-l gasim pe bosseanul deja logaT?[%d]\n", __LINE__);
				return;
			}
			sub->status = OFFLINE;
			// FD_CLR(sockfd, &(server->read_fds));
		} else {



			printf(" nush fra wrong op_code [%d]\n", __LINE__);
		}
	} else if (p->package_type == HEAVY) {
		printf("n-ai trimis pacetul cumtrebuie boss\n");
	}

	// daca nu e cerere de login si nici garbage inseamna ca e
	// ceva mesaj subscribe/unsubscribe topic
	// vedem ce zice si il rezolvam si pe el
	// printf("{{{lista de subscriberi}}}\n");
	// print_list((server->subscribers), print_sub);
	// printf("{{{lista de topicuri}}}\n");
	// print_list((server->topics), print_topic);
	// printf("~~~EO PARSE PACKAGE~~~[%d]\n", __LINE__);

	//sub = search_elem(server->subscribers, (void*)&i, fd_Search);
	//if(sub == NULL) {
		// asta ar fi cazul cand nu exitsa si il bagam i guess?
	//}
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

int accept_client(TServer* server) {
	socklen_t clilen;
	struct sockaddr_in cli_addr;

	int newsockfd = accept(server->tcp_sockfd, (struct sockaddr *) &cli_addr, 
						   &clilen);
	if(newsockfd < 0) {
		// some kind of error
		printf("<<<<YOU SHOULD NOT SEE THIS at [%d] in server>>>>\n", 
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

	printf("New client on socket[%d]\n", newsockfd);

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
	int i;
	for(i = 0; i <= server->fdmax; i++) {
		if(FD_ISSET(i, &(server->read_fds))){
			//shutdown(i, SHUT_RDWR);
			//close(i);
			FD_CLR(i, &(server->read_fds));
		}
	}
	shutdown(server->tcp_sockfd, SHUT_RDWR);
	shutdown(server->udp_sockfd, SHUT_RDWR);
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

	strncpy(newTopic->name, name, TOPIC_MAX_LEN);
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

void print_sub(void* sub) {
	TSubscriber* s = (TSubscriber*) sub;
	char ID[ID_MAX_LEN + 1];
	strncpy(ID, s->ID, ID_MAX_LEN);
	ID[ID_MAX_LEN] = '\0';
	printf("ID[%s]status[%d]sockfd[%d]\n", ID, s->status, s->fd);
}

void print_topic(void* topic) {
	if(!topic){

	}
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
}