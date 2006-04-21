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
// #define DATABASE "dbname=cc_reg user=pblaha host=petr"

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


ccReg::Response* ccReg_EPP_i::GetTransaction(CORBA::Long clientID, const char* clTRID, CORBA::Short errCode)
{
PQ  PQsql;

ccReg::Response *ret;
ret = new ccReg::Response;
// default
ret->errCode=COMMAND_FAILED; // chyba

if(  PQsql.OpenDatabase( DATABASE ) )
{
   if( PQsql.BeginAction( clientID ,  EPP_UnknowAction , (char * ) clTRID  ) )
   {

       
    // chybove hlaseni bere z clienta 
    ret->errCode = errCode;
    // zapis na konec action
    // zapis na konec action
    ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
   }

PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::ClientLogout(CORBA::Long clientID, const char* clTRID)
{
PQ  PQsql;
char sqlString[512];
ccReg::Response *ret;

cout << "Logout:  " << clientID << endl; 

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
ccReg::Response* ccReg_EPP_i::ClientLogin(const char* ClID, const char* passwd, const char* newpass, const char* clTRID, CORBA::Long& clientID)
{
PQ  PQsql;
char sqlString[1024];
int roid , id , i ;
bool pass=false;
ccReg::Response *ret;

ret = new ccReg::Response;

// default
ret->errCode=COMMAND_FAILED; // chyba
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup("DATABASE_FAILED"); // prazdna hodnota
 
clientID = 0;

if(  PQsql.OpenDatabase( DATABASE ) )
{
  // dotaz na ID registratora 
  roid  = PQsql.GetIDFrom( "REGISTRAR" , (char * ) ClID);   

  if( roid )
  {

   // kontrola hesla
  sprintf( sqlString , "SELECT password FROM REGISTRARACL WHERE registrarid=%d;" , roid);

  if( PQsql.ExecSelect( sqlString ) )
  {
     if( strcmp(  PQsql.GetFieldValue( 0 , 0 ) , passwd ) == 0   ) 
       {
         cout << "password accept "  << endl;
         pass = true;           
       }
      else {
            cout  << "bad password"  << endl;
            ret->errCode= COMMAND_AUTH_ERROR; 
            pass=false; 
           } 
     PQsql.FreeSelect();
   }




  if( pass ) // pokud je heslo spravne
  {
 
  // zmena hesla pokud je nejake nastaveno
  if( strlen( newpass ) )
    {
        sprintf( sqlString , "UPDATE REGISTRARACL SET password='%s' WHERE registrarid=%d;" , newpass  , roid );
        PQsql.ExecSQL( sqlString ); // uloz do tabulky
    }

  id =  PQsql.GetSequenceID( "login_id_seq" );   
   
// zapis do logovaci tabulky 
  sprintf( sqlString , "INSERT INTO  Login ( id , registrarid , logintrid ) VALUES ( %d , %d , \'%s\' );" , id ,  roid ,  clTRID );

  if(  PQsql.ExecSQL( sqlString ) ) // pokud se podarilo zapsat do tabulky
    {
     clientID = id;
     cout << "CLIENT ID " <<   clientID << endl;
     ret->errCode= COMMAND_OK; 
    }   

    // probehne action pro svrTrID 
    if( clientID )
    {
     if( PQsql.BeginAction( clientID , EPP_ClientLogin , (char * ) clTRID  ) )
      {
         // zapis na konec action
         ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
      } 


    }

  }
} 
 


PQsql.Disconnect();
}

return ret;  
}


ccReg::Response* ccReg_EPP_i::ContactCheck(const char* handle, CORBA::Boolean& avail, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024];
ccReg::Response *ret;

ret = new ccReg::Response;


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota

if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_ContactCheck , (char * ) clTRID  ) )
{
   sprintf( sqlString , "SELECT * FROM CONTACT WHERE handle=\'%s\'" , handle);


  if( PQsql.ExecSelect( sqlString ) )
  {
    if( PQsql.GetSelectRows() == 1 ) avail = false; // pokud uz je pouzivan
    else avail = true;  // pokud je dostupny

    // free select
    PQsql.FreeSelect();
    // comand OK
    ret->errCode=COMMAND_OK;
   }

   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;
}
 
 PQsql.Disconnect();
}
 
return ret;
}


