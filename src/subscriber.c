#include "subscriber.h"

int main(int argc, char *argv[]) {

	if(argc < 3) {
		usage(argv[0]);
	}

	TClient* client = init_client(argv[1], argv[2], argv[3]);

	loop(client);

	return 0;
}

void usage(char *file){
	fprintf(stderr, "./%s <ID_Client> <IP_Server> <Port_Server>\n", file);
	exit(0);
}

void start_subscriber(TClient* client, char* ip, char* port) {
	int ret;
	struct sockaddr_in serv_addr;

	client->server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(client->server_sockfd < 0, "socket client");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(port));
	ret = inet_aton(ip, &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton client");

	ret = connect(client->server_sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect client");
	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&(client->read_fds));
	FD_ZERO(&(client->tmp_fds));

	FD_SET(STDIN_FILENO, &(client->read_fds));
	FD_SET(client->server_sockfd, &(client->read_fds));
	client->fdmax = client->server_sockfd;

	// send log-in infos
	send_log_in_info(client);
}

void send_log_in_info(TClient* client) {
	TPkg p;
	memset(&p, 0, sizeof(TPkg));

	p.package_type = LIGHT;
	p.op_code = 0;
	strncpy(p.ID, client->ID, ID_MAX_LEN);
	int err;
	err = send(client->server_sockfd, (void*)(&p), LOG_IN_LEN, 0);
	if(err < 0) {
		perror("send log-in infos from TCP client");
	}
}

TClient* init_client(char* ID, char* ip, char* port) {
	TClient* client = (TClient*) calloc(1, sizeof(TClient));
	DIE(client == NULL, "client init");

	strncpy(client->ID, ID, 10);

	start_subscriber(client, ip, port);

	return client;
}

void loop(TClient* client) {

	// minitest
	char buffer[BUFFER_LEN];
	int i;
	int err;
	int n;
	TSubscriber* sub;
	TTopic* topic;
	TPkg* p;

	fd_set tmp_fds;

	while (1) {
		tmp_fds = client->read_fds; 
		
		err = select(client->fdmax + 1, &(tmp_fds), NULL, NULL, NULL);
		if(err < 0) {
			perror("eroare la select in server\n");
		}

		for(i = 0; i <= client->fdmax; i++) {
			if(FD_ISSET(i, &(tmp_fds))) {
				//printf("macar vede ca se prmeste input?\n");
				if(i == STDIN_FILENO) {
					err = my_parse_stdin(client);
					if(err) {
						shutdown_client(client);
						return;
					}
				} else {
					n = recv(i, buffer, BUFFER_LEN, 0);
					if(n < 0) {
						perror("err la primire packet de la server");
						continue;
					}

					p = (TPkg*)buffer;
					if(p->package_type == LIGHT) {
						shutdown_client(client);
						return;
					}

					printf("primeste pe socketul[%d]\n", i);
					printf("client->server_socket[%d]\n", client->server_sockfd);
				}
			}
		}
	}
}


void shutdown_client(TClient* client) {

	// de-alloc w/e was malloced
	// close every client socket
	printf("logging off!\n");

	close(client->server_sockfd);
}

// return an exit code or nothing much else
int my_parse_stdin(TClient* client) {
	char buffer[BUFFER_LEN];
	char topic[TOPIC_MAX_LEN + 1];
	int n;
	int SF;
	int code = 4;
	printf("~~~~<<<<INPUT TASTATURA>>>>~~~~\n");
	memset(buffer, 0, BUFFER_LEN);
	n = read(STDIN_FILENO, buffer, BUFFER_LEN - 1);

	TPkg p;

	if(n > 0) {
  		printf("n>0[%d]\n", __LINE__);
  		char* pch;
  		pch = strtok (buffer," ");

  		// int code = 0;


 		if(pch != NULL) {
  			printf("n>0[%d]\n", __LINE__);
 			if(strcmp(pch, "subscribe") == 0) {
 				// subscribe command
  				printf("subscribe[%d]\n", __LINE__);
 				code = 1;
 			} else if(strcmp(pch, "unsubscribe") == 0) {
 				// unsubscribe command
  				printf("usubscribe[%d]\n", __LINE__);
 				code = 2;
 			} else if(strcmp(pch, "exit") == 0) {
  				printf("exit[%d]\n", __LINE__);
 				// exit command
 				code = 3;
 				return code;
 			} else {
  				printf("garbage[%d]\n", __LINE__);
 				// garbage
 				code = 4;
 			}
 		} else {
 			return 0;
 		}

 		pch = strtok (buffer," ");
 		// asta e topicul

 		if(pch != NULL) {
 			strncpy(topic, pch, TOPIC_MAX_LEN);
 			topic[TOPIC_MAX_LEN] = '\0';
 		} else {
 			return 0;
 		}

 		pch = strtok (buffer," ");

 		if(pch != NULL) {
 			// asta e SF
 			if(strlen(pch) > 1) {
 				return 0;
 			} else {
 				SF = pch[0] - '0';
 				if(SF == 0 || SF == 1) {
 					// sf valid
 				} else {
 					return 0;
 				}
 			}
 		}

 		memset(&p, 0, sizeof(TPkg));
 		p.package_type = LIGHT;
 		p.op_code = code;
 		p.SF = SF;
 		strncpy(p.ID, client->ID, ID_MAX_LEN);
 		strncpy(p.topic, topic, TOPIC_MAX_LEN);

 		n = send(client->server_sockfd, &p, sizeof(TPkg), 0);
 		if(n < 0) {
 			printf("[%d]\n", __LINE__);
 			perror("trimitere packet subscribe/unsubscribe");
 		}
	} else {
		// some kind of error i think
		return 0;
	}
	return 0;
}