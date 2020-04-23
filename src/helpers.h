#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define DIE(condition, message) \
	do { \
		if ((condition)) { \
			fprintf(stderr, "[%d]: %s\n", __LINE__, (message)); \
			perror(""); \
			exit(1); \
		} \
	} while (0)

#define MAX_LEN 2000

typedef struct packet {
	// 0 - communication; 1 - subscribe; 2 - unsubscribe
	// if op_code is set to either 1 or 2 then the topic the client just
	// un/subscribed from/to is set accordingly
	uint8_t op_code;
	// 0 - if the client doesn't want SF enable
	// 1 - otherwise
	uint8_t SF;
	// the topic of the message
	char topic[50];
	// the data type of the message
	uint8_t data_type;
	// the actual message
	char paylaod[MAX_LEN];

} pkt;

#endif /* HELPERS_H */