ccReg::Response* ccReg_EPP_i::ContactInfo(const char* handle, ccReg::Contact_out c , CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024];
ccReg::Response *ret;
int id , clid , crid , upid;
int actionID=0;
c = new ccReg::Contact;
ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32);
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota

if( PQsql.OpenDatabase( DATABASE ) )
{

  
if( PQsql.BeginAction( clientID , EPP_ContactInfo , (char * ) clTRID  ) )
  {

  sprintf( sqlString , "SELECT * FROM CONTACT WHERE handle=\'%s\'" , handle);

  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )
    {

        clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) ); 
        crid = atoi( PQsql.GetFieldValueName("CrID" , 0 ) ); 
        upid = atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 

        c->stat = 0 ; // status

	c->ROID=CORBA::string_dup( PQsql.GetFieldValueName("ROID" , 0 ) ); // ROID     
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
        
        // identifikator registratora
        c->ClID =  CORBA::string_dup(   PQsql.GetHandleFrom( "REGISTRAR",  clid ) );
        c->CrID =  CORBA::string_dup(   PQsql.GetHandleFrom( "REGISTRAR",  upid ) );
        c->UpID =  CORBA::string_dup(   PQsql.GetHandleFrom( "REGISTRAR" , crid ) );

 

     }
    else 
     {
      PQsql.FreeSelect();
      ret->errCode =  COMMAND_OBJECT_NOT_EXIST;
     }    

   }

 
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;

}

 PQsql.Disconnect();
}




// vyprazdni kontakt pro navratovou hodnotu
if( ret->errCode != COMMAND_OK )
{
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
}


return ret;
}

ccReg::Response* ccReg_EPP_i::ContactDelete(const char* handle , CORBA::Long clientID, const char* clTRID )
{
ccReg::Response *ret;
PQ PQsql;
char sqlString[1024];
int regID=0 , id , clID = 0 ;
bool found=false;

ret = new ccReg::Response;


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

 if( PQsql.BeginAction( clientID , EPP_ContactDelete , (char * ) clTRID  ) )
 {

  sprintf( sqlString , "SELECT * FROM CONTACT WHERE handle=\'%s\'" , handle);


  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )ret->errCode=COMMAND_OBJECT_NOT_EXIST;
  else found =true;
  PQsql.FreeSelect();

  }

 if( found) // polud kontakt existuje
 {
  // get  registrator ID
  regID =   PQsql.GetLoginRegistrarID( clientID);


  if( regID )
  {
  sprintf(  sqlString , "SELECT  clID FROM Contact WHERE handle=\'%s\' ; " , handle );
  if(  PQsql.ExecSelect( sqlString ) )
    {
       if( PQsql.GetSelectRows() == 1 )
        {
          clID = atoi( PQsql.GetFieldValue( 0 , 0 ) );         
        }
        else  ret->errCode =COMMAND_OBJECT_NOT_EXIST;

       // free
       PQsql.FreeSelect();             
    }


    if( clID > 0 && clID == regID ) // pokud naslo clienta (objekt existuje) a je registratorem
      {
         sprintf(  sqlString , "DELETE FROM Contact WHERE handle=\'%s\' ; " , handle );         
         if(  PQsql.ExecSQL( sqlString ) ) ret->errCode =COMMAND_OK ; // pokud usmesne smazal
      } 

   
   }


 }
 
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
}

 
   PQsql.Disconnect();
}

return ret;
}



ccReg::Response* ccReg_EPP_i::ContactUpdate(const char* handle , const ccReg::Contact& c , CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
char sqlString[4096] , buf[1024];
int regID=0 , clID=0;
bool found;
int len;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota






if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_ContactUpdate , (char * ) clTRID  ) )
 {

  // get  registrator ID
  regID =   PQsql.GetLoginRegistrarID( clientID);


 

  sprintf(  sqlString , "SELECT  clID FROM Contact WHERE handle=\'%s\' ; " , handle );

  if(  PQsql.ExecSelect( sqlString ) )
    {
       if( PQsql.GetSelectRows() == 1 )
        {
          clID = atoi( PQsql.GetFieldValue( 0 , 0 ) );
        }
       else  ret->errCode =COMMAND_OBJECT_NOT_EXIST; // objekt nenalezen

       // free
       PQsql.FreeSelect();
    }



 


// TODO test jetsli clientID == clID

if( clID > 0 && clID == regID && regID > 0 ) // pokud naslo clienta (objekt existuje) a je registratorem
{
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
// na konec
sprintf( buf , " WHERE roid=\'%s\' " , CORBA::string_dup(c.ROID) );

strcat( sqlString , buf );


 if(   PQsql.ExecSQL( sqlString ) )   ret->errCode= 1000;

 }

   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
}

 
PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::ContactCreate(const char* handle ,const ccReg::Contact& c , CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[4096] , buf[1024];
ccReg::Response *ret;
int clid=0 , crid =0;


ret = new ccReg::Response;
 


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota






if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_ContactCreate , (char * ) clTRID  ) )
 {
 
  clid =  PQsql.GetIDFrom("REGISTRAR", CORBA::string_dup(c.ClID)  );
  crid =  PQsql.GetIDFrom("REGISTRAR", CORBA::string_dup(c.CrID)  );



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

   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
     
}

   PQsql.Disconnect();
}


