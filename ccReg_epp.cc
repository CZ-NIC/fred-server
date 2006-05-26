
//  implementing IDL interfaces for file ccReg.idl
// autor Petr Blaha petr.blaha@nic.cz

#include <fstream.h>
#include <iostream.h>

#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <ccReg.hh>
#include <ccReg_epp.h>

// funkce pro praci s postgres servrem
#include "pqsql.h"

// pmocne funkce
#include "util.h"

#include "action.h"    // kody funkci do ccReg
#include "response.h"  // vracene chybove kody
		
// prace se status flagy
#include "status.h"

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

/***********************************************************************
 *
 * FUNCTION:	GetTransaction
 *
 * DESCRIPTION: vraci pro klienta ze zadaneho clTRID
 *              vygenerovane  server transaction ID
 *
 * PARAMETERS:  clTRID - cislo transakce klienta
 *              clientID - identifikace klienta
 *              errCode - chybove hlaseni z klienta uklada se do tabulky action
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/



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
   cout << "svTRID: " << ret->svTRID << endl ; 
   }

PQsql.Disconnect();
}

return ret;
}

/***********************************************************************
 *
 * FUNCTION:    PollAcknowledgement
 *
 * DESCRIPTION: vraci pro klienta ze zadaneho clTRID server transaction ID
 *
 * PARAMETERS:  clTRID - cislo transakce klienta
 *              clientID - identifikace klienta
 *              errCode - chybove hlaseni z klienta uklada se do tabulky action
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response* ccReg_EPP_i::PollAcknowledgement(CORBA::Long msgID, CORBA::Short& count, CORBA::Long& newmsgID, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
ccReg::Response *ret;
ret = new ccReg::Response;


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota

count = 0 ;
newmsgID = 0 ;
if( PQsql.OpenDatabase( DATABASE ) )
{

  if( PQsql.BeginAction( clientID , EPP_PollAcknowledgement , (char * ) clTRID  ) )
  {
 
// TODO   
    
      // comand OK
      ret->errCode=COMMAND_NO_MESG;

      // zapis na konec action
      ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;
    
  }
 
 PQsql.Disconnect();
}
 
return ret;

}

ccReg::Response* ccReg_EPP_i::PollRequest(CORBA::Long& msgID, CORBA::Short& count, ccReg::timestamp& qDate, CORBA::String_out mesg, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
ccReg::Response *ret;
ret = new ccReg::Response;


//vyprazdni
qDate = 0 ;
count = 0;
msgID = 0;
mesg = CORBA::string_dup(""); // prazdna hodnota

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota

if( PQsql.OpenDatabase( DATABASE ) )
{

  if( PQsql.BeginAction( clientID , EPP_PollAcknowledgement , (char * ) clTRID  ) )
  {

      // TODO

      // comand OK
      ret->errCode=COMMAND_NO_MESG;

      // zapis na konec action
      ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;

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
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if(  PQsql.OpenDatabase( DATABASE ) )
{
  if( PQsql.BeginAction( clientID , EPP_ClientLogout , (char * ) clTRID  ) )
  {

    sprintf( sqlString , "UPDATE Login set logoutdate='now' , logouttrid=\'%s\' where id=%d;" , clTRID , clientID );

   if(  PQsql.ExecSQL( sqlString ) ) ret->errCode= COMMAND_LOGOUT; // uspesne oddlaseni

    // zapis na konec action
    ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;
  }

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
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota
 
clientID = 0;

if(  PQsql.OpenDatabase( DATABASE ) )
{
  // dotaz na ID registratora 
  roid  = PQsql.GetNumericFromTable( "REGISTRAR" , "id" , "handle" ,  (char * ) ClID);   

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

  id =  PQsql.GetSequenceID( "login" );   
   
// zapis do logovaci tabulky 
  sprintf( sqlString , "INSERT INTO  Login ( id , registrarid , logintrid ) VALUES ( %d , %d , \'%s\' );" , id ,  roid ,  clTRID );

  if(  PQsql.ExecSQL( sqlString ) ) // pokud se podarilo zapsat do tabulky
    {
     clientID = id;
     cout << "CLIENT ID " <<   clientID << endl;
     ret->errCode= COMMAND_OK; 
    }   


  }

    // probehne action pro svrTrID     
     if( PQsql.BeginAction( clientID , EPP_ClientLogin , (char * ) clTRID  ) )
      {
         // zapis na konec action
         ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
      } 


} 
 


PQsql.Disconnect();
}

return ret;  
}



ccReg::Response* ccReg_EPP_i::ContactCheck(const ccReg::Check& handle, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
ccReg::Response *ret;
int  len , av ;
long unsigned int i;
ret = new ccReg::Response;

a = new ccReg::Avail;

len = handle.length();
a->length(len);

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota

if( PQsql.OpenDatabase( DATABASE ) )
{

  if( PQsql.BeginAction( clientID , EPP_ContactCheck , (char * ) clTRID  ) )
  {
 
    for( i = 0 ; i < len ; i ++ )
     { 
      if( PQsql.GetNumericFromTable( "CONTACT" , "id" , "handle" , CORBA::string_dup(  handle[i] ) ) ) a[i] =  0;
      else   a[i]= 1;    // kontak je volny 
     }
    
      // comand OK
      ret->errCode=COMMAND_OK;

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
Status status;
char sqlString[1024];
ccReg::Response *ret;
char *cc;
int id , clid , crid , upid;
int actionID=0;
int len , i  , s ;

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

        clid =  atoi( PQsql.GetFieldValueName("ClID" , 0 ) ); 
        crid =  atoi( PQsql.GetFieldValueName("CrID" , 0 ) ); 
        upid =  atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 



        status.Make(  PQsql.GetFieldValueName("status" , 0 ) ) ; // status


	c->handle=CORBA::string_dup( PQsql.GetFieldValueName("handle" , 0 ) ); // handle
	c->ROID=CORBA::string_dup( PQsql.GetFieldValueName("ROID" , 0 ) ); // ROID     
	c->CrDate= get_time_t( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
	c->UpDate= get_time_t( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
	c->TrDate= get_time_t( PQsql.GetFieldValueName("TrDate" , 0 ) );  // datum a cas transferu
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
        cc = PQsql.GetFieldValueName("Country" , 0 ); // kod zeme

	c->AuthInfoPw=CORBA::string_dup(PQsql.GetFieldValueName("AuthInfoPw" , 0 )); // Zeme
	c->VAT=CORBA::string_dup(PQsql.GetFieldValueName("VAT" , 0 )); // DIC
	c->SSN=CORBA::string_dup(PQsql.GetFieldValueName("SSN" , 0 )); // SSN
	c->AuthInfoPw=CORBA::string_dup(PQsql.GetFieldValueName("authinfopw" , 0 )); // autentifikace

        
        c->DiscloseName = PQsql.GetFieldBooleanValueName( "DiscloseName" , 0 );
        c->DiscloseOrganization = PQsql.GetFieldBooleanValueName( "DiscloseOrganization" , 0 );
        c->DiscloseAddress = PQsql.GetFieldBooleanValueName( "DiscloseAddress" , 0 );
        c->DiscloseTelephone = PQsql.GetFieldBooleanValueName( "DiscloseTelephone" , 0 );
        c->DiscloseFax  = PQsql.GetFieldBooleanValueName( "DiscloseFax" , 0 );
        c->DiscloseEmail = PQsql.GetFieldBooleanValueName( "DiscloseEmail" , 0 );

 
    
        ret->errCode=COMMAND_OK;
    
        // free select
	PQsql.FreeSelect();


        // zpracuj pole statusu
        len =  status.Length();
        c->stat.length(len);
        cout << " status length: "  << len  << endl ;
        for( i = 0 ; i < len  ; i ++)
           {
              c->stat[i] = CORBA::string_dup( PQsql.GetStatusString(  status.Get(i)  ) );
           }

              
        // identifikator registratora
        c->ClID =  CORBA::string_dup(  PQsql.GetRegistrarHandle( clid ) );
        c->CrID =  CORBA::string_dup(  PQsql.GetRegistrarHandle( crid ) );
        c->UpID =  CORBA::string_dup(  PQsql.GetRegistrarHandle( upid ) );

//	c->Country=CORBA::string_dup( PQsql.GetValueFromTable("enum_country" , "country" , "id" ,  cc ) ); // uplny nazev zeme
        c->Country=CORBA::string_dup( cc ); // kod zeme

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
c->handle=CORBA::string_dup("");
c->ROID=CORBA::string_dup("");   
c->ClID=CORBA::string_dup("");    // identifikator registratora ktery ma pravo na zmeny
c->CrID=CORBA::string_dup("");    // identifikator registratora ktery vytvoril kontak
c->UpID=CORBA::string_dup("");    // identifikator registratora ktery provedl zmeny
c->CrDate=0; // datum a cas vytvoreni
c->UpDate=0; // datum a cas zmeny
c->TrDate=0;  // datum a cas transferu
c->stat.length(0); // status
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
Status status;
char sqlString[1024];
int regID=0 , id , clID = 0 ;
bool stat;

ret = new ccReg::Response;


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

 if( PQsql.BeginAction( clientID , EPP_ContactDelete , (char * ) clTRID  ) )
 {
  if( PQsql.BeginTransaction() )
  {

  if( ( id = PQsql.GetNumericFromTable(  "CONTACT"  , "id" , "handle" , (char * ) handle ) ) )
  {
   // get  registrator ID
   regID =  PQsql.GetLoginRegistrarID( clientID);
   // client contaktu 
   clID  =  PQsql.GetNumericFromTable(  "CONTACT"  , "clID" , "handle" , (char * ) handle );

    // zpracuj  pole statusu
   status.Make( PQsql.GetStatusFromTable( "CONTACT" , id ) );

   if( status.Test( STATUS_clientDeleteProhibited ) || status.Test( STATUS_serverDeleteProhibited )  ) stat = false;
   else stat = true; // status je OK

  

   if(  clID == regID && stat ==true ) // pokud je klient je registratorem a zaroven je status OK
     {
         //  uloz do historie
         if( PQsql.MakeHistory() ) 
           {
            if( PQsql.SaveHistory( "Contact" , "id" , id ) ) // uloz zaznam
              {
                 if(  PQsql.DeleteFromTable("CONTACT" , "id" , id  ) ) ret->errCode =COMMAND_OK ; // pokud usmesne smazal
              }
           }
          
                      
     } 

  } 
  else ret->errCode=COMMAND_OBJECT_NOT_EXIST; // pokud objekt neexistuje

         // pokud vse proslo
  if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();   // pokud uspesne nainsertovalo commitni zmeny
  else PQsql.RollbackTransaction(); // pokud nejake chyba zrus trasakci
  }
 
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
  }

 
PQsql.Disconnect();
}

return ret;
}



ccReg::Response* ccReg_EPP_i::ContactUpdate(const char* handle , const ccReg::ContactChange& c, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
char sqlString[4096] , buf[1024]  ;
char statusString[128] ;
int regID=0 , clID=0 , id;
bool stat;
int len , i ; 
Status status;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota

// NEMAZAT vypis status parametru
len =   status_add.length() ;
cout << "ADD "  << len << endl;
for( i = 0 ; i < len ; i ++ ) cout << i <<  status_add[i] << endl;

len =   status_rem.length() ;
cout << "REM "  << len << endl;
for( i = 0 ; i < len ; i ++ ) cout << i <<  status_rem[i] << endl;

// konec

if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_ContactUpdate , (char * ) clTRID  ) )
 {
  if(  PQsql.BeginTransaction() )  // zahajeni transakce
  {
  id = PQsql.GetNumericFromTable(  "CONTACT"  , "id" , "handle" , (char * ) handle );


  if( id == 0 ) ret->errCode= COMMAND_OBJECT_NOT_EXIST;
  else // pokud kontakt existuje 
  {
  // get  registrator ID
  regID =   PQsql.GetLoginRegistrarID( clientID);
  // client contaktu
  clID  =  PQsql.GetNumericFromTable(  "CONTACT"  , "clID" , "handle" , (char * ) handle );
  // zpracuj  pole statusu
  status.Make( PQsql.GetStatusFromTable( "CONTACT" , id ) );

   if( status.Test( STATUS_clientUpdateProhibited ) || status.Test( STATUS_serverUpdateProhibited )  ) stat = false;
   else stat = true; // status je OK

    if( clID == regID && stat ) // pokud je registrator clientem kontaktu a je status v poradku
      {
         //  uloz do historie
         if( PQsql.MakeHistory() )
           {
            if( PQsql.SaveHistory( "Contact" , "id" , id ) ) // uloz zaznam
              {
 
                // pridany status
                len  =   status_add.length();                             
                for( i = 0 ; i < len ; i ++) status.Add(  PQsql.GetStatusNumber( CORBA::string_dup( status_add[i] ) )  );

                   
                // zruseny status flagy
                len  =   status_rem.length();
                for( i = 0 ; i < len ; i ++) status.Rem( PQsql.GetStatusNumber( CORBA::string_dup( status_rem[i] ) )); 


                //  vygeneruj  novy status string array
                status.Array( statusString );

                  
                strcpy( sqlString , "UPDATE Contact SET " );    

                // pridat zmenene polozky
                add_field_value( sqlString , "Name" ,  CORBA::string_dup(c.Name)  ) ;
                add_field_value( sqlString , "Organization" ,  CORBA::string_dup(c.Organization)  ) ;
                add_field_value( sqlString , "Street1" ,  CORBA::string_dup(c.Street1)  ) ;
                add_field_value( sqlString , "Street2" ,  CORBA::string_dup(c.Street2)  ) ;
                add_field_value( sqlString , "Street3" ,  CORBA::string_dup(c.Street3)  ) ;
                add_field_value( sqlString , "City" ,  CORBA::string_dup(c.City)  ) ;
                add_field_value( sqlString , "StateOrProvince" ,  CORBA::string_dup(c.StateOrProvince)  ) ;
                add_field_value( sqlString , "PostalCode" ,  CORBA::string_dup(c.PostalCode)  ) ;
                add_field_value( sqlString , "Country" , CORBA::string_dup(c.Country) );
                add_field_value( sqlString , "Telephone" ,  CORBA::string_dup(c.Telephone)  ) ;
                add_field_value( sqlString , "Fax" ,  CORBA::string_dup(c.Fax)  ) ;
                add_field_value( sqlString , "Email" ,  CORBA::string_dup(c.Email)  ) ;
                add_field_value( sqlString , "NotifyEmail" ,  CORBA::string_dup(c.NotifyEmail)  ) ;
                add_field_value( sqlString , "VAT" ,  CORBA::string_dup(c.VAT)  ) ;
                add_field_value( sqlString , "SSN" ,  CORBA::string_dup(c.SSN)  ) ;
                add_field_value( sqlString , "AuthInfoPw" ,  CORBA::string_dup(c.AuthInfoPw)  ) ;

                //  Disclose parametry
                add_field_bool( sqlString , "DiscloseName" , c.DiscloseName );
                add_field_bool( sqlString , "DiscloseOrganization" , c.DiscloseOrganization );
                add_field_bool( sqlString , "DiscloseAddress" , c.DiscloseAddress );
                add_field_bool( sqlString , "DiscloseTelephone" , c.DiscloseTelephone );
                add_field_bool( sqlString , "DiscloseFax" , c.DiscloseFax );
                add_field_bool( sqlString , "DiscloseEmail" , c.DiscloseEmail );

                // datum a cas updatu  plus kdo zmenil zanzma na konec
                sprintf( buf , " UpDate=\'now\' ,   UpID=%d  , status=\'%s' WHERE id=%d  " , regID , statusString  , id );
                strcat(  sqlString ,  buf );

                if(   PQsql.ExecSQL( sqlString ) )  
                   {
                       if( PQsql.CommitTransaction() ) ret->errCode= COMMAND_OK; // comit transakce
                   }

              }
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

ccReg::Response* ccReg_EPP_i::ContactCreate(const char* handle , const ccReg::ContactChange& c, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[4096] , buf[1024] ;
ccReg::Response *ret;
int regID;


ret = new ccReg::Response;
 


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota


if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_ContactCreate , (char * ) clTRID  ) )
 {
 
// prvni test zdali kontakt uz existuje
 if(  PQsql.GetNumericFromTable("CONTACT" , "id" ,  "handle" , (char * ) handle ) ) ret->errCode=COMMAND_OBJECT_EXIST; // je uz zalozena
 else
// pokud kontakt nexistuje
 {
        // get  registrator ID
        regID =   PQsql.GetLoginRegistrarID( clientID);

      
	strcpy( sqlString , "INSERT INTO CONTACT ( handle , ROID ,  CrDate ,  ClID , CrID   " );
        create_field_fname(sqlString , "Name" , CORBA::string_dup(c.Name) );
        create_field_fname(sqlString , "Organization" , CORBA::string_dup(c.Organization) );
        create_field_fname(sqlString , "Street1" , CORBA::string_dup(c.Street1) );
        create_field_fname(sqlString , "Street2" , CORBA::string_dup(c.Street2) );
        create_field_fname(sqlString , "Street3" , CORBA::string_dup(c.Street3) );
        create_field_fname(sqlString , "City" , CORBA::string_dup(c.City) );
        create_field_fname(sqlString , "StateOrProvince" , CORBA::string_dup(c.StateOrProvince) );
        create_field_fname(sqlString , "PostalCode" , CORBA::string_dup(c.PostalCode) );
        create_field_fname(sqlString , "Country" , CORBA::string_dup(c.Country) );
        create_field_fname(sqlString , "Telephone" , CORBA::string_dup(c.Telephone) );
        create_field_fname(sqlString , "Fax" , CORBA::string_dup(c.Fax) );
        create_field_fname(sqlString , "Email" , CORBA::string_dup(c.Email) );
        create_field_fname(sqlString , "NotifyEmail" , CORBA::string_dup(c.NotifyEmail) );
        create_field_fname(sqlString , "VAT" , CORBA::string_dup(c.VAT) );
        create_field_fname(sqlString , "SSN" , CORBA::string_dup(c.SSN) );
        create_field_fname(sqlString , "AuthInfoPw" , CORBA::string_dup(c.AuthInfoPw) );

        if(  c.DiscloseName ) strcat( sqlString , " , DiscloseName " );
        if(  c.DiscloseOrganization ) strcat( sqlString , " , DiscloseOrganization " );
        if(  c.DiscloseAddress  ) strcat( sqlString , " , DiscloseAddress  " );
        if(  c.DiscloseTelephone ) strcat( sqlString , " , DiscloseTelephone " );
        if(  c.DiscloseFax ) strcat( sqlString , " , DiscloseFax  " );
        if(  c.DiscloseEmail ) strcat( sqlString , " , DiscloseEmail " );

	sprintf( buf  , " )  VALUES ( \'%s\' , \'%s\' ,  'now()' ,   %d , %d   " ,  (char * ) handle ,   (char * ) handle ,   regID , regID ); 

	strcat( sqlString , buf );

        create_field_value(sqlString , "Name" , CORBA::string_dup(c.Name) );
        create_field_value(sqlString , "Organization" , CORBA::string_dup(c.Organization) );
        create_field_value(sqlString , "Street1" , CORBA::string_dup(c.Street1) );
        create_field_value(sqlString , "Street2" , CORBA::string_dup(c.Street2) );
        create_field_value(sqlString , "Street3" , CORBA::string_dup(c.Street3) );
        create_field_value(sqlString , "City" , CORBA::string_dup(c.City) );
        create_field_value(sqlString , "StateOrProvince" , CORBA::string_dup(c.StateOrProvince) );
        create_field_value(sqlString , "PostalCode" , CORBA::string_dup(c.PostalCode) );
        create_field_value(sqlString , "Country" , CORBA::string_dup(c.Country) );
        create_field_value(sqlString , "Telephone" , CORBA::string_dup(c.Telephone) );
        create_field_value(sqlString , "Fax" , CORBA::string_dup(c.Fax) );
        create_field_value(sqlString , "Email" , CORBA::string_dup(c.Email) );
        create_field_value(sqlString , "NotifyEmail" , CORBA::string_dup(c.NotifyEmail) );
        create_field_value(sqlString , "VAT" , CORBA::string_dup(c.VAT) );
        create_field_value(sqlString , "SSN" , CORBA::string_dup(c.SSN) );
        create_field_value(sqlString , "AuthInfoPw" , CORBA::string_dup(c.AuthInfoPw) );
 
        if(  c.DiscloseName ) strcat( sqlString , " , 't' " );
        if(  c.DiscloseOrganization ) strcat( sqlString , ", 't' " );
        if(  c.DiscloseAddress  ) strcat( sqlString ,  " , 't' " );
        if(  c.DiscloseTelephone ) strcat( sqlString , " , 't' " );
        if(  c.DiscloseFax ) strcat( sqlString ,      "  , 't' " );
        if(  c.DiscloseEmail ) strcat( sqlString     , " , 't' " );
 
       

 	  // ukoncit retezec
           strcat(  sqlString ,  "  ); " );
	

	  // pokud se podarilo insertovat
	  if(  PQsql.ExecSQL( sqlString ) )   ret->errCode = COMMAND_OK;
        }    
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }
PQsql.Disconnect();
}


return ret;

}




ccReg::Response* ccReg_EPP_i::NSSetCheck(const ccReg::Check& handle, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
ccReg::Response *ret;
int  len , av ;
long unsigned int i;
ret = new ccReg::Response;

a = new ccReg::Avail;

len = handle.length();
a->length(len);

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota

if( PQsql.OpenDatabase( DATABASE ) )
{

  if( PQsql.BeginAction( clientID , EPP_NSsetCheck , (char * ) clTRID  ) )
  {

    for( i = 0 ; i < len ; i ++ )
     {
       if( PQsql.GetNumericFromTable( "NSSET" , "id" ,  "handle" , CORBA::string_dup(  handle[i] ) ) ) a[i]=0; // existuje
       else a[i] =1; // je volny
     }

      // comand OK
      ret->errCode=COMMAND_OK;

      // zapis na konec action
      ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;

  }

 PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::NSSetInfo(const char* handle, ccReg::NSSet_out n, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
Status status;
char sqlString[1024] ,  adres[1042] , adr[128] ;
ccReg::Response *ret;
int clid , crid , upid , nssetid;
int i , j  ,ilen , len , s ;

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

        status.Make(  PQsql.GetFieldValueName("status" , 0 ) ); 

        n->ROID=CORBA::string_dup( PQsql.GetFieldValueName("ROID" , 0 ) ); // ROID
        n->handle=CORBA::string_dup( PQsql.GetFieldValueName("handle" , 0 ) ); // ROID
        n->CrDate= get_time_t( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
        n->UpDate= get_time_t( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
        n->TrDate= get_time_t(PQsql.GetFieldValueName("TrDate" , 0 ) );  // datum a cas transferu

        n->AuthInfoPw = CORBA::string_dup( PQsql.GetFieldValueName("AuthInfoPw" , 0 ) ); // autentifikace

        ret->errCode=COMMAND_OK;


        // free select
        PQsql.FreeSelect();



        // zpracuj pole statusu
        len =  status.Length();
        n->stat.length(len);
        cout << " status length: "  << len  << endl ;
        for( i = 0 ; i <  len  ; i ++)
           {
              n->stat[i] = CORBA::string_dup( PQsql.GetStatusString(  status.Get(i)  ) );
           }


        n->ClID =  CORBA::string_dup( PQsql.GetRegistrarHandle( clid ) );
        n->CrID =  CORBA::string_dup( PQsql.GetRegistrarHandle( upid ) );
        n->UpID =  CORBA::string_dup( PQsql.GetRegistrarHandle( crid ) );

        // dotaz na DNS servry  na tabulky host
        sprintf( sqlString , "SELECT  fqdn , ipaddr FROM HOST   WHERE  nssetid=%d;" , nssetid);


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
n->stat.length(0); // status sequence
n->CrDate=0; // datum vytvoreni
n->UpDate=0; // datum zmeny
n->TrDate=0;
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
Status status;
int regID , id , clID = 0;
bool stat;

ret = new ccReg::Response;


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

 if( PQsql.BeginAction( clientID , EPP_NSsetDelete , (char * ) clTRID  ) )
 {
  if(  PQsql.BeginTransaction() )  // zahajeni transakce
  {

 // pokud NSSET existuje 
   if(  (id =  PQsql.GetNumericFromTable( "NSSET" , "id" ,  "handle" ,  (char * )  handle) )   )
   {
   // get  registrator ID
   regID = PQsql.GetLoginRegistrarID( clientID );
   clID =  PQsql.GetNumericFromTable( "NSSET" ,"ClID" , "id" , id );

    // zpracuj  pole statusu
   status.Make( PQsql.GetStatusFromTable( "NSSET" , id ) );

   if( status.Test( STATUS_clientDeleteProhibited ) || status.Test( STATUS_serverDeleteProhibited )  ) stat = false;
   else stat = true; // status je OK


   if( clID == regID ) // pokud je client registaratorem
     {
       //  uloz do historie
       if( PQsql.MakeHistory() )
         {
          if( PQsql.SaveHistory( "nsset_contact_map" , "nssetid" , id ) ) // historie tech kontakty
            {
               // na zacatku vymaz technicke kontakty
	       if(  PQsql.DeleteFromTable( "nsset_contact_map" , "nssetid" , id  ) )
	         {
                   
                   if( PQsql.SaveHistory( "HOST" , "nssetid" , id ) )
                     {  
   	               // vymaz nejdrive podrizene hosty
        	      if(  PQsql.DeleteFromTable( "HOST" ,  "nssetid" , id  ) )
                        {
                           if( PQsql.SaveHistory( "NSSET" , "id" , id ) )
                             {     
	                       // vymaz NSSET nakonec
          	              if(  PQsql.DeleteFromTable( "NSSET" , "id" , id ) ) ret->errCode =COMMAND_OK ; // pokud vse uspesne proslo
                             }
	                }
	             }
                 }
            }
         } // konec historie

     }
   }
   else ret->errCode =COMMAND_OBJECT_NOT_EXIST; 

   

   // pokud vse proslo 
   if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();   // pokud uspesne nainsertovalo commitni zmeny
   else PQsql.RollbackTransaction(); // pokud nejake chyba zrus trasakci
   } 
 
 
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
  }

 
PQsql.Disconnect();
}

return ret;
}


ccReg::Response* ccReg_EPP_i::NSSetCreate(const char* handle, const char* authInfoPw, const ccReg::TechContact& tech, const ccReg::DNSHost& dns, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024];
char Array[512];
ccReg::Response *ret;
int regID ,  id , techid;
int i , len , j ;
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
 if(  PQsql.GetNumericFromTable( "NSSET" , "id",   "handle" , (char * )  handle ) )  ret->errCode=COMMAND_OBJECT_EXIST; // nsset uz zalozen ID existuje
 else  // pokud nexistuje 
 if( PQsql.BeginTransaction() )  // zahaj transakci
   { 
     // get  registrator ID
     regID = PQsql.GetLoginRegistrarID( clientID );



     // ID je cislo ze sequence
     id =  PQsql.GetSequenceID( "nsset" );

        sprintf( sqlString , "INSERT INTO NSSET ( id , crdate ,  handle , ClID , CrID,  authinfopw  ) \
                           VALUES ( %d ,  'now'  , \'%s\' ,  %d , %d ,    \'%s\'  );" , 
                           id ,   CORBA::string_dup(handle) , regID , regID  ,  CORBA::string_dup(authInfoPw) );



    // zapis nejdrive nsset 
    if( PQsql.ExecSQL( sqlString ) )
    {

       ret->errCode = COMMAND_OK; // nastavit uspesne    

      // zapis technicke kontakty 
      len = tech.length();
      for( i = 0 ; i < len ; i ++ )
         {
             techid =  PQsql.GetNumericFromTable( "Contact" , "id" ,  "handle" , CORBA::string_dup(tech[i]) );              
             sprintf( sqlString , "INSERT INTO nsset_contact_map VALUES ( %d , %d );"  , id , techid );

              if( PQsql.ExecSQL( sqlString ) == false )  // pokud se nepodarilo pridat do tabulky
                {
                    ret->errCode = COMMAND_FAILED;
                    break;
                }
         } 
     
       // zapis do tabulky hostu
      len = dns.length();
      if( ret->errCode ==  COMMAND_OK ) // pokud se predchozi tabulka uspesne insertovala    
      { 
        for( i = 0 ; i < len ; i ++ )
          {
           // ip adresa DNS hostu


           // preved sequenci adres
           strcat( Array , " { " );
           for( j = 0 ; j < dns[i].inet.length() ; j ++ )
           {
              if( j > 0 ) strcat( Array , " , " );
              strcat( Array ,  CORBA::string_dup( dns[i].inet[j]  ));
            }

            strcat( Array , " } " );

             // HOST informace poouze ipaddr a fqdn 
             sprintf( sqlString , "INSERT INTO HOST ( nssetid , fqdn  , ipaddr )    VALUES ( %d , \'%s\'  ,  \'%s\' );" ,
                                           id , CORBA::string_dup( dns[i].fqdn )  , Array );


              if( PQsql.ExecSQL( sqlString ) == false )  // pokud se nepodarilo pridat do tabulky
                {
                    ret->errCode = COMMAND_FAILED;
                    break;
                }

           } 
       }
 

    }
       // pokud vse proslo
       if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();   // pokud uspesne nainsertovalo commitni zmeny
       else PQsql.RollbackTransaction(); // pokud nejake chyba zrus trasakci
   }

 
 

   // zapis na konec action
  ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;
 }
PQsql.Disconnect();
}


return ret;
}



ccReg::Response* ccReg_EPP_i::NSSetUpdate(const char* handle , const char* authInfo_chg, const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem, const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
Status status;
bool  stat;
char sqlString[4096] , buf[256] , Array[512] ,  statusString[128] ;
int regID=0 , clID=0 , id ,nssetid , contactid , techid ;
int i , j ,  len  , slen , hostID;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota

 // zmena DNS HOSTU
/*
 len = dns_chg.length();
 for( i = 0 ; i < len ; i ++ )
   {
      cout << "fqdn_chg: " <<  CORBA::string_dup( dns_chg[i].fqdn )  << endl;
      slen = dns_chg[i].inet.length();
      for( j = 0 ; j < slen ; j ++ )  cout << "\tadres: " <<  dns_chg[i].inet[j] << endl;  
   }
 
*/

