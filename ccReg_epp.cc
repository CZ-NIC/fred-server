//
// Example code for implementing IDL interfaces in file ccReg.idl
//

#include <fstream.h>
#include <iostream.h>

#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <ccReg.hh>
#include <ccReg_epp.h>

// funkce pro praci s postgres servrem
#include "pqsql.h"

// konverze casu
#include "util.h"

#include "action.h"    // kody funkci do ccReg
#include "response.h"  // vracene chybove kody
		

// definice pripojeno na databazi
// #define DATABASE "dbname=ccreg user=ccreg password=Eeh5ahSi"
#define DATABASE "dbname=ccReg user=pblaha"

//
// Example implementational code for IDL interface ccReg::EPP
//
ccReg_EPP_i::ccReg_EPP_i(){

// 

}
ccReg_EPP_i::~ccReg_EPP_i(){
  // add extra destructor code here
// PQsql.Disconnect();
}


ccReg::Response* ccReg_EPP_i::ClientLogout(CORBA::Short clientID, const char* clTRID)
{
PQ  PQsql;
char sqlString[512];
ccReg::Response *ret;
ret = new ccReg::Response;
// default
ret->errCode=COMMAND_FAILED; // chyba

if(  PQsql.OpenDatabase( DATABASE ) )
{

sprintf( sqlString , "UPDATE Login set logoutdate='now' , logouttrid=\'%s\' where id=%d;" , clTRID , clientID );

if(  PQsql.ExecSQL( sqlString ) ) ret->errCode= COMMAND_OK; // uspesne oddlaseni
PQsql.Disconnect();
}

return ret;
}

//   Methods corresponding to IDL attributes and operations
ccReg::Response* ccReg_EPP_i::ClientLogin(const char* ClID, const char* clTRID, CORBA::Short& clientID)
{
PQ  PQsql;
char sqlString[512];
int roid;
ccReg::Response *ret;

ret = new ccReg::Response;

// default
ret->errCode=COMMAND_FAILED; // chyba
clientID = 0;

if(  PQsql.OpenDatabase( DATABASE ) )
{
if( PQsql.BeginTransaction() ) 
{

  
  // dotaz na ID registratora
  sprintf( sqlString , "SELECT id FROM REGISTRAR WHERE roid=\'%s\'" , ClID);

//ret->errMsg =  CORBA::string_dup("Client Login" );


  if( PQsql.ExecSelect( sqlString ) )
  {
     roid = atoi( PQsql.GetFieldValue( 0 , 0 ) );
     cout << "REGISTRAR ID " << ClID << " -> "  << roid << endl;
     PQsql.FreeSelect();
   }

// zapis do logovaci tabulky 
  sprintf( sqlString , "INSERT INTO  Login ( registrarid , logintrid ) VALUES ( %d , \'%s\' );" , roid ,  clTRID );

  if(  PQsql.ExecSQL( sqlString ) )
    {

     // zjisti posledni nejvyssi ID a to vrat jako loginID
     strcpy( sqlString , "SELECT  id FROM Login ORDER BY  id DESC LIMIT  1;" );
    
       if( PQsql.ExecSelect( sqlString ) )
         {
               clientID = atoi( PQsql.GetFieldValue( 0 , 0 ) );
               cout << "CLIENT ID " <<  " -> "  << clientID << endl;

              PQsql.FreeSelect();
           }     
    }


if( PQsql.EndTransaction() ) // transakce prosla uspesne
 {
   if( clientID ) ret->errCode= COMMAND_OK; // naslo loginID
 }
}

 PQsql.Disconnect();
}

return ret;  
}

ccReg::Response* ccReg_EPP_i::ContactCheck(const char* roid , CORBA::Short clientID, const char* clTRID)
{
  // insert code here and remove the warning
 // #warning "Code missing in function <ccReg::Response* ccReg_EPP_i::ContactCheck(const char* roid)>"
// return ret
}
 


ccReg::Response* ccReg_EPP_i::ContactInfo(const char* roid, ccReg::Contact_out c , CORBA::Short clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024];
ccReg::Response *ret;
int id , clid , crid , upid;

c = new ccReg::Contact;
ret = new ccReg::Response;

