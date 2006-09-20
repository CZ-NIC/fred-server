//                      This is the client.
//
// Usage: client <object reference>
//
#include <fstream.h>
#include <iostream.h>
#include <ccReg.hh>

#include "time.h"

static CORBA::Object_ptr getObjectReference(CORBA::ORB_ptr orb);

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
//    filebuf *pbuf;
  //  char *buffer;
    char name[64];
    long size;
/*
    ifstream fd ("/tmp/ccReg.ref");
    
    // OPRAVIT NA NAMESERVICE!!!!
    exit(-1);
    
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

*/
  obj =  getObjectReference( orb );

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

//////////////////////////////////////////////////////////////////////

static CORBA::Object_ptr
getObjectReference(CORBA::ORB_ptr orb)
{
  CosNaming::NamingContext_var rootContext;

  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_var obj;
    obj = orb->resolve_initial_references("NameService");

    // Narrow the reference returned.
    rootContext = CosNaming::NamingContext::_narrow(obj);
    if( CORBA::is_nil(rootContext) ) {
      cerr << "Failed to narrow the root naming context." << endl;
      return CORBA::Object::_nil();
    }
  }
  catch (CORBA::NO_RESOURCES&) {
    cerr << "Caught NO_RESOURCES exception. You must configure omniORB "
         << "with the location" << endl
         << "of the naming service." << endl;
    return 0;
  }
  catch(CORBA::ORB::InvalidName& ex) {
    // This should not happen!
    cerr << "Service required is invalid [does not exist]." << endl;
    return CORBA::Object::_nil();
  }

  // Create a name object, containing the name test/context:
  CosNaming::Name name;
  name.length(2);

  name[0].id   = (const char*) "ccReg";       // string copied
  name[0].kind = (const char*) "context"; // string copied
  name[1].id   = (const char*) "Whois";
  name[1].kind = (const char*) "Object";
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (name.type -- e.g. test.ps = postscript etc.)

  try {
    // Resolve the name to an object reference.
    return rootContext->resolve(name);
  }
  catch(CosNaming::NamingContext::NotFound& ex) {
    // This exception is thrown if any of the components of the
    // path [contexts or the object] aren't found:
    cerr << "Context not found." << endl;
  }
  catch(CORBA::TRANSIENT& ex) {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "
         << "naming service." << endl
         << "Make sure the naming server is running and that omniORB is "
         << "configured correctly." << endl;

  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name()
         << " while using the naming service." << endl;
    return 0;
  }

  return CORBA::Object::_nil();
}
