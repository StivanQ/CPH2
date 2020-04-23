#include "helpers.h"

int main(int argc, char *argv[])
{
/*	int sockfd, i, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	if (argc < 3) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	ret = inet_aton(argv[1], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	//packet p;



	while (1) {

		tmp_fds = read_fds;
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
					// printf("aici[%d]sockfd[%d]\n", i, sockfd);
				if (i == sockfd) {
					// s-au primit date pe unul din socketii de server,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					packet* header = (packet*) (&buffer);

					// printf("am primit packet de la sursa[%d]\n", header->s_fd - '0');

					printf ("MESAJ DE LA [%d]:\"%s\"\n", header->s_fd - '0', buffer+1);

				} else if (i == STDIN_FILENO){
					// s-au primit date pe unul din socketii de STDIN,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = read(STDIN_FILENO, buffer, BUFLEN - 1);
					DIE(n < 0, "recv");

					if (strncmp(buffer, "exit", 4) == 0) {
						close(sockfd);
						return 0;
					}


					// MESAJUL ESTE DE FORMA "%d%d%s", sursa, desitnatie, mesaj
					// e.g.
					// 45Marius, ce mai faci?
					// va trimite lui 5, iar 5 va stii ca e de la 4.



					packet* header = (packet*) (&buffer);
					printf("sender=[%d]destination=[%d]\n", header->s_fd - '0', header->d_fd - '0');

					printf("am citit de la tastatura...\n trimit mai departe la server\n");
					n = send(sockfd, buffer, n, 0);
				}
			}
		}
	}

	close(sockfd);
*/
	return 0;
}
