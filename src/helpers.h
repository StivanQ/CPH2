#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "list.h"
#include "queue.h"


#define DIE(condition, message) \
	do { \
		if ((condition)) { \
			fprintf(stderr, "[%d]: %s\n", __LINE__, (message)); \
			perror(""); \
			exit(1); \
		} \
	} while (0)

#define ID_MAX_LEN 10
#define TOPIC_MAX_LEN 50

#define BUFFER_LEN 1600
#define MAX_LEN 1500

#define LIGHT 15
#define HEAVY 35 

#define LOG_IN 0
#define SUB 1
#define UNSUB 2

#define ONLINE 0
#define OFFLINE 1

#define LOG_IN_LEN 15

typedef struct topic {
	char name[TOPIC_MAX_LEN];
	uint8_t SF;
	list subs;
} TTopic;

/*typedef struct packet {
	uint8_t packet_type;
	int len;
	// 0 - first ping by the subscriber; 1 - subscribe; 2 - unsubscribe
	// if op_code is set to either 1 or 2 then the topic the client just
	// un/subscribed from/to is set accordingly
	uint8_t op_code;

	// the data type of the message
	uint8_t data_type;

	// if op_code is set to 0 then the first 10 characters in the payload
	// are the ID of the subscriber
	// the actual message
	char paylaod[MAX_LEN];

	// TODO: mai tinkeresc pe aici pe la structurile astea 
	// modific sa fie un singur payload cu niste compinatii de op_code uri
	// sa fie si topic in payload ca parca nu-i palce cum arata. w/e mai vad eu
} TPkt;
*/
// used for comunicating between 
typedef struct package{
	// packet_type = LIGHT/ HEAVY
	uint8_t package_type;
	// int len;
	// 0 - sign-up
	// 1 - subscribe
	// 2 - unsubscribe
	// is the tcp client is to recieve a LIGHT package from server is must
	// be a shutdown order
	uint8_t op_code;
	uint8_t SF;
	char ID[ID_MAX_LEN];
	char topic[TOPIC_MAX_LEN];
	char payload[MAX_LEN];
} TPkg;

typedef struct subscriber {
	// 1 - online; 0 - offline;
	uint8_t status;
	// client's fd; if status is 0 then fd should be negative and vice-versa
	int fd;
	char ID[ID_MAX_LEN];
	list topicList;
	// maybe log-in infos for plrintfs
} TSubscriber;

typedef struct udp_package{
	char topic[TOPIC_MAX_LEN];
	uint8_t data_type;
	char payload[MAX_LEN];

}TUDP;

#endif /* HELPERS_H */