#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>

#include "util.h"
#include "action.h"

#include "log.h"

// pro Objectcheck funkci
bool  get_CHECK( char *CHCK , const char *chck , int act )
{

switch(  act )
{
case  EPP_DomainCheck:
      return  get_FQDN(  CHCK , chck);
case  EPP_ContactCheck:
case  EPP_NSsetCheck:
  return get_HANDLE( CHCK , chck);
default:
  return false;
}

}

// vytvoreni roid
void get_roid( char *roid , char *prefix , int id )
{
sprintf(roid , "%s%010d-CZ" , prefix ,  id );
LOG( LOG_DEBUG ,  "get_ROID [%s] from prefix %s  id %d" , roid  , prefix , id  );

}
// prevod a testovani handlu
bool get_HANDLE( char  * HANDLE , const char *handle )
{
int i , len;

len = strlen( handle);

LOG( LOG_DEBUG ,  "get_HANDLE from [%s] len %d" , handle , len  );

// max a minimalni delka
if( len > 1 && len <= 40 )
{
  for( i = 0 ; i < len ; i ++ )
  {

    // TEST
    if( ( handle[i] >= 'a'  && handle[i] <= 'z' ) ||
        ( handle[i] >= 'A'  && handle[i] <= 'Z' ) ||
        ( handle[i] >= '0'  && handle[i] <= '9' ) ||
        ( handle[i] == '.' || handle[i] == '-' ||  handle[i] == '_' ) )
      {

          // PREVOD
          if( handle[i] >= 'a'  && handle[i] <= 'z' )
              HANDLE[i] = handle[i] - 0x20; // prevod na velka pismena
          else HANDLE[i] =  handle[i];
     }
    else
    {
        HANDLE[0] = 0 ;
        return false;
    }

 }

HANDLE[i] = 0 ;
LOG( LOG_DEBUG ,  "OK HANDLE [%s] " , HANDLE );
return true;
}

else return false;
}



bool get_FQDN( char *FQDN , const char *handle )
{
int i , len , max ;
int zone;

zone = get_zone( handle , true  );
max =  get_zone( handle , false  ); // konec nazvu

len = strlen( handle);

FQDN[0] = 0;

LOG( LOG_DEBUG ,  "get_FQDN [%s] zone %d max %d" , handle  , zone , max );

// maximalni delka
if( len > 63 ) { LOG( LOG_DEBUG ,  "out ouf maximal length %d" , len ); return false; }
if( max <= 2  ) { LOG( LOG_DEBUG ,  "minimal length" ); return false;}

// test na enum zonu
if( zone == ZONE_ENUM || zone == ZONE_CENUM  )
{
         for( i = 0 ; i <  max ; i ++ )
            {
              if(  ( handle[i] >= '0'  && handle[i] <= '9' ) ||  handle[i] == '.' )
                {
                     FQDN[i] = handle[i];
                }  
               else {  LOG( LOG_DEBUG ,  "character  %c not allowed"  , handle[i] );  FQDN[0] = 0 ;  return false; } 
            
            }

if( handle[i] == '.' )
 { 
   FQDN[i] =  0 ;          
   if( zone == ZONE_ENUM )strcat( FQDN , ".0.2.4.e164.arpa"  );
   if( zone == ZONE_CENUM ) strcat( FQDN , ".0.2.4.c.e164.arpa" );

   LOG( LOG_DEBUG ,  "OK ENUM domain [%s]" , FQDN );
   return true;
 }


}

if( zone == ZONE_CZ )    // DOMENA CZ
{

       // max a minimalni delka
        for( i = 0 ; i < max ; i ++ )
           {
              // TEST povolene znaky
              if( ( handle[i] >= 'a'  && handle[i] <= 'z' ) ||
                ( handle[i] >= 'A'  && handle[i] <= 'Z' ) ||
                ( handle[i] >= '0'  && handle[i] <= '9' ) ||  handle[i] == '-' )
                {

                       // PREVOD na mala pizmena
                       if( handle[i] >= 'A'  && handle[i] <= 'Z' )
                           FQDN[i] = handle[i] + 0x20; // prevod na mala pismena
                       else FQDN[i] =  handle[i];
              
                }
               else {  LOG( LOG_DEBUG ,  "character  %c not allowed"  , handle[i] );  FQDN[0] = 0 ;  return false; } 
            }

         if( handle[i] == '.' )
          {     
            FQDN[i] =  0 ;
            strcat( FQDN , ".cz" ); // konec
            LOG( LOG_DEBUG ,  "OK CZ domain [%s]" , FQDN );
            return true;
          }
       
    

}

return false;
}

// zarazeni do zony a kontrola nazvu domeny
int get_zone( const  char * fqdn , bool compare)
{
char zoneStr[3][64] = { "0.2.4.e164.arpa" , "0.2.4.c.e164.arpa" , "cz" };
int i , len  , slen , l ;

len = strlen( fqdn );
for(  i = 0 ; i < 3 ; i ++ )
   {
        slen = strlen(  zoneStr[i] );
        l = len - slen ;
        if( l > 0 )
         {
          if( fqdn[l-1] == '.' ) // case less comapare
             {
                if( compare )
                {
                     if(  strncasecmp(  fqdn+l ,  zoneStr[i] , slen ) == 0 ) return i +1; // zaradi do zony
                } else return l -1 ; // vraci konec nazvu 
             }
         }

   }
return 0;
}

// preveadi cas timestamp na unix time
time_t get_time_t(char *string )
{
struct tm dt;
time_t t;
memset(&dt,0,sizeof(dt));
double sec = 0;

if( strcmp( string , "NULL" ) == 0 )  return 0;
else
{

sscanf(string , "%4d-%02d-%02d %02d:%02d:%lf" ,
                &dt.tm_year ,  &dt.tm_mon , &dt.tm_mday ,
                &dt.tm_hour,   &dt.tm_min , &sec);

// prevod sekund
dt.tm_sec = (int ) sec ;
// mesic o jeden mene
dt.tm_mon = dt.tm_mon -1;
// rok - 1900
dt.tm_year = dt.tm_year - 1900;

t = mktime(&dt);
if( t < 0 ) return 0; // fix na velek roky

LOG( LOG_DEBUG , "get_time_t from [%s] = %ld" , string , t );
return t;
}


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
  sprintf( buf , "  %s=\'f\' , " , fname   );
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

