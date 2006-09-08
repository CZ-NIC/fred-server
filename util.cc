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


bool validateIPV4(const char *ipadd)
{
        unsigned b1, b2, b3, b4;
        int rc;

        rc = sscanf(ipadd, "%3u.%3u.%3u.%3u",  &b1, &b2, &b3, &b4);
        if (rc == 4 )
         {
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
  if( inet_pton(AF_INET6,  src, &a6.sin6_addr)  ) return IPV6;
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



int get_FQDN( char *FQDN , const char *fqdn )
{
int i , len , max ;
int zone;
int dot=0;

zone = get_zone( fqdn , true  );
max =  get_zone( fqdn , false  ); // konec nazvu

len = strlen( fqdn);

FQDN[0] = 0;

LOG( LOG_DEBUG ,  "get_FQDN [%s] zone %d max %d" , fqdn  , zone , max );

// maximalni delka
if( len > 63 ) { LOG( LOG_DEBUG ,  "out ouf maximal length %d" , len ); return 0; }
if( max == 0 ) { LOG( LOG_DEBUG ,  "minimal length" ); return 0;}

// test double dot .. and double --
for( i = 1 ; i <  len ; i ++ )
{
if( fqdn[i] == '.' && fqdn[i-1] == '.' )
  {
   LOG( LOG_DEBUG ,  "double \'.\' not allowed" );
   return 0;
  }

if( fqdn[i] == '-' && fqdn[i-1] == '-' )
  {
   LOG( LOG_DEBUG ,  "double  \'-\' not allowed" );
   return 0;
  }

}


if( fqdn[0] ==  '-' )
{
    LOG( LOG_DEBUG ,  "first \'-\' not allowed" ); 
    return 0;

}

// test na enum zonu
if( zone == ZONE_ENUM || zone == ZONE_CENUM  )
{
         for( i = 0 ; i <  max ; i ++ )
            {
              // TEST povolene znaky  
              if( isdigit( fqdn[i] )  ||  fqdn[i] == '.' )
                {
                     FQDN[i] = fqdn[i];
                }  
               else {  LOG( LOG_DEBUG ,  "character  %c not allowed"  , fqdn[i] );  FQDN[0] = 0 ;  return 0; } 
            
            }

if( fqdn[i] == '.' )
 { 
   FQDN[i] =  0 ; 
   strcat( FQDN ,"." );         
   if( zone == ZONE_ENUM )strcat( FQDN ,  ENUM_ZONE );
   if( zone == ZONE_CENUM ) strcat( FQDN , CENUM_ZONE  );

   LOG( LOG_DEBUG ,  "OK ENUM domain [%s]" , FQDN );
   return zone;
 }


}

if( zone == ZONE_CZ )    // DOMENA CZ
{

       // max a minimalni delka
        for( i = 0 ; i < max ; i ++ )
           {


              // TEST povolene znaky
              if( isalnum( fqdn[i]  ) ||  fqdn[i] == '-' ||  fqdn[i] == '.' )
                {
                    if( fqdn[i] == '.' ) 
                      { 
                       dot ++;
                       if( dot > 0 ) 
                           { 
                             LOG( LOG_DEBUG ,  "too much dots not allowed" );
                            FQDN[i] =  0 ;
                             return 0;
                            }
                       }
              
                    
                    // PREVOD na mala pismena
                    FQDN[i] = tolower( fqdn[i] );
                }
               else 
                {
               LOG( LOG_DEBUG ,  "character  %c not allowed"  , fqdn[i] ); 
               FQDN[0] = 0 ;  
               return 0; 
               } 

         }

       

         if( fqdn[i] == '.' )
          {     
            FQDN[i] =  0 ;
            strcat( FQDN ,"." );
            strcat( FQDN , CZ_ZONE ); // konec

            LOG( LOG_DEBUG ,  "OK CZ domain [%s]" , FQDN );
            return zone;
          }
    

}

return 0;
}

// zarazeni do zony a kontrola nazvu domeny
int get_zone( const  char * fqdn , bool compare)
{
char zoneStr[3][64] = { ENUM_ZONE  , CENUM_ZONE  , CZ_ZONE };
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


bool TestPeriodyInterval( int period , int min , int max )
{
int mod;
// musi lezet v intervalu
if( period >= min && period <= max )
{
mod = period % min;
if( mod == 0 ) return true;
else return false;
}
else  return false;
}


bool TestValidityExpDate( const char *timestamp , int max   )
{
// TODO implementace
return true;
/*
time_t now , ex;
if(  max ==  0) return true;
else
{

if(  val == 0 ) return true; // neporovanvej pokud neni zadano
else
{
now = time(NULL);
ex = expiry_time( now ,  max );

if( val > now && val < ex ) return true;
else return false;
}
}
*/
}

// preveadi cenu  halire bez konverze pres float
long get_price( const char *priceStr )
{
char str[32];
int i;

strcpy(  str , priceStr );

for( i = 0 ;i < strlen( str ) ; i ++ )
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

void convert_rfc3339_timestamp( char *dateStr , const char *string )
{
time_t t;

t =  get_time_t( string );
if( t <= 0 ) strcpy( dateStr , "" );
// preved vstupni string a vrat rfc3339 date time s casovou zonou
else get_rfc3339_timestamp(  t , dateStr );

}

void get_rfc3339_timestamp( time_t t , char *string)
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




sprintf(  string , "%4d-%02d-%02dT%02d:%02d:%02d%c" , 
    dt->tm_year + 1900  ,  dt->tm_mon + 1 ,  dt->tm_mday ,
    dt->tm_hour,   dt->tm_min , dt->tm_sec , sign );


if( diff != 0 ) 
 { 
   sprintf( tzstr ,   "%02d:%02d"  ,   diff / SECSPERHOUR  , ( diff % SECSPERHOUR )   / MINSPERHOUR );  // timezone
   strcat( string , tzstr );
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





// porovnava datum co je v databazi expDateStr podle datumu zadaneho jako curExpDate
bool test_expiry_date( const char * expDateStr , const char * curExpDate ) 
{
int eyear , emonth , emday;
int cyear , cmonth , cmday;

sscanf( expDateStr , "%4d-%02d-%02d" , &eyear , &emonth , &emday );
sscanf( curExpDate , "%4d-%02d-%02d" , &cyear , &cmonth , &cmday );
// porovnani datumu 
if( eyear == cyear  &&  emonth == cmonth && emday == cmday )return true;
else return false;
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


