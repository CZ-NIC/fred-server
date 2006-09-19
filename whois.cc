#include "whois.h"
#include "log.h"
#include "util.h"
#include "dbsql.h"

ccReg_Whois_i::ccReg_Whois_i(const std::string _database) : database(_database)
{}

ccReg_Whois_i::~ccReg_Whois_i()
{}

ccReg::DomainWhois* ccReg_Whois_i::getDomain(const char* domain_name, CORBA::String_out timestamp)
{
DB DBsql;
char sqlString[1024];
char dateStr[32];
ccReg::DomainWhois *dm;
int clid , did ,nssetid , id  ;
int i , len;
time_t t , created , expired ;
bool found = false;
int db_error=0;

// casova znacka
t = time(NULL);


dm = new ccReg::DomainWhois;

// dotaz na domenu
sprintf( sqlString , "SELECT * FROM DOMAIN WHERE fqdn=\'%s\'" , domain_name );

/*
  // struktura
  struct DomainWhois
  {
    WhoisDomainStatus status;   // status 1 REGISTRED - 0 FREE
    string created;
    string expired;
    string registrarName;       // jmeno registratora
    string registrarUrl;        // odkaz na jeho webovou adresu
    NameServers ns;             // sequence nameservru
    TechContact tech;           // sequence handle na technicke kontakty
  };
*/

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


if( DBsql.OpenDatabase( database.c_str() ) )
{
  if( DBsql.ExecSelect( sqlString ) )
  {
  if( DBsql.GetSelectRows() == 1 )
    {
 
       
     created = get_time_t( DBsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas  vytvoreni domeny
     expired = get_time_t( DBsql.GetFieldValueName("ExDate" , 0 ) )  ; // datum expirace

     get_rfc3339_timestamp( created , dateStr  );
     dm->created = CORBA::string_dup( dateStr );

     get_rfc3339_timestamp( expired , dateStr  );
     dm->expired = CORBA::string_dup( dateStr );

     clid = atoi(  DBsql.GetFieldValueName("clid" , 0 ) ); // client registrator
     did = atoi(  DBsql.GetFieldValueName("id" , 0 ) ); // id domeny
     nssetid = atoi(  DBsql.GetFieldValueName("nsset" , 0 ) ); // id nsset
     id = atoi(  DBsql.GetFieldValueName("id" , 0 ) ); // id nsset

     if( t  > expired )   dm->status = ccReg::WHOIS_EXPIRED;
     else dm->status = ccReg::WHOIS_ACTIVE;



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

    // dotaz na  dns hosty z nssetu
    sprintf( sqlString , "SELECT  fqdn  FROM HOST WHERE  nssetid=%d;" , nssetid);
 

    if( DBsql.ExecSelect( sqlString ) )
      {
    
         len =  DBsql.GetSelectRows(); // pocet DNS servru
         dm->ns.length(len); // sequence DNS servru
         for( i = 0 ; i < len ; i ++)
            {
              dm->ns[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );
            }

        DBsql.FreeSelect(); 
     } else dm->ns.length(0); // zadne DNS servry



    // dotaz na technicke kontakty
     sprintf( sqlString , "SELECT  handle FROM CONTACT  JOIN  nsset_contact_map ON nsset_contact_map.contactid=contact.id WHERE nsset_contact_map.nssetid=%d;" , nssetid  );

     if( DBsql.ExecSelect( sqlString ) )
          {
               len =  DBsql.GetSelectRows(); // pocet technickych kontaktu
               dm->tech.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) dm->tech[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );
               DBsql.FreeSelect();
          }
/*
      // dotaz na admin kontakty
     sprintf( sqlString , "SELECT  handle FROM CONTACT  JOIN  domain_contact_map ON domain_contact_map.contactid=contact.id WHERE domain_contact_map.domainid=%d;" , id  );

     if( DBsql.ExecSelect( sqlString ) )
          {
               len =  DBsql.GetSelectRows(); // pocet technickych kontaktu
               dm->admin.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) dm->admin[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );
               DBsql.FreeSelect();
          }
*/

    found = true;   
   }
 
  }
  else  db_error=2;
 
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

// vrat casove razitko
get_rfc3339_timestamp( t , dateStr  );
timestamp =  CORBA::string_dup( dateStr );


if( !found )
{
throw ccReg::Whois::DomainNotFound( dateStr );  
}

return dm;
}



