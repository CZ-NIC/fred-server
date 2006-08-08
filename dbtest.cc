#include <fstream.h>
#include <iostream.h>

#include<time.h>

#include "ctest.h"

main(int argc , char **argv )
{
//char db[] = "dbname=ccreg_test user=pblaha host=localhost";
// char db[] = "dbname=ccreg user=ccreg password=Eeh5ahSi host = curlew";
ccReg_EPP_test *CTest;
ContactChange *ch;
int ret, num , i , start;
timestamp crDate;
time_t t;
char handle[64];
int loginID;

for( i = 0 ; i < argc ; i ++ )  cout << "argc" << i << argv[i]   << endl;
if( argc >= 2)
{
CTest = new ccReg_EPP_test( argv[1]  );

start = atoi( argv[2] );

num = atoi( argv[3] );

 
#ifdef PERNAMENT
if( CTest->DatabaseConnect( )  )
#endif
{

loginID = CTest->Login("REG-LRR" , "login" , "");


  cout << "loginID " <<  loginID << endl;
if( loginID )
{
 t = time(NULL);

  cout << "START create: " <<   ctime( &t ) << "at " << start << " num: " << num  << endl;
 

  for( i = start ; i <start+ num ; i ++ )
  {
   sprintf( handle , "TEST%04d" , i );

    ch = new ContactChange;
   ch->Name = new char[64];
   ch->CC = new char[3];

   strcpy( ch->Name  ,  handle );
   strncpy( ch->CC ,  "CZ" , 2);
   ch->CC[2] = 0 ;
    cout << "contact: " << ch->Name <<   ch->CC << endl;
   
   ch->Organization =  new char[2];        // nazev organizace
   strcpy(  ch->Organization  , "" );
   
    ch->DiscloseName=0;
    ch->DiscloseName=0;      // povolovani zobrazeni
    ch->DiscloseOrganization=0;
    ch->DiscloseAddress=0;
    ch->DiscloseTelephone=0;
    ch->DiscloseFax=0;
    ch->DiscloseEmail=0;

   ret =  CTest->ContactCreate( handle , *ch ,  crDate ,  loginID , "test-contact-create"  , ""   );
   cout << "contact create  " << handle << "  "  << ret  << endl;
   t = crDate;
   cout << "Create date: " << ctime( &t )  << endl;
   delete ch;
  }

  t = time(NULL);
  cout << "START delete: " <<   ctime( &t )  << endl;

  for( i = start ; i <start+ num ; i ++ )
  {
   sprintf( handle , "TEST%04d" , i );
   ret =  CTest->ContactDelete(  handle ,  loginID , "test-contact-delete" , "" );
   cout << "contact delete " << handle << "  "  << ret <<  endl;
  }

  t = time(NULL);
  cout << "END delete: " <<   ctime( &t )  << endl;
}

#ifdef PERNAMENT
 CTest->DatabaseDisconnect();
#endif
}

}
else cout << "usage: " << argv[0] << " \"dbname=ccreg user=ccreg password=?? host=hostname\"  start num"  << endl;
}

