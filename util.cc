#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<ctype.h>

#include "util.h"
#include "action.h"

#include "log.h"

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

// pro Objectcheck funkci
bool  get_CHECK( char *CHCK , const char *chck , int act )
{

switch(  act )
{
case  EPP_DomainCheck:
      if(  get_FQDN(  CHCK , chck) == 0 ) return false;
      else return  true;
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
    if( isalnum( handle[i] ) ||   handle[i] == '.' || handle[i] == '-' ||  handle[i] == '_'  )
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
LOG( LOG_DEBUG ,  "OK HANDLE [%s] " , HANDLE );
return true;
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
if( max < 2  ) { LOG( LOG_DEBUG ,  "minimal length" ); return 0;}

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
int i , len , dot=0 , ddot=0 , num  , l ;
char numStr[16];
len = strlen( address );

LOG( LOG_DEBUG , "test InetAddress %s" , address );

for( i = 0 , l = 0   ; i < len ; i ++ )
   {
       if(   isxdigit( address[i] ) || address[i] == '.' || address[i] == ':' )
         {
             if( address[i] == '.'  ) // IPV4
               {
                  numStr[l] = 0; l = 0 ;
                  num = atoi(  numStr );
                  // LOG( LOG_DEBUG , "ipv4 %d %d [%s]" , dot , num , numStr ); 
                  if( num >  255 ) return false;
                  dot++;                    
               }
              else if(  address[i] == ':' ) // IPV6 
                     {
                       numStr[l] = 0; l = 0 ;
                       num = atoh( numStr );
                       // LOG( LOG_DEBUG , "ipv6 %d %d [%s]" , dot , num , numStr ); 

                       if( num > 0xffff ) return false;
                       ddot++; 
                     }
                    else { numStr[l] =  address[i] ; l ++ ; } 
  
         }
   }


if( dot == 3 ) 
{
numStr[l] = 0;
num = atoi(  numStr );
if( num <=  255 ) { LOG( LOG_DEBUG , "test ipv4OK" ) ; return true; } // IPV4 OK
}

if( ddot == 7 )
{
numStr[l] = 0;
num = atoh(  numStr );
if( num <= 0xfffff ){ LOG( LOG_DEBUG , "test ipv6 OK" ) ; return true; } // IPV6  OK
}

return false;
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


bool TestValidityExpDate( time_t val , int max )
{
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

