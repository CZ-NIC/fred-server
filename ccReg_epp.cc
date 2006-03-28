//
// Example code for implementing IDL interfaces in file ccReg.idl
//

#include <iostream>
#include <stdlib.h> 
#include <string.h>
#include <ccReg.hh>
#include <ccReg_epp.h>

// funkce pro praci s postgres
#include "pqsql.h"

// konverze casu
#include "timestamp.h"


// definice pripojeno na databazi
// #define DATABASE "dbname=ccreg user=ccreg password=Eeh5ahSi"
#define DATABASE "dbname=ccReg user=pblaha"

//
// Example implementational code for IDL interface ccReg::EPP
//
ccReg_EPP_i::ccReg_EPP_i(){
  // add extra constructor code here
}
ccReg_EPP_i::~ccReg_EPP_i(){
  // add extra destructor code here
}
//   Methods corresponding to IDL attributes and operations
ccReg::Response ccReg_EPP_i::Login(const char* clientID, const char* pass, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)
{
PQ PQsql;
char sqlString[128];
ccReg::Response ret=0;

sprintf( sqlString , "SELECT id FROM REGISTRAR WHERE roid=\'%s\'" , clientID);

svTRID = CORBA::string_alloc( 16);
svTRID = CORBA::string_dup("SV_LOGIN" );

errMsg =  CORBA::string_alloc( 64);
errMsg =  CORBA::string_dup("Client Login" );

if( PQsql.OpenDatabase( DATABASE ) )
{
  if( PQsql.ExecSelect( sqlString ) )
  {
     loginID = atoi( PQsql.GetFieldValue( 0 , 0 ) );
     printf("login id %d\n" , loginID );
     PQsql.FreeSelect();
   }
 PQsql.Disconnect();
}

return ret;  
}

ccReg::Response ccReg_EPP_i::Logout(const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)
{
ccReg::Response ret=0;

svTRID = CORBA::string_alloc( 16);
svTRID = CORBA::string_dup("SV_LOGOUT" );

errMsg =  CORBA::string_alloc( 64);
errMsg =  CORBA::string_dup("Client Logout" );

debug("Logout: %d\n" , loginID );

return ret;
}

ccReg::Response ccReg_EPP_i::ContactCheck(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
 // #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactCheck(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactInfo(const char* roid, const char* clTRID, ccReg::Contact_out c, CORBA::String_out errMsg, CORBA::String_out svTRID)
{
PQ PQsql;
char sqlString[128];
ccReg::Response ret=0;
int id , clid , crid , upid;

// cislo transakce
svTRID = CORBA::string_alloc( 16);
svTRID = CORBA::string_dup("SV_12345" );
errMsg = CORBA::string_alloc( 32);
c = new ccReg::Contact;


sprintf( sqlString , "SELECT * FROM CONTACT WHERE roid=\'%s\'" , roid);

if( PQsql.OpenDatabase( DATABASE ) )
{
  if( PQsql.ExecSelect( sqlString ) )
  {
    debug("Date %s date %s\n" , PQsql.GetFieldValueName("CrDate" , 0 )   ,  PQsql.GetFieldValueName("UpDate" , 0 )   );

        clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) ); 
        crid = atoi( PQsql.GetFieldValueName("CrID" , 0 ) ); 
        upid = atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 
        debug("roid %s\n" , roid );
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

	ret= 1000;
	errMsg =  CORBA::string_dup("Contact found" );
    
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


 PQsql.Disconnect();
}


// pokud neneslo kontakt
if( ret !=  1000  )
{
// vyprazdni
c->ROID=CORBA::string_dup("");   
c->ClID=CORBA::string_dup("");    // identifikator registratora ktery ma pravo na zmeny
c->CrID=CORBA::string_dup("");    // identifikator registratora ktery vytvoril kontak
c->UpID=CORBA::string_dup("");    // identifikator registratora ktery provedl zmeny
c->CrDate=0; // datum a cas vytvoreni
c->UpDate=0; // datum a cas zmeny
c->TrDate=0;  // datum a cas transferu
c->Name=CORBA::string_dup(""); // jmeno nebo nazev kontaktu
c->Organization=CORBA::string_dup(""); // nazev organizace
c->Street1=CORBA::string_dup(""); // adresa
c->Street2=CORBA::string_dup(""); // adresa
c->Street3=CORBA::string_dup(""); // adresa
c->City=CORBA::string_dup("");  // obec
c->StateOrProvince=CORBA::string_dup("");
c->PostalCode=CORBA::string_dup(""); // PSC
c->Telephone=CORBA::string_dup("");
c->Fax=CORBA::string_dup("");
c->Email=CORBA::string_dup("");
c->NotifyEmail=CORBA::string_dup(""); // upozornovaci email
c->VAT=CORBA::string_dup(""); // DIC
c->Country=CORBA::string_dup(""); // zeme
c->AuthInfoPw=CORBA::string_dup(""); // autentifikace
c->SSN=CORBA::string_dup(""); // SSN

errMsg = CORBA::string_dup("Contact not found");

}

  // insert code here and remove the warning
//  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactInfo(const char* roid, const char* clTRID, ccReg::Contact_out c, CORBA::String_out errMsg, CORBA::String_out svTRID)>"

debug("return %d " ,  ret );
return ret;
}

ccReg::Response ccReg_EPP_i::ContactDelete(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)
{
ccReg::Response ret=0;
PQ PQsql;
char sqlString[1024];

// cislo transakce
svTRID = CORBA::string_alloc( 16);
svTRID = CORBA::string_dup("SV_12345" );
errMsg = CORBA::string_alloc( 32);

sprintf(  sqlString , "DELETE FROM Contact WHERE roid=\'%s\' " , roid );

if( PQsql.OpenDatabase( DATABASE ) )
{

   PQsql.ExecSQL( sqlString );

   ret= 1000;
   errMsg =  CORBA::string_dup("Contact delete" );

   PQsql.Disconnect();
}

return ret;
}




ccReg::Response ccReg_EPP_i::ContactUpdate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)
{
ccReg::Response ret=0;
PQ PQsql;
char sqlString[4096] , buf[1024];
int len;
strcpy( sqlString , "UPDATE Contact SET " );


// cislo transakce
svTRID = CORBA::string_alloc( 16);
svTRID = CORBA::string_dup("SV_12345" );
errMsg = CORBA::string_alloc( 32);


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

   ret= 1000;
   errMsg =  CORBA::string_dup("Contact update" );

   PQsql.Disconnect();
}

return ret;
}

ccReg::Response ccReg_EPP_i::ContactCreate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)
{

PQ PQsql;
char sqlString[4096] , buf[1024];
ccReg::Response ret=0;
int clid=0 , crid =0;

// cislo transakce
svTRID = CORBA::string_alloc( 16);
svTRID = CORBA::string_dup("SV_12345" );
errMsg = CORBA::string_alloc( 32);


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



   PQsql.ExecSQL( sqlString );
  
   ret= 1000;
   errMsg =  CORBA::string_dup("Contact ADD" );
     
 PQsql.Disconnect();
}



}

