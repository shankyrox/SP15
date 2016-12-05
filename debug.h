#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<time.h>
#include<string.h>

//macro defn
typedef enum {INFO , WARNING , ERROR , FATAL , DEBUG} log_level;
int dbg_flag = 0;
int msg_size = 1000;
int to_file = 1;

//define timestamp
char *file = "sys_log";
FILE *fptr;

char* get_curr_time (void);
void create_log_file (char *);
void delete_log_file (void);
void write_to_logfile(char *);
void print_func_stack(void);
void kill_the_proc(void);
void print_sys_log(char *msg, log_level var);
void print_dbg_log(char *msg);