if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_NSsetUpdate , (char * ) clTRID  ) )
 {

   // pokud domena existuje
  if( (id = PQsql.GetNumericFromTable(  "NSSET"  , "id" , "handle" , (char * ) handle ) ) == 0 ) ret->errCode= COMMAND_OBJECT_NOT_EXIST;
  else
  if( PQsql.BeginTransaction() )  
  {
   // get  registrator ID
   regID =   PQsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  PQsql.GetNumericFromTable(  "NSSET"  , "clID" , "id" , id );

  // zpracuj  pole statusu
  status.Make( PQsql.GetStatusFromTable( "NSSET" , id ) );

   if( status.Test( STATUS_clientUpdateProhibited ) || status.Test( STATUS_serverUpdateProhibited )  ) stat = false;
   else stat = true; // status je OK

   if( clID == regID   && stat ) // pokud je registrator clientem kontaktu a status je v poradku
     {
         //  uloz do historie
         if( PQsql.MakeHistory() )
           {
            if( PQsql.SaveHistory( "NSSET" , "id" , id ) ) // uloz zaznam
              {

                // pridany status
                len  =   status_add.length();
                for( i = 0 ; i < len ; i ++) status.Add(  PQsql.GetStatusNumber( CORBA::string_dup( status_add[i] ) )  );


                // zruseny status flagy
                len  =   status_rem.length();
                for( i = 0 ; i < len ; i ++) status.Rem( PQsql.GetStatusNumber( CORBA::string_dup( status_rem[i] ) ));


                //  vygeneruj  novy status string array
                status.Array( statusString );



                // zmenit zaznam o domene
                sprintf( sqlString , "UPDATE NSSET SET UpDate=\'now\' , upid=%d , status=\'%s\' " , regID , statusString   );

                // zmena autentifikace   
                add_field_value( sqlString , "AuthInfoPw" ,  CORBA::string_dup(authInfo_chg)  ) ;

                sprintf( buf , " WHERE id=%d;" , id );
                strcat( sqlString , buf ); 


                   if(   PQsql.ExecSQL( sqlString ) )
                     {  
                       ret->errCode = COMMAND_OK; // nastavit uspesne
        
                        if( PQsql.SaveHistory( "nsset_contact_map" , "nssetID" , id ) ) // uloz do historie tech kontakty
                        {
                         // pridat tech kontakty
                         len = tech_add.length(); 
                         for( i = 0 ; i < len ; i ++ )
                          { 
                            techid =  PQsql.GetNumericFromTable( "Contact" , "id" , "handle" , CORBA::string_dup(tech_add[i]) );
                             cout << "add techid: "  << techid <<  CORBA::string_dup(tech_add[i]) << endl;
                             if( techid )
                               {
                                  if(  PQsql.CheckContactMap( "nsset" , id , techid ) == false ) // pokud kontak jeste neexistuje tak ho pridej
                                    {
                                      sprintf( sqlString , "INSERT INTO nsset_contact_map VALUES ( %d , %d );"  , id , techid );
                                      if(   PQsql.ExecSQL( sqlString ) == false ) { ret->errCode=COMMAND_FAILED; break; } 
                                    }
                               }

                           }

                         // vymaz  tech kontakty
                        len = tech_rem.length();   
                        for( i = 0 ; i < len ; i ++ )
                          {

                             techid =  PQsql.GetNumericFromTable( "Contact" , "id" , "handle" , CORBA::string_dup(tech_rem[i]) );
                             cout << "rem techid: "  << techid <<  CORBA::string_dup(tech_rem[i]) << endl;
           
                             if( techid )
                              {  
                                if(  PQsql.CheckContactMap( "nsset" , id , techid ) )
                                  { 
                                    sprintf( sqlString , "DELETE FROM domain_contact_map WHERE  domainid=%d and contactid=%d;" , id , techid );
                                    if(   PQsql.ExecSQL( sqlString ) == false ) { ret->errCode=COMMAND_FAILED; break; }
                                  }
                              
                              }
                          }
 
                        }
     
                      if( PQsql.SaveHistory( "host" , "nssetID" , id ) ) // uloz do historie hosty
                       {
                            // zmena DNS HOSTU     
                          /*
                            len = dns_chg.length();
                            for( i = 0 ; i < len ; i ++ )
                            {
                               // zjisti host.id pro zmenu 
                               sprintf( sqlString , "SELECT id FROM HOST WHERE nssetid=%d AND fqdn=\'%s\';" , id ,  CORBA::string_dup( dns_chg[i].fqdn )  );
                               if( PQsql.ExecSelect( sqlString ) )                    
                                 {
                                   hostID = atoi( PQsql.GetFieldValue( 0 , 0 ) );  
                                   PQsql.FreeSelect(); 
                                 }

                               cout << "hostID " << hostID << endl ;   
                               if( hostID ) // pokud nasel zmen jenom glue ipadresu hosta
                                 {
                                    // vytvor pole inet adres
                                    slen =  dns_chg[i].inet.length();
                                    strcpy(  Array , "{ " ); 
                                    for( j = 0 ; j  < slen; j ++ )
                                       {
                                          if( j > 0 ) strcat( Array , " , " );
                                          strcat( Array ,  CORBA::string_dup( dns_chg[i].inet[j] ) );
                                       }
                                    strcat( Array , " } " );
                                    sprintf( sqlString , "UPDATE HOST SET  ipaddr = \'%s\' WHERE id=%d;" , Array  , hostID );   
                                    if(   PQsql.ExecSQL( sqlString ) == false ) { ret->errCode=COMMAND_FAILED; break; }                             
                                 }  

                              }
                            */
                            // pridat DNS HOSTY
                            len = dns_add.length();
                            for( i = 0 ; i < len ; i ++ )
                               {
                                                                    
                                    // vytvor pole inet adres
                                    slen =  dns_add[i].inet.length() ;
                                    strcpy(  Array , "{ " );
                                    for( j = 0 ; j  < slen; j ++ )
                                       {
                                          if( j > 0 ) strcat( Array , " , " );
                                          strcat( Array ,  CORBA::string_dup( dns_add[i].inet[j] ) );
                                       }
                                    strcat( Array , " } " ) ;

              
                                    sprintf( sqlString , "INSERT INTO  HOST ( nssetid , fqdn  , ipaddr ) VALUES ( %d , \'%s\', \'%s\' );", 
                                                 id ,   CORBA::string_dup( dns_add[i].fqdn ) , Array );
                                    if(  PQsql.ExecSQL( sqlString ) == false ) { ret->errCode == COMMAND_FAILED; break ;}
                               
                               }

                            // smazat DNS HOSTY
                            len = dns_rem.length();
                            for( i = 0 ; i < len ; i ++ )
                               {
                                 sprintf( sqlString , "DELETE FROM HOST WHERE nssetid=%d AND fqdn=\'%s\';" , id ,  CORBA::string_dup(   dns_rem[i].fqdn ) );   
                                 if(  PQsql.ExecSQL( sqlString ) == false ) { ret->errCode == COMMAND_FAILED; break ;}   
                               }

                       }


                     }

                       // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
                      if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
                      else  PQsql.RollbackTransaction();

                  
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

ccReg::Response* ccReg_EPP_i::NSSetTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
char sqlString[1024];
char *pass;
bool auth;
int regID=0 , clID=0 , id , contactid;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota


if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_NSsetTransfer , (char * ) clTRID  ) )
 {

   // pokud domena existuje
  if( (id = PQsql.GetNumericFromTable(  "NSSET"  , "id" , "handle" , (char * ) handle ) ) == 0 ) ret->errCode= COMMAND_OBJECT_NOT_EXIST;
  else
  if( PQsql.BeginTransaction() )  
  {
   // get  registrator ID
   regID =   PQsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  PQsql.GetNumericFromTable(  "NSSET"  , "clID" , "id" , id );

   
   pass = PQsql.GetValueFromTable(  "NSSET"  , "authinfopw" , "id" , id ); // ulozene heslo
   // autentifikace
   if( strlen(  CORBA::string_dup(authInfo) )  )  
     {
        if( strcmp( pass ,  CORBA::string_dup(authInfo) ) == 0 ) auth = true; // OK
        else auth = false; // neplatne heslo  
     }
    else auth = false; //  autentifikace je nitna

   if(  auth  ) // pokud prosla autentifikace 
     {
         //  uloz do historie
       if( PQsql.MakeHistory() )
        {
          if( PQsql.SaveHistory( "NSSET" , "id" , id ) ) // uloz zaznam
           { 
                      // zmena registratora
                      sprintf( sqlString , "UPDATE NSSET SET TrDate=\'now\' , clid=%d  WHERE id=%d;" , regID , id );
                      if(   PQsql.ExecSQL( sqlString ) )  ret->errCode = COMMAND_OK; // nastavit OK                                  
           }
       }

    }
  
   // pokud nebyla chyba pri insertovani do tabulky 
   if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
   else PQsql.RollbackTransaction(); 
   }
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
}


PQsql.Disconnect();
}

return ret;
}


ccReg::Response*  ccReg_EPP_i::DomainCheck(const ccReg::Check& fqdn, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
ccReg::Response *ret;
int  len , av ;
long unsigned int i;
ret = new ccReg::Response;

a = new ccReg::Avail;

len = fqdn.length();
a->length(len);

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota

if( PQsql.OpenDatabase( DATABASE ) )
{

  if( PQsql.BeginAction( clientID , EPP_DomainCheck , (char * ) clTRID  ) )
  {

    for( i = 0 ; i < len ; i ++ )
     {
       if( PQsql.GetNumericFromTable( "DOMAIN" , "id" , "fqdn" ,   CORBA::string_dup(fqdn[i]) )  ) a[i]=0; // existuje
       else a[i] =1; // neexistuje domena je volna
     }

      // comand OK
      ret->errCode=COMMAND_OK;

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
Status status;
char sqlString[1024];
ccReg::Response *ret;
int id , clid , crid ,  upid , regid ,nssetid;
int i , len ;

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
        id = atoi( PQsql.GetFieldValueName("id" , 0 ) );
        clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) ); 
        crid = atoi( PQsql.GetFieldValueName("CrID" , 0 ) ); 
        upid = atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 
        regid = atoi( PQsql.GetFieldValueName("registrant" , 0 ) ); 
        nssetid = atoi( PQsql.GetFieldValueName("nsset" , 0 ) );  

        status.Make(  PQsql.GetFieldValueName("status" , 0 ) ) ; // status

	d->CrDate= get_time_t( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
	d->UpDate= get_time_t( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
	d->TrDate= get_time_t( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas transferu
	d->ExDate= get_time_t( PQsql.GetFieldValueName("ExDate" , 0 ) ); //  datum a cas expirace

	d->ROID=CORBA::string_dup( PQsql.GetFieldValueName("roid" , 0 )  ); // jmeno nebo nazev kontaktu
	d->name=CORBA::string_dup( PQsql.GetFieldValueName("fqdn" , 0 )  ); // jmeno nebo nazev kontaktu


    
        ret->errCode=COMMAND_OK;

    
        // free select
	PQsql.FreeSelect();
        
        // zpracuj pole statusu
        len =  status.Length();
        d->stat.length(len);
        cout << " status length: "  << len  << endl ; 
        for( i = 0 ; i < len ; i ++)
           {
              d->stat[i] = CORBA::string_dup( PQsql.GetStatusString(  status.Get(i)  ) );
           }


        d->ClID = CORBA::string_dup( PQsql.GetRegistrarHandle( clid ) );
        d->CrID = CORBA::string_dup( PQsql.GetRegistrarHandle( crid ) );
        d->UpID = CORBA::string_dup( PQsql.GetRegistrarHandle( upid ) );

        // vlastnik domeny
        d->Registrant=CORBA::string_dup( PQsql.GetValueFromTable( "CONTACT" , "handle" , "id" , regid ) );


        //  handle na nsset
        d->nsset=CORBA::string_dup( PQsql.GetValueFromTable( "NSSET" , "handle", "id" , nssetid ) );
    
        d->admin.length(0); // zadne admin kontakty       

        // dotaz na admin kontakty
        sprintf( sqlString , "SELECT  handle FROM CONTACT  JOIN  domain_contact_map ON domain_contact_map.contactid=contact.id WHERE domain_contact_map.domainid=%d;" ,  id );       

        if( PQsql.ExecSelect( sqlString ) )
          {
               len =  PQsql.GetSelectRows(); // pocet technickych kontaktu
               d->admin.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) 
                 {
                   d->admin[i] = CORBA::string_dup( PQsql.GetFieldValue( i , 0 )  );
                 }
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
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) );
   cout << "svTRID " << ret->svTRID << endl;

 }
 PQsql.Disconnect();
}


// pokud neneslo kontakt
if( ret->errCode !=  COMMAND_OK )
{
// vyprazdni
d->ROID =  CORBA::string_dup( "" ); // domena do ktere patri host
d->name=  CORBA::string_dup( "" ); // fqdn nazev domeny
d->nsset = CORBA::string_dup( "" ); // nsset
d->stat.length(0); // status sequence
d->CrDate=0; // datum vytvoreni
d->TrDate=0; // datum zmeny
d->ExDate=0; // datum zmeny
d->Registrant=CORBA::string_dup( "" ); 
d->ClID=  CORBA::string_dup( "" );    // identifikator registratora ktery vytvoril host
d->UpID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam
d->CrID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam
}


return ret;
}

