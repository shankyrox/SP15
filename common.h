/*
** common.h 
*/
#ifndef _COMMON_DEF_
#define _COMMON_DEF_

/*  List of header files to be included in both server and client
 * Any common header file needed should be placed here 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <jansson.h>

/*Common structures and events*/
#define TRUE 1
#define FALSE 0
#define SUCCESS 0
#define FAILURE -1
#define MAX_EVENTS 64

#define BUFFSIZE 10000// max number of bytes we can get at once

//#ifdef DEBUG_FLAG
#define PRINT printf
//#endif


static inline void *
mem_alloc(char *file, int line, int size){
	void *ptr = malloc(size);
	if (ptr == NULL) 
		printf("(Error) Malloc Failure in %s[%d] for size %d\n", file, line, size);
	return ptr;
}

#define MALLOC(size) mem_alloc(__FILE__, __LINE__, size)

typedef enum result
{
    FAIL =0,
    SUCC =1
}Result;

/*Currently supported events*/
typedef enum events
{   
	//CCLIENT = compute client, JCLIENT = job client
    SERVER_CCLIENT_GROUP_IDS_SUPPORTED =0,  
    SERVER_CCLIENT_CONNECTION_ACCEPTED =1,  // Never used so far
    SERVER_CCLIENT_DATA_TO_COMPUTE =2,      
    SERVER_JCLIENT_FINAL_COMPUTE_RESULT =3, 
    CCLIENT_SERVER_GROUP_ID_TO_JOIN =4, 
    CCLIENT_SERVER_GROUP_ID_EXIT =5,    
    CCLIENT_SERVER_COMPUTE_RESULT =6,  
    JCLIENT_SERVER_COMPUTE_MY_DATA =7,
	JCLIENT_SERVER_JOIN,
}Events;

/*Generic Messsage Structure for communication between
client and server*/
typedef struct 
{
    Events event;
    Result result;
    int client_id;
    int data_len;
    int *data;
}Message;

typedef struct node
{
    Message data; 
   struct node *next;
}Node;

typedef struct list
{
    Node *head;
    int count;
}List;

void pop_head(Node ** head, Message *arr);
void push_tail(Node** head, Message *new_data);

int populate_and_send_data(int event, int *data, int datalen, int fd, int client_id);

int size_list(Node *head);

void dserializeTask(unsigned char* msg, Message *t);
void serializeTask(unsigned char* msg, const Message *t);

int parseStruct(char **, Message *);
int parseJson(char *, Message *);

void display_message(Message *);
#endif
