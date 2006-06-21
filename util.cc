#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>


// vytvoreni roid
void get_roid( char *roid , char *prefix , int id )
{
sprintf(roid , "CZNIC-%s%12d" , id );
}

// zarazeni do zony a kontrola nazvu domeny
int get_zone( char * fqdn )
{
char zoneStr[3][32] = { "0.2.4.e164.arpa" , "0.2.4.c.e164.arpa" , "cz" };
int i , len  , slen , l ;

len = strlen( fqdn );
for(  i = 0 ; i < 3 ; i ++ )
   {
        slen = strlen(  zoneStr[i] );
        l = len - slen ;
        if( l > 0 )
         {
          if( fqdn[l-1] == '.' )
             if(  strncmp(  fqdn+l ,  zoneStr[i] , slen ) == 0 ) return i +1; // zaradi do zony
         }

   }
return 0;
}

// preveadi cas timestamp na unix time
time_t get_time_t(char *string )
{
struct tm dt;
memset(&dt,0,sizeof(dt));
double sec = 0;

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

// spocita cas expirace ze zadaneho casu plus period mesice
time_t expiry_time( time_t extime ,  int period )
{
struct tm dt;
int mon;


// preved cas
dt =   *gmtime(  &extime );

mon =  dt.tm_mon + period;
// printf("year %d  mon %d \n" ,  dt.tm_year +1900 , dt.tm_mon +1);
dt.tm_year = dt.tm_year + (mon / 12 );
dt.tm_mon = mon % 12;
dt.tm_hour = 0;
dt.tm_min = 0;
dt.tm_sec = 0;
//printf("mktime 0x%x\n" , mktime(&dt) );

return mktime(&dt);
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


// vraci numericky prvek pole
int get_array_numeric(char *array , int field )
{
int i , num , from;
char value[32];

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
                      return atoi( value );
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
   if( value[0] == 0x8 ) sprintf(buf , " %s=NULL ," ,  fname ); // vymaz polozku pri updatu
   else    sprintf(buf , " %s=\'%s\' ," ,  fname , value );
   strcat( string , buf );  
 }

}


// pri update nastaveni bool hodnot
void add_field_bool(  char *string , char *fname , int value )
{
char buf[64];


if( value  == 1 )  // hodota je true
{
  sprintf( buf , "  %s=\'t\' , " , fname   );
  strcat( string , buf );
}


if( value  == 0 )  // hodota je false
{
  sprintf( buf , "  %s=\'t\' , " , fname   );
  strcat( string , buf );
}

}
// pridavani nazvu pole pri create
void create_field_fname( char *string , char *fname , char *value )
{
char buf[1024];

if( strlen( value ) )
 {
     sprintf(buf , " , %s " , fname );
     strcat( string , buf );
 }
}



void create_field_value( char *string , char *fname , char *value )
{
char buf[1024];

if( strlen( value ) )
 {
     sprintf(buf , " , \'%s\' " , value );
     strcat( string , buf );
 }
}

