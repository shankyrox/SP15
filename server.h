/*
** server.h 
*/

#ifndef _SERVER_DEF_
#define _SERVER_DEF_

#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/resource.h>

#define MAX_EVENTS 64
#define MAX_CONN_SUPORTED 1000

#define MAX_GROUP 20
#define MAX_CLIENTS_PER_GROUP 100
#define MAX_NUM_WORKER_SERVER 4

/*Client Group structure*/
typedef struct client_groups {
    int grp_id;
    unsigned int num_of_client;
    int client_id[MAX_CLIENTS_PER_GROUP];
}client_group;

/*Function signatures*/
void init_group_val() ;
int add_client_to_group(int c_id, int g_id);
void remove_client_from_group(int c_id);
void display_group_data() ;


void set_rlimit();
int make_socket_non_blocking (int sfd);
int create_and_bind (char *port);

#endif
