//                      This is the client.
//
// Usage: client <object reference>
//
#include <fstream.h>
#include <iostream.h>
#include <ccReg.hh>

#include "time.h"

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
//     CORBA::String_var clID, pass , cc ; 
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);    
    ccReg::Contact *contact , *cc;
    CORBA::Short err;
    CORBA::Object_var obj ;
    CORBA::String_var errMsg , svTR;
    int i;
    time_t t;
    filebuf *pbuf;
    char *buffer;
    char name[64] , roid[32] , email[32];
    long size , max = 100 ;
    ifstream fd ("/tmp/ccReg.ref");
    // get pointer to associated buffer object
     pbuf=fd.rdbuf();

     // get file size using buffer's members
     size=pbuf->pubseekoff (0,ios::end,ios::in);
     pbuf->pubseekpos (0,ios::in);

     // allocate memory to contain file data
     buffer=new char[size+1];

     // get file data  
     pbuf->sgetn (buffer,size);
     buffer[size] = 0; // end line     
     fd.close ();


     cout << "IOR: "  << buffer<< endl;

     // get CORBA reference
     obj = orb->string_to_object(  buffer );



//    CORBA::Object_var obj = orb->string_to_object (uri);


  

//      EPP::EPP_var epp = ccReg::EPP::_narrow (obj);

    if (CORBA::is_nil (obj)) 
      {
        return 1;
      }
    ccReg::EPP_var EPP = ccReg::EPP::_narrow (obj);

  // clID = CORBA::string_dup("REG-CT");
  // pass = CORBA::string_dup("pass");
  // cc = CORBA::string_dup("CLIENT_AAA");


    //EPP->Login( "REG-CT" , "passwd" , "CLIENT_LGID"  , errMsg , svTR );

   // cout << errMsg << svTR << endl;
/*

    EPP->ContactInfo("XPET",  "XY-1234" , contact , errMsg , svTR );

    cout <<  contact->Name << contact->Email <<  endl;

    t = (time_t )  contact->CrDate;
    cout << asctime( gmtime( &t) ) << endl;


cc = new ccReg::Contact;
 
    cc->ROID =  CORBA::string_dup( "XPET" );
    cc->Fax  =  CORBA::string_dup( "1234567890" );

    cout << "ContactUpdate" <<  cc->ROID   << endl;
    
    EPP->ContactUpdate( *cc , "XX-1223" ,  errMsg , svTR );


delete cc;
    EPP->ContactDelete( "XPET",  "ZZ-aaaa" ,  errMsg , svTR );
*/

    cc = new ccReg::Contact;
/*
          cc->ROID =  CORBA::string_alloc( 32 );
          cc->Name =  CORBA::string_alloc( 64 );
          cc->Email =  CORBA::string_alloc( 64 );
          cc->Country =  CORBA::string_alloc( 3 );
          cc->CrID =  CORBA::string_alloc( 32 );
          cc->ClID =  CORBA::string_alloc( 32 );
*/
    for( i = 0 ; i < max ; i ++) 
       {
           sprintf( name, "NAME-%06d" , i+1 );
           sprintf( roid , "ID-%06d" , i+1 );

           cc->ROID = CORBA::string_dup(  roid );  
           cc->Name = CORBA::string_dup( name );
           cc->Country = CORBA::string_dup( "CZ" );
           cc->CrID = CORBA::string_dup( "REG-WEB4U" );
           cc->ClID = CORBA::string_dup( "REG-WEB4U" );
           cout << "Create: " << cc->ROID <<   cc->Name << endl;  
           EPP->ContactCreate( *cc , "XY-1234" , errMsg , svTR );
       } 


    for( i = 0 ; i < max ; i ++)
       {
           sprintf( roid , "ID-%06d" , i+1 );
 
           cout << "Info: " << roid  << endl;

           EPP->ContactInfo(  roid , "XY-1234" , contact , errMsg , svTR );

           sprintf( email, "%06d@neco.cz" , i+1 );
           contact->Email =  CORBA::string_dup( email );
           cout << "Update: " << cc->ROID <<   cc->Email << endl;

           EPP->ContactUpdate( *contact ,      "XY-1234" ,  errMsg , svTR );
       }



    for( i = 0 ; i < max ; i ++)
       {
               sprintf( roid , "ID-%06d" , i+1 );


               cout << "Delete: " << roid  << endl;
               EPP->ContactDelete( roid ,  "ZZ-aaaa" ,  errMsg , svTR );
       }

   delete cc;

//   printf("zeme %c%c [%s] \n" , contact->Country[0] ,  contact->Country[1] , contact->AuthInfoPw);


  //  EPP->Logout( "CLIENT_TRID" ,  errMsg , svTR );

    
    orb->destroy();
  }
  catch(CORBA::TRANSIENT&) {
   cerr << "Caught system exception TRANSIENT -- unable to contact the server." << endl ;
  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name() << endl;
  }
  catch(CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  catch(omniORB::fatalException& fe) {
   cerr << "Caught omniORB::fatalException:" << endl;
   cerr << "  file: " << fe.file() << endl;
   cerr << "  line: " << fe.line() << endl;
   cerr << "  mesg: " << fe.errmsg() << endl;
  }
  return 0;
}
