//
// Example code for implementing IDL interfaces in file ccReg.idl
//

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ccReg.hh>
#include <ccReg_whois.h>

// funkce pro praci s postgres
#include "pqsql.h"

// konverze casu
#include "util.h"


// definice pripojeno na databazi
// #define DATABASE "dbname=ccreg user=ccreg password=Eeh5ahSi"
// #define DATABASE "dbname=ccReg user=pblaha"


//
// Example implementational code for IDL interface ccReg::Whois
//
ccReg_Whois_i::ccReg_Whois_i(){
  // add extra constructor code here
}
ccReg_Whois_i::~ccReg_Whois_i(){
  // add extra destructor code here
}

ccReg::DomainWhois* ccReg_Whois_i::Domain(const char* domain_name)
{
PQ PQsql;
char sqlString[1024];
ccReg::DomainWhois *dm;
int clid , did ,nssetid ;
char  dns[1024] , ns[128];
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
dm->ns.length(0); // nulova sekvence

if( PQsql.OpenDatabase( DATABASE ) )
{
  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )
    {
 
   dm->name= CORBA::string_dup(  PQsql.GetFieldValueName("fqdn" , 0 ) )  ; // plnohodnotne jmeno domeny

   dm->created =  get_gmt_time( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas  vytvoreni domeny
   dm->expired =  get_gmt_time( PQsql.GetFieldValueName("ExDate" , 0 ) )  ; // datum expirace

   clid = atoi(  PQsql.GetFieldValueName("clid" , 0 ) ); // client registrator
   did = atoi(  PQsql.GetFieldValueName("id" , 0 ) ); // id domeny
   nssetid = atoi(  PQsql.GetFieldValueName("nsset" , 0 ) ); // id nsset

   dm->status = 1;



     // free select
    PQsql.FreeSelect();
    // dotaz na registratora
    sprintf( sqlString , "SELECT Name , Url FROM  REGISTRAR WHERE id=%d;" , clid ) ;

    if( PQsql.ExecSelect( sqlString ) )
      {
        dm->registrarName = CORBA::string_dup( PQsql.GetFieldValueName("Name" , 0 ) );
        dm->registrarUrl  = CORBA::string_dup( PQsql.GetFieldValueName("Url" , 0 ) );
        PQsql.FreeSelect();
      }

    // dotaz na hosty z nssetu
    sprintf( sqlString , "SELECT fqdn FROM HOST WHERE nssetid=%d;" , nssetid ) ;
 


    if( PQsql.ExecSelect( sqlString ) )
      {
    
         len =  PQsql.GetSelectRows(); // pocet DNS servru
         dm->ns.length(len); // sequence DNS servru
         for( i = 0 ; i < len ; i ++)
            {
              dm->ns[i] = CORBA::string_dup( PQsql.GetFieldValue( i , 0 )  );
            }

        PQsql.FreeSelect(); 
     } else dm->ns.length(0); // zadne DNS servry


 
   }
 
  }
 
 
 PQsql.Disconnect();
}

return dm;
}