ccReg::Response* ccReg_EPP_i::DomainDelete(const char* fqdn , CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
Status status;
char sqlString[1024];
int regID , clID , id;
bool stat;
ret = new ccReg::Response;


// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainDelete , (char * ) clTRID  ) )
 {

 if( PQsql.BeginTransaction() )
 {
  if( ( id =  PQsql.GetNumericFromTable( "DOMAIN" , "id" ,  "fqdn" , (char * )  fqdn) ) )
  {

   regID = PQsql.GetLoginRegistrarID( clientID ); // aktivni registrator
   clID =  PQsql.GetNumericFromTable( "DOMAIN" , "ClID" , "id" , id );  // client objektu

    // zpracuj  pole statusu
   status.Make( PQsql.GetStatusFromTable( "DOMAIN" , id ) );

   if( status.Test( STATUS_clientDeleteProhibited ) || status.Test( STATUS_serverDeleteProhibited )  ) stat = false;
   else stat = true; // status je OK

   if( regID == clID && stat == true ) // pokud je registrator klientem a status je OK      
     {
      //  uloz do historie
       if( PQsql.MakeHistory() )
         {
            if( PQsql.SaveHistory( "domain_contact_map" , "domainID" , id ) )
               { 
                    if(  PQsql.DeleteFromTable( "domain_contact_map" , "domainID" , id )  )
                      { 
                          if( PQsql.SaveHistory(  "DOMAIN" , "id" , id  ) )
                            { 
                              if(  PQsql.DeleteFromTable( "DOMAIN" , "id" , id  ) )  ret->errCode =COMMAND_OK ; // pokud usmesne smazal
                            }
                      }
               }
         }
     }


   }
  else  ret->errCode =COMMAND_OBJECT_NOT_EXIST;

   // pokud vse proslo
 if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();   // pokud uspesne nainsertovalo commitni zmeny
 else PQsql.RollbackTransaction(); // pokud nejake chyba zrus trasakci
 }


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }

PQsql.Disconnect();
}

