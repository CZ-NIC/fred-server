#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "utf8hack.h"

// debug print
#ifdef DEBUG
#define debug printf
#else
#define debug /* printf */
#endif

char * UTF8(char *string )
{
int i , p , len;
unsigned char *buffer;


len = strlen( string );
buffer =  malloc( len +1 );

// zkopiruj 
memcpy( buffer , string  , len );
buffer[len] = 0 ; // ukocit


debug("string len %d  [%s]\n" , len , string  );

// vynuluj puvodni retezec
memset( string , 0  , len );


for( i = 0 , p = 0  ; i < len ; i ++ )
{

 debug("string %d \'%c\' 0x%x \n", i ,  buffer[i]   ,  buffer[i]  );

  
  // takhle zacina utf8
 if(  ( buffer[i] & 0xe0 )  == 0xC0 )
   { 
    string[p] = ( buffer[i]  & 0xc0 ) |  ( buffer[i+1] & 0x0f ); // preved na escape sekvenci UTF
    p ++ ;
    string[p] = buffer[i+3];
    debug("UTF8 buf[%d]=0x%02x   buf[%d] = \'%c\'  0x%02x  \n" , p-1 , string[p-1] ,  p ,  string[p] ,  buffer[i+3] );
    p ++;
    i += 3; // dalsi znak 
	
   }
  else 
   {
      string[p] = buffer[i];
      debug("ASCII buf[%d] 0x%02x  \'%c\' \n" , p , string[p]  ,  buffer[i] );
      p ++ ;
    }

}


// uvolnit buffer
free(buffer);

// vratit zkraceny puvodni retezec
return string;
}