return ret;

}


ccReg::Response* ccReg_EPP_i::NSSetCheck(const char* handle, CORBA::Boolean& avial, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024];
ccReg::Response *ret;
int i;

cout  << "NSSetCheck: " << handle  << endl;

ret = new ccReg::Response;


// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_NSsetCheck , (char * ) clTRID  ) )
 {

  sprintf( sqlString , "SELECT * FROM NSSET WHERE handle=\'%s\'" , handle);

  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 ) avial=false;
  else avial = true;

  // free select
  PQsql.FreeSelect();
  ret->errCode=COMMAND_OK;
  }


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }

 PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::NSSetInfo(const char* handle, ccReg::NSSet_out n, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024] ,  adres[1042] , adr[128];
ccReg::Response *ret;
int clid , crid , upid , nssetid;
int i , j  ,ilen , len;

cout  << "NSSetInfo: " << handle  << endl;

ret = new ccReg::Response;
n = new ccReg::NSSet;

// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_NSsetInfo , (char * ) clTRID  ) )
 {

  sprintf( sqlString , "SELECT * FROM NSSET WHERE handle=\'%s\'" , handle);


  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )
    {
 


        nssetid = atoi( PQsql.GetFieldValueName("ID" , 0 ) );
        clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) );
        crid = atoi( PQsql.GetFieldValueName("CrID" , 0 ) );
        upid = atoi( PQsql.GetFieldValueName("UpID" , 0 ) );

        n->stat = 0 ; // status

        n->ROID=CORBA::string_dup( PQsql.GetFieldValueName("ROID" , 0 ) ); // ROID
        n->handle=CORBA::string_dup( PQsql.GetFieldValueName("handle" , 0 ) ); // ROID
        n->CrDate= get_gmt_time( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
        n->UpDate= get_gmt_time( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
        n->ExDate= get_gmt_time (PQsql.GetFieldValueName("ExDate" , 0 ) );  // datum a cas expirace

        n->AuthInfoPw = CORBA::string_dup( PQsql.GetFieldValueName("AuthInfoPw" , 0 ) ); // autentifikace

        ret->errCode=COMMAND_OK;


        // free select
        PQsql.FreeSelect();

        n->ClID =  CORBA::string_dup(   PQsql.GetHandleFrom("REGISTRAR", clid ) );
        n->CrID =  CORBA::string_dup(   PQsql.GetHandleFrom("REGISTRAR", upid ) );
        n->UpID =  CORBA::string_dup(   PQsql.GetHandleFrom("REGISTRAR", crid ) );

        // dotaz na DNS servry 
        sprintf( sqlString , "SELECT fqdn , ipaddr FROM HOST  WHERE nssetid=%d" , nssetid);

        if(  PQsql.ExecSelect( sqlString ) )
          {  
             len =  PQsql.GetSelectRows();
             

               
             n->dns.length(len);
 
             for( i = 0 ; i < len ; i ++)   
                {
                     
                   // fqdn DNS servru nazev  
                   n->dns[i].fqdn = CORBA::string_dup(  PQsql.GetFieldValueName("fqdn" , i ) );
 
                   // pole adres
                   strcpy( adres , PQsql.GetFieldValueName("ipaddr" , i ) );

                   // zpracuj pole adres
                   ilen =  get_array_length( adres );
                   n->dns[i].inet.length(ilen); // sequence ip adres
                   for( j = 0 ; j < ilen ; j ++)
                      {
                        get_array_value( adres , adr , j );
                        n->dns[i].inet[j] =CORBA::string_dup( adr );
                      }


                }

             PQsql.FreeSelect();
          }


        // dotaz na technicke kontakty
        sprintf( sqlString , "SELECT  handle FROM CONTACT  JOIN  nsset_contact_map ON nsset_contact_map.contactid=contact.id WHERE nsset_contact_map.nssetid=%d;" , nssetid );
        if(  PQsql.ExecSelect( sqlString ) )
          {
               len =  PQsql.GetSelectRows(); // pocet technickych kontaktu
               n->tech.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) n->tech[i] = CORBA::string_dup( PQsql.GetFieldValue( i , 0 )  );

               PQsql.FreeSelect();
          }


     }
   else
    {
     // free select
    PQsql.FreeSelect();
    ret->errCode=COMMAND_OBJECT_NOT_EXIST;
    }

   }


       
 

   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }

 PQsql.Disconnect();
}


