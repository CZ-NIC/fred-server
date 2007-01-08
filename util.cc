#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "util.h"
#include "action.h"

#include "log.h"



// vygeneruje nahodne heslo delky len obsahujici znaky [a-z] [A-Z] a [0-0]
void random_pass(char *str )
{
int len=PASS_LEN;
int i ;
char c;

for( i = 0 ; i < len ;  )
{
     c = 32+(int) (96.0*rand()/(RAND_MAX+1.0));

  if(  isalnum(c ) )  {    str[i] = c ; i ++ ; }

}
 str[i] = 0 ; // ukocit
}

bool validateIPV6(const char *ipadd)
{
int len;
// ukoncena adresa dvojteckou
len = strlen( ipadd);
if( ipadd[len-1] == ':' ) return false;

// loop back
if( strncmp( ipadd ,"::"  , 2 ) == 0 )  return false;

// TODO

return true;
}

bool validateIPV4(const char *ipadd)
{
        unsigned b1, b2, b3, b4;
        int rc;
        int len;
// ukoncena adresa teckou
len = strlen( ipadd);
if( ipadd[len-1] == '.' ) return false;


        rc = sscanf(ipadd, "%3u.%3u.%3u.%3u",  &b1, &b2, &b3, &b4);
        if (rc == 4 )
         {
           if( b1 == 0 &&  b2  == 0 && b3  == 0 && b4  == 0 ) return false; 
           if( b1 == 1 &&  b2  == 1 && b3  == 1 && b4  == 1 ) return false; 
           if ( (b1 | b2 | b3 | b4) > 255 ) return false; // max
           if (strspn(ipadd, "0123456789.") < strlen(ipadd)) return false;

           if( b1 == 127 )  return false;
           if( b1 == 10 )  return false;
           if( b1 == 172 && b2 >= 16 && b2 < 32 ) return false;
           if( b1 ==  192 && b2 == 168 ) return false ;
           if( b1 >= 224 )  return false ; // multicast

           return true; // OK
        }
      else return false;
}



int test_inet_addr( const char *src )
{
struct sockaddr_in a4;
struct sockaddr_in6  a6;

if( inet_pton(AF_INET,  src, &a4.sin_addr) == 0 )
// test IPV6
{
  if( inet_pton(AF_INET6,  src, &a6.sin6_addr)  )  if( validateIPV6( src ) )return IPV6;
  // TODO local adres for ipv6
}
else  if( validateIPV4( src ) ) return IPV4;  // validate for local adres


return 0;
}

// implementace funkce atoh
int atoh(const char *String)
{
    int Value = 0, Digit;
    char c;

    while ((c = *String++) != 0 ) {

        if (c >= '0' && c <= '9')
            Digit = (int) (c - '0');
        else if (c >= 'a' && c <= 'f')
            Digit = (int ) (c - 'a') + 10;
        else if (c >= 'A' && c <= 'F')
            Digit = (int) (c - 'A') + 10;
        else
            break;

        Value = (Value << 4) + Digit;
    }

    return Value;
}


// vytvoreni roid
void get_roid( char *roid , char *prefix , int id )
{
sprintf(roid , "%s%010d-CZ" , prefix ,  id );
LOG( LOG_DEBUG ,  "get_ROID [%s] from prefix %s  id %d" , roid  , prefix , id  );

}
// contact handle
bool get_CONTACTHANDLE(  char  * HANDLE , const char *handle  )
{
return  get_handle( HANDLE , handle ,1 );
}

// nsset handle
bool get_NSSETHANDLE(  char  * HANDLE , const char *handle  )
{
return  get_handle( HANDLE , handle ,2 );
}

// obecny handle
bool get_HANDLE(  char  * HANDLE , const char *handle  )
{
return  get_handle( HANDLE , handle , 0  );
}


// prevod a testovani handlu
bool get_handle( char  * HANDLE , const char *handle  , int typ )
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
    if( isalnum( handle[i] ) ||   handle[i] == '.' || handle[i] == '-' ||  handle[i] == '_'   || handle[i] == ':' )
      {
          // PREVOD  na velka pismena
          HANDLE[i] =   toupper( handle[i] ); 
     }
    else
    {

        HANDLE[0] = 0 ;
        return false;
    }

 }

