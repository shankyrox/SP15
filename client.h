/*
** client.h 
*/

#ifndef _CLIENT_DEF_
#define _CLIENT_DEF_

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include "common.h"
#include <arpa/inet.h>

#define PORT "5009" // the port client will be connecting to 

 
#define MAX_GROUPS 100
#define DELAY 2
#define MAX_NUM_WORKER_CLIENT 2

void *get_in_addr(struct sockaddr *sa);
int select_group_join(Message *);

#endif