return ret;
}

ccReg::Response* ccReg_EPP_i::DomainUpdate(const char* fqdn, const char* registrant_chg , const char* authInfo_chg, const char* nsset_chg, const ccReg::AdminContact& admin_add, const ccReg::AdminContact& admin_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
Status status;
bool stat;
char sqlString[4096] , buf[256] , statusString[128] ;
int regID=0 , clID=0 , id ,nssetid , contactid , adminid ;
int i , len , slen , j  ;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota


if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainUpdate , (char * ) clTRID  ) )
 {

   // pokud domena existuje
  if( (id = PQsql.GetNumericFromTable(  "DOMAIN"  , "id" , "fqdn" , (char * ) fqdn ) ) == 0 ) ret->errCode= COMMAND_OBJECT_NOT_EXIST;
  else
  if( PQsql.BeginTransaction() )  
  {
   // get  registrator ID
   regID =   PQsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  PQsql.GetNumericFromTable(  "DOMAIN"  , "clID" , "id" , id );

  // zpracuj  pole statusu
  status.Make( PQsql.GetStatusFromTable( "DOMAIN" , id ) );

   if( status.Test( STATUS_clientUpdateProhibited ) || status.Test( STATUS_serverUpdateProhibited )  ) stat = false;
   else stat = true; // status je OK



   if( clID == regID  && stat ) // pokud je registrator clientem kontaktu a probehla autentifikace a vyhovuje status flagy
     {
         //  uloz do historie
       if( PQsql.MakeHistory() )
        {
          if( PQsql.SaveHistory( "Domain" , "id" , id ) ) // uloz zaznam
           {

                
                nssetid =  PQsql.GetNumericFromTable("NSSET" , "id" , "handle" , CORBA::string_dup(nsset_chg) );
                contactid =  PQsql.GetNumericFromTable("CONTACT" , "id" , "handle", CORBA::string_dup(registrant_chg) );

                // pridany status
                len  =   status_add.length();
                for( i = 0 ; i < len ; i ++) status.Add(  PQsql.GetStatusNumber( CORBA::string_dup( status_add[i] ) )  );


                // zruseny status flagy
                len  =   status_rem.length();
                for( i = 0 ; i < len ; i ++) status.Rem( PQsql.GetStatusNumber( CORBA::string_dup( status_rem[i] ) ));


                //  vygeneruj  novy status string array
                status.Array( statusString );
 

                // zmenit zaznam o domene
                sprintf( sqlString , "UPDATE DOMAIN SET UpDate=\'now\' , upid=%d , status=\'%s\' " , regID , statusString   );
                // zmena nssetu
                if( nssetid ) { sprintf( buf , " ,  nsset=%d " , nssetid ); strcat( sqlString , buf ); } 
                // zmena drzitele domeny 
                if( contactid ) { sprintf( buf , " ,  registrant=%d " , contactid); strcat( sqlString , buf ); } 
                // zmena autentifikace
                add_field_value( sqlString , "AuthInfoPw" ,  CORBA::string_dup(authInfo_chg)  ) ;

 
                sprintf( buf , " WHERE id=%d;" , id );
                strcat( sqlString , buf ); 


                   if(   PQsql.ExecSQL( sqlString ) )
                     {  
                       ret->errCode = COMMAND_OK; // nastavit uspesne
        
                      if( PQsql.SaveHistory( "domain_contact_map" , "domainID" , id ) ) // uloz do historie admin kontakty
                      {
                       // pridat admin kontakty
                      len = admin_add.length(); 
                       for( i = 0 ; i < len ; i ++ )
                          { 
                                   adminid =  PQsql.GetNumericFromTable( "Contact" , "id" , "handle" , CORBA::string_dup(admin_add[i]) );
                                   sprintf( sqlString , "INSERT INTO domain_contact_map VALUES ( %d , %d );"  , id , adminid );
                           }

                     // vymaz admin kontakty
                       len = admin_rem.length();   
                       for( i = 0 ; i < len ; i ++ )
                          {
                                   adminid =  PQsql.GetNumericFromTable( "Contact" , "id" , "handle" , CORBA::string_dup(admin_rem[i]) );
                                   sprintf( sqlString , "DELETE FROM domain_contact_map WHERE  domainid=%d and contactid=%d;" , id , adminid );
                                    if(   PQsql.ExecSQL( sqlString ) == false ) { ret->errCode=COMMAND_FAILED; break; } ;
                          }
 
                        }     
                     }

                      // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
                      if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
                      else  PQsql.RollbackTransaction();

                  
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

ccReg::Response* ccReg_EPP_i::DomainCreate(const char* fqdn, const char* Registrant, const char* nsset, const char* AuthInfoPw , CORBA::Short period , const ccReg::AdminContact& admin, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
char sqlString[1024] , exDate[32];
ccReg::Response *ret;
int contactid , regID , nssetid , adminid , id;
int i , len , s ;
time_t t;

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
 if(  PQsql.GetNumericFromTable("DOMAIN" , "id" ,  "fqdn" , (char * ) fqdn ) ) ret->errCode=COMMAND_OBJECT_EXIST; // je uz zalozena
 else
// pokud domena nexistuje
 {
  // zahaj transakci
  if( PQsql.BeginTransaction() )
  {
   id =  PQsql.GetSequenceID( "domain" ); // id domeny

   // get  registrator ID
   regID =   PQsql.GetLoginRegistrarID( clientID);

   nssetid =  PQsql.GetNumericFromTable("NSSET" , "id" , "handle" , CORBA::string_dup(nsset) );
   contactid =  PQsql.GetNumericFromTable("CONTACT" , "id" , "handle", CORBA::string_dup(Registrant) );


   if( period )
   {
   t = time(NULL);
   // preved datum a cas expirace prodluz tim cas platnosti domeny
   get_timestamp( expiry_time( t  , period ) ,  exDate );
   } 
   
   sprintf( sqlString , "INSERT INTO DOMAIN ( zone , crdate , update , id , roid , fqdn , ClID , CrID,  Registrant  , exdate , authinfopw , nsset ) \
              VALUES ( 3 , 'now()' , 'now()' ,  %d ,   \'%s\' , \'%s\'  ,  %d , %d  , %d ,  \'%s\' ,  \'%s\'  , %d );" , 
              id,  CORBA::string_dup(fqdn) ,  CORBA::string_dup(fqdn) ,  regID  ,  regID , contactid , exDate ,  CORBA::string_dup(AuthInfoPw),   
nssetid );
 


   // pokud se insertovalo do tabulky
        if(    PQsql.ExecSQL( sqlString ) )
         {
          ret->errCode = COMMAND_OK; // nastavit uspesne
          // zapis admin kontakty
          len = admin.length();
          for( i = 0 ; i < len ; i ++ )
            {
              adminid =  PQsql.GetNumericFromTable( "Contact" , "id" , "handle" , CORBA::string_dup(admin[i]) );

              sprintf( sqlString , "INSERT INTO domain_contact_map VALUES ( %d , %d );"  , id , adminid );

              if( PQsql.ExecSQL( sqlString ) == false )  // pokud se nepodarilo pridat do tabulky
                {
	            ret->errCode = COMMAND_FAILED;                     
                    break;
                }
            }

        } 

       // pokud nebyla chyba pri insertovani do tabulky domain_contact_map 
       if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
       else PQsql.RollbackTransaction();   
  }
 }


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }

PQsql.Disconnect();
}


return ret;
}



ccReg::Response*  ccReg_EPP_i::DomainRenew(const char* fqdn, ccReg::timestamp curExpDate,  CORBA::Short period, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
Status status;
char sqlString[4096];
char exDate[24];
ccReg::Response *ret;
int clid , regID , id ;
bool stat;
int i , len;
time_t ex=0 ;
ret = new ccReg::Response;


// default


// default
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota



if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainRenew , (char * ) clTRID  ) )
 {
 
  regID = PQsql.GetLoginRegistrarID( clientID ); // aktivni registrator

// prvni test zdali domena uz existuje
 if(  ( id = PQsql.GetNumericFromTable("DOMAIN" , "id" ,  "fqdn" , (char * ) fqdn )  ) )
  {
      // zahaj transakci
      if( PQsql.BeginTransaction() )
        {
 
           sprintf( sqlString , "SELECT * FROM DOMAIN WHERE id=\'%d\'" , id);

           if( PQsql.ExecSelect( sqlString ) )
             {

                 clid = atoi( PQsql.GetFieldValueName("ClID" , 0 ) );
                 ex = get_time_t( PQsql.GetFieldValueName("ExDate" , 0 ) ); // datum a cas  expirace domeny
                 PQsql.FreeSelect();
              }     

          // zpracuj  pole statusu
           status.Make( PQsql.GetStatusFromTable( "DOMAIN" , id ) );

           if( status.Test( STATUS_clientRenewProhibited ) || status.Test( STATUS_serverRenewProhibited )  ) stat = false;
           else stat = true; // status je OK
 


           if( clid == regID && ex == curExpDate  && stat )
             {

                  // preved datum a cas expirace prodluz tim cas platnosti domeny
                  get_timestamp( expiry_time( ex  , period ) ,  exDate );
                  sprintf( sqlString , "UPDATE DOMAIN SET ExDate=\'%s\' WHERE id=%d;" , exDate , id );
                  if(   PQsql.ExecSQL( sqlString ) ) ret->errCode = COMMAND_OK;
             }             
        
 

       // pokud nebyla chyba pri insertovani do tabulky domain_contact_map 
       if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
       else PQsql.RollbackTransaction();   
     }

 
 } 
 else ret->errCode == COMMAND_OBJECT_NOT_EXIST; // domena neexistujea


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
}

PQsql.Disconnect();
}


return ret;
}
 

