//
// Example code for implementing IDL interfaces in file ccReg.idl
//

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ccReg.hh>
#include <ccReg_whois.h>


//
// Example implementational code for IDL interface ccReg::Whois
//
ccReg_Whois_i::ccReg_Whois_i(){
  // add extra constructor code here
}
ccReg_Whois_i::~ccReg_Whois_i(){
  // add extra destructor code here
}
//   Methods corresponding to IDL attributes and operations
// !!!!!!!! testovaci verze slouzi na propojeni s CORBOU
ccReg::DomainWhois* ccReg_Whois_i::Domain(const char* domain_name)
{
ccReg::DomainWhois *dm;
struct tm dt;
char buf[256] , ns[32];
int i;

// vytvor cas
dt.tm_year = 2006 - 1900 ;
dt.tm_mon = 2;
dt.tm_mday = 7;
dt.tm_hour = 18;
dt.tm_min = 32;
dt.tm_sec = 45;

dm = new ccReg::DomainWhois;

dm->name = CORBA::string_alloc( 64 );
dm->name = CORBA::string_dup( domain_name );
dm->description = CORBA::string_alloc( 64 );
dm->description = CORBA::string_dup( "POPIS domeny" );
dm->status = 1;
dm->registered = mktime(&dt); // cas vytvoreni domeny
dm->registrarName =  CORBA::string_alloc( 32 );
dm->registrarName = CORBA::string_dup( "JMENO_REGISTRATORA" );
dm->registrarUrl  = CORBA::string_alloc( 64 );
dm->registrarUrl  = CORBA::string_dup("http://www.registrator.cz" );

// vynuluj 
memset( buf , 256 ,0 );
for( i = 0 ; i < 4 ; i ++ )
{
  sprintf( ns , "ns%d.%s.cz" , i + 1 ,  domain_name );

  if( i > 0 ) strcat( buf , "\t" );

  strcat( buf , ns ); 
}
dm->NameServers  =  CORBA::string_alloc( 256 );
dm->NameServers  =  CORBA::string_dup( buf );

return dm;
}