// vyprazdni
if( ret->errCode != COMMAND_OK )
{
n->ROID =  CORBA::string_dup( "" ); // fqdn nazev domeny
n->handle =  CORBA::string_dup( "" ); // handle nssetu
n->stat=0; // status sequence
n->CrDate=0; // datum vytvoreni
n->UpDate=0; // datum zmeny
n->ExDate=0;
n->AuthInfoPw=  CORBA::string_dup( "" ); 
n->dns.length(0);
n->tech.length(0);
}


cout << "return  " << endl;

return ret;
}

ccReg::Response* ccReg_EPP_i::NSSetDelete(const char* handle, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
char sqlString[1024];
int regID=0 , id , clID = 0 ,nssetID=0;
bool found=false;

ret = new ccReg::Response;


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

 if( PQsql.BeginAction( clientID , EPP_NSsetDelete , (char * ) clTRID  ) )
 {

  sprintf( sqlString , "SELECT * FROM NSSET WHERE handle=\'%s\'" , handle);


  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )ret->errCode=COMMAND_OBJECT_NOT_EXIST;
  else found =true;
  PQsql.FreeSelect();

  }

  if( found) // polud kontakt existuje
 {
 // get  registrator ID
   regID = PQsql.GetLoginRegistrarID( clientID );
 

  if( regID )
  {
  sprintf(  sqlString , "SELECT  *  FROM NSSET WHERE handle=\'%s\' ; " , handle );

  if(  PQsql.ExecSelect( sqlString ) )
    {
       if( PQsql.GetSelectRows() == 1 )
        {
          clID = atoi( PQsql.GetFieldValueName( "ClID" , 0 ) );         
          nssetID = atoi( PQsql.GetFieldValueName( "ID" , 0 ) ); // nsnsetid
        }
        else  ret->errCode =COMMAND_OBJECT_NOT_EXIST;

       // free
       PQsql.FreeSelect();             
    }


    if( clID > 0 && clID == regID ) // pokud naslo clienta (objekt existuje) a je registratorem
      {
         sprintf(  sqlString , "DELETE FROM NSSET WHERE handle=\'%s\' ; " , handle );         
         if(  PQsql.ExecSQL( sqlString ) )
         {
         // vymaz take podrizen hosty
         sprintf(  sqlString , "DELETE FROM HOST  WHERE nssetid=%d; " , nssetID );
         if(  PQsql.ExecSQL( sqlString ) ) ret->errCode =COMMAND_OK ; // pokud usmesne smazal
         }

      } 

   
   }


 }
 
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
}

 
   PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::NSSetCreate(const char* handle, const ccReg::NSSet& n, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[4096] , buf[1024];
char exDate[24] , Array[512];
ccReg::Response *ret;
int clid=0 , crid =0 , upid = 0 ,  nssetid=0 , techid=0;
bool  ins=false;
int i ,j  , len;

ret = new ccReg::Response;



// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_NSsetCreate , (char * ) clTRID  ) )
 {
 

    // prvni test zdali domena uz existuje
 if(  PQsql.GetIDFrom( "NSSET" ,  (char * )  handle ) )
  {
    ret->errCode=COMMAND_OBJECT_EXIST; // nsset uz zalozen ID existuje
  }   
  // pokud nexistuje
  else
   {

    clid =  PQsql.GetIDFrom( "REGISTRAR" ,  CORBA::string_dup(n.ClID) );
    crid =  PQsql.GetIDFrom( "REGISTRAR" ,  CORBA::string_dup(n.CrID) );
    upid =  PQsql.GetIDFrom( "REGISTRAR" ,  CORBA::string_dup(n.UpID) );



    // preved datum a cas expirace
    get_timestamp( n.ExDate , exDate );




  // ID je cislo ze sequence
  nssetid =  PQsql.GetSequenceID( "nsset_id_seq" );


      // zapis technicke kontakty 
      len = n.tech.length();
      for( i = 0 ; i < len ; i ++ )
         {
             techid =  PQsql.GetIDFrom( "Contact" , CORBA::string_dup(n.tech[i]) );
              
              sprintf( sqlString , "INSERT INTO nsset_contact_map VALUES ( %d , %d );"  , nssetid , techid );
              if( PQsql.ExecSQL( sqlString ) )  ins=true;  // pridej do tabulky 
              else { ins = false ; break; }  
         } 
 
        
       // zapis do tabulky hostu
      len = n.dns.length();
      if(ins )
      { 
        for( i = 0 ; i < len ; i ++ )
          {
           // ip adresa DNS hostu


           // preved sequenci adres
           strcat( Array , " { " );
           for( j = 0 ; j < n.dns[i].inet.length() ; j ++ )
           {
              if( j > 0 ) strcat( Array , " , " );
              strcat( Array ,  CORBA::string_dup( n.dns[i].inet[j]  ));
            }

            strcat( Array , " } " );

              
             sprintf( sqlString , "INSERT INTO HOST ( nssetid , fqdn , clid, upid , crdate  , update, status , ipaddr ) \
                                        VALUES ( %d , \'%s\'  , %d , %d ,  'now' , \'now\'  , '{0}' ,  \'%s\' );" ,
                                           nssetid , CORBA::string_dup( n.dns[i].fqdn ) ,  clid  ,  clid , Array );

             PQsql.ExecSQL( sqlString );  // pridej do tabulky

              if( PQsql.ExecSQL( sqlString ) )  ins=true;  // pridej do tabulky
              else { ins = false ; break; }

           } 

      }
 
       if( ins )
        {
           sprintf( sqlString , "INSERT INTO NSSET ( id ,  Status , ClID , CrID, UpID,   ROID , handle  , crdate , update , exdate , authinfopw  ) \
                           VALUES ( %d ,  \'{0}\' , %d , %d ,   \'%s\' ,    \'%s\' , 'now' , \'now\' , \'%s\' ,   \'%s\'  );" , 
            nssetid ,  clid  ,  crid , upid ,  CORBA::string_dup(n.ROID) ,  CORBA::string_dup(n.handle)  ,  exDate  ,  CORBA::string_dup(n.AuthInfoPw ) );

           // pokud se insertovalo do tabulky
           if(    PQsql.ExecSQL( sqlString ) )   ret->errCode = COMMAND_OK;
        }


     
    }


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;
 }

PQsql.Disconnect();
}


