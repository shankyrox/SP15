/*
** epoll_server.c 
*/

#include "server.h"
#include "common.h"
/*Global variable*/
client_group client_grps[MAX_GROUP];

struct epoll_event event;
int sfd, efd;
int jclient_fd = 0;
List list;

pthread_t worker[MAX_NUM_WORKER_SERVER];
pthread_cond_t cond1;
pthread_mutex_t mutex1;
Message global_message;

/*Function declarations*/
int process_function(int *done, int event, int fd);
int accept_new_conn();
void *worker_thread_fun(void *thread_id);

/*Driver program*/
int
main (int argc, char *argv[])
{
    struct epoll_event *events;

    
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    printf("\nInitilizing Server.. \n");  
    /*Setting rlimit*/
    set_rlimit();
    
    /*Ptherad init for the worker thread*/
    pthread_mutex_init(&mutex1, 0);
    pthread_cond_init(&cond1, 0);

    init_group_val();

    sfd = create_and_bind (argv[1]);
    if (sfd == -1)
        exit (EXIT_FAILURE);

    if((make_socket_non_blocking (sfd)) == -1)
    {
        exit (EXIT_FAILURE);
    }

     /*Start listening on the opened FD*/
    if(listen (sfd, MAX_CONN_SUPORTED) == -1)
    {
        perror ("listen");
        exit (EXIT_FAILURE);
    }
   

    /*Create epoll fd*/
    efd = epoll_create(MAX_CONN_SUPORTED);
    if (efd == -1)
    {
        perror ("epoll_create");
        exit (EXIT_FAILURE);
    }
    printf("\nepoll_create done efd=%d \n", efd);

    /*Set the epoll_ctl for main epol fd*/
    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    if(epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event) == -1)
    {
        perror ("epoll_ctl");
        exit (EXIT_FAILURE);
    }

    /* Buffer where events are returned */
    events = calloc (MAX_EVENTS, sizeof (event));

    printf("\nServer initialized!! Waiting for client connections . . \n");
    /* The event loop */
    
    /*Init linked list*/
    list.head=NULL;
    list.count=0;

    int i =0, *thread_id;
    /*Creat thread*/
    for(i=0; i < MAX_NUM_WORKER_SERVER; i++) 
	{
		thread_id = MALLOC(sizeof(int));
		*thread_id = i;
        pthread_create(&worker[i], NULL, worker_thread_fun, thread_id);
    }

	// The Event loop
    while (1)
    {
        int num_fd_ready = 0, i=0;

          PRINT("\n *********** On epoll_wait  ********** \n"); 

        /*This will wait enfinitely only returns when there is an event*/
        num_fd_ready = epoll_wait (efd, events, MAX_EVENTS, -1);
        if((num_fd_ready<0) && (errno != EINTR))
        {
            perror ("epoll_wait");
            exit (EXIT_FAILURE);
        }
        for (i = 0; i < num_fd_ready; i++)
        {

            //printf("\nNew Event recieved events[%d].data.fd = %d, %d \n",i, events[i].data.fd, sfd);  

            if (sfd == events[i].data.fd)
            {
                /*If the issue is no the server socket then just exti*/
                if((events[i].events & EPOLLERR) || 
                        (events[i].events & EPOLLHUP)) 
                {
                    PRINT("\nEpoll error on server FD : %d ,Exiting.. \n", sfd);
                    exit (EXIT_FAILURE);
                }
                /* We have a notification on the listening socket, which
                   means one or more incoming connections. */
                while (1)
                {
                    if(accept_new_conn() == SUCCESS)
                    {
                        break;        
                    }
                }
                
                continue;
            }
            else
            {
                /* We have data on the fd waiting to be read. Read and
                   display it. We must read whatever data is available
                   completely, as we are running in edge-triggered mode
                   and won't get a notification again for the same
                   data. */
                int done = 0;
                /*If the issue is no the server socket then just exti*/
                if((events[i].events & EPOLLERR) || 
                        (events[i].events & EPOLLHUP)) 
                {
                    PRINT("\nEpoll error on client FD : %d , closig the connection \n", events[i].data.fd);
                    done =1;
                }
                else
                {
                    while (1)
                    {
                        if(process_function(&done, events[i].events, events[i].data.fd) == SUCCESS)
                        {
                            break;
                        }
                    } //While

                } // else

                if (done)
                {
                    PRINT ("\nClosed connection on descriptor %d\n",
                            events[i].data.fd);

                    remove_client_from_group(events[i].data.fd);
                    display_group_data();

                    /* Closing the descriptor will make epoll remove it
                       from the set of descriptors which are monitored. */
                    close (events[i].data.fd);
                }
            }
        }
    }

    free (events);
    close (sfd);

    return EXIT_SUCCESS;
}

