#include "common.h"
#include "client.h"

extern int sockfd; //server fd to send data

static int selectGroup(int nGroups)
{
    int randIdx, x;
    struct timeval curr_time;
	
    srand(time(NULL));
    gettimeofday(&curr_time, NULL);

/*
    while(groupId != NULL)
    { 
        groups[nGroups] = groupId;
        printf("  %s",groups[nGroups]);
        nGroups+=1;
        groupId = strtok(NULL, ",");
    }
*/
    x= curr_time.tv_usec % nGroups;

    randIdx = (rand() + x) % nGroups;
    return randIdx;
}

int select_group_join(Message *msg){
	Events event_id = CCLIENT_SERVER_GROUP_ID_TO_JOIN;
	int nGroups = msg->data_len;
	int grp_idx = selectGroup(nGroups);
	grp_idx = grp_idx < 0 ? 0 : grp_idx; //To make sure index is not -ve
	int grp_to_join = msg->data[grp_idx];
	// We will have to make client aware of the client id for future 
	// purposes when we introduce heartbeat mechanism
	populate_and_send_data(event_id, &grp_to_join, 1, sockfd, sockfd);
	return SUCCESS;
}
