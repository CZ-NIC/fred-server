#include<time.h>
#include<stdio.h>
#include<time.h>
#include<string.h>

// preveadi cas timestamp na unix time
time_t get_time_t(char *string )
{
struct tm dt;
double sec;

sscanf(string , "%4d-%02d-%02d %02d:%02d:%lf" ,
                &dt.tm_year ,  &dt.tm_mon , &dt.tm_mday ,
                &dt.tm_hour,   &dt.tm_min , &sec);

// prevod sekund
dt.tm_sec = (int ) sec ;
// mesic o jeden mene
dt.tm_mon = dt.tm_mon -1;
// rok - 1900
dt.tm_year = dt.tm_year - 1900;


return mktime(&dt);
}



void get_timestamp( time_t t , char *string)
{
struct tm *dt;

// preved cas
dt =   gmtime(  &t );

sprintf(string ,  "%4d-%02d-%02d %02d:%02d:%02d" ,
                  dt->tm_year+1900 ,  dt->tm_mon +1, dt->tm_mday ,
                  dt->tm_hour,   dt->tm_min , dt->tm_sec);

}

// vraci pocet prvku v poli
int get_array_length(char *array)
{
int i , num;
// nulova velikost pole

if( array[0] != '{' ) return -1; // neni pole

if( array[0] == '{' &&  array[1] == '}' ) return 0;

for( i = 0 , num = 1 ; i < strlen( array) ; i ++ )
{
  if(  array[i] == ',' ) num ++;
}

return num;
}


// vraci prvek pole
void get_array_value(char *array ,  char *value , int field )
{
int i , num , from;

// default value
strcpy( value , "" );

if( array[0] == '{' )
{

   for( i = 1 , num = 0 , from = 1 ; i < strlen ( array ) ; i ++ )
      {
            if(  array[i] == ',' ||  array[i] ==  '}' )
              {
                 if( num == field )
                   {
                      strncpy( value , array+from , i - from ); // zkopiruj retezec
                      value[i - from] = 0 ; // zakoncit
                      break;
                   }
                 else { num ++ ; from = i + 1; }
               }
      }
}

}


// pridavani retezce pri update
void add_field_value( char *string , char *fname , char *value )
{
char buf[1024];

if( strlen( value ) )
 {
   sprintf(buf , " %s=\'%s\' ," ,  fname , value );
   strcat( string , buf );  
 }

}

