#include<stdio.h> 
#include <stdarg.h>


#include "log.h"

// #define MAXSTRING 1024 

#ifndef SYSLGOG
void logprintf(int level ,  char *fmt, ... ) 
{ 
char levels[8][8] = {  "EMERG" , "ALERT" , "CRIT" , "ERR" , "WARNING" , "NOTICE" , "INFO" , "SQL" };

printf( "%-8s: " , levels[level] );
va_list args; 
// char message[MAXSTRING]; 
va_start(args, fmt); 
vprintf(  fmt, args); 
va_end(args); 
// konec radku
printf("\n");
}
#endif