return ret;
}




ccReg::Response* ccReg_EPP_i::NSSetUpdate(const char* handle, const ccReg::NSSet& n, CORBA::Long clientID, const char* clTRID)
{
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response* ccReg_EPP_i::NSSetUpdate(const char* handle, const ccReg::NSSet& n, CORBA::Long clientID, const char* clTRID)>"
}


ccReg::Response* ccReg_EPP_i::HostCheck(const char* name , CORBA::Boolean& avial ,  CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024];
ccReg::Response *ret;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_HostCheck , (char * ) clTRID  ) )
 {

   sprintf( sqlString , "SELECT * FROM HOST WHERE fqdn=\'%s\'" , name);




   if( PQsql.ExecSelect( sqlString ) )
   {
   if( PQsql.GetSelectRows() == 1 ) avial=false;
   else avial = true;

   // free select
   PQsql.FreeSelect();
   ret->errCode=COMMAND_OK;
   }
 
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;

  }

 PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::HostInfo(const char* name, ccReg::Host_out h , CORBA::Long clientID, const char* clTRID )
{
PQ PQsql;
char sqlString[1024] , adres[1042] , adr[128]  ;
ccReg::Response *ret;
int id , clid , upid , nssetid;
int len , i;

h = new ccReg::Host;
ret = new ccReg::Response;


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota





if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_HostInfo , (char * ) clTRID  ) )
 {

   sprintf( sqlString , "SELECT * FROM HOST WHERE fqdn=\'%s\'" , name);


  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )
    {


        clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) ); 
        upid = atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 
        nssetid = atoi( PQsql.GetFieldValueName("NSSETID" , 0 ) );
 

        h->stat = 0 ; // status

	h->CrDate= get_gmt_time( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
	h->UpDate= get_gmt_time( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny

	h->fqdn=CORBA::string_dup( PQsql.GetFieldValueName("fqdn" , 0 )  ); // jmeno nebo nazev kontaktu


    
        ret->errCode=COMMAND_OK;

        // pole adres
        strcpy( adres , PQsql.GetFieldValueName("ipaddr" , 0 ) );       
    
        // free select
	PQsql.FreeSelect();

        h->nsset=CORBA::string_dup(  PQsql.GetHandleFrom( "NSSET" , nssetid ) );
         

        // registrator handle
        h->ClID= CORBA::string_dup( PQsql.GetHandleFrom( "REGISTRAR", clid ) ); 
        h->UpID= CORBA::string_dup( PQsql.GetHandleFrom( "REGISTRAR", upid ) ); 



 
      len =  get_array_length( adres );
      h->inet.length(len); // sequence ip adres
      for( i = 0 ; i < len ; i ++) 
       {
           get_array_value( adres , adr , i );
           h->inet[i] =CORBA::string_dup( adr );
       }

    }
  else 
    {
      PQsql.FreeSelect();
      ret->errCode =  COMMAND_OBJECT_NOT_EXIST;
    }
  
   }

  }
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;

 PQsql.Disconnect();
}