ccReg::Response* ccReg_EPP_i::DomainTransfer(const char* fqdn, const char* registrant, const char* authInfo, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
Status status;
char sqlString[1024];
char *pass;
bool auth , stat ;
int regID=0 , clID=0 , id , registrantid , contactid;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota


if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainTransfer , (char * ) clTRID  ) )
 {

   // pokud domena existuje
  if( (id = PQsql.GetNumericFromTable(  "DOMAIN"  , "id" , "fqdn" , (char * ) fqdn ) ) == 0 ) ret->errCode= COMMAND_OBJECT_NOT_EXIST;
  else
  if( PQsql.BeginTransaction() )  
  {
   // get  registrator ID
   regID =   PQsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  PQsql.GetNumericFromTable(  "DOMAIN"  , "clID" , "id" , id );

   // drzitel domeny
   registrantid = PQsql.GetNumericFromTable(  "DOMAIN"  , "registrant" , "id" , id );

  // drzitel zadajici prevod
   contactid =  PQsql.GetNumericFromTable("CONTACT" , "id" , "handle", CORBA::string_dup(registrant) );

    // zpracuj  pole statusu
    status.Make( PQsql.GetStatusFromTable( "DOMAIN" , id ) );

    if( status.Test( STATUS_clientTransferProhibited ) || status.Test( STATUS_serverTransferProhibited )  ) stat = false;
    else stat = true; // status je OK

   
   pass = PQsql.GetValueFromTable(  "DOMAIN"  , "authinfopw" , "id" , id ); // ulozene heslo
   // autentifikace
   if( strlen(  CORBA::string_dup(authInfo) )  )  
     {
        if( strcmp( pass ,  CORBA::string_dup(authInfo) ) == 0 ) auth = true; // OK
        else auth = false; // neplatne heslo  
     }
    else auth = false; //  autentifikace je nitna

   if(  auth && registrantid == contactid && stat ) // pokud prosla autentifikace  a je drzitelem domeny a status OK 
     {
         //  uloz do historie
       if( PQsql.MakeHistory() )
        {
          if( PQsql.SaveHistory( "Domain" , "id" , id ) ) // uloz zaznam
           { 
                      // zmena registratora
                      sprintf( sqlString , "UPDATE DOMAIN SET TrDate=\'now\' ,  clid=%d  WHERE id=%d;" , regID , id );
                      if(   PQsql.ExecSQL( sqlString ) )  ret->errCode = COMMAND_OK; // nastavit OK                                  
           }
       }

    }
  
   // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
   if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
   else PQsql.RollbackTransaction(); 
   }
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
}


