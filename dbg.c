#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<time.h>
#include<string.h>
#include<execinfo.h>
#include "debug.h"

char* get_curr_time ()
{
    time_t rawtime;
    struct tm *timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    //printf ( "Current local time and date: %s", asctime (timeinfo) );
    return(asctime(timeinfo));
}

void create_log_file (char *file) 
{
   /*gets created at the run time if the flag is mentioned
    uses current working dir to create the files */
    snprintf(file , "_%s_syslog", get_curr_time());
    fptr = fopen(file , "w");
    return ;
}

void delete_log_file ()
{
   if (!fptr) {
      printf ("\n Null pointer..can't delete the file\n");
      return ;
    }
   fclose(fptr);
   int ret = remove(file);
   if (ret == 0)
      printf("\n File %s deleted successfully ", file);
   return ;
}

void write_to_logfile(char  *msg) {
   if (!fptr) {
     printf ("\n NULL pointer..can't write into the file");
     return ;  
   }
  fprintf(fptr,"%s",msg);
  return ;
}

//fill it in later
void print_func_stack() {
   void *arr[10];
   size_t size, i;
   char **fname;
   char *tmp_str = malloc(1000);
   size = backtrace(arr,10);
   fname = backtrace_symbols(arr,size);
   for (i=0; i<size ; i++ ) {
       snprintf(tmp_str , 1000 ,fname[i]);
     }    
   strcat("\nTraceback:",tmp_str);
   printf("%s", tmp_str);
   if (to_file) {
     write_to_logfile(tmp_str);
   }
return;
}

//fill it in later
void kill_the_proc() {
 return;
}

void print_sys_log(char *msg, log_level var ) 
{
   char *tmp_str = malloc(1000);
   int pid=0; // get the pid later
   //check if msg contains all fields necessary later
   if (!msg) {
      printf("\n Null string returned for log message \n");
      return;
     }
 
  //form the log string
  snprintf(tmp_str, msg_size,"%s::%u:%s", get_curr_time() , var ,msg);
  if (to_file) {
     write_to_logfile(tmp_str);
    }
  switch (var) {
     case INFO:
         printf("\n%s",tmp_str); 
         break;
     case WARNING:
         printf("\n%s", tmp_str);
         break;
         //check if # of warning before calling err is required , later
     case ERROR:
         printf("\n%s", tmp_str);
         printf("\nTraceback:");
         (void) print_func_stack();
         break;
     case FATAL:
         printf("\n%s", tmp_str);
         printf("\nTraceback:");
         (void) print_func_stack();
         break;
 //        (void) kill_the_proc();
     case DEBUG:
         print_dbg_log(msg);
         break;
   }
  free(tmp_str);
  return;
}

void print_dbg_log (char *msg) 
{
   char *tmp_str= malloc(1000); 
   if (!msg) {
      printf("\n NULL string returned for log msg \n");
      return;
    }
   if (dbg_flag) {
   snprintf(tmp_str, msg_size,"%s::%s:%s", get_curr_time() ,"DBG",msg);
   if (to_file == 1) {
      write_to_logfile(tmp_str);
    }
   printf("\n%s",tmp_str);
  }
  free(tmp_str);
return;

 }

/*int  main(int argc, char *argv[]) {
   //fetch dbg_flag and sys_file o/p
  
   for (int i =1 ;i <argc ; i++) {
      if(strcmp((argv+i), "-dbg") == 0) {
         dbg_flag = 1;
      }
      if (strcmp((argv+i), "-syslog_display") == 0) {
         to_file = 0;
      }
    }     
   return 0;
}*/
