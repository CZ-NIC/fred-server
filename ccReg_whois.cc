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
#include "timestamp.h"


// definice pripojeno na databazi
// #define DATABASE "dbname=ccreg user=ccreg password=Eeh5ahSi"
#define DATABASE "dbname=ccReg user=pblaha"


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
int clid , did ;
char  ns[256];
int i;


dm = new ccReg::DomainWhois;

// dotaz na domenu
sprintf( sqlString , "SELECT * FROM DOMAIN WHERE fqdn=\'%s\'" , domain_name );

if( PQsql.OpenDatabase( DATABASE ) )
{
  if( PQsql.ExecSelect( sqlString ) )
  {
   dm->name= CORBA::string_dup(  PQsql.GetFieldValueName("fqdn" , 0 ) )  ; // plnohodnotne jmeno domeny

   dm->status = 0; // TODO
   dm->created =  get_gmt_time( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas  vytvoreni domeny
   dm->expired =  get_gmt_time( PQsql.GetFieldValueName("ExDate" , 0 ) )  ; // datum expirace

   clid = atoi(  PQsql.GetFieldValueName("clid" , 0 ) ); // client registrator
   did = atoi(  PQsql.GetFieldValueName("id" , 0 ) ); // id domeny



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

    // dotaz na nameservry
       sprintf( sqlString , "SELECT fqdn  FROM  HOST WHERE domainid=%d;" , did ) ;

       memset( ns , 256 ,0 );

    if( PQsql.ExecSelect( sqlString ) )
      {

        for( i = 0 ; i <  PQsql.GetSelectRows() ; i ++ )   
           {
               if( i > 0 ) strcat( ns , "\t" );
               strcat( ns ,  PQsql.GetFieldValueName("fqdn" , i ) );
           }

         dm->NameServers  =  CORBA::string_dup( ns ); // uloz nameservry
        PQsql.FreeSelect();
      }
 
   }

 PQsql.Disconnect();
}

return dm;
}