sprintf( sqlString , "SELECT * FROM CONTACT WHERE roid=\'%s\'" , roid);


ret->errCode=COMMAND_FAILED;

if( PQsql.OpenDatabase( DATABASE ) )
{
  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )
    {

        clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) ); 
        crid = atoi( PQsql.GetFieldValueName("CrID" , 0 ) ); 
        upid = atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 
        debug("roid %s\n" , roid );
        c->stat = 0 ; // status

	c->ROID=CORBA::string_dup( roid);           
	c->CrDate= get_gmt_time( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
	c->UpDate= get_gmt_time( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
	c->TrDate= get_gmt_time (PQsql.GetFieldValueName("TrDate" , 0 ) );  // datum a cas transferu
	c->Name=CORBA::string_dup( PQsql.GetFieldValueName("Name" , 0 )  ); // jmeno nebo nazev kontaktu
	c->Organization=CORBA::string_dup( PQsql.GetFieldValueName("Organization" , 0 )); // nazev organizace
	c->Street1=CORBA::string_dup( PQsql.GetFieldValueName("Street1" , 0 ) ); // adresa
	c->Street2=CORBA::string_dup( PQsql.GetFieldValueName("Street2" , 0 ) ); // adresa
	c->Street3=CORBA::string_dup( PQsql.GetFieldValueName("Street3" , 0 ) ); // adresa
	c->City=CORBA::string_dup( PQsql.GetFieldValueName("City" , 0 ) );  // obec
	c->StateOrProvince=CORBA::string_dup( PQsql.GetFieldValueName("StateOrProvince"  , 0 ));
	c->PostalCode=CORBA::string_dup(PQsql.GetFieldValueName("PostalCode" , 0 )); // PSC
	c->Telephone=CORBA::string_dup( PQsql.GetFieldValueName("Telephone" , 0 ));
	c->Fax=CORBA::string_dup(PQsql.GetFieldValueName("Fax" , 0 ));
	c->Email=CORBA::string_dup(PQsql.GetFieldValueName("Email" , 0 ));
	c->NotifyEmail=CORBA::string_dup(PQsql.GetFieldValueName("NotifyEmail" , 0 )); // upozornovaci email
	c->Country=CORBA::string_dup(PQsql.GetFieldValueName("Country" , 0 )); // Zeme
	c->VAT=CORBA::string_dup(PQsql.GetFieldValueName("VAT" , 0 )); // DIC
	c->SSN=CORBA::string_dup(PQsql.GetFieldValueName("SSN" , 0 )); // SSN
	c->AuthInfoPw=CORBA::string_dup(PQsql.GetFieldValueName("authinfopw" , 0 )); // autentifikace

    
        ret->errCode=COMMAND_OK;
    
        // free select
	PQsql.FreeSelect();
        
        sprintf( sqlString , "SELECT  roid FROM REGISTRAR WHERE id=%d;" , crid );
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator registratora ktery vytvoril kontak
             c->CrID=CORBA::string_dup(PQsql.GetFieldValueName("roid" , 0 ));  
             PQsql.FreeSelect(); 
          }


        sprintf( sqlString , "SELECT  roid FROM REGISTRAR WHERE id=%d;" , clid ); 
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator registratora ktery ma pravo na zmenu
             c->ClID=CORBA::string_dup(PQsql.GetFieldValueName("roid" , 0 ));  
             PQsql.FreeSelect(); 
          }


        sprintf( sqlString , "SELECT  roid FROM REGISTRAR WHERE id=%d;" , upid ); 
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator registratora ktery zmenil
             c->UpID=CORBA::string_dup(PQsql.GetFieldValueName("roid" , 0 ));  
             PQsql.FreeSelect(); 
          }

     }

   }


 PQsql.Disconnect();
}


