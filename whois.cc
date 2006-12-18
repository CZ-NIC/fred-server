#include "whois.h"
#include "log.h"
#include "util.h"
#include "dbsql.h"

#include "register/register.h"

#define MAX_LONG 63 // maximalni delka domeny

ccReg_Whois_i::ccReg_Whois_i(const std::string _database) : database(_database)
{}

ccReg_Whois_i::~ccReg_Whois_i()
{}

ccReg::DomainWhois* ccReg_Whois_i::getDomain(const char* domain_name, CORBA::String_out timestamp)
{

DB DBsql;
char sqlString[1024];
char fqdn[64];
char timestampStr[32];
ccReg::DomainWhois *dm;
int clid ,nssetid ;
int id;
int i , len;
int zone;
bool en;
time_t t ,  expired ;
bool found = false;
int db_error=0;
Register::CheckHandleList chdl;
Register::CheckHandle* chd = NULL;
if( DBsql.OpenDatabase( database.c_str() ) )
{

  std::auto_ptr<Register::Manager> r(Register::Manager::create(&DBsql));


// casova znacka
t = time(NULL);


// vrat casove razitko
get_rfc3339_timestamp( t , timestampStr  );
timestamp =  CORBA::string_dup( timestampStr );

dm = new ccReg::DomainWhois;


// free
// dm->name= CORBA::string_dup( "" );
 dm->created =  CORBA::string_dup( "" ) ;
 dm->expired =  CORBA::string_dup( "" ) ;
//dm->status = 0 ;
dm->registrarName = CORBA::string_dup( "" );
dm->registrarUrl  = CORBA::string_dup( "" );
//dm->nsset  = CORBA::string_dup( "" );
//dm->ns.length(0); // nulova sekvence
dm->tech.length(0); // nulova sekvence
// dm->admin.length(0); // nulova sekvence
 dm->fqdn = CORBA::string_dup( "" ) ;

len =  (int )  strlen( (const char * ) domain_name ) ;
if( len < MAX_LONG )
  {
   // preved na mala pismena
    for( i = 0 ; i < len ; i ++ )  fqdn[i] = tolower( domain_name[i] );
    fqdn[i] = 0 ; // ukoncit 
    r->checkHandle( fqdn ,chdl);
    LOG( LOG_DEBUG ,  "WHOIS: checkHandle %s " , fqdn );

  }
else
 {
   LOG( LOG_DEBUG ,  "WHOIS: domain too long len = %d  %s\n" , len ,  domain_name  );
   throw ccReg::Whois::DomainError( timestampStr , ccReg::WE_DOMAIN_LONG );
 }

for (unsigned j=0;j<chdl.size();j++) {
  if (chdl[j].type == Register::HT_ENUM_DOMAIN ||
      chdl[j].type == Register::HT_DOMAIN) {
    chd = &chdl[j];
    break;
  }
}
if(chd) 
{
if( chd->type == Register::HT_ENUM_DOMAIN){ zone = ZONE_ENUM;  en=true;}
else {zone = ZONE_CZ ; en=false ; }


if( ( id = DBsql.GetDomainID( fqdn , en ) )  )
{
 if(  DBsql.SELECTOBJECTID( "DOMAIN" , "fqdn" ,  id )    )
  {
 
     dm->enum_domain = en; // jestli je domena enumova dodelano pro bug #405       
     dm->created = CORBA::string_dup(  DBsql.GetFieldDateTimeValueName("CrDate" , 0 ) );
     dm->expired = CORBA::string_dup( DBsql.GetFieldDateValueName("ExDate" , 0 )  );

     clid =  DBsql.GetFieldNumericValueName("ClID" , 0 ); // client registrator
     nssetid = DBsql.GetFieldNumericValueName( "nsset" , 0 ); // id nsset

      // test jestli je domena expirovana
     expired = get_time_t( DBsql.GetFieldValueName("ExDate" , 0 ) ); 
     if( t  > expired )   dm->status = ccReg::WHOIS_EXPIRED;
     else dm->status = ccReg::WHOIS_ACTIVE;

     dm->fqdn =  CORBA::string_dup( DBsql.GetFieldValueName("fqdn" , 0 ) );


     // free select
    DBsql.FreeSelect();
    // dotaz na registratora
    sprintf( sqlString , "SELECT Name , Url FROM  REGISTRAR WHERE id=%d;" , clid ) ;

    if( DBsql.ExecSelect( sqlString ) )
      {
        dm->registrarName = CORBA::string_dup( DBsql.GetFieldValueName("Name" , 0 ) );
        dm->registrarUrl  = CORBA::string_dup( DBsql.GetFieldValueName("Url" , 0 ) );
        DBsql.FreeSelect();
      }

 

    if(   DBsql.SELECTONE( "HOST" , "nssetid" , nssetid  )  )
      {
    
         len =  DBsql.GetSelectRows(); // pocet DNS servru
         dm->ns.length(len); // sequence DNS servru
         for( i = 0 ; i < len ; i ++)
            {
              dm->ns[i] = CORBA::string_dup(  DBsql.GetFieldValueName("fqdn" , i )   );
            }

        DBsql.FreeSelect(); 
     } else dm->ns.length(0); // zadne DNS servry




     // dotaz na technicke kontakty
     if(  DBsql.SELECTCONTACTMAP( "nsset"  , nssetid ) )
          {
               len =  DBsql.GetSelectRows(); // pocet technickych kontaktu
               dm->tech.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) dm->tech[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );
               DBsql.FreeSelect();
          }

    found = true;   
   } else  db_error=2;

 }
  else found = false;
 
 DBsql.Disconnect();
}
else db_error=1;

switch( db_error )
{
  case 1:
        throw ccReg::Whois::WhoisError( "database connect error" );
        break;

  case 2:
        throw ccReg::Whois::WhoisError( "database select error" );
        break;

}
if( !found )
{
throw ccReg::Whois::DomainError( timestampStr , ccReg::WE_NOTFOUND  );  
}

}
else // ostatni chyny
if(  chd && chd->handleClass   ==  Register::CH_UNREGISTRABLE_LONG ) 
  throw ccReg::Whois::DomainError( timestampStr , ccReg::WE_DOMAIN_LONG );
  else
  if(  chd && chd->handleClass   ==  Register::CH_UNREGISTRABLE )
    throw ccReg::Whois::DomainError( timestampStr , ccReg::WE_DOMAIN_BAD_ZONE );
   else
  // default
   throw ccReg::Whois::DomainError( timestampStr , ccReg::WE_INVALID );






return dm;
}