HANDLE[i] = 0 ;

switch( typ )
{
 case 1:
          if( strncmp( HANDLE ,  "CID:"   , 4 ) == 0 ) 
             {
                LOG( LOG_DEBUG ,  "OK CONTACT HANDLE [%s] " , HANDLE );
                return true;
             }
          else return false; 
 case 2:
          if( strncmp( HANDLE ,  "NSSID:"   , 6 ) == 0 ) 
             {
                LOG( LOG_DEBUG ,  "OK NSSET HANDLE [%s] " , HANDLE );
                return true;
             }
          else return false;
 default:
          LOG( LOG_DEBUG ,  "OK HANDLE [%s] " , HANDLE );
          return true;
}

}

else return false;
}




// prevadi DNS host  na mala pismena
bool convert_hostname( char *HOST ,const  char *fqdn )
{
int i , len ;

len = strlen( fqdn );

for( i = 0  ; i < len ; i ++ )
{
      if(  isalnum( fqdn[i] ) ||  fqdn[i] == '-' ||  fqdn[i] == '.' )
        {
              HOST[i] =  tolower(  fqdn[i] ); // preved na mala pismena
        }
       else { HOST[0] = 0 ; return false; }
}

// koenec
HOST[i] = 0 ;
return true;
}
// test hostname 
bool TestDNSHost( const char *fqdn )
{
int i , len , dot , num ;

len = strlen( fqdn );

LOG( LOG_DEBUG , "test DNSHost %s" , fqdn );


// konci na tecku
if( fqdn[len-1] == '.' ) { LOG( LOG_DEBUG , "ERORR dots on end" );  return false; }

for(  i = len -1  ; i > 0 ; i -- )
{
 if(  fqdn[i] == '.' ) break;
 else if(  isalpha(  fqdn[i] )  == 0 )  { LOG( LOG_DEBUG , "ERORR not tld" );  return false; }
}

 
// minimalni a maximalni velikost
if( len > 3  &&  len <= 255 ) 
{
for( i = 0 , num = 0  , dot = 0 ; i < len ; i ++ )
   {
      if(  isalnum( fqdn[i] ) ||  fqdn[i] == '-' ||  fqdn[i] == '.' )
        {
                      if( fqdn[i] == '.' ) 
                       {
                          if( num > 63 ) return false; // prilis dlouhy nazev
                          num = 0 ; 
                          dot ++;   
                       }
                      else num ++ ;
        }  
     else return false; // spatne zadany nazev DNS hostu 
  }


// minimalne dve tecky
if( dot >= 2 ){ LOG( LOG_DEBUG , "test OK dots %d" , dot  ) ; return true; } 
}

return false;
}

// test inet addres ipv4 a ipv6
bool TestInetAddress(const char *address )
{
int t;

t = test_inet_addr( address ); 
LOG( LOG_DEBUG , "test InetAddress %s -> %d" , address , t );
if( t ) return true;
else return false;
}


bool TestExDate( const char *curExDate , const char * ExDate )
{
LOG( LOG_DEBUG , "test curExDate [%s] ExDate [%s]" , curExDate ,  ExDate );

if( strcmp( curExDate ,  ExDate ) == 0 ){ LOG( LOG_DEBUG , "test OK" ); return true; }
else { LOG( LOG_DEBUG , "test fail " );  return false; }
}

int TestPeriodyInterval( int period , int min , int max )
{
int mod;
LOG( LOG_DEBUG , "test periody interval perido %d min %d max %d" ,  period ,  min ,   max  );

// musi lezet v intervalu
if( period > 0  && period <= max )
{
mod = period % min;
if( mod == 0 ) return 0; // je to OK
else return 1; // interval je sice spravne ale neni v rozsahu obdobi 12 .. 24 .. 36  mesicu
}
else  return 2 ;  // period je mimo stanoveny rozsah
}


