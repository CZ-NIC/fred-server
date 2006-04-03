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
    ccReg::Response *ret;
    ccReg::Host *host;
    ccReg::Domain *domain;
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


    ret =  EPP->ClientLogin( "REG-LRR"   );

    cout << "err code " <<  ret->errCode  <<  ret->errMsg  << endl;


    ret =  EPP->ContactInfo( "XPET",  cc );
    cout << "err code " << ret->errCode  <<  ret->errMsg  << endl;

    cout << "info "  << cc->Name  <<  endl;
 
//    cout <<  contact->Name << contact->Email <<  endl;

  //  t = (time_t )  contact->CrDate;
   // cout << asctime( gmtime( &t) ) << endl;

    ret =  EPP->HostInfo( "dns.test.cz",  host );
    cout << "err code " << ret->errCode  <<  ret->errMsg  << endl;

     cout << "host "  << host->name  << host->domain << endl;

     for( i = 0 ; i < host->inet.length() ; i ++ ) cout << "InetAddress: " <<  host->inet[i]  << endl ; 


    ret =  EPP->DomainInfo( "test.cz",  domain );
    cout << "err code " << ret->errCode  <<  ret->errMsg  << endl;

     cout << "domain "  << domain->name  << domain->ROID << endl;

     for( i = 0 ; i < domain->ns.length() ; i ++ ) cout << "DNS: " <<  domain->ns[i]  << endl ; 
/*

cc = new ccReg::Contact;
 
    cc->ROID =  CORBA::string_dup( "XPET" );
    cc->Fax  =  CORBA::string_dup( "1234567890" );

    cout << "ContactUpdate" <<  cc->ROID   << endl;
    
    EPP->ContactUpdate( *cc , "XX-1223" ,  errMsg , svTR );


delete cc;
    EPP->ContactDelete( "XPET",  "ZZ-aaaa" ,  errMsg , svTR );
*/

/*
    cc = new ccReg::Contact;

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
*/
//   printf("zeme %c%c [%s] \n" , contact->Country[0] ,  contact->Country[1] , contact->AuthInfoPw);


   ret =  EPP->ClientLogout();
   cout  << "client logout "<< endl;
    cout << "err code " << ret->errCode  <<  ret->errMsg  << endl;
     
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
