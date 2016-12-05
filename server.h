/*
** server.h 
*/

#ifndef _SERVER_DEF_
#define _SERVER_DEF_

#include <netdb.h>
#include <unistd.h>
#include "common.h"
#include <fcntl.h>

#include <sys/resource.h>

#define MAX_CONN_SUPORTED 1000

#define MAX_GROUP 20
#define MAX_CLIENTS_PER_GROUP 100
#define MAX_NUM_WORKER_SERVER 4

/*Client Group structure*/
typedef struct client_groups {
    int grp_id;
    unsigned int num_of_client;
    int client_id[MAX_CLIENTS_PER_GROUP];
	unsigned int active_clients;
}client_group;

struct {
	int result_arr_size;
	int result_arr[MAX_CLIENTS_PER_GROUP];
}grp_results[MAX_GROUP];

/*Function signatures*/
void init_group_val() ;
int handle_group_join(Message *);
void remove_client_from_group(int c_id);
void display_group_data() ;


void set_rlimit();
int make_socket_non_blocking (int sfd);
int create_and_bind (char *port);

#endif
