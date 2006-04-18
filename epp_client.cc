//                      This is the client.
//
// Usage: client <object reference>
//
#include <fstream.h>
#include <iostream.h>
#include <ccReg.hh>

#include <time.h>

//////////////////////////////////////////////////////////////////////


void random_string(char *str , int len)
{
int i , j ;
for( i = 0 ; i < len ; i ++ )
{
     j=1+(int) (25.0*rand()/(RAND_MAX+1.0));
    str[i] = 'a' + j ;
}
 str[i] = 0 ; // ukocit
}

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
    int i , len , max = 512, d;
    CORBA::Long loginID;
    struct tm dt;
    time_t t;
    filebuf *pbuf;
    char *buffer;
    char name[64] , roid[32] , email[32] , clTRID[32];
    long size  ;
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



    ret =  EPP->ClientLogin(  "REG-LRR"  ,  "heslo" , "" ,     "X2-login" , loginID );

    cout << "err code " <<  ret->errCode    << endl;

    ret =  EPP->HostInfo( "modry.neco.cz",  host  , loginID ,  "XX-host"  );
    cout << "err code " << ret->errCode  <<  ret->svTRID  << endl;

     cout << "host "  << host->fqdn << " nsset "  << host->nsset << endl;

     for( i = 0 ; i < host->inet.length() ; i ++ ) cout << "InetAddress: " <<  host->inet[i]  << endl ; 


     ret =  EPP->GetTransaction( loginID, "unknwo act" , 2104);

    cout << "get Transaction code " << ret->errCode  <<  ret->svTRID  << endl;


/*

    ret =  EPP->ContactInfo( "XPET",  cc , loginID , "XPET_info" );
    cout << "err code " << ret->errCode << " serverTRID " <<  ret->svTRID  << endl;

    cout << "info "  << cc->Name  <<  endl;
 
//    cout <<  contact->Name << contact->Email <<  endl;

  //  t = (time_t )  contact->CrDate;
   // cout << asctime( gmtime( &t) ) << endl;




  domain = new ccReg::Domain;


for( i = 0 ; i < max ; i ++ )
{


     len =2+(int) (18.0*rand()/(RAND_MAX+1.0));



     random_string( name , len);

     domain->ROID =  CORBA::string_dup( name );
     strcat( name , ".cz" );
     domain->name =  CORBA::string_dup( name );
     domain->Registrant =  CORBA::string_dup( "XPET" );
     domain->CrID =  CORBA::string_dup( "REG-IPEX" );
     domain->ClID =  CORBA::string_dup( "REG-IPEX" );

    cout << "domain " << i  <<  name    << endl;

    ret =  EPP->DomainCreate(  *domain , loginID , "XX-create" );

    cout << "err code " << ret->errCode   << endl;

} 
delete domain;
*/

/*
  domain = new ccReg::Domain;
 
dt.tm_sec = 0;
dt.tm_hour = 0;
dt.tm_min = 0;
dt.tm_mday = 31;        
dt.tm_mon = 0;        
tm_year = 108;       

strcpy( name , "superweb" );

     domain->ROID =  CORBA::string_dup( name );
     strcat( name , ".cz" );
     domain->name =  CORBA::string_dup( name );
     domain->Registrant =  CORBA::string_dup( "XPET" );
     domain->CrID =  CORBA::string_dup( "REG-CT" );
     domain->ClID =  CORBA::string_dup( "REG-CT" );
     domain->ExDate = mktime( &dt );
     domain->ns.length( 3 );
     domain->ns[0] = CORBA::string_dup( "dns.web.net" );
     domain->ns[1] = CORBA::string_dup( "dns.searsch.net" );
     domain->ns[2] = CORBA::string_dup( "dns.superWWW.net" );

     cout << "domain " << i  <<  name    << endl;

    ret =  EPP->DomainCreate( name ,  *domain , loginID , "XX-create" );

    cout << "err code " << ret->errCode   << endl;


max =1000;
 for( d = 0 ; d < max ; d ++ )
 {
     random_string( name , 2 );
     strcat( name , ".cz" );

  //  t = (time_t )  contact->CrDate;
   // cout << asctime( gmtime( &t) ) << endl;
   t = time(NULL);
   cout << "info: " <<  d <<  " " <<   asctime( gmtime( &t) ) << name  << endl;

   sprintf( clTRID , "%06d-%06d" , loginID , d ); 
   ret =  EPP->DomainInfo(  name ,  domain , loginID , clTRID );
   cout << "err code " << ret->errCode   << endl;

   cout << "domain "  << domain->name << "client: " << domain->ClID << endl;

    for( i = 0 ; i < domain->ns.length() ; i ++ ) cout << "DNS: " <<  domain->ns[i]  << endl ; 

 delete domain;
}




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
     random_string( name , 10 );

           sprintf( roid , "ID-%s" , name );

           cc->ROID = CORBA::string_dup(  roid );  
           cc->Name = CORBA::string_dup( name );
           cc->Country = CORBA::string_dup( "CZ" );
           cc->CrID = CORBA::string_dup( "REG-WEB4U" );
           cc->ClID = CORBA::string_dup( "REG-WEB4U" );
           cout << "Create: "  << i  << cc->ROID <<   cc->Name << endl;  
           EPP->ContactCreate( *cc ,loginID ,  "CREATE" );
       } 
*/
/*
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


   ret =  EPP->ClientLogout( loginID , "XXXX" );
   cout  << "client logout "<< endl;
    cout << "err code " <<  ret->errCode  << endl;
     
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
