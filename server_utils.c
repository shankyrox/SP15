#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include "common.h"

extern client_group client_grps[MAX_GROUP];
extern int jclient_fd;

/*Initialize group structure*/
void init_group_val() 
{
    int i=0,j=0;
    for(i=0 ; i<MAX_GROUP; i++)
    {
        client_grps[i].num_of_client = 0;
        client_grps[i].grp_id = i;
        for(j=0 ; j<MAX_CLIENTS_PER_GROUP; j++)
        {
            client_grps[i].client_id[j]= 0xffff;
        }
    }
    printf("\nGroup structure init done!\n");
}

/*Add the client to a group*/
static int add_client_to_group(int c_id, int g_id) 
{

    int n = client_grps[g_id].num_of_client;

    if(g_id >= MAX_GROUP) 
    {
        printf("ERROR:: Group doesn't exist\n");
        return FAILURE;
    } 
    else 
    {
        if (n == MAX_CLIENTS_PER_GROUP) 
        {
            printf("ERROR::Max client per group exceeded..\n");
            return FAILURE;
        }
        else
        {
            client_grps[g_id].client_id[n] = c_id;
            client_grps[g_id].num_of_client++;
        }
    }

    return SUCCESS;
}     

/*Remove the client from group*/     
static int remove_client_from_group(int c_id) 
{
    int g=0, i=0;
    int flag = 0 , tmp =0;

	//Handle job_client connection closed
	if (c_id == jclient_fd){
		printf("job_client closed \n");
		jclient_fd = 0;
		return FAILURE;
	}

    for(g=0; (g<MAX_GROUP) && (flag!=1); g++)
    {
        for (i=0; i < client_grps[g].num_of_client; i++) 
        {
            if(client_grps[g].client_id[i] == c_id)
            {
                tmp = i;
                while(tmp < client_grps[g].num_of_client)
                {
                    client_grps[g].client_id[tmp] = client_grps[g].client_id[tmp + 1];
                    tmp++;
                }
                client_grps[g].num_of_client--;
                flag = 1 ;
                printf("\nClient %d removed successfuly \n", c_id);
                return SUCCESS;
            }
        }
    }
    if (flag == 0) 
    {
        printf("ERROR:: Client not found in any group \n");
        return FAILURE;
    }
}

void display_group_data() 
{
    int i=0,j=0 ;
    unsigned int count=0;

    for(i=0;i<MAX_GROUP;i++)
    {
        printf("\nGroup[%d]:: ", i);
        for (j =0 ; j< client_grps[i].num_of_client; j++  ) 
        {
            printf(" %d ",client_grps[i].client_id[j]);
            count++;
            if(j==50)
                printf("\n");
        }
    }
         
    printf("\n\nServer: Total number of clients active at the moment = %d \n", count);
}

int handle_group_join(Message *msg)
{

	PRINT("\nClient wanted to joing group : %d\n", msg->data[0]);
   	//Add client to group
   	if(add_client_to_group(msg->client_id, msg->data[0])!=SUCCESS)
   	{
   		printf("\n Group Join failed for msg->client_id =  %d ", msg->client_id);
        close(msg->client_id);
        return FAILURE;
    }
    //display the full group data
    display_group_data(); 
	return SUCCESS;
}

int handle_group_exit(Message *msg)
{
      char *tmp;
      snprintf(tmp, 1000,"\nClient %d to be removed from group %d",msg->client_id, msg->data[0]);
      print_sys_log(tmp , INFO);
      if(remove_client_from_group(msg->client_id , msg->data[0]) != SUCCESS)
         {
           print_sys_log("\nClient removal failed" , ERROR);
           return FAILURE;
         }
     close (msg->client_id);
     display_group_data();
     return SUCCESS;
}


