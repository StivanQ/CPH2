#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <math.h>
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
#define CONSTANTA 1664

#define LIGHT 15
#define HEAVY 35 
#define SMALL 49 

#define LOG_IN 0
#define SUB 1
#define UNSUB 2
#define LOG_OUT 3

#define ONLINE 0
#define OFFLINE 1

#define LOG_IN_LEN 15

// used for comunicating between server and tcp client
typedef struct package{
	// packet_type = LIGHT/ HEAVY
	uint8_t package_type;
	// int len;
	// 0 - sign-up/ log-in
	// 1 - subscribe
	// 2 - unsubscribe
	// 3 - log-out
	// is the tcp client is to recieve a LIGHT package from server is must
	// be a shutdown order
	uint8_t op_code;
	uint8_t SF;
	char ID[ID_MAX_LEN];
	char topic[TOPIC_MAX_LEN];
	// char payload[BUFFER_LEN + 1];
} TPkg;

typedef struct small_p {
	uint8_t package_type;
	char payload[BUFFER_LEN + 1];
} TSmall;

typedef struct subscriber {
	// 1 - online; 0 - offline;
	uint8_t status;
	// client's fd; if status is 0 then fd should be negative and vice-versa
	int fd;
	char ID[ID_MAX_LEN];
	list topicList;
	queue storage;
	// maybe log-in infos for plrintfs
} TSubscriber;

typedef struct tuplu{
	TSubscriber* sub;
	int SF;
}TPair;

typedef struct topic {
	char name[TOPIC_MAX_LEN];
	uint8_t SF;
	list pair_of_subs;
} TTopic;

typedef struct udp_package{
	char topic[TOPIC_MAX_LEN];
	uint8_t data_type;
	char payload[MAX_LEN];
}TUDP;

// daca am 
// TUDP* p;
// int numar
// if(p->data_type == INT){
// 		numar = *((int*)(p->payload));
// }



#endif /* HELPERS_H */