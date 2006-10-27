#include<stdio.h>
#include<string.h>

#include "messages.h"
#include "log.h"

Mesg::Mesg(int num)
{
lang=0;
add=0;
numMsg = num;

errID = new int[num];
errMsg =  new char *[num];
errMsg_cs =  new char *[num];
}

void Mesg::AddMesg( int id, const char *msg , const char *msg_cs )
{
int len;
if( add < numMsg )
{
// id zpravy
errID[add] = id;
// orginalni zneni
len = strlen( msg ) +1;
LOG(DEBUG_LOG, "alloc errMsg[%d] size %d\n" , add , len );
errMsg[add] =  new char[len];
memset( errMsg[add] , 0 , len );
strcpy( errMsg[add] , msg );
// cesky preklad
len = strlen( msg_cs )+1;
LOG(DEBUG_LOG ,"alloc errMsg_cs[%d] size %d\n" , add , len );
errMsg_cs[add] =  new char[len];
memset( errMsg_cs[add] , 0 , len );
strcpy( errMsg_cs[add] , msg_cs );    
add++;
}
   
}

Mesg::~Mesg()
{
delete [] errID;
delete [] errMsg;
delete [] errMsg_cs;
}


char * Mesg::GetMesg(int id)
{
int i;

for( i = 0 ;  i < numMsg ; i ++ )
  {  
    if( errID[i] == id ) return  errMsg[i] ; 
  }

return ""; // prazdy popis
}


char *   Mesg::GetMesg_CS(int id)
{
int i;

for( i = 0 ;  i < numMsg ; i ++ )
  {  
    if( errID[i] == id ) return  errMsg_cs[i] ; 
  }

return ""; // prazdy popis
}
