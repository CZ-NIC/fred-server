//
// Example code for implementing IDL interfaces in file ccReg.idl
//

#include <iostream>
#include <ccReg.hh>
#include <ccReg_epp.h>

// funkce pro praci s postgres
#include "pqsql.h"



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
ccReg::Response ccReg_EPP_i::Login(const char* clientID, const char* pass, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::Login(const char* clientID, const char* pass, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::Logout(const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::Logout(const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactCheck(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactCheck(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactInfo(const char* roid, const char* clTRID, ccReg::Contact_out c, CORBA::String_out errMsg, CORBA::String_out svTRID)
{
PQ PQsql;
char sqlString[128];
ccReg::Response ret=0;

svTRID = CORBA::string_dup("SV_12345" );

c = new ccReg::Contact;


sprintf( sqlString , "SELECT * FROM CONTACT WHERE roid=\'%s\'" , roid);

if( PQsql.OpenDatabase() )
{
  if( PQsql.ExecSelect( sqlString ) )
  {
    printf("Name %s email %s\n" , PQsql.GetFieldValueName("name" , 0 )   ,  PQsql.GetFieldValueName("email" , 0 )   );

c->ROID=CORBA::string_dup( roid);   
c->ClID=CORBA::string_dup("");    // identifikator registratora ktery ma pravo na zmeny
c->CrID=CORBA::string_dup("");    // identifikator registratora ktery vytvoril kontak
c->UpID=CORBA::string_dup("");    // identifikator registratora ktery provedl zmeny
c->CrDate=0; // datum a cas vytvoreni
c->UpDate=0; // datum a cas zmeny
c->TrDate=0;  // datum a cas transferu
c->Name=CORBA::string_dup("MArtin Peterka"); // jmeno nebo nazev kontaktu
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
  
  ret= 1000;
    errMsg =  CORBA::string_dup("Contact found" );

    
    PQsql.FreeSelect();
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

errMsg = CORBA::string_dup("Contact not found");

}

  // insert code here and remove the warning
//  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactInfo(const char* roid, const char* clTRID, ccReg::Contact_out c, CORBA::String_out errMsg, CORBA::String_out svTRID)>"

debug("return %d " ,  ret );
return ret;
}

ccReg::Response ccReg_EPP_i::ContactDelete(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactDelete(const char* roid, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactUpdate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactUpdate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

ccReg::Response ccReg_EPP_i::ContactCreate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID){
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response ccReg_EPP_i::ContactCreate(const ccReg::Contact& c, const char* clTRID, CORBA::String_out errMsg, CORBA::String_out svTRID)>"
}

char* ccReg_EPP_i::clientID(){
  // insert code here and remove the warning
  #warning "Code missing in function <char* ccReg_EPP_i::clientID()>"
}



