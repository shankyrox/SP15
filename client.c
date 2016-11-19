/*
** client.c 
*/
#include <stdio.h>
#include <stdlib.h>
#include "client.h"
#include "common.h"
#include "server.h"

pthread_t worker[MAX_NUM_WORKER_CLIENT];
pthread_cond_t cond1;
pthread_mutex_t mutex1;
int client_id = 0xffff;

int efd;

List list;

int process_function(int *done, int event, int fd);
void *worker_thread_fun(void *thread_id);

char* selectGroup(char *msg)
{
    char *groups[MAX_GROUPS], *groupId;
    int nGroups = 0, randIdx, x= 0;
    srand(time(NULL));
    groupId = strtok(msg, ",");

    struct timeval time;
    gettimeofday(&time, NULL);

    printf("\nGroup ids supported at server are : ");
    while(groupId != NULL)
    { 
        groups[nGroups] = groupId;
        printf("  %s",groups[nGroups]);
        nGroups+=1;
        groupId = strtok(NULL, ",");
    }

    x= time.tv_usec % nGroups;

    randIdx = ( rand() + x) % nGroups;
    return groups[randIdx];
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct epoll_event event;

int main(int argc, char *argv[])
{
    int sockfd;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 3) {
        fprintf(stderr,"usage: client <server add> <server port>\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    /*Init linked list*/
    list.head=NULL;
    list.count=0;

    int i =0, *thread_id;
    /*Creat thread*/
    for(i=0; i < MAX_NUM_WORKER_CLIENT; i++)
    {    
		thread_id = MALLOC(sizeof(int));
		*thread_id = i;
        pthread_create(&worker[i], NULL,  worker_thread_fun, thread_id);
    }


    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        /*Create socket*/
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) 
        {
            perror("client: socket");
            continue;
        }

        /*Connect*/
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    efd = epoll_create(10);
    event.data.fd = sockfd;
    event.events = EPOLLIN | EPOLLET;
    if(epoll_ctl (efd, EPOLL_CTL_ADD, sockfd, &event) == -1)
    {
        perror ("epoll_ctl");
        exit (EXIT_FAILURE);
    }

     struct epoll_event *events  = calloc (MAX_EVENTS, sizeof event);


    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("\nconnecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

  while (1)
    {
        int num_fd_ready =0, i=0, done =0;

        //  printf("\n *********** On epoll_wait  ********** \n"); 

        /*This will wait enfinitely only returns when there is an event*/
        num_fd_ready = epoll_wait (efd, events, MAX_EVENTS, -1);
        if((num_fd_ready<0) && (errno != EINTR))
        {
            perror ("epoll_wait");
            exit (EXIT_FAILURE);
        }
        for (i = 0; i < num_fd_ready; i++)
        {

            if((events[i].events & EPOLLERR) || 
                        (events[i].events & EPOLLHUP)) 
                {
                   perror("epoll_wait");
                   exit(EXIT_FAILURE); 
                }
                
                while (1)
				{
					if(!(process_function(&done, events[i].events, events[i].data.fd)))
					{
						break;
					}
				} //While
				if(done)
                   exit(EXIT_FAILURE); 
        }
    }

    close(sockfd);

    return 0;
}

int process_function(int *done, int evt, int fd)
{
    char buf[BUFFSIZE];
    ssize_t count;

    if(evt & EPOLLIN)
    {
		count = read (fd, buf, sizeof buf);
		if (count == -1)
		{
			/* If errno == EAGAIN, that means we have read all
			   data. So go back to the main loop. */
			if (errno != EAGAIN)
			{
				perror ("read");
				*done = 1;
			}
		   // printf("\nRead EAGAIN, Just Break, events[%d].data.fd = %d \n", i, events[i].data.fd);
		   return FAILURE; 
		}
		else if (count == 0)
		{
			/* End of file. The remote has closed the
			   connection. */
			*done = 1;
			return FAILURE; 
		}
		buf[count] = '\0';
		
		/*Deserialize the buf and put it into Message buffer */
		Message msg_buff = {0};

		parseJson(buf, &msg_buff);
		//msg_buff.client_id = fd;
	 
	   /**/ 
		pthread_mutex_lock(&mutex1);
		push_tail(&list.head, &msg_buff);
		pthread_cond_broadcast(&cond1);
		pthread_mutex_unlock(&mutex1);

	  //  printf("\nsize_list = %d\n", size_list(list.head));
		printf("\n Main thread : Rcvd data from fd = %d :: %s \n",fd, buf);

     }
    else if (evt & EPOLLOUT)
    {

        
    }

   return SUCCESS;
}

/* Client */
void event_handler(Message *data)
{
    //Verify the data here
    PRINT("\nevent_handler event = %d\n", data->event);
    
    switch(data->event)
    {
        case SERVER_CCLIENT_GROUP_IDS_SUPPORTED: 
            //select_group_join(data);
        break;

        case SERVER_CCLIENT_CONNECTION_ACCEPTED: 
           // handle_group_exit(data);
        break;
        
        case SERVER_CCLIENT_DATA_TO_COMPUTE: 
            //compute_data_and_process(data);
        break;
        
        case SERVER_JCLIENT_FINAL_COMPUTE_RESULT:
           // process_final_compute_result(data);
        break;
        default : 
             PRINT("\nNot a valid Event %d\n", data->event);
        break;
 }

}

void *worker_thread_fun(void *thread_id)
{
    int mythread_id = *(int *)thread_id; 
	free (thread_id);
    Message *data;


    printf("\nWorker thread [%d] initialized!!\n", mythread_id);
    while(1)
    {
        pthread_mutex_lock(&mutex1);
        while(size_list(list.head) < 1)
        {
            pthread_cond_wait(&cond1, &mutex1);
        }

        while(size_list(list.head))
        {
            data = (Message*)malloc(sizeof(Message));
            pop_head(&(list.head), data);  //use double pointer

            PRINT("\nsize_list after pop = %d\n", size_list(list.head));
            event_handler(data);
            free(data);
        
        }
        pthread_mutex_unlock(&mutex1);
    }
}

