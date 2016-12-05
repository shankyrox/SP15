/*
** client.c 
*/
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "common.h"

int efd, sockfd;
struct epoll_event event;
pthread_t worker;

int process_function(int *done, int event, int fd);
int submit_job();
void event_handler(Message *);
void *worker_thread_fun();

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
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
	
	/* Create thread for accepting job from user*/
	pthread_create(&worker, NULL, worker_thread_fun, NULL);

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
		

        /*This will wait infinitely only returns when there is an event*/
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
					if(process_function(&done, events[i].events, events[i].data.fd) == SUCCESS)
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
			return SUCCESS; 
		}
		buf[count] = '\0';
		
		/*Deserialize the buf and put it into Message buffer */
		Message msg_buff = {0};

		parseJson(buf, &msg_buff);
		printf("\n Main thread : Rcvd data from fd = %d :: %s \n",fd, buf);
		event_handler(&msg_buff);


     }
    else if (evt & EPOLLOUT)
    {

        
    }
   return SUCCESS;
}

void process_final_compute_result(Message *msg){
	int data_len, *data;
	data_len = msg->data_len;
	if (data_len != 1) 
		PRINT("INVALID DATA RECEIVED \n");
	data = msg->data;
	PRINT("Final result = %d\n", data[0]);
}

/* Event handler for job_client  */
void event_handler(Message *data)
{
    switch(data->event)
    {
		case SERVER_CCLIENT_GROUP_IDS_SUPPORTED:
			populate_and_send_data(JCLIENT_SERVER_JOIN, NULL, 0, sockfd, sockfd);
			break;
		
        case SERVER_JCLIENT_FINAL_COMPUTE_RESULT:
            process_final_compute_result(data);
        	break;
		
        default : 
            PRINT("\nNot a valid Event %d\n", data->event);
       	 	break;
 	}
}

#define MAX_FILENAME_LEN 256
int submit_job(){
	char filename[MAX_FILENAME_LEN];
	FILE *fp = NULL;
	int op, ctr, data_len, *data;
	printf("\nEnter job file name : ");
	scanf("%s",filename);
	fp = fopen(filename, "r");
	if (!fp) { 
		printf("Unable to open file \n");
		return FAILURE;
	}
	fscanf(fp, "%d %d", &op, &data_len);
	data = malloc(data_len * sizeof(int));
	for (ctr = 0; ctr < data_len; ctr ++){
		fscanf(fp, "%d", &data[ctr]);
		printf("data[%d] = %d\n", ctr, data[ctr]);
	}	 
	fclose(fp);
	populate_and_send_data(JCLIENT_SERVER_COMPUTE_MY_DATA, data, data_len, sockfd, sockfd);
	return SUCCESS; // add error handling code to return failure 
}

void *worker_thread_fun(){
//wait for user to submit job
	while(1) {
		submit_job();
	}
}
