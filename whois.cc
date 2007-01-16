#include "whois.h"
#include "log.h"
#include "util.h"
#include "dbsql.h"

#include "register/register.h"

#define MAX_LONG 63 // maximalni delka domeny

ccReg_Whois_i::ccReg_Whois_i(const std::string _database) 
  : database(_database)
{}

ccReg_Whois_i::~ccReg_Whois_i()
{}

ccReg::DomainWhois* ccReg_Whois_i::getDomain(
  const char* domain_name, CORBA::String_out timestamp)
{
  
  DB DBsql;
  char sqlString[1024];
  char timestampStr[32];
  ccReg::DomainWhois *dm;
  int clid ,nssetid ;
  int i , len;
  int zone;
  bool en;
  time_t t ,  expired ;
  if (!DBsql.OpenDatabase(database.c_str()) )
    throw ccReg::Whois::WhoisError( "database connect error" );
  
  // casova znacka
  t = time(NULL);
  
  // vrat casove razitko
  get_rfc3339_timestamp( t , timestampStr, false  );
  timestamp =  CORBA::string_dup( timestampStr );
  
  dm = new ccReg::DomainWhois;
  dm->created =  CORBA::string_dup( "" ) ;
  dm->expired =  CORBA::string_dup( "" ) ;
  dm->registrarName = CORBA::string_dup( "" );
  dm->registrarUrl  = CORBA::string_dup( "" );
  dm->tech.length(0); // nulova sekvence
  dm->fqdn = CORBA::string_dup( "" ) ;
  
  len =  (int )  strlen( (const char * ) domain_name ) ;
  if( len > MAX_LONG )  {
     LOG(LOG_DEBUG ,"WHOIS: domain too long len = %d  %s\n" ,len ,domain_name);
     throw ccReg::Whois::DomainError( timestampStr , ccReg::WE_DOMAIN_LONG );
  }
  
  Register::Domain::CheckAvailType caType; // result of check
  Register::NameIdPair caConflict; // id and name of discovered domain 
  std::auto_ptr<Register::Zone::Manager> zm; // zone manager
  std::auto_ptr<Register::Domain::Manager> dman; // domain manager
  try {
    zm.reset(Register::Zone::Manager::create(&DBsql));
    dman.reset(Register::Domain::Manager::create(&DBsql,zm.get()));
    caType = dman->checkAvail(domain_name,caConflict);
    LOG(LOG_DEBUG,
        "WHOIS: checkHandle %s, type %u, id %lu" ,
        domain_name ,caType, (long unsigned)caConflict.id);
  }
  catch (...) { // only SQL ERROR can be thrown
    DBsql.Disconnect();
    LOG(LOG_DEBUG, "WHOIS: checkHandle database error");
    throw ccReg::Whois::WhoisError( "database select error" );  
  }
  switch (caType) {
    case Register::Domain::CA_BAD_LENGHT :
      DBsql.Disconnect();
      throw ccReg::Whois::DomainError(timestampStr, ccReg::WE_DOMAIN_LONG);
    case Register::Domain::CA_BAD_ZONE :
      DBsql.Disconnect();
      throw ccReg::Whois::DomainError(timestampStr, ccReg::WE_DOMAIN_BAD_ZONE);
    case Register::Domain::CA_INVALID_HANDLE :
      DBsql.Disconnect();
      throw ccReg::Whois::DomainError(timestampStr, ccReg::WE_INVALID);
    default:
      // continue
      break;
  };
  
  if(!caConflict.id) {
    DBsql.Disconnect();
    throw ccReg::Whois::DomainError(timestampStr, ccReg::WE_NOTFOUND);
  }  
  
  if (dman->checkEnumDomainSuffix(domain_name)) {
     zone = ZONE_ENUM; en=true;
  }
  else {
     zone = ZONE_CZ ; en=false; 
  }
  
  if(!DBsql.SELECTOBJECTID( "DOMAIN" , "fqdn" ,  caConflict.id)) {
    DBsql.Disconnect();
    LOG(LOG_DEBUG, "WHOIS: checkHandle database error");
    throw ccReg::Whois::WhoisError( "database select error" );  
  }    

  dm->enum_domain = en; // jestli je domena enumova dodelano pro bug #405       
  dm->created = CORBA::string_dup(DBsql.GetFieldDateTimeValueName("CrDate",0));
  dm->expired = CORBA::string_dup(DBsql.GetFieldDateValueName("ExDate",0));  
  clid =  DBsql.GetFieldNumericValueName("ClID",0); // client registrator
  nssetid = DBsql.GetFieldNumericValueName("nsset",0); // id nsset
  
  // test jestli je domena expirovana
  expired = get_time_t( DBsql.GetFieldValueName("ExDate" , 0 ) ); 
  if( t  > expired )   dm->status = ccReg::WHOIS_EXPIRED;
  else dm->status = ccReg::WHOIS_ACTIVE;

  dm->fqdn =  CORBA::string_dup(caConflict.name.c_str());

  // free select
  DBsql.FreeSelect();
  // dotaz na registratora
  sprintf( sqlString , "SELECT Name , Url FROM  REGISTRAR WHERE id=%d;",clid);

  if( DBsql.ExecSelect( sqlString ) )
  {
    dm->registrarName = CORBA::string_dup( DBsql.GetFieldValueName("Name",0));
    dm->registrarUrl  = CORBA::string_dup( DBsql.GetFieldValueName("Url",0));
    DBsql.FreeSelect();
  }

  if( DBsql.SELECTONE( "HOST" , "nssetid" , nssetid  )  )
  { 
    len =  DBsql.GetSelectRows(); // pocet DNS servru
    dm->ns.length(len); // sequence DNS servru
    for( i = 0 ; i < len ; i ++)
    {
      dm->ns[i] = CORBA::string_dup(  DBsql.GetFieldValueName("fqdn" , i ));
    }

    DBsql.FreeSelect(); 
  } else dm->ns.length(0); // zadne DNS servry
  
  // dotaz na technicke kontakty
  if(  DBsql.SELECTCONTACTMAP( "nsset"  , nssetid ) )
  {
    len =  DBsql.GetSelectRows(); // pocet technickych kontaktu
    dm->tech.length(len); // technicke kontaktry handle
    for( i = 0 ; i < len ; i ++) 
      dm->tech[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );
    DBsql.FreeSelect();
  }

  DBsql.Disconnect();
  return dm;
}
  