// pokud neneslo kontakt
if( ret->errCode ==  COMMAND_FAILED  )
{
// vyprazdni
c->ROID=CORBA::string_dup("");   
c->ClID=CORBA::string_dup("");    // identifikator registratora ktery ma pravo na zmeny
c->CrID=CORBA::string_dup("");    // identifikator registratora ktery vytvoril kontak
c->UpID=CORBA::string_dup("");    // identifikator registratora ktery provedl zmeny
c->CrDate=0; // datum a cas vytvoreni
c->UpDate=0; // datum a cas zmeny
c->TrDate=0;  // datum a cas transferu
c->stat = 0 ; // status
c->Name=CORBA::string_dup(""); // jmeno nebo nazev kontaktu
c->Organization=CORBA::string_dup(""); // nazev organizace
c->Street1=CORBA::string_dup(""); // adresa
c->Street2=CORBA::string_dup(""); // adresa
c->Street3=CORBA::string_dup(""); // adresa
c->City=CORBA::string_dup("");  // obec
c->StateOrProvince=CORBA::string_dup("");
c->PostalCode=CORBA::string_dup(""); // PSC
c->Country=CORBA::string_dup(""); // zeme
c->Telephone=CORBA::string_dup("");
c->Fax=CORBA::string_dup("");
c->Email=CORBA::string_dup("");
c->NotifyEmail=CORBA::string_dup(""); // upozornovaci email
c->VAT=CORBA::string_dup(""); // DIC
c->SSN=CORBA::string_dup(""); // SSN
c->AuthInfoPw=CORBA::string_dup(""); // autentifikace


//ret->errMsg = CORBA::string_dup("Contact not found");
ret->errCode=2000;
}


return ret;
}

ccReg::Response* ccReg_EPP_i::ContactDelete(const char* roid , CORBA::Short clientID, const char* clTRID )
{
/*
ccReg::Response *ret;
PQ PQsql;
char sqlString[1024];

ret = new ccReg::Response;


sprintf(  sqlString , "DELETE FROM Contact WHERE roid=\'%s\' " , roid );

if( PQsql.OpenDatabase( DATABASE ) )
{

  if(  PQsql.ExecSQL( sqlString ) )
    {
      ret->errCode = 1000;
//      ret->errMsg =  CORBA::string_dup("Contact delete" );
    }


   PQsql.Disconnect();
}

return ret;
*/
}



ccReg::Response* ccReg_EPP_i::ContactUpdate(const ccReg::Contact& c , CORBA::Short clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
char sqlString[4096] , buf[1024];
int len;

ret = new ccReg::Response;


// TODO test jetsli clientID == clID

sprintf( sqlString , "UPDATE Contact SET upID=%d , " , clientID );



// jmeno nebo nazev
if( strlen(CORBA::string_dup(c.Name) ) ) {  sprintf( buf , "name=\'%s\' ," , CORBA::string_dup(c.Name) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Organization) ) ) {  sprintf( buf , "Organization=\'%s\' ," , CORBA::string_dup(c.Name) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Street1) ) ) {  sprintf( buf , "Street1=\'%s\' ," , CORBA::string_dup(c.Street1) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Street2) ) ) {  sprintf( buf , "Street2=\'%s\' ," , CORBA::string_dup(c.Street2) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Street3) ) ) {  sprintf( buf , "Street3=\'%s\' ," , CORBA::string_dup(c.Street3) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.City) ) ) {  sprintf( buf , "City=\'%s\' ," , CORBA::string_dup(c.City) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.StateOrProvince) ) ) {  sprintf( buf , "StateOrProvince=\'%s\' ," , CORBA::string_dup(c.StateOrProvince) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.PostalCode) ) ) {  sprintf( buf , "PostalCode=\'%s\' ," , CORBA::string_dup(c.PostalCode)  ) ; strcat( sqlString , buf ); }// PSC
if( strlen(CORBA::string_dup(c.Telephone) ) ) {  sprintf( buf, "Telephone=\'%s\' ," , CORBA::string_dup(c.Telephone) ) ; strcat( sqlString , buf ); } 
if( strlen(CORBA::string_dup(c.Fax) ) ) {  sprintf( buf , "Fax=\'%s\' ," , CORBA::string_dup(c.Fax) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Email) ) ) {  sprintf( buf , "Email=\'%s\' ," , CORBA::string_dup(c.Email) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.NotifyEmail) ) ) {  sprintf( buf, "NotifyEmail=\'%s\' ," , CORBA::string_dup(c.NotifyEmail) ) ; strcat( sqlString , buf ); } // upozornovaci email
if( strlen(CORBA::string_dup(c.VAT) ) ) {  sprintf( buf , "VAT=\'%s\' ," , CORBA::string_dup(c.VAT) ) ; strcat( sqlString , buf ); } // DIC
if( strlen(CORBA::string_dup(c.Country) ) ) {  sprintf( buf , "Country=\'%s\' ," , CORBA::string_dup(c.Country) ) ; strcat( sqlString , buf ); } // DIC
if( strlen(CORBA::string_dup(c.SSN) ) ) {  sprintf( buf , "SSN=\'%s\' ," , CORBA::string_dup(c.SSN) ) ; strcat( sqlString , buf ); } // SSN


