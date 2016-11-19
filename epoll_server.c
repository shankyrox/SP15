/*
** epoll_server.c 
*/

#include "server.h"
#include "common.h"
/*Global variable*/
client_group client_grps[MAX_GROUP];

struct epoll_event event;
int sfd, efd;
List list;


Compute current_comp_req = {0};

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

            //  PRINT("\nNew Event recieved events[%d].event = %d \n",i, events[i].events);  

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
                while (SUCCESS)
                {
                    if(accept_new_conn())
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
                    while (SUCCESS)
                    {
                        if(!(process_function(&done, events[i].events, events[i].data.fd)))
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


int accept_new_conn()
{
    struct sockaddr in_addr;
    socklen_t in_len;
    int infd, s;
    char buf[BUFFSIZE];
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
    memset(buf, 0x00,  strlen(buf));

   int arr[20] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

    populate_and_send_data(SERVER_CCLIENT_GROUP_IDS_SUPPORTED, arr, 20, infd, infd);

    
return SUCCESS;
}



int process_function(int *done, int evt, int fd)
{
    char buf[BUFFSIZE];
    ssize_t count;
    memset(buf, 0x00,  strlen(buf));
                       
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
        return FAILURE; 
    }
    buf[count] = '\0';
    
    /*Deserialize the buf and put it into Message buffer */
    Message msg_buff = {0};

    parseJson(buf, &msg_buff);
    msg_buff.client_id = fd;

    pthread_mutex_lock(&mutex1);

    push_tail(&list.head, &msg_buff);
    
    pthread_cond_broadcast(&cond1);

    pthread_mutex_unlock(&mutex1);

  //  PRINT("\nsize_list = %d\n", size_list(list.head));

    PRINT("\n Main thread : Rcvd data from fd = %d :: %s \n",fd, buf);


    
    }
    else if (evt & EPOLLOUT)
    {

        
    }

   return SUCCESS;
}

void handle_group_join(Message *msg)
{

      PRINT("\nClient wanted to joing group : %d\n", msg->data[0]);
        /*Add client to group*/
        if(!add_client_to_group(msg->client_id, msg->data[0]))
        {
            PRINT("\n Group Join failed for msg->client_id =  %d ", msg->client_id);
            close(msg->client_id);
            return;
        }
        //display the full group data
        display_group_data(); 
}

void divide_array_and_send(int cfd, int start,int end,int *array)
{
    
    int i, data[300];
    for(i =0; i<end; i++)
    {
        data[i] = array[start++];    
    }

    populate_and_send_data(SERVER_CCLIENT_DATA_TO_COMPUTE, data, i, cfd, cfd);
}

int get_mcast_index()
{


    int mcast_id = -1;

    for(mcast_id=0; mcast_id<MAX_GROUP; mcast_id++)
    {
        
        if(client_grps[mcast_id].num_of_client >= 2)
        {
          
          return mcast_id;
       }
    }
    return -1;
}


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

    populate_and_send_data(SERVER_JCLIENT_FINAL_COMPUTE_RESULT, max, 1, msg->client_id, msg->client_id);
}

void process_new_compute_req(Message *msg)
{

    static int id = 0;
    current_comp_req.client_id = msg->client_id;
    current_comp_req.req_id = ++id;
    current_comp_req.status = 0;
    current_comp_req.result = 0;
    memcpy(&current_comp_req.data, msg->data, sizeof(int)*msg->data_len);

   if(-1 == divide_work(current_comp_req.data, msg->data_len))
   {
       compute_the_result_and_send(msg);
   }
}


/* Client */
void event_handler(Message *data)
{
    //Verify the data here
    PRINT("\nevent_handler event = %d\n", data->event);
    
    switch(data->event)
    {
        case CCLIENT_SERVER_GROUP_ID_TO_JOIN : 
            handle_group_join(data);
        break;

        case CCLIENT_SERVER_GROUP_ID_EXIT: 
           // handle_group_exit(data);
        break;
        
        case CCLIENT_SERVER_COMPUTE_RESULT: 
            //collect_compute_result_and_process(data);
        break;
        
        case JCLIENT_SERVER_COMPUTE_MY_DATA:
            process_new_compute_req(data);
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
            pop_head(&(list.head), data); //use double pointer here

            PRINT("\nsize_list after pop = %d\n", size_list(list.head));
            event_handler(data);  
            free(data);
        
        }
        pthread_mutex_unlock(&mutex1);
    }
}