PQsql.Disconnect();
}

return ret;
}


ccReg::Response* ccReg_EPP_i::DomainTrade(const char* fqdn, const char* old_registrant, const char* new_registrant, const char* authInfo, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
char sqlString[1024];
char *pass;
bool auth;
int regID=0 , clID=0 , id , newid , oldid   , registrantid ;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota


if( PQsql.OpenDatabase( DATABASE ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainTrade , (char * ) clTRID  ) )
 {

   // pokud domena existuje
  if( (id = PQsql.GetNumericFromTable(  "DOMAIN"  , "id" , "fqdn" , (char * ) fqdn ) ) == 0 ) ret->errCode= COMMAND_OBJECT_NOT_EXIST;
  else
  if( PQsql.BeginTransaction() )  
  {
   // get  registrator ID
   regID =   PQsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  PQsql.GetNumericFromTable(  "DOMAIN"  , "clID" , "id" , id );

   // drzitel domeny
   registrantid = PQsql.GetNumericFromTable(  "DOMAIN"  , "registrant" , "id" , id );
  // stavajici drzitel                    
   oldid =  PQsql.GetNumericFromTable("CONTACT" , "id" , "handle", CORBA::string_dup(old_registrant) );

   pass = PQsql.GetValueFromTable(  "DOMAIN"  , "authinfopw" , "id" , id ); // ulozene heslo
   // autentifikace
   if( strlen(  CORBA::string_dup(authInfo) )  )  
     {
        if( strcmp( pass ,  CORBA::string_dup(authInfo) ) == 0 ) auth = true; // OK
        else auth = false; // neplatne heslo  
     }
    else auth = true; // neni pozadovana autentifikace

   if( clID == regID && oldid == registrantid  && auth ) // pokud je registrator clientem kontaktu a zaroven je soucasny drzitel drzitelem domeny
     {
         //  uloz do historie
       if( PQsql.MakeHistory() )
        {
          if( PQsql.SaveHistory( "Domain" , "id" , id ) ) // uloz zaznam
           {

                      // budouci drzitel
                      newid =  PQsql.GetNumericFromTable("CONTACT" , "id" , "handle", CORBA::string_dup(new_registrant) );

                      // zmenit zaznam o domene
                      sprintf( sqlString , "UPDATE DOMAIN SET UpDate=\'now\' , upid=%d ,  registrant=%d  WHERE id=%d;" , regID , newid  , id );
                      if(   PQsql.ExecSQL( sqlString ) ) ret->errCode = COMMAND_OK; // nastavit uspesne                                 
           }
       }

    }

   // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
   if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
   else PQsql.RollbackTransaction();
   
   }
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
}


PQsql.Disconnect();
}

return ret;
}



