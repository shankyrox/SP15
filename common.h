/*
** common.h 
*/
#ifndef _COMMON_DEF_
#define _COMMON_DEF_

/*Common structures and events*/
#include <jansson.h>

#define TRUE 1
#define FALSE 0

#define MAX_NUM_WORKER_SERVER 4
#define MAX_NUM_WORKER_CLIENT 2

#define BUFFSIZE 1500// max number of bytes we can get at once

#ifdef DEBUG_FLAG
#define PRINT printf;
#endif

typedef enum result
{
    FAIL =0,
    SUCC =1
}Result;

/*Currently supported events*/
typedef enum events
{
    SERVER_CCLIENT_GROUP_IDS_SUPPORTED =0,  // Int array
    SERVER_CCLIENT_CONNECTION_ACCEPTED =1,  // String
    SERVER_CCLIENT_DATA_TO_COMPUTE =2,       // Int Array
    SERVER_JCLIENT_FINAL_COMPUTE_RESULT =3, // Int array, Size one
    CCLIENT_SERVER_GROUP_ID_TO_JOIN =4, // Int arr, size 1, 
    CCLIENT_SERVER_GROUP_ID_EXIT =5,    // Int arr, Size 1,
    CCLIENT_SERVER_COMPUTE_RESULT =6,   // Int arr, size 1,
    JCLIENT_SERVER_COMPUTE_MY_DATA =7,  // int array, size huge
}Events;

/*Generic Messsage Structure for communication between
client and server*/
typedef struct Generic_message
{
    Events event;
    Result result;
    int client_id;
    int data_len;
    int data[300];
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

typedef struct compute_req
{
    int req_id;
    int client_id;
    int status;
    int result;
    int data[300];
    
}Copute;


void pop_head(Node ** head, Message *arr);
void push_tail(Node** head, Message *new_data);



int size_list(Node *head);

void dserializeTask(unsigned char* msg, Message *t);

void serializeTask(unsigned char* msg, const Message *t);

int parseStruct(char **, Message *);
int parseJson(char *, Message *);
#endif
