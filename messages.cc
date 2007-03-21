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
// id of messaged
errID[add] = id;
// orginalni EN
len = strlen( msg ) +1;
LOG(DEBUG_LOG, "alloc errMsg[%d] size %d\n" , add , len );
errMsg[add] =  new char[len];
memset( errMsg[add] , 0 , len );
strcpy( errMsg[add] , msg );
// lokal translate to czech
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
LOG(DEBUG_LOG ,"GetMesg id %d" ,  id );
for( i = 0 ;  i < numMsg ; i ++ )
  {  
    if( errID[i] == id ) 
     {
        LOG(DEBUG_LOG ,"return mesg [%s]" , errMsg[i] );
        return  errMsg[i] ; 
     }
  }

return ""; // empty desc
}


char *   Mesg::GetMesg_CS(int id)
{
int i;

LOG(DEBUG_LOG ,"GetMesg_CS %d" ,  id );

for( i = 0 ;  i < numMsg ; i ++ )
  {  
    if( errID[i] == id ) 
       {
          LOG(DEBUG_LOG ,"return CS mesg [%s]" ,  errMsg_cs[i] );
          return  errMsg_cs[i] ; 
       }
  }

return ""; // empty desc

}