int send_groups_to_client(int infd){
	int arr[20] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
    populate_and_send_data(SERVER_CCLIENT_GROUP_IDS_SUPPORTED, arr, 20, infd, infd);
	return SUCCESS;
}

//Should be moved to server_socket.c file later. Ensure all variables needed are present
int accept_new_conn()
{
    struct sockaddr in_addr;
    socklen_t in_len;
    int infd, s;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    in_len = sizeof in_addr;
    infd = accept (sfd, &in_addr, &in_len);
    if (infd == -1)
    {
        if ((errno == EAGAIN) ||
                (errno == EWOULDBLOCK))
        {
            // PRINT("\n Accept EAGAIN/EWOULDBLOCK, Just Break \n");
            /* We have processed all incoming
               connections. */
            return SUCCESS;
        }
        else
        {
            perror ("accept");
            return SUCCESS;
        }
    }

    s = getnameinfo (&in_addr, in_len,
            hbuf, sizeof hbuf,
            sbuf, sizeof sbuf,
            NI_NUMERICHOST | NI_NUMERICSERV);
    if (s == 0)
    {
        PRINT("Accepted connection on descriptor : %d ", infd);
    }


    s = make_socket_non_blocking (infd);
    if (s == -1)
        exit (EXIT_FAILURE);

    event.data.fd = infd;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
    if (s == -1)
    {
        perror ("epoll_ctl");
        exit (EXIT_FAILURE);
    }

	//Send available groups to join to client
	send_groups_to_client(infd);

	return SUCCESS;
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
		   // PRINT("\nRead EAGAIN, Just Break, events[%d].data.fd = %d \n", i, events[i].data.fd);
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
		
		display_group_data();
		/*Deserialize the buf and put it into Message buffer */
		Message msg_buff = {0};

		parseJson(buf, &msg_buff);
		msg_buff.client_id = fd;
		pthread_mutex_lock(&mutex1);
		push_tail(&list.head, &msg_buff);
		
		pthread_cond_broadcast(&cond1);

		pthread_mutex_unlock(&mutex1);

		PRINT("\n MESSAGE RECEIVED : \n");
		display_message(&msg_buff);
		
    }
    else if (evt & EPOLLOUT)
    {
		printf("evt is epollout");
    }

   return SUCCESS;
}

void divide_array_and_send(int cfd, int start, int end, int *array)
{
    int i, *data, data_size;
	data_size = end-start;
	data = MALLOC(data_size * sizeof (int));
    for(i =0; i<data_size; i++)
    {
        data[i] = array[start++];    
    }
    populate_and_send_data(SERVER_CCLIENT_DATA_TO_COMPUTE, data, data_size, cfd, cfd);
}

int get_mcast_index()
{
    int mcast_id = -1;

    for(mcast_id=0; mcast_id<MAX_GROUP; mcast_id++)
    {
        if(client_grps[mcast_id].num_of_client >= 1)
        {
          return mcast_id;
       	}
    }
    return -1;
}

