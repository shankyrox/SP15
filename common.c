#include <stdlib.h>
#include "common.h"

unsigned char * serialize_int(unsigned char *buffer, int value)
{
    buffer[0] = value >> 24;
    buffer[1] = value >> 16;
    buffer[2] = value >> 8;
    buffer[3] = value;
    return buffer + 4;
}

unsigned char * serialize_char(unsigned char *buffer, char value)
{
    buffer[0] = value;
    return buffer + 1;
}

int deserialize_int(unsigned char *buffer)
{
    int value = 0;

    value |= buffer[0] << 24;
    value |= buffer[1] << 16;
    value |= buffer[2] << 8;
    value |= buffer[3];
    return value;

}

char deserialize_char(unsigned char *buffer)
{
    return buffer[0];
}

void  serializeTask(unsigned char* msg, const Message *t)
{
    int i=0;
    msg = serialize_int(msg,t->event);
    msg = serialize_int(msg,t->result);
    msg = serialize_int(msg,t->data_len);
    for(i=0; i< t->data_len; i++)
            msg = serialize_int(msg,t->data[i]);
}


void dserializeTask(unsigned char* msg, Message *t)
{
    int i = 0;
    t->event = deserialize_int(msg);
    t->result = deserialize_int(msg+4);
    t->data_len = deserialize_int(msg+8);
    int size = 8;
    for(i=0; i<t->data_len; i++)
    {
        t->data[i] = deserialize_int(msg+ size);
    }
}

//Need to write error handling code/invalid data received check in each function

const char* eventEnumToStr[] = {
		"SERVER_CCLIENT_GROUP_IDS_SUPPORTED",
		"SERVER_CCLIENT_CONNECTION_ACCEPTED",
	  	"SERVER_CCLIENT_DATA_TO_COMPUTE",
	    "SERVER_JCLIENT_FINAL_COMPUTE_RESULT",
	    "CCLIENT_SERVER_GROUP_ID_TO_JOIN",
	    "CCLIENT_SERVER_GROUP_ID_EXIT",
	    "CCLIENT_SERVER_COMPUTE_RESULT",
	    "JCLIENT_SERVER_COMPUTE_MY_DATA",
		"MAX_CLIENT_SERVER_EVENTS"
	};

const char* resultEnumToStr[] = {
		"FAIL",
		"SUCCESS"	
	};

/*
static int parseStringStructToJson(json_t *obj, Message *msg){
	json_object_set(obj, "event", json_integer(msg->event));
	json_object_set(obj, "result", json_integer(msg->result));
	json_object_set(obj, "data_len", json_integer(msg->data_len));
	json_object_set(obj, "data", json_string(msg->data));
	return SUCCESS;
}
*/

static int parseIntStructToJson(json_t *obj, Message *msg){
	int x = 0, i;
	json_object_set(obj, "event", json_integer(msg->event));
	json_object_set(obj, "result", json_integer(msg->result));
	json_object_set(obj, "data_len", json_integer(msg->data_len));
	json_object_set(obj, "client_id", json_integer(msg->client_id));
	json_t *temp_arr = json_array();
	for (i=0; i < msg->data_len; i++) {
		x = msg->data[i];
		json_array_append(temp_arr,json_integer(x));
	}
	json_object_set(obj, "data", temp_arr);
	return SUCCESS;
}

int parseStruct(char **str, Message *msg){
	json_t *obj = json_object();
	switch(msg->event){
		case SERVER_CCLIENT_CONNECTION_ACCEPTED : ;
		case SERVER_CCLIENT_GROUP_IDS_SUPPORTED : ;
		case SERVER_CCLIENT_DATA_TO_COMPUTE : ;
		case SERVER_JCLIENT_FINAL_COMPUTE_RESULT : ;
		case CCLIENT_SERVER_GROUP_ID_TO_JOIN : ;
		case CCLIENT_SERVER_GROUP_ID_EXIT : ;
		case CCLIENT_SERVER_COMPUTE_RESULT : ;
		case JCLIENT_SERVER_COMPUTE_MY_DATA :
			parseIntStructToJson(obj, msg);
			break;
		default : return -1;
	}
	*str = json_dumps(obj,0);
	json_decref(obj);
	return SUCCESS;
}

/*
static int parseJsonToStringStruct(json_t *obj, Message *msg){
	msg->result = json_integer_value(json_object_get(obj, "result"));
	msg->data_len = (unsigned int)json_integer_value(json_object_get(obj, "data_len"));
	msg->data = (char *)malloc(msg->data_len);
	strcpy(msg->data, json_string_value(json_object_get(obj, "data")));
	return SUCCESS;
}
*/

static int parseJsonToIntStruct(json_t *obj, Message *msg){
	int x = 0, i;
	msg->result = json_integer_value(json_object_get(obj, "result"));
	msg->data_len =(int)json_integer_value(json_object_get(obj, "data_len"));
	msg->client_id = json_integer_value(json_object_get(obj, "client_id"));
	msg->data = (int *)malloc(msg->data_len*sizeof(int));
	json_t *temp_arr = json_object_get(obj, "data");
	for (i=0; i < msg->data_len; i++){
		x = json_integer_value(json_array_get(temp_arr, i));
		msg->data[i] = x;
		//memcpy((int *)msg->data+i, &x, sizeof(int));
	}
	return SUCCESS;
}

int parseJson(char *strMsg, Message *msg){
	json_error_t err = {0};
	json_t *obj = json_loads( strMsg, 0, &err);
	msg->event = json_integer_value(json_object_get(obj, "event"));
	printf("event p = %d\n", msg->event);
	switch(msg->event){
		case SERVER_CCLIENT_CONNECTION_ACCEPTED : 
		case SERVER_CCLIENT_GROUP_IDS_SUPPORTED : 
		case SERVER_CCLIENT_DATA_TO_COMPUTE : 
		case SERVER_JCLIENT_FINAL_COMPUTE_RESULT :
		case CCLIENT_SERVER_GROUP_ID_TO_JOIN : 
		case CCLIENT_SERVER_GROUP_ID_EXIT :
		case CCLIENT_SERVER_COMPUTE_RESULT : 
		case JCLIENT_SERVER_COMPUTE_MY_DATA :
			parseJsonToIntStruct(obj, msg);
			break;
		default : 
			json_decref(obj);
			return -1;
	}
	json_decref(obj);
	return SUCCESS;
}


int populate_and_send_data(int event, int *data, int datalen, int fd, int client_id)
{
    Message msg= {0};
	char *msg_str = NULL;
    msg.client_id = client_id;
    msg.event = event;
    msg.data_len = datalen;
    msg.result = 1;
	msg.data = MALLOC(datalen * sizeof(int));
    memcpy(msg.data, data, sizeof(int)*datalen);
    parseStruct(&msg_str, &msg);

    if(-1 ==  send(fd, msg_str, strlen(msg_str), 0))
    {
        perror("send");
        return -1;
    }
	return SUCCESS;
}