// datum a cas updatu 
strcat(  sqlString ,  " UpDate=\'now\'" );
// dej na konec
sprintf( buf , " WHERE roid=\'%s\' " , CORBA::string_dup(c.ROID) );

strcat( sqlString , buf );

if( PQsql.OpenDatabase( DATABASE ) )
{

   PQsql.ExecSQL( sqlString );

   ret->errCode= 1000;
//   ret->errMsg =  CORBA::string_dup("Contact update" );

   PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::ContactCreate(const ccReg::Contact& c , CORBA::Short clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[4096] , buf[1024];
ccReg::Response *ret;
int clid=0 , crid =0;


ret = new ccReg::Response;
 
ret->errCode=COMMAND_FAILED;
 
if( PQsql.OpenDatabase( DATABASE ) )
{


    sprintf( sqlString , "SELECT  id FROM REGISTRAR WHERE roid= \'%s\';" ,  CORBA::string_dup(c.ClID) );
  
   if(  PQsql.ExecSelect( sqlString ) )
     {
        // id registratora clienta
        clid = atoi( PQsql.GetFieldValue( 0 , 0 ) );
        PQsql.FreeSelect();
     }


   sprintf( sqlString , "SELECT  id FROM REGISTRAR WHERE roid= \'%s\';" ,  CORBA::string_dup(c.CrID) );
   if(  PQsql.ExecSelect( sqlString ) )
     {
        // id registratora clienta
        crid = atoi( PQsql.GetFieldValue( 0 , 0 ) );
        PQsql.FreeSelect();
     }

strcpy( sqlString , "INSERT INTO CONTACT ( ROID , Handle , ClID , CrID, " );

if( strlen(CORBA::string_dup(c.Name) ) ) strcat( sqlString , " Name, " );
if( strlen(CORBA::string_dup(c.Organization) ) ) strcat( sqlString , "Organization," );
if( strlen(CORBA::string_dup(c.Street1) ) ) strcat( sqlString , " Street1, ");
if( strlen(CORBA::string_dup(c.Street2) ) ) strcat( sqlString , " Street2, ");
if( strlen(CORBA::string_dup(c.Street3) ) ) strcat( sqlString , " Street3, ");

if( strlen(CORBA::string_dup(c.City) ) ) strcat( sqlString ,  " City, " );
if( strlen(CORBA::string_dup(c.StateOrProvince) ) ) strcat( sqlString , " StateOrProvince," ); 
if( strlen(CORBA::string_dup(c.Fax) ) ) strcat( sqlString , " Fax, " );
if( strlen(CORBA::string_dup(c.Email) ) ) strcat( sqlString , " Email," ); 
if( strlen(CORBA::string_dup(c.NotifyEmail) ) ) strcat( sqlString , " NotifyEmail, ");
if( strlen(CORBA::string_dup(c.Country) ) ) strcat( sqlString , " Country," );
if( strlen(CORBA::string_dup(c.VAT) ) ) strcat( sqlString , " VAT," );
if( strlen(CORBA::string_dup(c.SSN) ) ) strcat( sqlString , " SSN," );

// datum a cas vytvoreni
strcat(  sqlString ,  " CrDate " );

sprintf( buf  , " )  VALUES ( \'%s\' ,  \'%s\' , %d , %d ,  " , CORBA::string_dup(c.ROID) ,  CORBA::string_dup(c.ROID) ,  clid  ,  crid ) ;
strcat( sqlString , buf );

if( strlen(CORBA::string_dup(c.Name) ) ) {  sprintf( buf , " \'%s\' , " , CORBA::string_dup(c.Name) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Organization) ) ) {  sprintf( buf , " \'%s\', " , CORBA::string_dup(c.Name) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Street1) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.Street1) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Street2) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.Street2) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Street3) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.Street3) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.City) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.City) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.StateOrProvince) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.StateOrProvince) ) ; strcat( sqlString ,  buf ); }
if( strlen(CORBA::string_dup(c.PostalCode) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.PostalCode)  ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Telephone) ) ) {  sprintf( buf, " \'%s\' ," , CORBA::string_dup(c.Telephone) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Fax) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.Fax) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.Email) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.Email) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.NotifyEmail) ) ) {  sprintf( buf, " \'%s\' ," , CORBA::string_dup(c.NotifyEmail) ) ; strcat( sqlString , buf ); } 
if( strlen(CORBA::string_dup(c.Country) ) ) {  sprintf( buf, " \'%s\' ," , CORBA::string_dup(c.Country) ) ; strcat( sqlString , buf ); }
if( strlen(CORBA::string_dup(c.VAT) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.SSN) ) ; strcat( sqlString , buf ); } // VAT
if( strlen(CORBA::string_dup(c.SSN) ) ) {  sprintf( buf , " \'%s\' ," , CORBA::string_dup(c.SSN) ) ; strcat( sqlString , buf ); } // SSN


