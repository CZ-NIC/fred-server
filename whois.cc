#include "whois.h"
#include "log.h"
#include "util.h"
#include "dbsql.h"

ccReg_Whois_i::ccReg_Whois_i(const std::string _database) : database(_database)
{}

ccReg_Whois_i::~ccReg_Whois_i()
{}


ccReg::DomainWhois* ccReg_Whois_i::Domain(const char* domain_name)
{
DB DBsql;
char sqlString[1024];
ccReg::DomainWhois *dm;
int clid , did ,nssetid , id  ;
int i , len;


dm = new ccReg::DomainWhois;

// dotaz na domenu
sprintf( sqlString , "SELECT * FROM DOMAIN WHERE fqdn=\'%s\'" , domain_name );

// free
dm->name= CORBA::string_dup( "" );
dm->created =  0 ;
dm->expired =  0 ;
dm->status = 0 ;
dm->registrarName = CORBA::string_dup( "" );
dm->registrarUrl  = CORBA::string_dup( "" );
dm->nsset  = CORBA::string_dup( "" );
dm->ns.length(0); // nulova sekvence
dm->tech.length(0); // nulova sekvence
dm->admin.length(0); // nulova sekvence

if( DBsql.OpenDatabase( database.c_str() ) )
{
  if( DBsql.ExecSelect( sqlString ) )
  {
  if( DBsql.GetSelectRows() == 1 )
    {
 
   dm->name= CORBA::string_dup(  DBsql.GetFieldValueName("fqdn" , 0 ) )  ; // plnohodnotne jmeno domeny

   dm->created =  get_time_t( DBsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas  vytvoreni domeny
   dm->expired =  get_time_t( DBsql.GetFieldValueName("ExDate" , 0 ) )  ; // datum expirace

   clid = atoi(  DBsql.GetFieldValueName("clid" , 0 ) ); // client registrator
   did = atoi(  DBsql.GetFieldValueName("id" , 0 ) ); // id domeny
   nssetid = atoi(  DBsql.GetFieldValueName("nsset" , 0 ) ); // id nsset
   id = atoi(  DBsql.GetFieldValueName("id" , 0 ) ); // id nsset

   dm->status = 1;



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


    // dotaz na NSSET
     //  handle na nsset
     dm->nsset=CORBA::string_dup( DBsql.GetValueFromTable( "NSSET" , "handle", "id" , nssetid ) );

    // dotaz na technicke kontakty
     sprintf( sqlString , "SELECT  handle FROM CONTACT  JOIN  nsset_contact_map ON nsset_contact_map.contactid=contact.id WHERE nsset_contact_map.nssetid=%d;" , nssetid  );

     if( DBsql.ExecSelect( sqlString ) )
          {
               len =  DBsql.GetSelectRows(); // pocet technickych kontaktu
               dm->tech.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) dm->tech[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );
               DBsql.FreeSelect();
          }

      // dotaz na admin kontakty
     sprintf( sqlString , "SELECT  handle FROM CONTACT  JOIN  domain_contact_map ON domain_contact_map.contactid=contact.id WHERE domain_contact_map.domainid=%d;" , id  );

     if( DBsql.ExecSelect( sqlString ) )
          {
               len =  DBsql.GetSelectRows(); // pocet technickych kontaktu
               dm->admin.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) dm->admin[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );
               DBsql.FreeSelect();
          }


   
 
   }
 
  }
 
 
 DBsql.Disconnect();
}

return dm;
}



