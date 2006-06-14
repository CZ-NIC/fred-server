#include<stdio.h>
#include<string.h>
#include <stdlib.h>

#include "status.h"

bool Status::Add( int status )
{
int j;


// projdi pole prvku a pokud uz tamje tak nepridavej
for( j = 0  ; j < slen ; j ++ )
   {     
     if( stat[j] == status )   return false;
   }

// pokud najdes nulove tak ho pridej tam
for( j = 0  ; j < slen ; j ++ )
   {   
    if( stat[j] == 0 ) { stat[j] = status ; return true ; } 
   }

//  pridavej  az na konec
if( slen < MAX_STATUS )  { stat[slen] = status ;  slen ++ ; return true ; } 

// nepodarilo se pridat
return false;
}

bool Status::Rem( int status )
{
int i ,  j;


// projdi pole prvku a pokud uz tamje tak nepridavej
for( j = 0  ; j < slen ; j ++ )
   {
     if( stat[j] == status )  
       {
           stat[j] = 0 ;
           for( i = j ; i < slen -1; i ++ ) stat[i] = stat[i+1] ;
           slen -- ; // odecti 
           return true;
       }
   }

// nepodarilo se 
return false;
}


void Status::Array(char *string)
{
int j  ;
char numStr[12];


//  vygeneruj  novy status string  retezec pole
strcpy( string , "{ " );

for( j = 0  ; j < slen ; j ++ )
   {
     
         if( j )  strcat( string , " , " );
         sprintf( numStr , " %d ", stat[j] );
         strcat( string ,  numStr );
        
   }

strcat( string , " }" );
}

// vytvori numericke pole ze string
int Status::Make(char *array)
{
int i, num , from , p ;
char value[16];



// nulova velikost pole

if( array[0] != '{' ) return -1; // neni pole

if( array[0] == '{' &&  array[1] == '}' ) return 0;

for( i = 0 , num = 1 ; i < strlen( array) ; i ++ )
{
  if(  array[i] == ',' ) num ++;
}


if( num < MAX_STATUS )
{
slen = num;

 if( array[0] == '{' )
 {

   for( i = 1 , p = 0 , from =  1 ; i < strlen ( array ) ; i ++ )
      {
            if(  array[i] == ',' ||  array[i] ==  '}' )
              {
                      strncpy( value , array+from , i - from ); // zkopiruj retezec
                      value[i - from] = 0 ; // zakoncit
                      if( p < MAX_STATUS ) 
                      {
                      stat[p] =  atoi( value );
                      from = i + 1;
                      p ++ ;
                     }
               } 
      }
  }

}
return slen;
}

bool Status::Test( int status_flag )
{
int i;

for( i = 0 ; i < slen ; i ++ ) 
{ 
  switch( status_flag )
        {
            case STATUS_DELETE:
                 if( IsDeleteStatus(  stat[i] ) ) return true;
            case STATUS_UPDATE:
                 if( IsUpdateStatus(  stat[i] ) ) return true;
            case STATUS_RENEW:
                 if( IsRenewStatus(  stat[i] ) ) return true;
            case STATUS_TRANSFER:
                 if( IsTransferStatus(  stat[i] ) ) return true;
        }            

}

// default nenasel status flag
return false;
}

/*
void  Status::Debug()
{
int i;

printf("STATUS: { ");
for( i = 0 ; i < slen ; i ++ )  printf(" %d " ,  stat[i] ); 
printf("}\n");

}
*/

char * Status::GetStatusString( int status )
{

switch( status )
{
   case STATUS_ok:
           return "ok";
//   case STATUS_inactive:
//           return "inactive";
   case STATUS_linked:
           return "linked";
   case STATUS_clientDeleteProhibited:
           return "clientDeleteProhibited";
   case STATUS_serverDeleteProhibited:
           return "serverDeleteProhibited";
   case STATUS_clientHold:
           return "clientHold";
   case STATUS_serverHold:
           return "serverHold";
   case STATUS_clientRenewProhibited:
           return "clientRenewProhibited";
   case STATUS_serverRenewProhibited:
           return "serverRenewProhibited";
   case STATUS_clientTransferProhibited:
           return "clientTransferProhibited";
   case STATUS_serverTransferProhibited:
           return "serverTransferProhibited";
   case STATUS_clientUpdateProhibited:
           return "clientUpdateProhibited";
   case STATUS_serverUpdateProhibited:
           return "serverUpdateProhibited";
   default:
           return "";
}

return "";
}

int Status::GetStatusNumber(const char  *status )
{
     if( strcmp( status , "ok" ) == 0 ) return STATUS_ok;
//     if( strcmp( status , "inactive" ) == 0 ) return STATUS_inactive;
     if( strcmp( status , "linked" ) == 0 ) return STATUS_linked;
     if( strcmp( status , "clientDeleteProhibited" ) == 0 ) return  STATUS_clientDeleteProhibited;
     if( strcmp( status , "serverDeleteProhibited" ) == 0 ) return  STATUS_serverDeleteProhibited;
     if( strcmp( status , "clientHold" ) == 0 ) return STATUS_clientHold;
     if( strcmp( status , "serverHold" ) == 0 ) return  STATUS_serverHold;
     if( strcmp( status , "clientRenewProhibited" ) == 0 ) return   STATUS_clientRenewProhibited;
     if( strcmp( status , "serverRenewProhibited" ) == 0 ) return   STATUS_serverRenewProhibited;
     if( strcmp( status , "clientTransferProhibited" ) == 0 ) return   STATUS_clientTransferProhibited;
     if( strcmp( status , "serverTransferProhibited" ) == 0 ) return   STATUS_serverTransferProhibited;
     if( strcmp( status , "clientUpdateProhibited" ) == 0 ) return   STATUS_clientUpdateProhibited;
     if( strcmp( status , "serverUpdateProhibited" ) == 0 ) return   STATUS_serverUpdateProhibited;

// default
return 0;
}