// datum a cas vytvoreni
strcat(  sqlString ,  " \'now\' ) " );


  // pokud se podarilo insertovat
  if(  PQsql.ExecSQL( sqlString ) )    ret->errCode = COMMAND_OK;
     
 PQsql.Disconnect();
}


return ret;

}



ccReg::Response* ccReg_EPP_i::HostCheck(const char* name , CORBA::Short clientID, const char* clTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response** ccReg_EPP_i::HostCheck(const char* name)>"
}

ccReg::Response* ccReg_EPP_i::HostInfo(const char* name, ccReg::Host_out h , CORBA::Short clientID, const char* clTRID )
{
PQ PQsql;
char sqlString[1024] , adres[1042] , adr[128]  ;
ccReg::Response *ret;
int id , clid , domainid , upid;
int len , i;

h = new ccReg::Host;
ret = new ccReg::Response;

sprintf( sqlString , "SELECT * FROM HOST WHERE fqdn=\'%s\'" , name);

ret->errCode=COMMAND_FAILED;

if( PQsql.OpenDatabase( DATABASE ) )
{
  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )
    {


        clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) ); 
        upid = atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 

        domainid = atoi( PQsql.GetFieldValueName("domainid" , 0 ) );

        h->stat = 0 ; // status

	h->CrDate= get_gmt_time( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
	h->UpDate= get_gmt_time( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny

	h->name=CORBA::string_dup( PQsql.GetFieldValueName("fqdn" , 0 )  ); // jmeno nebo nazev kontaktu


    
        ret->errCode=COMMAND_OK;

        // pole adres
        strcpy( adres , PQsql.GetFieldValueName("ipaddr" , 0 ) );       
    
        // free select
	PQsql.FreeSelect();
        
        sprintf( sqlString , "SELECT  roid FROM REGISTRAR WHERE id=%d;" , upid );
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator registratora ktery vytvoril kontak
             h->UpID=CORBA::string_dup(PQsql.GetFieldValueName("roid" , 0 ));  
             PQsql.FreeSelect(); 
          }


        sprintf( sqlString , "SELECT  roid FROM REGISTRAR WHERE id=%d;" , clid ); 
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator registratora ktery ma pravo na zmenu
             h->ClID=CORBA::string_dup(PQsql.GetFieldValueName("roid" , 0 ));  
             PQsql.FreeSelect(); 
          }


        sprintf( sqlString , "SELECT  fqdn FROM DOMAIN WHERE id=%d;" , domainid ); 
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator registratora ktery zmenil
             h->domain=CORBA::string_dup(PQsql.GetFieldValueName("fqdn" , 0 ));  
             PQsql.FreeSelect(); 
          }
 
      len =  get_array_length( adres );
      h->inet.length(len); // sequence ip adres
      for( i = 0 ; i < len ; i ++) 
       {
           get_array_value( adres , adr , i );
           h->inet[i] =CORBA::string_dup( adr );
       }

    }

  }

 PQsql.Disconnect();
}


