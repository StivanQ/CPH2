#include "server.h"

TServer* init_server() {
	TServer* server = (TServer* ) calloc(1, sizeof(TServer));
	DIE(server == NULL, "init_server");


	// TODO: stuff



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
	DIE(server->tcp_sockfd < 0, "socket");

	// tcp socket
	server->tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(server->tcp_sockfd < 0, "socket");

	// udp socket
	server->udp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(server->udp_sockfd < 0, "socket");


	/*memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;*/

}


void loop(TServer* server) {

	// while (1) {
	// 	tmp_fds = read_fds; 
		
	// 	ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
	// 	DIE(ret < 0, "select");

	// 	for (i = 0; i <= fdmax; i++) {
	// 		if (FD_ISSET(i, &tmp_fds)) {
	// 			if (i == sockfd) {
	// 				// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
	// 				// pe care serverul o accepta
	// 				clilen = sizeof(cli_addr);
	// 				newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	// 				DIE(newsockfd < 0, "accept");

	// 				// sockets[nr_sockets] = newsockfd;
	// 				// nr_sockets++;
					
	// 				// se adauga noul socket intors de accept() la multimea descriptorilor de citire
	// 				if (newsockfd > fdmax) { 
	// 					printf("fdmax_vechi=[%d]fdmax_nou=[%d]\n", fdmax, newsockfd);
	// 					fdmax = newsockfd;
	// 				}

	// 				printf("Noua conexiune de la %s, port %d, socket client %d\n",
	// 						inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);

	// 				// aici trimit lista cu participanti la baiatul care abia o intrat
	// 				sprintf(mesaj, "a%d|", newsockfd);
	// 				for(j = sockfd + 1, k = 0; j <= fdmax; j++) {
	// 					if(FD_ISSET(j, &read_fds)) {
	// 						sprintf(mesaj + k * 2 + 3, "%d ", j);
	// 						k++;
	// 					}
	// 				}
	// 				FD_SET(newsockfd, &read_fds);
	// 				n = send(newsockfd, mesaj, strlen(mesaj), 0);
	// 				DIE(n < 0, "send");

	// 				// x has koined the chat trimis la fiecare in afara de ala care o intrat
	// 				for(j = sockfd + 1; j < fdmax; j++) {
	// 					if(FD_ISSET(j, &read_fds)) {
	// 						sprintf(mesaj, "%c%cBaiatul [%d] has entered the chat", 'a', j + '0', newsockfd);
	// 						n = send(j, mesaj, strlen(mesaj), 0);
	// 						DIE(n < 0, "send");
	// 					}
	// 				}

	// 			} else {
	// 				// s-au primit date pe unul din socketii de client,
	// 				// asa ca serverul trebuie sa le receptioneze
	// 				memset(buffer, 0, BUFLEN);
	// 				n = recv(i, buffer, sizeof(buffer), 0);
	// 				DIE(n < 0, "recv");

	// 				if (n == 0) {
	// 					// conexiunea s-a inchis
	// 					printf("Socket-ul client %d a inchis conexiunea\n", i);
	// 					close(i);
	// 					FD_CLR(i, &read_fds);
	// 					for(j = sockfd + 1; j <= fdmax + 1; j++) {
	// 						if(FD_ISSET(j, &read_fds)) {
	// 							sprintf(mesaj, "%c%cBaiatul [%d] has left the chat", 'a', j + '0', i);
	// 							n = send(j, mesaj, strlen(mesaj), 0);
	// 							DIE(n < 0, "send");
	// 						}
	// 					}

						
	// 					// se scoate din multimea de citire socketul inchis 
	// 					// FD_CLR(i, &read_fds);
	// 				} else {
	// 					packet* header = (packet*) (&buffer);
	// 					int fd_dest = header->d_fd - '0';
	// 					printf("detinatee =[{{%d}}]\n", fd_dest);
	// 					n = send(fd_dest, buffer, n, 0);

	// 					printf ("primit de la[%d]%s\n", i, buffer);
	// 				}
	// 			}
	// 		}
	// 	}
	// }

}


void usage(char *file) {
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}


int main() {

	TServer* server = init_server();

	set_up_server(server, "322");


	// ceva
}