#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define MAX_LINE 256
#define KEY_MAX 20

#include "conf.h"


int trim(char *Str )
{
int i ,  j , len;

len = strlen( Str);

if( len > 0 )
  {
    for( i = len -1 ; i > 0 ; i -- )
        {
           if(  Str[i] == ' ' || Str[i] == '\n' || Str[i] == '\r'  ) Str[i] = 0 ;
           else break;
         }
   }

len = strlen( Str);

for( i = 0 ; i < len ;  i ++ )
   {
        if( Str[i] > ' ' ) break;
   }


for( j = 0 ; j < len-i ;  j ++ )
   {
        Str[j] = Str[i+j];
   }
Str[j] = 0 ;

len = strlen( Str);
return len;
}

bool Conf::ReadConfigFile(const char *filename )
{
FILE *f;
char buf[MAX_LINE];
char keys[MAX_KEYS][KEY_MAX] = { "dbname" , "user" , "password" , "host" , "port" , "log_mask" ,  "log_level" , "log_local" };
int key;
char keyname[KEY_MAX];
char value[MAX_LINE];
int i , k , l , len , line ;

if( ( f = fopen( filename ,  "r" ) ) != NULL ) 
  {
   fprintf( stderr , "open config file %s\n" , filename);

    for(line=0 ;; line ++)
    {
      fgets( buf , MAX_LINE , f );
      if(  feof(f) ) break;

      if( buf[0] != '#' ) // neni komentar
        {
          //printf("%s" , buf );
          len = trim( buf);  // trim bufefr
          //printf("[%s]\n" , buf );

          // parse  config file
          for( i = 0 , key = 0  ; i < KEY_MAX  , i < len ; i ++ )
             {                                                
                if(  buf[i] == '=' ) 
                  {
                    keyname[i] = 0 ; 
                    for( k = 0 ; k < MAX_KEYS ; k ++ )
                       {
                         if( strcmp( keys[k] ,  keyname ) == 0 ) 
                           {
                              key = k + 1; 
                              for( l = i +1 ; l < len ; l ++ ) if( buf[l]  > ' ' ) break;
                              strcpy( value , buf + l  );
                              break ;
                            }     
                       } 
                    break ;
                   } 
                else 
                   {
                      if( buf[i] > ' ' )  keyname[i] = buf[i]; 
                      else keyname[i] = 0 ;
                   }
             }


          //  if( key ) printf("KEY %d %s value[%s]\n" , key , keyname  , value );
           // get value by key
           //  "dbname" , "user" , "password" , "host" , "port" , "log_mask" ,  "log_level" , "log_local"

           switch( key)
             {
                case KEY_dbname:           
                           strcpy( dbname , value ); 
                           break; 
                case KEY_user:
                           strcpy( user , value );
                           break; 
                case KEY_pass:
                           strcpy( password  , value );
                           break; 
                case KEY_host:
                           strcpy( host , value );
                           break; 
                case KEY_port:
                           strcpy( port ,  value );
                           break; 
                case KEY_log_mask:
                           break; 
                case KEY_log_level:
                           break; 
                case KEY_log_local:
                           break; 
               
                default:
                         fprintf( stderr , "parse error on line %d  [%s]\n" , line , buf );
                         break;
             }

            if(key > 0  &&  key < KEY_log_mask ) 
              {                
                strcat( conninfo , keys[key-1] );
                strcat( conninfo , "=" );
                strcat( conninfo , value );
                strcat( conninfo , " " );
              }
           
        }

     }

    fclose(f);
    return true;
  }
else
  {
   fprintf( stderr , "Error open config file %s\n" , filename);
   return false;
  }


}
