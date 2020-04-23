#include "subscriber.h"

int main(int argc, char *argv[]) {

	if(argc < 3) {
		usage(argv[0]);
	}

	TClient* client = init_client(argv[1], argv[2], argv[3]);


	return 0;
}

void usage(char *file)
{
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

}


TClient* init_client(char* ID, char* ip, char* port) {
	TClient* client = (TClient*) calloc(1, sizeof(TClient));
	DIE(client == NULL, "client init");

	strncpy(client->ID, ID, 10);

	start_subscriber(client, ip, port);

	return client;
}