// vyprazdni
if( ret->errCode != COMMAND_OK )
{
h->fqdn =  CORBA::string_dup( "" ); // fqdn nazev domeny
h->nsset =  CORBA::string_dup( "" ); // handle nssetu
h->stat=0; // status sequence
h->CrDate=0; // datum vytvoreni
h->UpDate=0; // datum zmeny
h->ClID=  CORBA::string_dup( "" );    // identifikator registratora ktery vytvoril host
h->UpID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam
h->inet.length(0); // sequence ip adres 
}




return ret;
}

ccReg::Response* ccReg_EPP_i::HostDelete(const char* name , CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
char sqlString[1024];
int regID=0 , id , clID = 0 ;
bool found= false;

ret = new ccReg::Response;



ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_HostDelete , (char * ) clTRID  ) )
 {


   sprintf( sqlString , "SELECT * FROM HOST WHERE fqdn=\'%s\'" , name);


  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )ret->errCode=COMMAND_OBJECT_NOT_EXIST;
  else found =true;
  PQsql.FreeSelect();
  }

  if( found )
  {
   // get  registrator ID
   regID = PQsql.GetLoginRegistrarID(  clientID );

  if( regID )
  {
  sprintf(  sqlString , "SELECT  clID FROM HOST WHERE fqdn=\'%s\' ; " ,name );
  if(  PQsql.ExecSelect( sqlString ) )
    {
       if( PQsql.GetSelectRows() == 1 )
        {
          clID = atoi( PQsql.GetFieldValue( 0 , 0 ) );         
        }
        else  ret->errCode =COMMAND_OBJECT_NOT_EXIST;

       // free
       PQsql.FreeSelect();             
    }


    if( clID > 0 && clID == regID ) // pokud naslo clienta (objekt existuje) a je registratorem
      {
         sprintf(  sqlString , "DELETE FROM HOST WHERE fqdn=\'%s\' ; " , name );         
         if(  PQsql.ExecSQL( sqlString ) ) ret->errCode =COMMAND_OK ; // pokud usmesne smazal
      } 

   
    }

   }

   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }


   PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::HostCreate(const char* name , const ccReg::Host& h , CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[4096];
char  Array[512] ;
char *domainStr;
ccReg::Response *ret;
int clid=0  , nssetid=0;
bool notFound=false;
int i;

ret = new ccReg::Response;


// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_HostCreate , (char * ) clTRID  ) )
 {



// prvni test zdali domena uz existuje
    sprintf( sqlString , "SELECT id  FROM HOST  WHERE fqdn= \'%s\';" ,  name );

   if(  PQsql.ExecSelect( sqlString ) )
     {
         if( PQsql.GetSelectRows() == 1 )  ret->errCode=COMMAND_OBJECT_EXIST; // domana uz je zalozena
         else notFound=true;
         PQsql.FreeSelect();
     }

  // druhy dotaz na nsset 

   nssetid =  PQsql.GetIDFrom( "NSSET" , CORBA::string_dup(h.nsset)  );


   clid = PQsql.GetIDFrom( "REGISTRAR" , CORBA::string_dup(h.ClID) ) ;


 
// pokud domena nexistuje
  if( notFound && nssetid && clid )
  {

  // preved sequenci adres
   strcat( Array , " { " );
   for( i = 0 ; i < h.inet.length() ; i ++ )
   {
   if( i > 0 ) strcat( Array , " , " );
   strcat( Array ,  h.inet[i] );
   }

    strcat( Array , " } " );



     sprintf( sqlString , "INSERT INTO HOST (status , nssetid , fqdn , clid , upid , crdate , update , ipaddr ) VALUES (  \'{0}\' ,  %d , \'%s\' , %d  , %d , 'now' , \'now\' ,  \'%s\' );" , 
     nssetid ,  CORBA::string_dup(h.fqdn) , clid , clid , Array);


  // pokud se insertovalo do tabulky
   if(  PQsql.ExecSQL( sqlString ) )  ret->errCode = COMMAND_OK;   
  }


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }
 
PQsql.Disconnect();
}


