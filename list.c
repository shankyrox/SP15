#include<stdio.h>
#include"common.h"

void push_tail(Node** head, Message *new_data)
{
    Node* new_node = (struct node*) malloc(sizeof(Node));
    Node * tmp = *head;

    memcpy(&new_node->data, new_data, sizeof(Message));
	new_node->data.data = MALLOC(new_data->data_len * sizeof(int));
	memcpy(new_node->data.data, new_data->data, sizeof(int)*new_data->data_len); 
    new_node->next =  NULL;

    if(*head == NULL)
    {
        *head = new_node;
    }
    else
    {
        while(tmp->next != NULL){
           tmp = tmp->next;    
		}
        tmp->next = new_node;
    }
}

void pop_head(Node ** head, Message *arr)
{
    if(*head == NULL)
    {    
        return ;
    }
    else
    {
        Node *tmp = *head;
        *head= (*head)->next;
        memcpy(arr, &tmp->data, sizeof(Message));
		arr->data = MALLOC(arr->data_len * sizeof(int));
		memcpy(arr->data, tmp->data.data, arr->data_len * sizeof(int));
        free(tmp->data.data);
		free(tmp);
        /*Must free the node*/
    }
}

int size_list(Node *head)
{
    int count=0;
    Node *tmp = head;
    while(tmp != NULL)
    {
        count ++;
        tmp=tmp->next;
        
        }
    return count;
 }
