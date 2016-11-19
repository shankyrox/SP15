/*
** client.h 
*/

#ifndef _CLIENT_DEF_
#define _CLIENT_DEF_

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT "5009" // the port client will be connecting to 

 
#define MAX_GROUPS 100
#define DELAY 2

char* selectGroup(char *msg);
void *get_in_addr(struct sockaddr *sa);

#endif
