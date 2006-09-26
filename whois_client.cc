//                      This is the client.
//
// Usage: client <object reference>
//
#include <fstream.h>
#include <iostream.h>
#include <ccReg.hh>

#include "nameservice.h"
#include "time.h"


//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
//     CORBA::String_var clID, pass , cc ; 
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);    
    ccReg::Contact *contact , cc;
    CORBA::Short err;
    CORBA::Object_var obj ;
    CORBA::String_var errMsg , svTR , tim;
    int i , n;
    time_t t;
    char name[64];
    long size;
    NameService ns(orb);

    obj =  ns.resolve("Whois");

    if (CORBA::is_nil (obj)) 
      {
        return 1;
      }

    ccReg::Whois_var Whois = ccReg::Whois::_narrow (obj);
    ccReg::DomainWhois *dm;

    strcpy( name , "4.4.4.0.2.4.e164.arpa"  );
    cout << "getDomain" << name << endl;

    dm = Whois->getDomain( name  , tim );

    cout << "WHOIS: " << dm->fqdn << endl;
    cout << "fqdn" << dm->fqdn << endl;
    cout << "status" <<  dm->status << endl; 

    for( n = 0 ; n < dm->ns.length() ; n ++ )
    cout <<  "NameServers: " << dm->ns[n] <<  endl;

    cout << "Registrator: " << dm->registrarName << "url: " << dm->registrarUrl <<  endl;


    cout << "registered: "  <<   dm->created  << endl;


    cout << "expired: "  <<   dm->expired << endl;

    cout << "timestamp: "  <<   tim << endl;

//    for( n = 0 ; n < dm->tech.length() ; n ++ )
//    cout <<  "Tech contact: " << dm->tech[n] <<  endl;


//    for( n = 0 ; n < dm->admin.length() ; n ++ )
  //  cout <<  dm->name << "Admin contact: " << dm->admin[n] <<  endl;
   

    orb->destroy();
  }
  catch(CORBA::TRANSIENT&) {
   cerr << "Caught system exception TRANSIENT -- unable to contact the server." << endl ;
  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name() << endl;
  }


  catch(ccReg::Whois::DomainError& ex) {
    cerr << "Caught DomainError::timestamp: " << ex.timestamp << " type: "  << ex.type << endl;    
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