// pokud neneslo kontakt
if( ret->errCode ==  COMMAND_FAILED  )
{
// vyprazdni
h->domain =  CORBA::string_dup( "" ); // domena do ktere patri host
h->name=  CORBA::string_dup( "" ); // fqdn nazev domeny
h->stat=0; // status sequence
h->CrDate=0; // datum vytvoreni
h->UpDate=0; // datum zmeny
h->ClID=  CORBA::string_dup( "" );    // identifikator registratora ktery vytvoril host
h->UpID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam
h->inet.length(0); // sequence ip adres 


//ret->errMsg = CORBA::string_dup("Host not found");
ret->errCode=2000;
}


return ret;
}

ccReg::Response* ccReg_EPP_i::HostDelete(const char* name , CORBA::Short clientID, const char* clTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response** ccReg_EPP_i::HostDelete(const char* name)>"
}

ccReg::Response* ccReg_EPP_i::HostCreate(const ccReg::Host& h , CORBA::Short clientID, const char* clTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response** ccReg_EPP_i::HostCreate(const ccReg::Host& h)>"
}

ccReg::Response* ccReg_EPP_i::HostUpdate(const ccReg::Host& h, CORBA::Short clientID, const char* clTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response** ccReg_EPP_i::HostUpdate(const ccReg::Host& h)>"
}

ccReg::Response* ccReg_EPP_i::DomainCheck(const char* name , CORBA::Short clientID, const char* clTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response** ccReg_EPP_i::DomainCheck(const char* name)>"
}

ccReg::Response* ccReg_EPP_i::DomainInfo(const char* name, ccReg::Domain_out d , CORBA::Short clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024] , dns[1042]  , ns[128] ;
ccReg::Response *ret;
int id , clid , crid ,  upid , regid;
int i , len;

d = new ccReg::Domain;
ret = new ccReg::Response;

sprintf( sqlString , "SELECT * FROM DOMAIN WHERE fqdn=\'%s\'" , name);

ret->errCode=COMMAND_FAILED;

if( PQsql.OpenDatabase( DATABASE ) )
{
  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )
    {

        clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) ); 
        crid = atoi( PQsql.GetFieldValueName("CrID" , 0 ) ); 
        upid = atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 
        regid = atoi( PQsql.GetFieldValueName("registrant" , 0 ) ); 


        d->stat = 0 ; // status

	d->CrDate= get_gmt_time( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
	d->TrDate= get_gmt_time( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
	d->ExDate= get_gmt_time( PQsql.GetFieldValueName("ExDate" , 0 ) ); //  expirace

	d->ROID=CORBA::string_dup( PQsql.GetFieldValueName("roid" , 0 )  ); // jmeno nebo nazev kontaktu
	d->name=CORBA::string_dup( PQsql.GetFieldValueName("fqdn" , 0 )  ); // jmeno nebo nazev kontaktu


    
//	ret->errMsg =  CORBA::string_dup("Domain found" );
        ret->errCode=COMMAND_OK;

        // pole dns servru
        strcpy( dns , PQsql.GetFieldValueName("nameserver" , 0 ) );       
    
        // free select
	PQsql.FreeSelect();
        
        sprintf( sqlString , "SELECT  roid FROM REGISTRAR WHERE id=%d;" , upid );
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator registratora ktery vytvoril kontak
             d->UpID=CORBA::string_dup(PQsql.GetFieldValueName("roid" , 0 ));  
             PQsql.FreeSelect(); 
          }


        sprintf( sqlString , "SELECT  roid FROM REGISTRAR WHERE id=%d;" , clid ); 
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator registratora ktery ma pravo na zmenu
             d->ClID=CORBA::string_dup(PQsql.GetFieldValueName("roid" , 0 ));  
             PQsql.FreeSelect(); 
          }


        sprintf( sqlString , "SELECT  roid FROM REGISTRAR WHERE id=%d;" , crid );
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator registratora ktery ma pravo na zmenu
             d->CrID=CORBA::string_dup(PQsql.GetFieldValueName("roid" , 0 ));
             PQsql.FreeSelect();
          }


        sprintf( sqlString , "SELECT  handle FROM CONTACT WHERE id=%d;" , regid ); 
        if(  PQsql.ExecSelect( sqlString ) )
          {
             // identifikator kontaktu
             d->Registrant=CORBA::string_dup(PQsql.GetFieldValueName("handle" , 0 ));  
             PQsql.FreeSelect(); 
          }

      len =  get_array_length( dns );
      d->ns.length(len); // sequence DNS servru
      for( i = 0 ; i < len ; i ++)
       {
           get_array_value( dns , ns , i );
           d->ns[i] =CORBA::string_dup( ns );
       }


     }

   }


 PQsql.Disconnect();
}


