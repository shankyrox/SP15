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

//Returns the index of maximum element in the array
int find_maximum(int *a, int n) {
  int c, max, index;
  if (n <=0) return -1;
 
  max = a[0];
  index = 0;
 
  for (c = 1; c < n; c++) {
    if (a[c] > max) {
       index = c;
       max = a[c];
    }
  }
 
  return index;
}

//Returns the index of minimum element in the array
int find_minimum(int *a, int n) {
  int c, min, index;
  if (n <=0) return -1;
 
  min = a[0];
  index = 0;
 
  for (c = 1; c < n; c++) {
    if (a[c] < min) {
       index = c;
       min = a[c];
    }
  }
 
  return index;
}

//Need to have a field which specifies what is the job required to be done
//Assuming that need to compute maximum
int compute_data(Message *msg){
	int *arr, size, result;
	arr = msg->data;
	size = msg->data_len;
	result = find_maximum(arr, size);

	//create a new msg for result and send to server
	populate_and_send_data(CCLIENT_SERVER_COMPUTE_RESULT, &result, 1, sockfd, msg->client_id);
	return SUCCESS;
}