// preveadi cenu  halire bez konverze pres float
long get_price( const char *priceStr )
{
char str[32];
int i , len;

strcpy(  str , priceStr );
len = strlen( priceStr );
for( i = 0 ;i < len  ; i ++ )
{
        if( str[i] == '.' ) {  str[i] =  str[i+1] ;  str[i+1]  = str[i+2] ;  str[i+2]  = 0 ; break ; }
}

// default
return atol( str  ) ;
}


// prevadi cenu v halirich na string
void get_priceStr(char *priceStr  , long price)
{
sprintf( priceStr , "%ld%c%02ld" , price/100 , '.' ,  price %100 );
}

void get_dateStr(  char *dateStr , const char *string )
{
// provizorni reseni ZULU 
get_zulu_t( dateStr , string );
}
  

void get_zulu_t(  char *dateStr , const char *string )
{

if( strcmp( string , "NULL" ) == 0 )  strcpy( dateStr , "" );
else
{
strcpy(  dateStr , string );
if( dateStr[10] == ' ' )  dateStr[10] = 'T' ; 
strcat( dateStr , "Z" );
}

}

// preveadi cas timestamp na unix time
time_t get_time_t(const char *string )
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

t = timegm(&dt);
if( t < 0 ) return 0; // fix na velek roky

LOG( LOG_DEBUG , "get_time_t from [%s] = %ld" , string , t );
return t;
}


}


// prevod lokalniho datumu na UTC timestamp pro SQL
void get_utctime_from_localdate( char *utctime , char *dateStr )
{
struct tm dt;
time_t t;
int year , month  , day ;

memset(&dt,0,sizeof(dt));

sscanf( dateStr , "%4d-%02d-%02d" , &year ,&month , &day );
dt.tm_mday  = day;
dt.tm_mon = month -1;
dt.tm_year = year - 1900;
dt.tm_isdst = -1; // negative if the information is not available

LOG( LOG_DEBUG , "tm_year  %d tm_mon  %d tm_mday %d hour %d min %d sec %d" , dt.tm_year , dt.tm_mon , dt.tm_mday   , dt.tm_hour,   dt.tm_min , dt.tm_sec );
 t = mktime( &dt );

get_timestamp(  t , utctime );

LOG( LOG_DEBUG , "get_utctime_from_localdate:  date [%s] utctime [%s]" , dateStr , utctime );

} 

// prevod datau z DB SQL na date 
void convert_rfc3339_date( char *dateStr , const char *string )
{
time_t t;

t =  get_time_t( string ); // preved na GMT
if( t <= 0 ) strcpy( dateStr , "" ); // chyba ERROR
// preved vstupni string a vrat rfc3339 date  
else get_rfc3339_timestamp(  t , dateStr , true); // vrat jako lokalni DATUM
}

void convert_rfc3339_timestamp( char *dateStr , const char *string )
{
time_t t;

t =  get_time_t( string );
if( t <= 0 ) strcpy( dateStr , "" ); // chyba ERROR
// preved vstupni string a vrat rfc3339 date time s casovou zonou
else get_rfc3339_timestamp(  t , dateStr , false );
}


void get_rfc3339_timestamp( time_t t , char *string , bool day)
{
struct tm *dt;
int diff;
char sign , tzstr[6] ; 

// preved n alokalni cas
dt = localtime( &t );

diff =   dt->tm_gmtoff;


if( diff == 0 ) sign = 'Z' ; // UTC zulu time 
else if (diff < 0)
       {
        sign = '-';
        diff = -diff;
       }
     else  sign = '+';


// preved pouze den
if( day ) sprintf(  string , "%4d-%02d-%02d" ,  dt->tm_year + 1900  ,  dt->tm_mon + 1 ,  dt->tm_mday  );
else 
{
sprintf(  string , "%4d-%02d-%02dT%02d:%02d:%02d%c" , 
    dt->tm_year + 1900  ,  dt->tm_mon + 1 ,  dt->tm_mday ,
    dt->tm_hour,   dt->tm_min , dt->tm_sec , sign );


if( diff != 0 ) 
 { 
   sprintf( tzstr ,   "%02d:%02d"  ,   diff / SECSPERHOUR  , ( diff % SECSPERHOUR )   / MINSPERHOUR );  // timezone
   strcat( string , tzstr );
  }

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




