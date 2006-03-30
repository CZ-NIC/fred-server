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
    ccReg::Contact *contact , cc;
    CORBA::Short err;
    CORBA::Object_var obj ;
    CORBA::String_var errMsg , svTR;
    int i;
    time_t t;
    filebuf *pbuf;
    char *buffer;
    char name[64] , roid[32];
    long size;
    ifstream fd ("/tmp/ccWhois.ref");
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


    if (CORBA::is_nil (obj)) 
      {
        return 1;
      }

    ccReg::Whois_var Whois = ccReg::Whois::_narrow (obj);
    ccReg::DomainWhois *dm;


    dm =  Whois->Domain("neco.cz" );

    cout <<  dm->name << "NameServers: " << dm->NameServers <<  endl;
    cout <<"Registrator: " << dm->registrarName << "url: " << dm->registrarUrl <<  endl;

    t = (time_t )  dm->created;
    cout << "registered: "  << asctime( gmtime( &t) ) << endl;


    t = (time_t )  dm->expired;
    cout << "expired: "  << asctime( gmtime( &t) ) << endl;

    
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