return ret;

}

ccReg::Response* ccReg_EPP_i::HostUpdate(const char* name , const ccReg::Host& h, CORBA::Long clientID, const char* clTRID)
{
  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response** ccReg_EPP_i::HostUpdate(const ccReg::Host& h)>"
}

ccReg::Response* ccReg_EPP_i::DomainCheck(const char* fqdn , CORBA::Boolean& avial , CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024];
ccReg::Response *ret;
int i;

ret = new ccReg::Response;


// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainCheck , (char * ) clTRID  ) )
 {

  sprintf( sqlString , "SELECT * FROM DOMAIN WHERE fqdn=\'%s\'" , fqdn);

  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 ) avial=false;
  else avial = true; 

  // free select
  PQsql.FreeSelect();
  ret->errCode=COMMAND_OK;  
  }


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;
 }

 PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::DomainInfo(const char* fqdn, ccReg::Domain_out d , CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024] , dns[1042]  , ns[128] ;
ccReg::Response *ret;
int id , clid , crid ,  upid , regid ,nssetid;
int i , len;

d = new ccReg::Domain;
ret = new ccReg::Response;









// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainInfo , (char * ) clTRID  ) )
 {


  sprintf( sqlString , "SELECT * FROM DOMAIN WHERE fqdn=\'%s\'" , fqdn);

  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )
    {

        clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) ); 
        crid = atoi( PQsql.GetFieldValueName("CrID" , 0 ) ); 
        upid = atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 
        regid = atoi( PQsql.GetFieldValueName("registrant" , 0 ) ); 
        nssetid = atoi( PQsql.GetFieldValueName("nsset" , 0 ) );  

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
        
        d->ClID = CORBA::string_dup( PQsql.GetHandleFrom( "REGISTRAR" , clid ) );
        d->CrID = CORBA::string_dup( PQsql.GetHandleFrom( "REGISTRAR" , crid ) );
        d->UpID = CORBA::string_dup( PQsql.GetHandleFrom( "REGISTRAR" , upid ) );

        // vlastnik domeny
        d->Registrant=CORBA::string_dup( PQsql.GetHandleFrom( "CONTACT" , regid ) );


        //  handle na nsset
        d->nsset=CORBA::string_dup( PQsql.GetHandleFrom( "NSSET" , regid ) );
         

     }
   else
    {
     // free select
    PQsql.FreeSelect();
    ret->errCode=COMMAND_OBJECT_NOT_EXIST;
    }

   }



   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }
 PQsql.Disconnect();
}


// pokud neneslo kontakt
if( ret->errCode !=  COMMAND_OK )
{
debug("vyprazdneni");
// vyprazdni
d->ROID =  CORBA::string_dup( "" ); // domena do ktere patri host
d->name=  CORBA::string_dup( "" ); // fqdn nazev domeny
d->nsset = CORBA::string_dup( "" ); // nsset
d->stat=0; // status sequence
d->CrDate=0; // datum vytvoreni
d->TrDate=0; // datum zmeny
d->ExDate=0; // datum zmeny
d->Registrant=CORBA::string_dup( "" ); 
d->ClID=  CORBA::string_dup( "" );    // identifikator registratora ktery vytvoril host
d->UpID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam
d->CrID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam

//ret->errMsg = CORBA::string_dup("Host not found");
}


return ret;
}