int divide_work(int *array, int data_num)
{
    int g, i, data_per_client=0, num_clients_to_use = 0, num_clients=0;
    float a, b, ratio = 0;
    unsigned char start, end;
    
    g = get_mcast_index();
	if (g < 0){
		printf ("ERROR : No available groups\n");
		return FAILURE;
	}
    num_clients = client_grps[g].num_of_client;

    a = data_num;
    b = num_clients;
    ratio = a/b;

    if(ratio == 1)
    {
        /* Number of clients = Number of data items */
        num_clients_to_use = num_clients/4;
		if(num_clients_to_use == 0) num_clients_to_use = 1;
        data_per_client = data_num/num_clients_to_use;
    }
    else if(ratio < 1)
    {
        if (data_num <= 10) {
        /* Number of clients > Number of data items */
			if((num_clients - data_num) >= data_num)
			{
				num_clients_to_use = (num_clients-data_num)/2;
			}
			else
			{
				num_clients_to_use = num_clients/2;
			}
			data_per_client = data_num/num_clients_to_use;
      }
      else if (data_num <= 100){
        if (ratio <= 0.9 && ratio > 0.5)
        { 
            num_clients_to_use = (data_num/3);
        }
        else
        {
           num_clients_to_use = (data_num/5);
        }       
        data_per_client = data_num/num_clients_to_use;
      }
      else if (data_num <= 1000) {
        if (ratio <= 0.9 && ratio > 0.5)
        { 
            num_clients_to_use = (data_num/30);
        }
        else
        {
           num_clients_to_use = (data_num/50);
        }       
        data_per_client = data_num/num_clients_to_use;
      }
      else if (data_num <= 10000) {
        if (ratio <= 0.9 && ratio > 0.5)
        { 
            num_clients_to_use = (data_num/300);
        }
        else
        {
           num_clients_to_use = (data_num/500);
        }       
        data_per_client = data_num/num_clients_to_use;
      }
    }
    else
    {
        /* Number of clients < Number of data items */
        if((data_num - num_clients) < num_clients)
        {
            num_clients_to_use = num_clients/2; 
        }
        else
        {
            num_clients_to_use = num_clients;
        }
		if (num_clients_to_use == 0) num_clients_to_use++;
        data_per_client = data_num/num_clients_to_use;
    }

    start = 0;
    end  = data_per_client;

    for(i=0; i < num_clients_to_use; i++)
    {
		if(end > data_num)
		end = data_num;

	    divide_array_and_send(client_grps[g].client_id[i], start, end, array);
		client_grps[g].active_clients++;
		start = end;
		end = end + data_per_client;
    }
    return SUCCESS;
}

/*
int divide_work(int *array, int numentry)
{
	int i, g, div;
    g = get_mcast_index();
    div = numentry/client_grps[g].num_of_client;
  	unsigned char start, end;


    if(div > 2)
    {
        start = 0;
        end = div;
     for (i=0; i < client_grps[g].num_of_client; i++) 
     {
    
            if(end > numentry)
                end = numentry;

            divide_array_and_send(client_grps[g].client_id[i],start, end, array);
            
            start= end;
            end = end + div;
           
     }
     return SUCC;
    }
    else
        return -1;
}

void compute_the_result_and_send(Message *msg)
{
    int max[300];
    if(!msg)
        return;

    int i=0; 
	max[0] = msg->data[0];
    for(i=1; i< msg->data_len; i++)
    {
        if(max[0] < msg->data[i])
            max[0] = msg->data[i];
    }

    //populate_and_send_data(SERVER_JCLIENT_FINAL_COMPUTE_RESULT, max, 1, msg->client_id, msg->client_id);
}


void process_new_compute_req(Message *msg)
{

    static int id = 0;
    current_comp_req.client_id = msg->client_id;
    current_comp_req.req_id = ++id;
    current_comp_req.status = 0;
    current_comp_req.result = 0;
    memcpy(&current_comp_req.data, msg->data, sizeof(int)*msg->data_len);

   if(-1 == divide_work(msg->data, msg->data_len))
   {
       //compute_the_result_and_send(msg);
   }
}
*/

void process_new_compute_req(Message *msg){
	divide_work(msg->data, msg->data_len);
}

void set_jclient_fd(Message *msg){
	int fd = msg->client_id;	
	if (fd <= 0){
		printf("ERROR: Invalid jclient_fd received\n");
		return ;
	}
	if (jclient_fd != 0){
		printf("Deleting existing jclient_fd and adding new\n");
		close(jclient_fd);
	}
	printf("Adding new job client. jclient_fd = %d\n", fd);
	jclient_fd = fd;
}

/* Server event handler */
void event_handler(Message *data)
{
    switch(data->event)
    {
        case CCLIENT_SERVER_GROUP_ID_TO_JOIN : 
            handle_group_join(data);
	        break;

        case CCLIENT_SERVER_GROUP_ID_EXIT: 
            // handle_group_exit(data);
			break;
        
        case CCLIENT_SERVER_COMPUTE_RESULT: 
            collate_results(data);
        	break;
        
        case JCLIENT_SERVER_COMPUTE_MY_DATA:
            process_new_compute_req(data);
			break;

		case JCLIENT_SERVER_JOIN:
			set_jclient_fd(data);
			break;

        default : 
             PRINT("\nNot a valid Event %d\n", data->event);
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
            pop_head(&(list.head), data); 
			printf("\n\n");
            event_handler(data);  
            free(data);
        }
        pthread_mutex_unlock(&mutex1);
    }
}