// pokud neneslo kontakt
if( ret->errCode ==  COMMAND_FAILED )
{
debug("vyprazdneni");
// vyprazdni
d->ROID =  CORBA::string_dup( "" ); // domena do ktere patri host
d->name=  CORBA::string_dup( "" ); // fqdn nazev domeny
d->stat=0; // status sequence
d->CrDate=0; // datum vytvoreni
d->TrDate=0; // datum zmeny
d->ExDate=0; // datum zmeny
d->Registrant=CORBA::string_dup( "" ); 
d->ClID=  CORBA::string_dup( "" );    // identifikator registratora ktery vytvoril host
d->UpID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam
d->CrID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam
d->ns.length(0); // sequence ip adres 

//ret->errMsg = CORBA::string_dup("Host not found");
}


return ret;
}

ccReg::Response* ccReg_EPP_i::DomainDelete(const char* name , CORBA::Short clientID, const char* clTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response** ccReg_EPP_i::DomainDelete(const char* name)>"
}

ccReg::Response* ccReg_EPP_i::DomainUpdate(const ccReg::Domain& d , CORBA::Short clientID, const char* clTRID)
{

  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response** ccReg_EPP_i::DomainUpdate(const ccReg::Domain& d)>"
}

ccReg::Response* ccReg_EPP_i::DomainCreate(const ccReg::Domain& d , CORBA::Short clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[4096] , buf[1024];
ccReg::Response *ret;
int clid=0 , crid =0 , regid;


ret = new ccReg::Response;


// default
ret->errCode=COMMAND_FAILED; // chyba
 

if( PQsql.OpenDatabase( DATABASE ) )
{
   


    sprintf( sqlString , "SELECT  id FROM CONTACT WHERE roid= \'%s\';" ,  CORBA::string_dup(d.Registrant) );
  
   if(  PQsql.ExecSelect( sqlString ) )
     {
        // id registratora clienta
        regid = atoi( PQsql.GetFieldValue( 0 , 0 ) );
        PQsql.FreeSelect();
     }

    sprintf( sqlString , "SELECT  id FROM REGISTRAR WHERE roid= \'%s\';" ,  CORBA::string_dup(d.ClID) );
  
   if(  PQsql.ExecSelect( sqlString ) )
     {
        // id registratora clienta
        clid = atoi( PQsql.GetFieldValue( 0 , 0 ) );
        PQsql.FreeSelect();
     }


   sprintf( sqlString , "SELECT  id FROM REGISTRAR WHERE roid= \'%s\';" ,  CORBA::string_dup(d.CrID) );
   if(  PQsql.ExecSelect( sqlString ) )
     {
        // id registratora clienta
        crid = atoi( PQsql.GetFieldValue( 0 , 0 ) );
        PQsql.FreeSelect();
     }

strcpy( sqlString , "INSERT INTO DOMAIN ( zone , Status , ClID , CrID,  Registrant , ROID , fqdn  , crdate , update , exdate , nameserver ) VALUES ( 3 , \'{0}\' , " );


// datum a cas vytvoreni

sprintf( buf  , "  %d , %d , %d ,  \'%s\' ,    \'%s\' , 'now' , \'now\' , \'01-01-2008\' ,   \'{ dns.%s.net } \'  ); " ,   clid  ,  crid , regid , 
CORBA::string_dup(d.ROID) ,  CORBA::string_dup(d.name)  ,   CORBA::string_dup(d.ROID) ) ;
strcat( sqlString , buf );


// pokud se insertovalo do tabulky
if(    PQsql.ExecSQL( sqlString ) )  ret->errCode = COMMAND_OK;
     
PQsql.Disconnect();
}


return ret;
}