ccReg::Response* ccReg_EPP_i::DomainDelete(const char* fqdn , CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
char sqlString[1024];
int regID=0 , id , clID = 0 ;
bool found=false;
ret = new ccReg::Response;


// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainDelete , (char * ) clTRID  ) )
 {


 sprintf( sqlString , "SELECT * FROM DOMAIN WHERE fqdn=\'%s\'" , fqdn);


  if( PQsql.ExecSelect( sqlString ) )
  {
  if( PQsql.GetSelectRows() == 1 )ret->errCode=COMMAND_OBJECT_NOT_EXIST;
  else found =true;
  PQsql.FreeSelect();
  }

  if( found) // polud domena existuje
  {

  // get  registrator ID
 sprintf(  sqlString , "SELECT registrarid FROM Login WHERE id=%d ; " , clientID );


  if(  PQsql.ExecSQL( sqlString ) )
    {
        regID = atoi( PQsql.GetFieldValue( 0 , 0 ) );
        PQsql.FreeSelect();
    }

  if( regID )
  {
  sprintf(  sqlString , "SELECT  clID FROM DOMAIN WHERE fqdn=\'%s\' ; " , fqdn );
  if(  PQsql.ExecSelect( sqlString ) )
    {
       if( PQsql.GetSelectRows() == 1 )
        {
          clID = atoi( PQsql.GetFieldValue( 0 , 0 ) );
        }
        else  ret->errCode =COMMAND_OBJECT_NOT_EXIST;

       // free
       PQsql.FreeSelect();
    }


    if( clID > 0 && clID == regID ) // pokud naslo clienta (objekt existuje) a je registratorem
      {
         sprintf(  sqlString , "DELETE FROM DOMAIN WHERE fqdn=\'%s\' ; " , fqdn );
         if(  PQsql.ExecSQL( sqlString ) ) ret->errCode =COMMAND_OK ; // pokud usmesne smazal
      }


   }

  }


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }
   PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::DomainUpdate(const char* fqdn , const ccReg::Domain& d , CORBA::Long clientID, const char* clTRID)
{

  // insert code here and remove the warning
  #warning "Code missing in function <ccReg::Response** ccReg_EPP_i::DomainUpdate(const ccReg::Domain& d)>"
}

ccReg::Response* ccReg_EPP_i::DomainCreate(const char* fqdn , const ccReg::Domain& d , CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[4096] , buf[1024];
char exDate[24] , nsArray[512];
ccReg::Response *ret;
int clid=0 , crid =0 , regid , nssetid;
bool notFound=false;
int i;

ret = new ccReg::Response;


// default


// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainCreate , (char * ) clTRID  ) )
 {
 

// prvni test zdali domena uz existuje
    sprintf( sqlString , "SELECT id  FROM DOMAIN  WHERE fqdn= \'%s\';" ,  fqdn );

   if(  PQsql.ExecSelect( sqlString ) )
     {
         if( PQsql.GetSelectRows() == 1 )  ret->errCode=COMMAND_OBJECT_EXIST; // domana uz je zalozena
         else notFound=true;
         PQsql.FreeSelect();
     }

// pokud domena nexistuje
if( notFound )
{
    sprintf( sqlString , "SELECT  id FROM NSSET WHERE roid= \'%s\';" ,  CORBA::string_dup(d.nsset) );
  
   if(  PQsql.ExecSelect( sqlString ) )
     {
        // id registratora clienta
        nssetid = atoi( PQsql.GetFieldValue( 0 , 0 ) );
        PQsql.FreeSelect();
     }

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


  // preved datum a cas expirace
  get_timestamp( d.ExDate , exDate );


  sprintf( sqlString , "INSERT INTO DOMAIN ( zone , Status , ClID , CrID,  Registrant , ROID , fqdn  , crdate , update , exdate , authinfopw , nsset ) VALUES ( 3 , \'{0}\' , %d , %d , %d ,  \'%s\' ,    \'%s\' , 'now' , \'now\' , \'%s\' ,   \'%s\'  , %d " , 
          clid  ,  crid , regid ,  CORBA::string_dup(d.ROID) ,  CORBA::string_dup(d.name)  ,  exDate  ,  CORBA::string_dup(d.AuthInfoPw ) , nssetid );





   // pokud se insertovalo do tabulky
   if(    PQsql.ExecSQL( sqlString ) )  ret->errCode = COMMAND_OK;   
   }


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }

PQsql.Disconnect();
}


return ret;
}



