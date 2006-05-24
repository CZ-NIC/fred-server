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

bool Status::Test( int status )
{
int i;

for( i = 0 ; i < slen ; i ++ ) 
{ 
  if( stat[i] == status ) return true; // nasel status flag
}

return false;
}

