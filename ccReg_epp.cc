// *INDENT-OFF*
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

// log
#include "log.h"
//
// Example implementational code for IDL interface ccReg::EPP
//
ccReg_EPP_i::ccReg_EPP_i(){
}
ccReg_EPP_i::~ccReg_EPP_i(){
}
// test spojeni na databazi
bool ccReg_EPP_i::TestDatabaseConnect(char *db)
{
PQ  PQsql;

// zkopiruj pri vytvoreni instance
strcpy( database , db ); // retezec na  pripojeni k Postgres SQL

if(  PQsql.OpenDatabase( database ) )
{
LOG( NOTICE_LOG ,  "succefuly connect to:  [%s]" , database );
PQsql.Disconnect();
return true;
}
else
{
LOG( ERROR_LOG , "can not connect to database: [%s]" , database );
return false;
}

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
 *          
 * RETURNED:    svTRID a errCode errMsg
 *
 ***********************************************************************/




ccReg::Response * ccReg_EPP_i::GetTransaction( CORBA::Long clientID, const char *clTRID, CORBA::Short errCode )
{
PQ PQsql;
ccReg::Response * ret;
ret = new ccReg::Response;

// default
ret->errCode = 0;

LOG( NOTICE_LOG, "GetTransaction: clientID -> %d clTRID [%s] ", clientID, clTRID );

  if( PQsql.OpenDatabase( database ) )
    {
      if( errCode > 0 )
        {
          if( PQsql.BeginAction( clientID, EPP_UnknowAction, ( char * ) clTRID ) )
            {
              // chybove hlaseni bere z clienta 
              ret->errCode = errCode;
              // zapis na konec action
              // zapis na konec action
              ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
              ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

              LOG( NOTICE_LOG, "GetTransaction: svTRID [%s] errCode -> %d msg [%s] ", ( char * ) ret->svTRID, ret->errCode, ( char * ) ret->errMsg );
            }
        }

      PQsql.Disconnect();
    }

  if( ret->errCode == 0 )
    {
      ret->errCode = 0;         // obecna chyba
      ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
      ret->errMsg = CORBA::string_dup( "" );
      ret->errors.length( 0 );
    }

return ret;
}

/***********************************************************************
 *
 * FUNCTION:    PollAcknowledgement
 *
 * DESCRIPTION: potvrezeni prijeti zpravy msgID 
 *              vraci pocet zbyvajicich zprav count 
 *              a dalsi zpravu newmsgID
 *
 * PARAMETERS:  msgID - cislo zpravy ve fronte
 *        OUT:  count -  pocet zprav
 *        OUT:  newmsgID - cislo nove zpravy
 *              clTRID - cislo transakce klienta
 *              clientID - identifikace klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response * ccReg_EPP_i::PollAcknowledgement( CORBA::Long msgID, CORBA::Short & count, CORBA::Long & newmsgID, CORBA::Long clientID, const char *clTRID )
{
PQ PQsql;
ccReg::Response * ret;
char sqlString[1024], str[64];
int regID, rows;

ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );
count = 0;
newmsgID = 0;

LOG( NOTICE_LOG, "PollAcknowledgement: clientID -> %d clTRID [%s] msgID -> %d", clientID, clTRID, msgID );

  if( PQsql.OpenDatabase( database ) )
    {

      // get  registrator ID
      regID = PQsql.GetLoginRegistrarID( clientID );

      if( PQsql.BeginAction( clientID, EPP_PollAcknowledgement, ( char * ) clTRID ) )
        {

          // test msg ID
          sprintf( sqlString, "SELECT * FROM MESSAGE WHERE id=%d;", msgID );
          rows = 0;
          if( PQsql.ExecSelect( sqlString ) )
            {
              rows = PQsql.GetSelectRows();
              if( rows == 0 )
                {
                  LOG( ERROR_LOG, "unknow msgID %d", msgID );
                  ret->errors.length( 1 );
                  ret->errCode = COMMAND_PARAMETR_ERROR;
                  ret->errors[0].code = ccReg::pollAck_msgID;   // spatna msg ID
                  sprintf( str, "%d", msgID );
                  ret->errors[0].value = CORBA::string_dup( str );
                  sprintf( str, "unknow msgID %d", msgID );
                  ret->errors[0].reason = CORBA::string_dup( str );
                }
              PQsql.FreeSelect();
            }
          else
            ret->errCode = COMMAND_FAILED;

          if( rows == 1 )       // pokud tam ta zprava existuje
            {
              // oznac zpravu jako prectenou  
              sprintf( sqlString, "UPDATE MESSAGE SET seen='t' WHERE id=%d AND clID=%d;", msgID, regID );

              if( PQsql.ExecSQL( sqlString ) )
                {
                  // zjisteni dalsi ID zpravy ve fronte
                  sprintf( sqlString, "SELECT id  FROM MESSAGE  WHERE clID=%d AND seen='f' AND exDate > 'now()' ;", regID );
                  if( PQsql.ExecSelect( sqlString ) )
                    {

                      rows = PQsql.GetSelectRows();   // pocet zprav
                      if( rows > 0 )    // pokud jsou nejake zpravy ve fronte
                        {
                          count = rows; // pocet dalsich zprav
                          newmsgID = atoi( PQsql.GetFieldValue( 0, 0 ) );
                          ret->errCode = COMMAND_ACK_MESG;      // zpravy jsou ve fronte
                          LOG( NOTICE_LOG, "PollAcknowledgement: newmsgID -> %d count -> %d", newmsgID, count );
                        }
                      else
                        ret->errCode = COMMAND_NO_MESG; // zadne zpravy ve fronte

                      PQsql.FreeSelect();
                    }
                  else
                    ret->errCode = COMMAND_FAILED;

                }
              else
                ret->errCode = COMMAND_FAILED;

            }
          else
            ret->errCode = COMMAND_FAILED;
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }
      else
        ret->errCode = COMMAND_FAILED;

      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }

  if( ret->errCode == 0 )
    {
      ret->errCode = COMMAND_FAILED;    // obecna chyba
      ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
      ret->errMsg = CORBA::string_dup( "" );
    }


  return ret;
}


/***********************************************************************
 *
 * FUNCTION:    PollAcknowledgement
 *
 * DESCRIPTION: ziskani zpravy msgID z fronty 
 *              vraci pocet zprav ve fronte a obsah zpravy        
 *
 * PARAMETERS: 
 *        OUT:  msgID - cislo zpozadovane pravy ve fronte
 *        OUT:  count -  pocet 
 *        OUT:  qDate - datum a cas zpravy
 *        OUT:  mesg  - obsah zpravy  
 *              clTRID - cislo transakce klienta
 *              clientID - identifikace klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::PollRequest( CORBA::Long & msgID, CORBA::Short & count, ccReg::timestamp & qDate, CORBA::String_out mesg, CORBA::Long clientID, const char *clTRID )
{
PQ PQsql;
char sqlString[1024];
ccReg::Response * ret;
int regID;
int rows;

ret = new ccReg::Response;


//vyprazdni
qDate = 0;
count = 0;
msgID = 0;
mesg = CORBA::string_dup( "" );       // prazdna hodnota

ret->errCode = 0;
ret->errors.length( 0 );


LOG( NOTICE_LOG, "PollRequest: clientID -> %d clTRID [%s]", clientID, clTRID, msgID );

  if( PQsql.OpenDatabase( database ) )
    {

      // get  registrator ID
      regID = PQsql.GetLoginRegistrarID( clientID );


      if( PQsql.BeginAction( clientID, EPP_PollAcknowledgement, ( char * ) clTRID ) )
        {

          // vypsani zprav z fronty
          sprintf( sqlString, "SELECT *  FROM MESSAGE  WHERE clID=%d AND seen='f' AND exDate > 'now()' ;", regID );

          if( PQsql.ExecSelect( sqlString ) )
            {
              rows = PQsql.GetSelectRows();   // pocet zprav 

              if( rows > 0 )    // pokud jsou nejake zpravy ve fronte
                {
                  count = rows;
                  qDate = get_time_t( PQsql.GetFieldValueName( "CrDate", 0 ) );
                  msgID = atoi( PQsql.GetFieldValueName( "ID", 0 ) );
                  mesg = CORBA::string_dup( PQsql.GetFieldValueName( "message", 0 ) );
                  ret->errCode = COMMAND_ACK_MESG;      // zpravy jsou ve fronte
                  LOG( NOTICE_LOG, "PollRequest: msgID -> %d count -> %d mesg [%s]", msgID, count, CORBA::string_dup( mesg ) );
                }
              else
                ret->errCode = COMMAND_NO_MESG; // zadne zpravy ve fronte

              PQsql.FreeSelect();
            }
          else
            ret->errCode = COMMAND_FAILED;

          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );

        }
      else
        ret->errCode = COMMAND_FAILED;


      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }

if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;    // obecna chyba
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

return ret;
}



/***********************************************************************
 *
 * FUNCTION:    ClientLogout
 *
 * DESCRIPTION: odhlaseni clienta za zapis do tabulky login
 *              o datu odhlaseni
 * PARAMETERS:  clientID - id pripojeneho klienta
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::ClientLogout( CORBA::Long clientID, const char *clTRID )
{
PQ PQsql;
ccReg::Response * ret;


ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "ClientLogout: clientID -> %d clTRID [%s]", clientID, clTRID );


  if( PQsql.OpenDatabase( database ) )
    {
      if( PQsql.BeginAction( clientID, EPP_ClientLogout, ( char * ) clTRID ) )
        {

           PQsql.UPDATE( "Login" );
           PQsql.SET( "logoutdate" , "now" );
           PQsql.SET( "logouttrid" , clTRID );
           PQsql.WHEREID( clientID );   


          if( PQsql.EXEC() )  ret->errCode = COMMAND_LOGOUT;      // uspesne oddlaseni
          else ret->errCode = COMMAND_FAILED;


          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }
      else
        ret->errCode = COMMAND_FAILED;


      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;    // obecna chyba
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

return ret;
}

/***********************************************************************
 *
 * FUNCTION:    ClientLogin
 *
 * DESCRIPTION: prihlaseni clienta ziskani clientID  z tabulky login
 *              prihlaseni pres heslo registratora a jeho mozna zmena
 *              
 * PARAMETERS:  ClID - identifikator registratora
 *              passwd - stavajici heslo 
 *              newpasswd - nove heslo pro zmenu
 *        OUT:  clientID - id pripojeneho klienta
 *              clTRID - cislo transakce klienta
 *              certID - fingerprint certifikatu 
 *              language - komunikacni jazyk klienta  en nebo cs prazdna hodnota = en
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response * ccReg_EPP_i::ClientLogin( const char *ClID, const char *passwd, const char *newpass, const char *clTRID, CORBA::Long & clientID, const char *certID, ccReg::Languages lang )
{
PQ PQsql;
int roid, id;
bool change=true;
ccReg::Response * ret;
ret = new ccReg::Response;

// default
ret->errCode = 0;
ret->errors.length( 0 );
clientID = 0;

LOG( NOTICE_LOG, "ClientLogin: username-> [%s] clTRID [%s] passwd [%s]  newpass [%s] ", ClID, clTRID, passwd, newpass );
LOG( NOTICE_LOG, "ClientLogin:  certID  [%s] language  [%d] ", certID, lang );


if( PQsql.OpenDatabase( database ) )
  {

    if( PQsql.BeginTransaction() )
      {

        // dotaz na ID registratora 
        if( ( roid = PQsql.GetNumericFromTable( "REGISTRAR", "id", "handle", ( char * ) ClID ) ) == 0 )
          {
            LOG( NOTICE_LOG, "bad username [%s]", ClID ); // spatne username 
            ret->errCode = COMMAND_AUTH_ERROR;
          }
        else
          {


             // ziskej heslo z databaza a  provnej heslo a pokud neni spravne vyhod chybu
            if( strcmp(  PQsql.GetValueFromTable("REGISTRARACL" , "password" , "registrarid" ,  roid )  , passwd )  )
              {
                LOG( NOTICE_LOG, "bad password [%s] accept", passwd );
                ret->errCode = COMMAND_AUTH_ERROR;
              }
            else
              {
                LOG( NOTICE_LOG, "password [%s] accept", passwd );

                // zmena hesla pokud je nejake nastaveno
                if( strlen( newpass ) )
                  {
                    LOG( NOTICE_LOG, "change password  [%s]  to  newpass [%s] ", passwd, newpass );


                    PQsql.UPDATE( "REGISTRARACL" );
                    PQsql.SET( "password" , newpass );         
                    PQsql.WHERE( "registrarid" , roid  );

                    if( PQsql.EXEC() == false )
                      {
                        ret->errCode = COMMAND_FAILED;
                        change = false;
                      }
                  }

                if( change ) // pokud projde zmena defaulte je nastava true
                  {
                    id = PQsql.GetSequenceID( "login" ); // ziskani id jako sequence

                    // zapis do logovaci tabulky 

                    PQsql.INSERT( "Login" );
                    PQsql.INTO( "id" );
                    PQsql.INTO( "registrarid" );
                    PQsql.INTO( "logintrid" );
                    PQsql.VALUE( id );
                    PQsql.VALUE( roid );
                    PQsql.VALUE( clTRID );
   
                    if( PQsql.EXEC() )    // pokud se podarilo zapsat do tabulky
                      {
                        clientID = id;
                        LOG( NOTICE_LOG, "GET clientID  -> %d", clientID );

                        ret->errCode = COMMAND_OK; // zikano clinetID OK

                        // nankonec zmena komunikacniho  jazyka pouze na cestinu
                        if( lang == ccReg::CS )
                          {
                            LOG( NOTICE_LOG, "SET LANG to CS" ); 

                            PQsql.UPDATE( "Login" );
                            PQsql.SET( "lang" , "cs"  );
                            PQsql.WHEREID( clientID  );

                            if( PQsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;    // pokud se nezdarilo
                          }

                      }
                    else ret->errCode = COMMAND_FAILED;

                  }

              }

          }

        // probehne action pro svrTrID   musi byt az na mkonci pokud zna clientID
        if( PQsql.BeginAction( clientID, EPP_ClientLogin, ( char * ) clTRID ) )
          {
            // zapis na konec action
            ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
          }


        ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

        // pokud vse proslo
        if( ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();  // pokud uspesne
        else PQsql.RollbackTransaction();        // pokud nejake chyba zrus trasakci

      }

    PQsql.Disconnect();
  }

if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;    // obecna chyba
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

return ret;
}


/***********************************************************************
 *
 * FUNCTION:    ObjectCheck
 *
 * DESCRIPTION: obecna kontrola objektu nsset domain nebo kontakt
 *              
 * PARAMETERS:  
 *              act - typ akce check 
 *              table - jmeno tabulky CONTACT NSSET ci DOMAIN
 *              fname - nazev pole v databazi HANDLE ci FQDN
 *              chck - sequence stringu objektu typu  Check 
 *        OUT:  a - (1) objekt neexistuje a je volny 
 *                  (0) objekt uz je zalozen
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response* ccReg_EPP_i::ObjectCheck( short act , char * table , char *fname , const ccReg::Check& chck , ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
ccReg::Response *ret;
int  len , av ;
long unsigned int i;
ret = new ccReg::Response;

a = new ccReg::Avail;
ret->errCode=0;
ret->errors.length(0);

len = chck.length();
a->length(len);

LOG( NOTICE_LOG ,  "OBJECT %d  Check: clientID -> %d clTRID [%s] " , act  , clientID , clTRID );
 

if( PQsql.OpenDatabase( database ) )
{

  if( PQsql.BeginAction( clientID , EPP_ContactCheck , (char * ) clTRID  ) )
  {
 
    for( i = 0 ; i < len ; i ++ )
     { 
       
      switch( PQsql.CheckObject( table , fname , chck[i] ) )
           {
             case 1:
                       a[i]= 0;    // objekt existuje
                       //LOG( NOTICE_LOG ,  "object %s exist not Avail" , (char * ) chck[i] );
                       break;
             case 0:
                       a[i]= 1;    // objekt existuje
                       //LOG( NOTICE_LOG ,  "object %s not exist  Avail" ,(char * ) chck[i] );
                       break; 
             default: // error
                      ret->errCode=COMMAND_FAILED;
                      break;             
            }
  
     }
    
     
      // comand OK
     if( ret->errCode == 0 ) ret->errCode=COMMAND_OK; // vse proslo OK zadna chyba 

      // zapis na konec action
      ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;    
  }

ret->errMsg =  CORBA::string_dup(   PQsql.GetErrorMessage(  ret->errCode  ) ) ;
 
PQsql.Disconnect();
}


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;    // obecna chyba
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }


 
return ret;
}


ccReg::Response* ccReg_EPP_i::ContactCheck(const ccReg::Check& handle, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID)
{
return ObjectCheck( EPP_ContactCheck , "CONTACT"  , "handle" , handle , a , clientID , clTRID);
}

ccReg::Response* ccReg_EPP_i::NSSetCheck(const ccReg::Check& handle, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID)
{
return ObjectCheck( EPP_NSsetCheck ,  "NSSET"  , "handle" , handle , a ,  clientID , clTRID );
}


ccReg::Response*  ccReg_EPP_i::DomainCheck(const ccReg::Check& fqdn, ccReg::Avail_out a, CORBA::Long clientID, const char* clTRID)
{
return ObjectCheck(  EPP_DomainCheck , "DOMAIN"  , "fqdn" ,   fqdn , a ,  clientID , clTRID );
}





/***********************************************************************
 *
 * FUNCTION:    ContactInfo
 *
 * DESCRIPTION: vraci detailni informace o  kontaktu 
 *              prazdnou hodnotu pokud kontakt neexistuje              
 * PARAMETERS:  handle - identifikator kontaktu
 *        OUT:  c - struktura Contact detailni popis
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ContactInfo(const char* handle, ccReg::Contact_out c , CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
Status status;
ccReg::Response *ret;
// char countryCode[3];
int id , clid , crid , upid;
int actionID=0;
int len , i  , s ;

c = new ccReg::Contact;
ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ContactInfo: clientID -> %d clTRID [%s] handle [%s] " , clientID , clTRID , handle );
 
if( PQsql.OpenDatabase( database ) )
{

  
if( PQsql.BeginAction( clientID , EPP_ContactInfo , (char * ) clTRID  ) )
  {

  if( PQsql.SELECT( "CONTACT" , "HANDLE" , handle )  )
  {
  if( PQsql.GetSelectRows() == 1 )
    {

        crid =  atoi( PQsql.GetFieldValueName("CrID" , 0 ) ); 
        upid =  atoi( PQsql.GetFieldValueName("UpID" , 0 ) ); 



        status.Make(  PQsql.GetFieldValueName("status" , 0 ) ) ; // status


	c->handle=CORBA::string_dup( PQsql.GetFieldValueName("handle" , 0 ) ); // handle
	c->ROID=CORBA::string_dup( PQsql.GetFieldValueName("ROID" , 0 ) ); // ROID     
	c->CrDate= get_time_t( PQsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
	c->UpDate= get_time_t( PQsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
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
  //      strncpy( countryCode ,  PQsql.GetFieldValueName("Country" , 0 ) , 2 ); // 2 mistny ISO kod zeme
    //    countryCode[2] = 0;
        c->CountryCode=CORBA::string_dup(  PQsql.GetFieldValueName("Country" , 0 )  ); // vracet pouze ISO kod


	c->VAT=CORBA::string_dup(PQsql.GetFieldValueName("VAT" , 0 )); // DIC
	c->SSN=CORBA::string_dup(PQsql.GetFieldValueName("SSN" , 0 )); // SSN

        
        c->DiscloseName = PQsql.GetFieldBooleanValueName( "DiscloseName" , 0 );
        c->DiscloseOrganization = PQsql.GetFieldBooleanValueName( "DiscloseOrganization" , 0 );
        c->DiscloseAddress = PQsql.GetFieldBooleanValueName( "DiscloseAddress" , 0 );
        c->DiscloseTelephone = PQsql.GetFieldBooleanValueName( "DiscloseTelephone" , 0 );
        c->DiscloseFax  = PQsql.GetFieldBooleanValueName( "DiscloseFax" , 0 );
        c->DiscloseEmail = PQsql.GetFieldBooleanValueName( "DiscloseEmail" , 0 );

 
    
    
        // free select
	PQsql.FreeSelect();


        // zpracuj pole statusu
        len =  status.Length();
        c->stat.length(len);

        for( i = 0 ; i < len  ; i ++)
           {
              c->stat[i] = CORBA::string_dup( status.GetStatusString(  status.Get(i)  ) );
           }

              
        // identifikator registratora
        c->CrID =  CORBA::string_dup(  PQsql.GetRegistrarHandle( crid ) );
        c->UpID =  CORBA::string_dup(  PQsql.GetRegistrarHandle( upid ) );

        ret->errCode=COMMAND_OK; // VASE OK


/*
         // kod zeme cesky
        if( PQsql.GetClientLanguage() == LANG_CS ) c->Country=CORBA::string_dup( PQsql.GetCountryNameCS( countryCode ) );
	else c->Country=CORBA::string_dup( PQsql.GetCountryNameEN( countryCode ) );
*/

     }
    else 
     {
      PQsql.FreeSelect();
      LOG( WARNING_LOG  ,  "object handle [%s] NOT_EXIST" , handle );
      ret->errCode =  COMMAND_OBJECT_NOT_EXIST;
     }    

   }

 
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) ) ;

}

ret->errMsg =  CORBA::string_dup(   PQsql.GetErrorMessage(  ret->errCode  ) );

PQsql.Disconnect();
}




// vyprazdni kontakt pro navratovou hodnotu
if( ret->errCode == 0 )
{
ret->errCode = COMMAND_FAILED;    // obecna chyba
ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
ret->errMsg = CORBA::string_dup( "" );
c->handle=CORBA::string_dup("");
c->ROID=CORBA::string_dup("");   
c->CrID=CORBA::string_dup("");    // identifikator registratora ktery vytvoril kontak
c->UpID=CORBA::string_dup("");    // identifikator registratora ktery provedl zmeny
c->CrDate=0; // datum a cas vytvoreni
c->UpDate=0; // datum a cas zmeny
c->stat.length(0); // status
c->Name=CORBA::string_dup(""); // jmeno nebo nazev kontaktu
c->Organization=CORBA::string_dup(""); // nazev organizace
c->Street1=CORBA::string_dup(""); // adresa
c->Street2=CORBA::string_dup(""); // adresa
c->Street3=CORBA::string_dup(""); // adresa
c->City=CORBA::string_dup("");  // obec
c->StateOrProvince=CORBA::string_dup("");
c->PostalCode=CORBA::string_dup(""); // PSC
c->CountryCode=CORBA::string_dup(""); // zeme
c->Telephone=CORBA::string_dup("");
c->Fax=CORBA::string_dup("");
c->Email=CORBA::string_dup("");
c->NotifyEmail=CORBA::string_dup(""); // upozornovaci email
c->VAT=CORBA::string_dup(""); // DIC
c->SSN=CORBA::string_dup(""); // SSN
}


return ret;
}


/***********************************************************************
 *
 * FUNCTION:    ContactDelete
 *
 * DESCRIPTION: maze kontakt z tabulky Contact a uklada je do historie
 *              vraci bud kontakt nenalezen nebo kontakt ma jeste vazby
 *              na dalsi tabulky a nemuze byt smazan
 *              SMAZAT kontakt smi jen registrator ktery ho vytvoril
 * PARAMETERS:  handle - identifikator kontaktu
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response* ccReg_EPP_i::ContactDelete(const char* handle , CORBA::Long clientID, const char* clTRID )
{
ccReg::Response *ret;
PQ PQsql;
Status status;
int regID=0 , id ,  crID =0  ;

ret = new ccReg::Response;


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota
ret->errMsg = CORBA::string_alloc(64);
ret->errMsg = CORBA::string_dup("");
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ContactDelete: clientID -> %d clTRID [%s] handle [%s] " , clientID , clTRID , handle );



  if( PQsql.OpenDatabase( database ) )
    {

      if( PQsql.BeginAction( clientID, EPP_ContactDelete, ( char * ) clTRID ) )
        {
          if( PQsql.BeginTransaction() )
            {

              if( ( id = PQsql.GetNumericFromTable( "CONTACT", "id", "handle", ( char * ) handle ) ) == 0 )
                {
                  LOG( WARNING_LOG, "object handle [%s] NOT_EXIST", handle );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;      // pokud objekt neexistuje
                }
              else 
                {
                  // get  registrator ID
                  regID = PQsql.GetLoginRegistrarID( clientID );
                  // client contaktu ktery ho vytvoril
                  crID = PQsql.GetNumericFromTable( "CONTACT", "crID", "handle", ( char * ) handle );


                  if( regID != crID )   // pokud neni tvurcem kontaktu 
                    {
                      LOG( WARNING_LOG, "bad autorization not  creator of handle [%s]", handle );
                      ret->errCode = COMMAND_AUTOR_ERROR; // spatna autorizace
                    }                               
                  else                                                                                           
                    {
                      // zpracuj  pole statusu
                      status.Make( PQsql.GetStatusFromTable( "CONTACT", id ) );

                      if( status.Test( STATUS_DELETE ) )
                        {
                          LOG( WARNING_LOG, "status DeleteProhibited" );
                          ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                        }
                      else // status je OK
                        {
                          // test na vazbu do tabulky domain domain_contact_map a nsset_contact_map
                          if( PQsql.TestContactRelations( id ) )        // kontakt nemuze byt smazan ma vazby  
                            {
                              LOG( WARNING_LOG, "test contact handle [%s] relations: PROHIBITS_OPERATION", handle );
                              ret->errCode = COMMAND_PROHIBITS_OPERATION;
                            }
                          else
                            {
                              //  uloz do historie
                              if( PQsql.MakeHistory() )
                                {
                                  if( PQsql.SaveHistory( "Contact", "id", id ) ) // uloz zaznam
                                    {
                                      if( PQsql.DeleteFromTable( "CONTACT", "id", id ) ) ret->errCode = COMMAND_OK;      // pokud usmesne smazal
                                    }
                                }


                            }

                        }

                    }


                }

              // pokud vse proslo
              if( ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo commitni zmeny
              else PQsql.RollbackTransaction();  // pokud nejake chyba zrus trasakci
            }

          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }


      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }

  return ret;
}



/***********************************************************************
 *
 * FUNCTION:    ContactUpdate
 *
 * DESCRIPTION: zmena informaci u  kontaktu a ulozeni do historie
 *              ZMENIT kontakt smi jen registrator ktery ho vytvoril
 *              nebo ten ktery kontaktu vede nejakou domenu
 * PARAMETERS:  handle - identifikator kontaktu
 *              c      - ContactChange  zmenene informace o kontaktu
 *              status_add - pridane status priznaky 
 *              status_rem - status priznaky na zruseni
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response * ccReg_EPP_i::ContactUpdate( const char *handle, const ccReg::ContactChange & c, 
                                              const ccReg::Status & status_add, const ccReg::Status & status_rem,
                                              CORBA::Long clientID, const char *clTRID )
{
ccReg::Response * ret;
PQ PQsql;
char statusString[128];
int regID = 0, crID = 0, clID = 0, id;
bool policy_error = false;
int len, i;
Status status;

ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "ContactUpdate: clientID -> %d clTRID [%s] handle [%s] ", clientID, clTRID, handle );

  // test status flagu pokud nejsou typu server 
  len = status_add.length();
  for( i = 0; i < len; i++ )
    {
      LOG( NOTICE_LOG, "status_add [%s] ", CORBA::string_dup( status_add[i] ) );
      if( status.IsServerStatus( status.GetStatusNumber( status_add[i] ) ) )
        policy_error = true;
    }

  len = status_rem.length();
  for( i = 0; i < len; i++ )
    {
      LOG( NOTICE_LOG, "status_rem [%s] ", CORBA::string_dup( status_rem[i] ) );
      if( status.IsServerStatus( status.GetStatusNumber( status_rem[i] ) ) )
        policy_error = true;
    }




  if( PQsql.OpenDatabase( database ) )
    {

      if( PQsql.BeginAction( clientID, EPP_ContactUpdate, ( char * ) clTRID ) )
        {
          if( PQsql.BeginTransaction() )      // zahajeni transakce
            {
              if( ( id = PQsql.GetNumericFromTable( "CONTACT", "id", "handle", ( char * ) handle ) ) == 0 )
                {
                  LOG( WARNING_LOG, "object handle [%s] NOT_EXIST", handle );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;
                }
              // pokud kontakt existuje 
              else
                {
                  // get  registrator ID
                  regID = PQsql.GetLoginRegistrarID( clientID );
                  // zjistit kontak u domeny            
                  clID = PQsql.GetClientDomainRegistrant( regID, clientID );
                  // client contaktu ktery ho vytvoril
                  crID = PQsql.GetNumericFromTable( "CONTACT", "crID", "handle", ( char * ) handle );
                  // klient spravuje nejakou domenu kontaktu nebo je jeho tvurcem   
                  if( crID == regID || clID == regID )
                    {
                      if( PQsql.TestCountryCode( c.CC ) )       // test kodu zeme pokud je nastavena
                        {

                          // zjisti  pole statusu
                          status.Make( PQsql.GetStatusFromTable( "CONTACT", id ) );

                          if( status.Test( STATUS_UPDATE ) )
                            {
                              LOG( WARNING_LOG, "status UpdateProhibited" );
                              ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                            }
                          else  // status je OK
                            {
                              if( policy_error )
                                {
                                  LOG( WARNING_LOG, "status flag policy" );
                                  ret->errCode = COMMAND_PARAMETR_VALUE_POLICY_ERROR;
                                }
                              else      // zadny PARAMETR_VALUE_POLICY_ERROR u add ci rem  status flagu
                                {
                                  //  uloz do historie
                                  if( PQsql.MakeHistory() )
                                    {
                                      if( PQsql.SaveHistory( "Contact", "id", id ) )    // uloz zaznam
                                        {

                                          // pridany status
                                          for( i = 0; i < status_add.length(); i++ )
                                            status.Add( status.GetStatusNumber( status_add[i] ) );


                                          // zruseny status flagy
                                          for( i = 0; i < status_rem.length(); i++ )
                                            status.Rem( status.GetStatusNumber( status_rem[i] ) );


                                          //  vygeneruj  novy status string array
                                          status.Array( statusString );

                                          // zahaj update
                                          PQsql.UPDATE( "Contact" );

                                          // pridat zmenene polozky 
                                          PQsql.SET( "Name", c.Name );
                                          PQsql.SET( "Organization", c.Organization );
                                          PQsql.SET( "Street1", c.Street1 );
                                          PQsql.SET( "Street2", c.Street2 );
                                          PQsql.SET( "Street3", c.Street3 );
                                          PQsql.SET( "City", c.City );
                                          PQsql.SET( "StateOrProvince", c.StateOrProvince );
                                          PQsql.SET( "PostalCode", c.PostalCode );
                                          PQsql.SET( "Country", c.CC );
                                          PQsql.SET( "Telephone", c.Telephone );
                                          PQsql.SET( "Fax", c.Fax );
                                          PQsql.SET( "Email", c.Email );
                                          PQsql.SET( "NotifyEmail", c.NotifyEmail );
                                          PQsql.SET( "VAT", c.VAT );
                                          PQsql.SET( "SSN", c.SSN );

                                          //  Disclose parametry
                                          PQsql.SETBOOL( "DiscloseName", c.DiscloseName );
                                          PQsql.SETBOOL( "DiscloseOrganization", c.DiscloseOrganization );
                                          PQsql.SETBOOL( "DiscloseAddress", c.DiscloseAddress );
                                          PQsql.SETBOOL( "DiscloseTelephone", c.DiscloseTelephone );
                                          PQsql.SETBOOL( "DiscloseFax", c.DiscloseFax );
                                          PQsql.SETBOOL( "DiscloseEmail", c.DiscloseEmail );

                                          // datum a cas updatu  plus kdo zmenil zanzma na konec
                                          PQsql.SET( "UpDate", "now" );
                                          PQsql.SET( "UpID", regID );
                                          PQsql.SET( "status", statusString );

                                          // podminka na konec 
                                          PQsql.WHEREID( id );

                                          if( PQsql.EXEC() ) ret->errCode = COMMAND_OK;
                                          else ret->errCode = COMMAND_FAILED;
                                        }
                                    }

                                }

                            }
                        }
                      else      // neplatny kod zeme
                        {
                          ret->errCode = COMMAND_PARAMETR_ERROR;
                          LOG( WARNING_LOG, "unknow country code" );
                          ret->errors.length( 1 );
                          ret->errors[0].code = ccReg::contactUpdate_cc;        // spatne zadany neznamy country code
                          ret->errors[0].value = CORBA::string_dup( c.CC );
                          ret->errors[0].reason = CORBA::string_dup( "unknow country code" );
                        }

                    }
                  else          // client neni tvurcem kontaktu ani nespravuje zadnou jeho domenu
                    {
                      LOG( WARNING_LOG, "bad autorization of handle [%s]", handle );
                      ret->errCode = COMMAND_AUTOR_ERROR;       // spatna autorizace
                    }


                }

              // pokud vse proslo
              if( ret->errCode == COMMAND_OK )
                PQsql.CommitTransaction();    // pokud uspesne
              else
                PQsql.RollbackTransaction();  // pokud nejake chyba zrus trasakci
            }

          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }


      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;    // obecna chyba
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

return ret;
}



/***********************************************************************
 *
 * FUNCTION:    ContactCreate
 *
 * DESCRIPTION: vytvoreni kontaktu 
 *
 * PARAMETERS:  handle - identifikator kontaktu
 *              c      - ContactChange informace o kontaktu
 *        OUT:  crDate - datum vytvoreni objektu
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response * ccReg_EPP_i::ContactCreate( const char *handle, const ccReg::ContactChange & c, 
                                              ccReg::timestamp & crDate, CORBA::Long clientID, const char *clTRID )
{
PQ PQsql;
char createDate[32];
char roid[64];
ccReg::Response * ret;
int regID, id;
time_t now;
// default
ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

crDate = 0;


LOG( NOTICE_LOG, "ContactCreate: clientID -> %d clTRID [%s] handle [%s] ", clientID, clTRID, handle );


  if( PQsql.OpenDatabase( database ) )
    {
      if( PQsql.BeginAction( clientID, EPP_ContactCreate, ( char * ) clTRID ) )
        {
          if( PQsql.BeginTransaction() )      // zahajeni transakce
            {
              // test zdali kontakt uz existuje
              if( PQsql.GetNumericFromTable( "CONTACT", "id", "handle", ( char * ) handle ) )
                {
                  LOG( WARNING_LOG, "object handle [%s] EXIST", handle );
                  ret->errCode = COMMAND_OBJECT_EXIST;  // je uz zalozena
                }
              else              // pokud kontakt nexistuje
                {
                  // test zdali country code je existujici zeme
                  if( PQsql.TestCountryCode( c.CC ) )
                    {
                      // datum vytvoreni kontaktu
                      now = time( NULL );
                      crDate = now;
                      get_timestamp( now, createDate );

                      id = PQsql.GetSequenceID( "contact" );
                      // get  registrator ID
                      regID = PQsql.GetLoginRegistrarID( clientID );

                      // vytvor roid kontaktu
                      get_roid( roid, "C", id );

                      PQsql.INSERT( "CONTACT" );
                      PQsql.INTO( "id" );
                      PQsql.INTO( "roid" );
                      PQsql.INTO( "handle" );
                      PQsql.INTO( "CrDate" );
                      PQsql.INTO( "CrID" );
                      PQsql.INTO( "status" );

                      PQsql.INTOVAL( "Name", c.Name );
                      PQsql.INTOVAL( "Organization", c.Organization );
                      PQsql.INTOVAL( "Street1", c.Street1 );
                      PQsql.INTOVAL( "Street2", c.Street2 );
                      PQsql.INTOVAL( "Street3", c.Street3 );
                      PQsql.INTOVAL( "City", c.City );
                      PQsql.INTOVAL( "StateOrProvince", c.StateOrProvince );
                      PQsql.INTOVAL( "PostalCode", c.PostalCode );
                      PQsql.INTOVAL( "Country", c.CC );
                      PQsql.INTOVAL( "Telephone", c.Telephone );
                      PQsql.INTOVAL( "Fax", c.Fax );
                      PQsql.INTOVAL( "Email", c.Email );
                      PQsql.INTOVAL( "NotifyEmail", c.NotifyEmail );
                      PQsql.INTOVAL( "VAT", c.VAT );
                      PQsql.INTOVAL( "SSN", c.SSN );


                      if( c.DiscloseName > 0 ) PQsql.INTO( "DiscloseName" );
                      if( c.DiscloseOrganization > 0 ) PQsql.INTO( "DiscloseOrganization" );
                      if( c.DiscloseAddress > 0 ) PQsql.INTO( "DiscloseAddress" );
                      if( c.DiscloseTelephone > 0 ) PQsql.INTO( "DiscloseTelephone" );
                      if( c.DiscloseFax > 0 ) PQsql.INTO( "DiscloseFax" );
                      if( c.DiscloseEmail > 0 )PQsql.INTO( "DiscloseEmail" );

                      PQsql.VALUE( id );
                      PQsql.VALUE( roid );
                      PQsql.VALUE( handle );
                      PQsql.VALUE( createDate );
                      PQsql.VALUE( regID );
                      PQsql.VALUE( "{ 1 }" );   // OK status


                      PQsql.VAL( c.Name );
                      PQsql.VAL( c.Organization );
                      PQsql.VAL( c.Street1 );
                      PQsql.VAL( c.Street2 );
                      PQsql.VAL( c.Street3 );
                      PQsql.VAL( c.City );
                      PQsql.VAL( c.StateOrProvince );
                      PQsql.VAL( c.PostalCode );
                      PQsql.VAL( c.CC );
                      PQsql.VAL( c.Telephone );
                      PQsql.VAL( c.Fax );
                      PQsql.VAL( c.Email );
                      PQsql.VAL( c.NotifyEmail );
                      PQsql.VAL( c.VAT );
                      PQsql.VAL( c.SSN );


                      if( c.DiscloseName > 0 ) PQsql.VALUE( "t" );
                      if( c.DiscloseOrganization > 0 ) PQsql.VALUE( "t" );
                      if( c.DiscloseAddress > 0 ) PQsql.VALUE( "t" );
                      if( c.DiscloseTelephone > 0 ) PQsql.VALUE( "t" );
                      if( c.DiscloseFax > 0 ) PQsql.VALUE( "t" );
                      if( c.DiscloseEmail > 0 ) PQsql.VALUE( "t" );




                      // pokud se podarilo insertovat
                      if( PQsql.EXEC() )      //   ret->errCode = COMMAND_OK;
                        {       //  uloz do historie
                          if( PQsql.MakeHistory() )
                            {
                              if( PQsql.SaveHistory( "Contact", "id", id ) )    // uloz zaznam
                                {
                                  ret->errCode = COMMAND_OK;    // pokud se ulozilo do Historie
                                }
                            }
                        }

                    }
                  else          // neplatny kod zeme  
                    {
                      ret->errCode = COMMAND_PARAMETR_ERROR;
                      LOG( WARNING_LOG, "unknow country code" );
                      ret->errors.length( 1 );
                      ret->errors[0].code = ccReg::contactCreate_cc;    // spatne zadany neznamy country code
                      ret->errors[0].value = CORBA::string_dup( c.CC );
                      ret->errors[0].reason = CORBA::string_dup( "unknow country code" );
                    }
                }


              // pokud vse proslo OK
              if( ret->errCode == COMMAND_OK )
                PQsql.CommitTransaction();    // pokud uspesne
              else
                PQsql.RollbackTransaction();  // pokud nejake chyba zrus trasakci
            }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }
      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

return ret;
}





/***********************************************************************
 *
 * FUNCTION:    NSSetInfo
 *
 * DESCRIPTION: vraci detailni informace o nssetu a
 *              podrizenych hostech DNS 
 *              prazdnou hodnotu pokud kontakt neexistuje              
 *              
 * PARAMETERS:  handle - identifikator kontaktu
 *        OUT:  n - struktura NSSet detailni popis
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::NSSetInfo(const char* handle, ccReg::NSSet_out n, CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
Status status;
char  adres[1042] , adr[128] ;
ccReg::Response *ret;
int clid , crid , upid , nssetid , regID;
int i , j  ,ilen , len , s ;

ret = new ccReg::Response;
n = new ccReg::NSSet;

// default
ret->errCode = 0;
ret->errors.length(0);
LOG( NOTICE_LOG ,  "NSSetInfo: clientID -> %d clTRID [%s] handle [%s] " , clientID , clTRID , handle );
 


if( PQsql.OpenDatabase( database ) )
{

if( PQsql.BeginAction( clientID , EPP_NSsetInfo , (char * ) clTRID  ) )
 {

   // get  registrator ID
   regID = PQsql.GetLoginRegistrarID( clientID );


  if(  PQsql.SELECT( "NSSET" , "HANDLE" , handle ) )
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

        if( regID == clid ) // pokud je registrator clientem obdrzi autentifikaci
           n->AuthInfoPw = CORBA::string_dup( PQsql.GetFieldValueName("AuthInfoPw" , 0 ) ); // autentifikace
         else  n->AuthInfoPw = CORBA::string_dup( "" ); // jinak prazdny retezec

        ret->errCode=COMMAND_OK;


        // free select
        PQsql.FreeSelect();



        // zpracuj pole statusu
        len =  status.Length();
        n->stat.length(len);
        for( i = 0 ; i <  len  ; i ++)
           {
              n->stat[i] = CORBA::string_dup( status.GetStatusString(  status.Get(i)  ) );
           }


        n->ClID =  CORBA::string_dup( PQsql.GetRegistrarHandle( clid ) );
        n->CrID =  CORBA::string_dup( PQsql.GetRegistrarHandle( upid ) );
        n->UpID =  CORBA::string_dup( PQsql.GetRegistrarHandle( crid ) );

        // dotaz na DNS servry  na tabulky host
        if(   PQsql.SELECT( "HOST" , "nssetid" , nssetid  ) )
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
          } else ret->errCode=COMMAND_FAILED;
 



        // dotaz na technicke kontakty
        if(  PQsql.SELECTCONTACTMAP( "nsset"  , nssetid ) )
          {
               len =  PQsql.GetSelectRows(); // pocet technickych kontaktu
               n->tech.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) n->tech[i] = CORBA::string_dup( PQsql.GetFieldValue( i , 0 )  );

               PQsql.FreeSelect();
          } else ret->errCode=COMMAND_FAILED;


     }
   else
    {
     // free select
    PQsql.FreeSelect();
    LOG( WARNING_LOG  ,  "object handle [%s] NOT_EXIST" , handle );
    ret->errCode=COMMAND_OBJECT_NOT_EXIST;
    }

   } else ret->errCode=COMMAND_FAILED;


       
 

   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }

ret->errMsg =  CORBA::string_dup(   PQsql.GetErrorMessage(  ret->errCode  ) ) ;

PQsql.Disconnect();
}


// vyprazdni
if( ret->errCode == 0 )
{
ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
ret->errMsg = CORBA::string_dup( "" );
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



return ret;
}

/***********************************************************************
 *
 * FUNCTION:    NSSetDelete
 *
 * DESCRIPTION: vymazani NSSetu a ulozeni do historie
 *              SMAZAT muze pouze registrator ktery ho vytvoril 
 *              nebo ten ktery ho spravuje 
 *              nelze smazat nsset pokud ma vazbu v tabulce domain
 * PARAMETERS:  handle - identifikator nssetu
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::NSSetDelete(const char* handle, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
Status status;
int regID , id , clID = 0;
bool stat , del;


ret = new ccReg::Response;


ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetDelete: clientID -> %d clTRID [%s] handle [%s] " , clientID , clTRID , handle );


  if( PQsql.OpenDatabase( database ) )
    {

      if( PQsql.BeginAction( clientID, EPP_NSsetDelete, ( char * ) clTRID ) )
        {
          if( PQsql.BeginTransaction() )      // zahajeni transakce
            {

              // pokud NSSET existuje 
              if( ( id = PQsql.GetNumericFromTable( "NSSET", "id", "handle", ( char * ) handle ) ) == 0 )
                {
                  LOG( WARNING_LOG, "object handle [%s] NOT_EXIST", handle );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;      // pokud objekt neexistuje
                }
              else
                {
                  // get  registrator ID
                  regID = PQsql.GetLoginRegistrarID( clientID );
                  clID = PQsql.GetNumericFromTable( "NSSET", "ClID", "id", id );

                  if( regID != clID )   // pokud neni klientem 
                    {
                      LOG( WARNING_LOG, "bad autorization not client of nsset [%s]", handle );
                      ret->errCode = COMMAND_AUTOR_ERROR;       // spatna autorizace
                    }
                  else
                    {
                      // zpracuj  pole statusu
                      status.Make( PQsql.GetStatusFromTable( "NSSET", id ) );

                      if( status.Test( STATUS_DELETE ) )
                        {
                          LOG( WARNING_LOG, "status DeleteProhibited" );
                          ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                          stat = false;
                        }
                      else      // status je OK
                        {
                          // test na vazbu do tabulky domain jestli existuji vazby na  nsset
                          if( PQsql.TestNSSetRelations( id ) )  //  nemuze byt smazan

                            {
                              LOG( WARNING_LOG, "database relations" );
                              ret->errCode = COMMAND_PROHIBITS_OPERATION;
                              del = false;
                            }
                          else
                            {
                              //  uloz do historie
                              if( PQsql.MakeHistory() )
                                {
                                  if( PQsql.SaveHistory( "nsset_contact_map", "nssetid", id ) ) // historie tech kontakty
                                    {
                                      // na zacatku vymaz technicke kontakty
                                      if( PQsql.DeleteFromTable( "nsset_contact_map", "nssetid", id ) )
                                        {

                                          if( PQsql.SaveHistory( "HOST", "nssetid", id ) )
                                            {
                                              // vymaz nejdrive podrizene hosty
                                              if( PQsql.DeleteFromTable( "HOST", "nssetid", id ) )
                                                {
                                                  if( PQsql.SaveHistory( "NSSET", "id", id ) )
                                                    {
                                                      // vymaz NSSET nakonec
                                                      if( PQsql.DeleteFromTable( "NSSET", "id", id ) ) ret->errCode = COMMAND_OK;   // pokud vse OK
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }      

                            }
                        }

                    }
                }

              // pokud vse proslo 
              if( ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo commitni zmeny
              else  PQsql.RollbackTransaction();  // pokud nejake chyba zrus trasakci
            }


          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

 
return ret;
}



/***********************************************************************
 *
 * FUNCTION:    NSSetCreate
 *
 * DESCRIPTION: vytvoreni NSSetu a  podrizenych DNS hostu
 *
 * PARAMETERS:  handle - identifikator nssetu
 *              authInfoPw - autentifikace 
 *              tech - sequence technickych kontaktu
 *              dns - sequence DNS zaznamu  
 *        OUT:  crDate - datum vytvoreni objektu
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response * ccReg_EPP_i::NSSetCreate( const char *handle, const char *authInfoPw, 
                                            const ccReg::TechContact & tech, const ccReg::DNSHost & dns,
                                            ccReg::timestamp & crDate, CORBA::Long clientID, const char *clTRID )
{
PQ PQsql;
char Array[512];
char createDate[32];
char roid[64];
ccReg::Response * ret;
int regID, id, techid;
int i, len, j;
time_t now;

ret = new ccReg::Response;
// default
ret->errCode = 0;
ret->errors.length( 0 );
crDate = 0;

LOG( NOTICE_LOG, "NSSetCreate: clientID -> %d clTRID [%s] handle [%s]  authInfoPw [%s]", clientID, clTRID, handle , authInfoPw  );

  if( PQsql.OpenDatabase( database ) )
    {

      if( PQsql.BeginAction( clientID, EPP_NSsetCreate, ( char * ) clTRID ) )
        {

          // prvni test zdali nsset uz existuje          
          if( PQsql.GetNumericFromTable( "NSSET", "id", "handle", ( char * ) handle ) )
            {
               LOG( WARNING_LOG, "nsset handle [%s] EXIST", handle );
               ret->errCode = COMMAND_OBJECT_EXIST;  // je uz zalozen
           }
          else                  // pokud nexistuje 
          if( PQsql.BeginTransaction() )      // zahaj transakci
            {

              // get  registrator ID
              regID = PQsql.GetLoginRegistrarID( clientID );


              // ID je cislo ze sequence
              id = PQsql.GetSequenceID( "nsset" );

              // vytvor roid nssetu
              get_roid( roid, "N", id );

              // datum a cas vytvoreni
              now = time( NULL );
              crDate = now;
              get_timestamp( now, createDate );



              PQsql.INSERT( "NSSET" );
              PQsql.INTO( "id" );
              PQsql.INTO( "roid" );
              PQsql.INTO( "handle" );
              PQsql.INTO( "CrDate" );
              PQsql.INTO( "CrID" );
              PQsql.INTO( "ClID" );
              PQsql.INTO( "status" );
              PQsql.INTOVAL( "authinfopw", authInfoPw );

              PQsql.VALUE( id );
              PQsql.VALUE( roid );
              PQsql.VALUE( handle );
              PQsql.VALUE( createDate );
              PQsql.VALUE( regID );
              PQsql.VALUE( regID );
              PQsql.VALUE( "{ 1 }" );   // status OK
              PQsql.VAL( authInfoPw );

              // zapis nejdrive nsset 
              if( PQsql.EXEC() )
                {


                  // zapis technicke kontakty 
                  for( i = 0; i <  tech.length() ;  i++ )
                    {
                      techid = PQsql.GetNumericFromTable( "Contact", "id", "handle", tech[i] );
                      if( techid )
                        {
                          PQsql.INSERT( "nsset_contact_map" );
                          PQsql.VALUE( id );
                          PQsql.VALUE( techid );

                          if( PQsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;
                        }
                      else
                        {
                          LOG( WARNING_LOG, "NSSetCreate: unknow tech Contact " );
                          // TODO error value 
                          ret->errCode = COMMAND_PARAMETR_ERROR;
                        }
                    }

                  // zapis do tabulky hostu
                  for( i = 0; i < dns.length() ; i++ )
                    {
                      // ip adresa DNS hostu


                      // preved sequenci adres
                      strcpy( Array, " { " );
                      for( j = 0; j < dns[i].inet.length(); j++ )
                        {
                          if( j > 0 ) strcat( Array, " , " );
                          strcat( Array, CORBA::string_dup( dns[i].inet[j] ) );
                        }
                      strcat( Array, " } " );

                      // LOG( NOTICE_LOG ,  "NSSetCreate: DNS Host %s [%s] ",  CORBA::string_dup( dns[i].fqdn )  , Array    );

                      // HOST informace pouze ipaddr a fqdn 
                      PQsql.INSERT( "HOST" );
                      PQsql.INTO( "NSSETID" );
                      PQsql.INTO( "fqdn" );
                      PQsql.INTO( "ipaddr" );
                      PQsql.VALUE( id );
                      PQsql.VALUE( dns[i].fqdn );
                      PQsql.VALUE( Array );

                      if( PQsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;
                    }

                }


              //  uloz do historie
              if( ret->errCode == 0 )   // pokud zatim neni zadna chyba 
                {
                  if( PQsql.MakeHistory() )
                    {
                      if( PQsql.SaveHistory( "nsset_contact_map", "nssetid", id ) )     // historie tech kontakty
                        {

                          if( PQsql.SaveHistory( "HOST", "nssetid", id ) )
                            {
                              //  uloz podrizene hosty
                              if( PQsql.SaveHistory( "NSSET", "id", id ) ) ret->errCode = COMMAND_OK;
                            }
                        }
                    }          

                }

              // pokud vse proslo
              if( ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo commitni zmeny
              else PQsql.RollbackTransaction();  // pokud nejake chyba zrus trasakci
            }




          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }

if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }


return ret;
}



/***********************************************************************
 *
 * FUNCTION:    NSSetUpdate
 *
 * DESCRIPTION: zmena NSSetu a  podrizenych DNS hostu a tech kontaktu
 *              a ulozeni zmen do historie
 * PARAMETERS:  handle - identifikator nssetu
 *              authInfo_chg - zmena autentifikace 
 *              dns_add - sequence  pridanych DNS zaznamu  
 *              dns_rem - sequence   DNS zaznamu   pro smazani
 *              tech_add - sequence pridannych technickych kontaktu
 *              tech_rem - sequence technickych kontaktu na smazani
 *              status_add - pridane status flagy
 *              status_rem - status flagy na smazani
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/



ccReg::Response* ccReg_EPP_i::NSSetUpdate(const char* handle , const char* authInfo_chg, const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem, const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
Status status;
bool  stat , check;
char sqlString[4096] , buf[256] , Array[512] ,  statusString[128] ;
int regID=0 , clID=0 , id ,nssetid , contactid , techid ;
int i , j ,  len  , slen , hostID;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota
ret->errMsg = CORBA::string_alloc(64);
ret->errMsg = CORBA::string_dup("");
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetUpdate: clientID -> %d clTRID [%s] handle [%s] " , clientID , clTRID , handle );
LOG( NOTICE_LOG ,  "NSSetUpdate: authInfo_chg [%s] ", authInfo_chg );


if( PQsql.OpenDatabase( database ) )
{

if( PQsql.BeginAction( clientID , EPP_NSsetUpdate , (char * ) clTRID  ) )
 {

   // pokud domena existuje
  if( (id = PQsql.GetNumericFromTable(  "NSSET"  , "id" , "handle" , (char * ) handle ) ) == 0 )
  {
    LOG( WARNING_LOG  ,  "object [%s] NOT_EXIST" ,  handle );
    ret->errCode= COMMAND_OBJECT_NOT_EXIST;
  }
  else
  if( PQsql.BeginTransaction() )  
  {
   // get  registrator ID
   regID =   PQsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  PQsql.GetNumericFromTable(  "NSSET"  , "clID" , "id" , id );

  // zpracuj  pole statusu
  status.Make( PQsql.GetStatusFromTable( "NSSET" , id ) );

   if( status.Test( STATUS_UPDATE  ) ) 
    {
      LOG( WARNING_LOG  ,  "status UpdateProhibited");
      ret->errCode =  COMMAND_STATUS_PROHIBITS_OPERATION;
      stat = false;
    }
   else stat = true; // status je OK

   // TEST add a rem statusu flagu jestli nejsou server flag
    len  =   status_add.length();
    for( i = 0 ; i < len ; i ++)
       {
         if( status.IsServerStatus(  status.GetStatusNumber(  status_add[i]  )  ) )
           {
               LOG( WARNING_LOG  ,  "add status flag policy error  %s" , CORBA::string_dup( status_add[i] ) );
               ret->errCode =  COMMAND_PARAMETR_VALUE_POLICY_ERROR;
               stat = false;
           }
        }

    len  =   status_rem.length();
    for( i = 0 ; i < len ; i ++)  
       {
         if( status.IsServerStatus(  status.GetStatusNumber(  status_rem[i]  )  ) ) 
           {
               LOG( WARNING_LOG  ,  "rem status flag policy error  %s" , CORBA::string_dup( status_rem[i] ) );
               ret->errCode =  COMMAND_PARAMETR_VALUE_POLICY_ERROR;
               stat = false; 
           }
        }


   if( clID == regID   && stat ) // pokud je registrator clientem kontaktu a status je v poradku
     {
         //  uloz do historie
         if( PQsql.MakeHistory() )
           {
            if( PQsql.SaveHistory( "NSSET" , "id" , id ) ) // uloz zaznam
              {

                // pridany status
                len  =   status_add.length();
                for( i = 0 ; i < len ; i ++) status.Add(  status.GetStatusNumber(  status_add[i]  )  );


                // zruseny status flagy
                len  =   status_rem.length();
                for( i = 0 ; i < len ; i ++) status.Rem( status.GetStatusNumber(  status_rem[i] ) );


                //  vygeneruj  novy status string array
                status.Array( statusString );



                // zmenit zaznam o domene
                sprintf( sqlString , "UPDATE NSSET SET UpDate=\'now\' , upid=%d , status=\'%s\' " , regID , statusString   );


                // zmena autentifikace
                if( strlen( CORBA::string_dup(authInfo_chg)  ) )
                 { 
                   LOG( NOTICE_LOG ,  "change authInfo  [%s]" , authInfo_chg );
                   sprintf( buf , " , AuthInfoPw='\%s\' " , authInfo_chg  );
                   strcat( sqlString , buf ); 
                  }  


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
                            check = PQsql.CheckContactMap( "nsset" ,  id , techid );
                            if( techid == 0 ) LOG( WARNING_LOG , "contact handle [%s] not exist" , CORBA::string_dup(tech_rem[i]) );

                            if( check  ) LOG( WARNING_LOG , "Tech contact [%s]  exist" , CORBA::string_dup(tech_rem[i]) );

                             if( techid && !check )
                               {
                                  LOG( NOTICE_LOG ,  "add techid ->%d [%s]" ,  techid ,  CORBA::string_dup( tech_add[i]) );
                                  if(  PQsql.CheckContactMap( "nsset" , id , techid ) == false ) // pokud kontak jeste neexistuje tak ho pridej
                                    {
                                      sprintf( sqlString , "INSERT INTO nsset_contact_map VALUES ( %d , %d );"  , id , techid );
                                      if(   PQsql.ExecSQL( sqlString ) == false ) { ret->errCode=COMMAND_FAILED; break; } 
                                    }
                               }
                            else
                              {
                                   ret->errCode=COMMAND_FAILED;
                                   break;
                              }

                           }

                         // vymaz  tech kontakty
                        len = tech_rem.length();   
                        for( i = 0 ; i < len ; i ++ )
                          {
                             techid =  PQsql.GetNumericFromTable( "Contact" , "id" , "handle" , CORBA::string_dup(tech_rem[i]) );
                             check = PQsql.CheckContactMap( "nsset" ,  id , techid );
                             if( techid == 0 ) LOG( WARNING_LOG , "contact handle [%s] not exist" , CORBA::string_dup(tech_rem[i]) );
                             if( !check  ) LOG( WARNING_LOG , "Tech contact [%s]  not exist" , CORBA::string_dup(tech_rem[i]) );

                             if( techid && check )
                              {  
                                 LOG( NOTICE_LOG ,  "rem techid ->%d [%s]" ,  techid , CORBA::string_dup(tech_rem[i] ) ); 
                                if(  PQsql.CheckContactMap( "nsset" , id , techid ) )
                                  { 
                                    sprintf( sqlString , "DELETE FROM domain_contact_map WHERE  domainid=%d and contactid=%d;" , id , techid );
                                    if(   PQsql.ExecSQL( sqlString ) == false ) { ret->errCode=COMMAND_FAILED; break; }
                                  }
                              
                              }
                            else
                              {
                                   ret->errCode=COMMAND_FAILED; 
                                   break;
                              }
                          }
 
                        }
     
                      if( PQsql.SaveHistory( "host" , "nssetID" , id ) ) // uloz do historie hosty
                       {
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

                                   LOG( NOTICE_LOG ,  "add dns [%s] %s" , CORBA::string_dup( dns_add[i].fqdn ) , Array );                                

              
                                    sprintf( sqlString , "INSERT INTO  HOST ( nssetid , fqdn  , ipaddr ) VALUES ( %d , \'%s\', \'%s\' );", 
                                                 id ,   CORBA::string_dup( dns_add[i].fqdn ) , Array );
                                    if(  PQsql.ExecSQL( sqlString ) == false ) { ret->errCode == COMMAND_FAILED; break ;}
                               
                               }

                            // smazat DNS HOSTY
                            len = dns_rem.length();
                            for( i = 0 ; i < len ; i ++ )
                               {
                                 LOG( NOTICE_LOG ,  "delete dns [%s]" , CORBA::string_dup( dns_rem[i].fqdn )  );                                

                                 sprintf( sqlString , "DELETE FROM HOST WHERE nssetid=%d AND fqdn=\'%s\';" , id ,  CORBA::string_dup(   dns_rem[i].fqdn ) );   
                                 if(  PQsql.ExecSQL( sqlString ) == false ) { ret->errCode == COMMAND_FAILED; break ;}   
                               }

                       }


                     }


                  
               }


              
           }



       }

    // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
   if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
   else  PQsql.RollbackTransaction();
  }

   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }


ret->errMsg =  CORBA::string_dup(   PQsql.GetErrorMessage(  ret->errCode  ) ) ;

PQsql.Disconnect();
}

return ret;
}


/***********************************************************************
 *
 * FUNCTION:    NSSetTransfer
 *
 * DESCRIPTION: prevod NSSetu ze stavajiciho na noveho registratora
 *              a ulozeni zmen do historie
 * PARAMETERS:  handle - identifikator nssetu
 *              authInfo - autentifikace heslem
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 *
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response* ccReg_EPP_i::NSSetTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
bool auth;
int regID=0 , clID=0 , id , contactid;

ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetTransfer: clientID -> %d clTRID [%s] handle [%s] authInfo [%s] " , clientID , clTRID , handle , authInfo );
LOG( NOTICE_LOG ,  "NSSetTransfer: authInfo [%s]" , authInfo ); 

if( PQsql.OpenDatabase( database ) )
{

if( PQsql.BeginAction( clientID , EPP_NSsetTransfer , (char * ) clTRID  ) )
 {

   // pokud domena neexistuje
  if( (id = PQsql.GetNumericFromTable(  "NSSET"  , "id" , "handle" , (char * ) handle ) ) == 0 ) 
    {
     LOG( WARNING_LOG  ,  "object [%s] NOT_EXIST" ,  handle );
      ret->errCode= COMMAND_OBJECT_NOT_EXIST;
    }
  else
  if( PQsql.BeginTransaction() )  
  {
   // get  registrator ID
   regID =   PQsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  PQsql.GetNumericFromTable(  "NSSET"  , "clID" , "id" , id );




   if(  PQsql.AuthTable(  "DOMAIN"  , (char *)authInfo , id )   ) // pokud prosla autentifikace 
     {
         //  uloz do historie
       if( PQsql.MakeHistory() )
        {
          if( PQsql.SaveHistory( "NSSET" , "id" , id ) ) // uloz zaznam
           { 
                // zmena registratora
                PQsql.UPDATE( "NSSET");
                PQsql.SET( "TrDate" , "now" );
                PQsql.SET( "clID" , regID );
                PQsql.WHEREID( id ); 
                if(   PQsql.EXEC() )  ret->errCode = COMMAND_OK; // nastavit OK                                  
                else  ret->errCode = COMMAND_FAILED;
           }
       }

    }
   else
   {
        LOG( WARNING_LOG , "autorization failed");
        ret->errCode = COMMAND_AUTOR_ERROR; // spatna autorizace
    }


  
   // pokud nebyla chyba pri insertovani do tabulky 
   if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
   else PQsql.RollbackTransaction(); 
   }
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
}


ret->errMsg =  CORBA::string_dup(   PQsql.GetErrorMessage(  ret->errCode  ) ) ;

PQsql.Disconnect();
}


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

return ret;
}






/***********************************************************************
 *
 * FUNCTION:    DomainInfo
 *
 * DESCRIPTION: vraci detailni informace o  kontaktu 
 *              prazdnou hodnotu pokud kontakt neexistuje              
 * PARAMETERS:  fqdn - identifikator domeny jeji nazev 
 *        OUT:  d - struktura Domain  detailni popis
 *              clientID - id klienta
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::DomainInfo(const char* fqdn, ccReg::Domain_out d , CORBA::Long clientID, const char* clTRID)
{
PQ PQsql;
Status status;
ccReg::timestamp valexDate;
ccReg::ENUMValidationExtension *enumVal;
ccReg::Response *ret;
int id , clid , crid ,  upid , regid ,nssetid , regID;
int i , len ;

d = new ccReg::Domain;
ret = new ccReg::Response;




// default
ret->errCode=COMMAND_FAILED;
ret->errors.length(0);


LOG( NOTICE_LOG ,  "DomainInfo: clientID -> %d clTRID [%s] fqdn  [%s] " , clientID , clTRID  , fqdn );


d->ext.length(0); // extension

if( PQsql.OpenDatabase( database ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainInfo , (char * ) clTRID  ) )
 {

   // get  registrator ID
   regID = PQsql.GetLoginRegistrarID( clientID );


  if(  PQsql.SELECT( "DOMAIN" , "fqdn" , fqdn )  )
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
	d->TrDate= get_time_t( PQsql.GetFieldValueName("TrDate" , 0 ) ); // datum a cas transferu
	d->ExDate= get_time_t( PQsql.GetFieldValueName("ExDate" , 0 ) ); //  datum a cas expirace

	d->ROID=CORBA::string_dup( PQsql.GetFieldValueName("roid" , 0 )  ); // jmeno nebo nazev kontaktu
	d->name=CORBA::string_dup( PQsql.GetFieldValueName("fqdn" , 0 )  ); // jmeno nebo nazev kontaktu


        if( regID == clid ) // pokud je registrator clientem obdrzi autentifikaci
           d->AuthInfoPw = CORBA::string_dup( PQsql.GetFieldValueName("AuthInfoPw" , 0 ) ); // autentifikace
         else  d->AuthInfoPw = CORBA::string_dup( "" ); // jinak prazdny retezec
 


    
        ret->errCode=COMMAND_OK;

    
        // free select
	PQsql.FreeSelect();
        
        // zpracuj pole statusu
        len =  status.Length();
        d->stat.length(len);

        for( i = 0 ; i < len ; i ++)
           {
              d->stat[i] = CORBA::string_dup( status.GetStatusString(  status.Get(i)  ) );
           }


        d->ClID = CORBA::string_dup( PQsql.GetRegistrarHandle( clid ) );
        d->CrID = CORBA::string_dup( PQsql.GetRegistrarHandle( crid ) );
        d->UpID = CORBA::string_dup( PQsql.GetRegistrarHandle( upid ) );

        // vlastnik domeny
        d->Registrant=CORBA::string_dup( PQsql.GetValueFromTable( "CONTACT" , "handle" , "id" , regid ) );


        //  handle na nsset
        d->nsset=CORBA::string_dup( PQsql.GetValueFromTable( "NSSET" , "handle", "id" , nssetid ) );
    

        // dotaz na admin kontakty
        // dotaz na technicke kontakty
        if(  PQsql.SELECTCONTACTMAP( "domain"  , id ) )
          {
               len =  PQsql.GetSelectRows(); // pocet technickych kontaktu
               d->admin.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) 
                 {
                   d->admin[i] = CORBA::string_dup( PQsql.GetFieldValue( i , 0 )  );
                 }
               PQsql.FreeSelect();
           }else ret->errCode=COMMAND_FAILED;


       // uloz extension pokud existuje
        if( PQsql.SELECT( "enumval" , "domainID" , id ) )
          {    
            if( PQsql.GetSelectRows() == 1 )
              {
                valexDate =  get_time_t( PQsql.GetFieldValueName("ExDate" , 0 ) );

                enumVal = new ccReg::ENUMValidationExtension;
                enumVal->valExDate = valexDate ;
                d->ext.length(1); // preved na  extension
                d->ext[0] <<= enumVal;
                LOG( NOTICE_LOG , "enumValExtension ExDate %d" ,   valexDate );
              }

           PQsql.FreeSelect();
         } else ret->errCode=COMMAND_FAILED;
   

     }
   else
    {
     // free select
    PQsql.FreeSelect();
    LOG( WARNING_LOG  ,  "domain [%s] NOT_EXIST" , fqdn );
    ret->errCode =  COMMAND_OBJECT_NOT_EXIST;

    }

   }

   // zapis na konec action
   ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) );

 }

ret->errMsg =  CORBA::string_dup(   PQsql.GetErrorMessage(  ret->errCode  ) ) ;

PQsql.Disconnect();
}


// pokud neneslo kontakt
if( ret->errCode == 0 )
{
   ret->errCode = COMMAND_FAILED;
   ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
   ret->errMsg = CORBA::string_dup( "" );

// vyprazdni
d->ROID =  CORBA::string_dup( "" ); // domena do ktere patri host
d->name=  CORBA::string_dup( "" ); // fqdn nazev domeny
d->nsset = CORBA::string_dup( "" ); // nsset
d->AuthInfoPw  = CORBA::string_dup( "" ); //  autentifikace
d->stat.length(0); // status sequence
d->UpDate=0; // datuum zmeny
d->CrDate=0; // datum vytvoreni
d->TrDate=0; // datum transferu
d->ExDate=0; // datum vyprseni
d->Registrant=CORBA::string_dup( "" ); 
d->ClID=  CORBA::string_dup( "" );    // identifikator registratora ktery vytvoril host
d->UpID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam
d->CrID=  CORBA::string_dup( "" );    // identifikator registratora ktery zmenil zaznam
}


return ret;
}

/***********************************************************************
 *
 * FUNCTION:    DomainDelete
 *
 * DESCRIPTION: smazani domeny a ulozeni do historie
 *
 * PARAMETERS:  fqdn - identifikator domeny jeji nazev 
 *              clientID - id klienta
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::DomainDelete(const char* fqdn , CORBA::Long clientID, const char* clTRID)
{
ccReg::Response *ret;
PQ PQsql;
Status status;
int regID , clID , id;
bool stat;
ret = new ccReg::Response;


// default
ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "DomainDelete: clientID -> %d clTRID [%s] fqdn  [%s] " , clientID , clTRID  , fqdn );


  if( PQsql.OpenDatabase( database ) )
    {

      if( PQsql.BeginAction( clientID, EPP_DomainDelete, ( char * ) clTRID ) )
        {

          if( PQsql.BeginTransaction() )
            {
              if( ( id = PQsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", ( char * ) fqdn ) ) == 0 )
                {
                  LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;
                }
              else
                {

                  regID = PQsql.GetLoginRegistrarID( clientID );        // aktivni registrator
                  clID = PQsql.GetNumericFromTable( "DOMAIN", "ClID", "id", id );       // client objektu

                  if( regID != clID )
                    {
                      LOG( WARNING_LOG, "bad autorization not client of fqdn [%s]", fqdn );
                      ret->errCode = COMMAND_AUTOR_ERROR;       // spatna autorizace
                    }
                  else
                    {
                      // zpracuj  pole statusu
                      status.Make( PQsql.GetStatusFromTable( "DOMAIN", id ) );

                      if( status.Test( STATUS_DELETE ) )
                        {
                          LOG( WARNING_LOG, "status DeleteProhibited" );
                          ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                          stat = false;
                        }
                      else      // status je OK
                        {
                          //  uloz do historie
                          if( PQsql.MakeHistory() )
                            {
                              if( PQsql.SaveHistory( "domain_contact_map", "domainID", id ) )
                                {
                                  if( PQsql.DeleteFromTable( "enumval", "domainID", id ) )      // enumval extension
                                    {
                                      if( PQsql.DeleteFromTable( "domain_contact_map", "domainID", id ) )
                                        {
                                          if( PQsql.SaveHistory( "DOMAIN", "id", id ) )
                                            {
                                              if( PQsql.DeleteFromTable( "DOMAIN", "id", id ) )  ret->errCode = COMMAND_OK; // pokud usmesne smazal
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                }

              // pokud vse proslo
              if( ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo commitni zmeny
              else  PQsql.RollbackTransaction();  // pokud nejake chyba zrus trasakci
            }


          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }



if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

  return ret;
}



/***********************************************************************
 *
 * FUNCTION:    DomainUpdate
 *
 * DESCRIPTION: zmena informaci o domene a ulozeni do historie
 *
 * PARAMETERS:  fqdn - identifikator domeny jeji nazev 
 *              registrant_chg - zmena vlastnika domeny
 *              authInfo_chg  - zmena hesla
 *              nsset_chg - zmena nssetu 
 *              admin_add - sequence pridanych admin kontaktu
 *              admin_rem - sequence smazanych admin kontaktu
 *              status_add - pridane status flagy
 *              status_rem - status flagy na smazani
 *              clientID - id klienta
 *              clTRID - cislo transakce klienta
 *              ext - ExtensionList
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::DomainUpdate(const char* fqdn, const char* registrant_chg , const char* authInfo_chg, const char* nsset_chg, const ccReg::AdminContact& admin_add, const ccReg::AdminContact& admin_rem, const ccReg::Status& status_add, const ccReg::Status& status_rem, CORBA::Long clientID, const char* clTRID , const ccReg::ExtensionList& ext)
{
ccReg::Response *ret;
PQ PQsql;
Status status;
const ccReg::ENUMValidationExtension *enumVal;
bool stat , check;
char  valexpiryDate[32] ;
char sqlString[4096] , buf[256] , statusString[128] ;
int regID=0 , clID=0 , id ,nssetid , contactid , adminid ;
int i , len , slen , j  ;
time_t valExpDate=0 ;

ret = new ccReg::Response;

ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota
ret->errMsg = CORBA::string_alloc(64);
ret->errMsg = CORBA::string_dup("");
ret->errors.length(0);



LOG( NOTICE_LOG ,  "DomainUpdate: clientID -> %d clTRID [%s] fqdn  [%s] " , clientID , clTRID  , fqdn );
LOG( NOTICE_LOG ,  "DomainUpdate: registrant_chg  [%s] authInfo_chg [%s]  nsset_chg [%s] " , registrant_chg , authInfo_chg , nsset_chg );

// parse extension
len =  ext.length();

if( len > 0 )
{
  LOG( NOTICE_LOG , "extension length %d" ,  ext.length() );
  for( i = 0 ; i < len ; i ++ )
  {
  if(  ext[i] >>= enumVal   )
    {
       valExpDate = enumVal->valExDate ;
       LOG( NOTICE_LOG , "enumVal %d " ,  enumVal->valExDate );
    }
  else
    {
      LOG( ERROR_LOG , "Unknown value extension[%d]" , i );
      break;
    }

 }
}


if( PQsql.OpenDatabase( database ) )
{

if( PQsql.BeginAction( clientID , EPP_DomainUpdate , (char * ) clTRID  ) )
 {

   // pokud domena existuje
  if( (id = PQsql.GetNumericFromTable(  "DOMAIN"  , "id" , "fqdn" , (char * ) fqdn ) ) == 0 )
    {
      LOG( WARNING_LOG  ,  "domain  [%s] NOT_EXIST" , fqdn );
      ret->errCode= COMMAND_OBJECT_NOT_EXIST;
     }
  else
  if( PQsql.BeginTransaction() )  
  {
   // get  registrator ID
   regID =   PQsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  PQsql.GetNumericFromTable(  "DOMAIN"  , "clID" , "id" , id );

  // zpracuj  pole statusu
  status.Make( PQsql.GetStatusFromTable( "DOMAIN" , id ) );

   if( status.Test( STATUS_UPDATE )  )
     {
        LOG( WARNING_LOG  ,  "status UpdateProhibited");
        ret->errCode =  COMMAND_STATUS_PROHIBITS_OPERATION;
        stat = false;
     }
    else stat = true; // status je OK

   // TEST add a rem statusu flagu jestli nejsou server flag
    len  =   status_add.length();
    for( i = 0 ; i < len ; i ++)
       {
         if( status.IsServerStatus(  status.GetStatusNumber(  status_add[i]  )  ) )
           {
               LOG( WARNING_LOG  ,  "add status flag policy error  %s" , CORBA::string_dup( status_add[i] ) );
               ret->errCode =  COMMAND_PARAMETR_VALUE_POLICY_ERROR;
               stat = false;
           }
        }

    len  =   status_rem.length();
    for( i = 0 ; i < len ; i ++)  
       {
         if( status.IsServerStatus(  status.GetStatusNumber(  status_rem[i]  )  ) ) 
           {
               LOG( WARNING_LOG  ,  "rem status flag policy error  %s" , CORBA::string_dup( status_rem[i] ) );
               ret->errCode =  COMMAND_PARAMETR_VALUE_POLICY_ERROR;
               stat = false; 
           }
        }




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
                for( i = 0 ; i < len ; i ++) status.Add(  status.GetStatusNumber(  status_add[i]  ) );


                // zruseny status flagy
                len  =   status_rem.length();
                for( i = 0 ; i < len ; i ++) status.Rem( status.GetStatusNumber( status_rem[i]  ) );


                //  vygeneruj  novy status string array
                status.Array( statusString );
 

                // zmenit zaznam o domene
                sprintf( sqlString , "UPDATE DOMAIN SET UpDate=\'now\' , upid=%d , status=\'%s\' " , regID , statusString   );
                // zmena nssetu
                if( nssetid ) { sprintf( buf , " ,  nsset=%d " , nssetid ); strcat( sqlString , buf ); } 
                // zmena drzitele domeny 
                if( contactid ) { sprintf( buf , " ,  registrant=%d " , contactid); strcat( sqlString , buf ); } 
                // zmena autentifikace
                if( strlen( CORBA::string_dup(authInfo_chg)  ) )
                 { sprintf( buf , " , AuthInfoPw='\%s\' " , CORBA::string_dup(authInfo_chg)  );  strcat( sqlString , buf ); }  

 
                sprintf( buf , " WHERE id=%d;" , id );
                strcat( sqlString , buf ); 


                   if(   PQsql.ExecSQL( sqlString ) )
                     {  
                       ret->errCode = COMMAND_OK; // nastavit uspesne

                      if( PQsql.SaveHistory( "enumval" , "domainID" , id ) ) // uloz do historie 
                       {
                             // zmena extension
                            if(   valExpDate )
                             {
                                get_timestamp(   valExpDate ,  valexpiryDate );
                                LOG( NOTICE_LOG ,  "change valExpDate %s " , valexpiryDate ); 

                                sprintf( sqlString , "UPDATE  enumval set  exdate=\'%s\' WHERE domainID=%d; " , valexpiryDate , id );
                                if( PQsql.ExecSQL( sqlString ) == false )  ret->errCode=COMMAND_FAILED;
                              }
                       }     
        
                      if( PQsql.SaveHistory( "domain_contact_map" , "domainID" , id ) ) // uloz do historie admin kontakty
                      {




                       // pridat admin kontakty
                      len = admin_add.length(); 
                       for( i = 0 ; i < len ; i ++ )
                          { 
                              adminid =  PQsql.GetNumericFromTable( "Contact" , "id" , "handle" , CORBA::string_dup(admin_add[i]) );
                              check = PQsql.CheckContactMap( "domain" ,  id , adminid );

                             if(  check ) LOG( WARNING_LOG ,  "ADMIN contact [%s] exist in table " ,  CORBA::string_dup(admin_add[i])  );
                             if( adminid  == 0 ) LOG( WARNING_LOG ,  "contact  handle  [%s] not exist " ,  CORBA::string_dup(admin_add[i])  );

                              if( adminid && !check)
                                {  
                                    LOG( NOTICE_LOG ,  "ADD admin contact id %d [%s]  " , adminid , CORBA::string_dup(admin_add[i])  );
                                    sprintf( sqlString , "INSERT INTO domain_contact_map VALUES ( %d , %d );"  , id , adminid );
                                    if(   PQsql.ExecSQL( sqlString ) == false ) 
                                      { 
                                        LOG( WARNING_LOG ,  "can not insert admin_contact id %d [%s]" ,  adminid , CORBA::string_dup(admin_add[i]) );
                                        ret->errCode=COMMAND_FAILED; 
                                        break; 
                                      } 
                                }
                              else 
                               {
                                  ret->errCode=COMMAND_FAILED;
                                  break;
                               }
  
                           }

                     // vymaz admin kontakty
                       len = admin_rem.length();   
                       for( i = 0 ; i < len ; i ++ )
                          {
                           adminid =  PQsql.GetNumericFromTable( "Contact" , "id" , "handle" , CORBA::string_dup(admin_rem[i]) );
                           check = PQsql.CheckContactMap( "domain" ,  id , adminid );

                           if(  adminid == 0 ) LOG( WARNING_LOG ,  "contact  handle  [%s] not exist " ,  CORBA::string_dup(admin_add[i])  );
                           if(  !check ) LOG( WARNING_LOG ,  "ADMIN contact [%s] not exist in table " ,  CORBA::string_dup(admin_add[i])  );
                           // pokud kontakt existuje a je v tabulce domain_contact_map
                           if( adminid && check )
                             {
                              LOG( NOTICE_LOG ,  "DEL admin contact id %d [%s]  " , adminid , CORBA::string_dup(admin_add[i])  );
                              sprintf( sqlString , "DELETE FROM domain_contact_map WHERE  domainid=%d and contactid=%d;" , id , adminid );
                                    if(   PQsql.ExecSQL( sqlString ) == false ) 
                                      { 
                                        LOG( WARNING_LOG ,  "can not delte admin_contact id %d [%s] " ,  adminid , CORBA::string_dup(admin_add[i])  );
                                        ret->errCode=COMMAND_FAILED; 
                                        break; 
                                      } 
                              }
                              else
                               {                                   
                                  ret->errCode=COMMAND_FAILED;
                                  break;
                               }
     
                          }
 
                        }     
                     }


                  
               }
           }
       }

  // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
  if(  ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
  else  PQsql.RollbackTransaction();
  }

 // zapis na konec action
 ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode  ) ) ;
 }


ret->errMsg =  CORBA::string_dup(   PQsql.GetErrorMessage(  ret->errCode  ) ) ;

PQsql.Disconnect();
}

return ret;
}

/***********************************************************************
 *
 * FUNCTION:    DomainCreate
 *
 * DESCRIPTION: vytvoreni zaznamu o domene
 *
 * PARAMETERS:  fqdn - identifikator domeny jeji nazev 
 *              registrant -  vlastnik domeny
 *              nsset -  identifikator nssetu  
 *              period - doba platnosti domeny v mesicich
 *              AuthInfoPw  -  heslo
 *              admin - sequence  admin kontaktu
 *        OUT:  crDate - datum vytvoreni objektu
 *        OUT:  exDate - datum expirace objektu 
 *              clientID - id klienta
 *              clTRID - cislo transakce klienta
 *              ext - ExtensionList
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::DomainCreate( const char *fqdn, const char *Registrant, const char *nsset, const char *AuthInfoPw, CORBA::Short period,
                                             const ccReg::AdminContact & admin, ccReg::timestamp & crDate, ccReg::timestamp & exDate, 
                                             CORBA::Long clientID, const char *clTRID, const ccReg::ExtensionList & ext )
{
PQ PQsql;
const ccReg::ENUMValidationExtension * enumVal;
char expiryDate[32], valexpiryDate[32], createDate[32];
char roid[64];
ccReg::Response * ret;
int contactid, regID, nssetid, adminid, id;
int i, len, s, zone;
bool insert = true;
time_t t, valExpDate;

ret = new ccReg::Response;


// default
valExpDate = 0;

// default
ret->errCode = 0;
ret->errors.length( 0 );
crDate = 0;
exDate = 0;



LOG( NOTICE_LOG, "DomainCreate: clientID -> %d clTRID [%s] fqdn  [%s] ", clientID, clTRID, fqdn );
LOG( NOTICE_LOG, "DomainCreate:  Registrant  [%s]  nsset [%s]  AuthInfoPw [%s] period %d", Registrant, nsset, AuthInfoPw, period );

// parse extension
  len = ext.length();
  if( len > 0 )
    {
      LOG( NOTICE_LOG, "extension length %d", ext.length() );
      for( i = 0; i < len; i++ )
        {
          if( ext[i] >>= enumVal )
            {
              valExpDate = enumVal->valExDate;
              LOG( NOTICE_LOG, "enumVal %d ", enumVal->valExDate );
            }
          else
            {
              LOG( ERROR_LOG, "Unknown value extension[%d]", i );
              break;
            }

        }
    }



  if( PQsql.OpenDatabase( database ) )
    {

      if( PQsql.BeginAction( clientID, EPP_DomainCreate, ( char * ) clTRID ) )
        {

          //  test zdali domena uz existuje
          if( PQsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", ( char * ) fqdn ) )
            {
              ret->errCode = COMMAND_OBJECT_EXIST;      // je uz zalozena
              LOG( WARNING_LOG, "domain  [%s] EXIST", fqdn );
            }
          else // pokud domena nexistuje             
          if( PQsql.BeginTransaction() )
            {
              id = PQsql.GetSequenceID( "domain" );     // id domeny

              // vytvor roid domeny
              get_roid( roid, "D", id );

              zone = get_zone( ( char * ) fqdn );       // kontrola nazvu domeny a automaticke zarazeni do zony

              if( zone == 0 )
                {
                   LOG( WARNING_LOG, "can not get zone for domain  [%s] ", fqdn );
                   ret->errCode = COMMAND_PARAMETR_ERROR; // TODO errors 
                }
              else
                {


                  // get  registrator ID
                  regID = PQsql.GetLoginRegistrarID( clientID );

                  if( ( nssetid = PQsql.GetNumericFromTable( "NSSET", "id", "handle", nsset ) ) == 0 )
                    {
                      LOG( WARNING_LOG, "unknow nsset handle %s", nsset );
                      ret->errCode = COMMAND_PARAMETR_ERROR;
                      // TODO errors
                    }
                  else
                    {
                      if( ( contactid = PQsql.GetNumericFromTable( "CONTACT", "id", "handle", Registrant ) ) == 0 )
                        {
                          LOG( WARNING_LOG, "unknow registrant handle %s", Registrant );
                          ret->errCode = COMMAND_PARAMETR_ERROR; // TODO errors                          
                        }
                      else
                        {
                          t = time( NULL );
                          crDate = t;   // datum a cas vytvoreni objektu
                          exDate = expiry_time( t, period );    // datum a cas expirace o pulnoci
                          // preved datum a cas expirace prodluz tim cas platnosti domeny
                          get_timestamp( exDate, expiryDate );
                          get_timestamp( crDate, createDate );


                          PQsql.INSERT( "DOMAIN" );
                          PQsql.INTO( "zone" );
                          PQsql.INTO( "id" );
                          PQsql.INTO( "roid" );
                          PQsql.INTO( "fqdn" );
                          PQsql.INTO( "CrDate" );
                          PQsql.INTO( "Exdate" );
                          PQsql.INTO( "ClID");
                          PQsql.INTO( "CrID" );
                          PQsql.INTO( "status" );
                          PQsql.INTO( "Registrant" );
                          PQsql.INTO( "nsset");
                          PQsql.INTOVAL( "authinfopw" , AuthInfoPw);
                                                                  
                                                                  
                          PQsql.VALUE( zone );
                          PQsql.VALUE( id );
                          PQsql.VALUE( roid );
                          PQsql.VALUE( fqdn );
                          PQsql.VALUE( createDate );
                          PQsql.VALUE( expiryDate );
                          PQsql.VALUE( regID );
                          PQsql.VALUE( regID );
                          PQsql.VALUE( "{ 1 }" ); // status OK
                          PQsql.VALUE( contactid );
                          PQsql.VALUE( nssetid );
                          PQsql.VAL(  AuthInfoPw);   

                          // pokud se insertovalo do tabulky
                          if( PQsql.EXEC() )
                            {

                              // pridej enum  extension
                              if( valExpDate )
                                {
                                  get_timestamp( valExpDate, valexpiryDate );
                                  PQsql.INSERT( "enumval" );
                                  PQsql.VALUE( id );
                                  PQsql.VALUE( valexpiryDate ); 
                                  if( PQsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;;
                                }

                              // zapis admin kontakty
                              len = admin.length();
                              if( len > 0  )
                                {
                                  for( i = 0; i < len; i++ )
                                    {
                                      adminid = PQsql.GetNumericFromTable( "Contact", "id", "handle", admin[i]  );

                                      if( adminid )
                                       {
                                          PQsql.INSERT( "domain_contact_map" );
                                          PQsql.VALUE( id );
                                          PQsql.VALUE( adminid );
                                         // pokud se nepodarilo pridat do tabulky                                     
                                         if( PQsql.EXEC() == false )   ret->errCode = COMMAND_FAILED;
                                       }   
                                      else
                                      {
                                       ret->errCode = COMMAND_PARAMETR_ERROR; // TODO errors                                       
                                      } 
                                    }
                                }

                              if( ret->errCode == 0 )   // pokud zadna chyba uloz do historie
                                {
                                  //  uloz do historie
                                  if( PQsql.MakeHistory() )
                                    {
                                      if( PQsql.SaveHistory( "enumval", "domainID", id ) )
                                        {
                                          if( PQsql.SaveHistory( "domain_contact_map", "domainID", id ) )
                                            {
                                              if( PQsql.SaveHistory( "DOMAIN", "id", id ) ) ret->errCode = COMMAND_OK;
                                            }
                                        }
                                    }
                                }

                            }

                        }

                    }

                }

              // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
              if( ret->errCode == COMMAND_OK )  PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
              else PQsql.RollbackTransaction();
            }

          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }



return ret;
}


/***********************************************************************
 *
 * FUNCTION:    DomainRenew
 *
 * DESCRIPTION: prodlouzeni platnosti domeny o  periodu
 *              a ulozeni zmen do historie
 * PARAMETERS:  fqdn - nazev domeny nssetu
 *              curExpDate - datum vyprseni domeny pred prodlouzenim
 *              period - doba prodlouzeni v mesicich
 *        OUT:  exDate - datum a cas nove expirace domeny   
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 *              ext - ExtensionList 
 *
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::DomainRenew( const char *fqdn, ccReg::timestamp curExpDate, CORBA::Short period, 
                                            ccReg::timestamp & exDate, CORBA::Long clientID,
                                            const char *clTRID, const ccReg::ExtensionList & ext )
{
  PQ PQsql;
  Status status;
  const ccReg::ENUMValidationExtension * enumVal;
  char exDateStr[24], valexpiryDate[32];

  ccReg::Response * ret;
  int clid, regID, id, len, i;
  bool stat;
  time_t ex = 0, t, valExpDate = 0;

  ret = new ccReg::Response;

  t = curExpDate;

// default
  exDate = 0;

// default
  ret->errCode = 0;
  ret->errors.length( 0 );


  LOG( NOTICE_LOG, "DomainRenew: clientID -> %d clTRID [%s] fqdn  [%s] period %d ", clientID, clTRID, fqdn, period );


  len = ext.length();

  if( len > 0 )
    {
      LOG( NOTICE_LOG, "extension length %d", ext.length() );
      for( i = 0; i < len; i++ )
        {
          if( ext[i] >>= enumVal )
            {
              valExpDate = enumVal->valExDate;
              LOG( NOTICE_LOG, "enumVal %d ", enumVal->valExDate );
            }
          else
            {
              LOG( ERROR_LOG, "Unknown value extension[%d]", i );
              break;
            }

        }
    }



  if( PQsql.OpenDatabase( database ) )
    {

      if( PQsql.BeginAction( clientID, EPP_DomainRenew, ( char * ) clTRID ) )
        {

          regID = PQsql.GetLoginRegistrarID( clientID );        // aktivni registrator
          if( ( id = PQsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", ( char * ) fqdn ) ) == 0 )
            // prvni test zdali domena  neexistuje 
            {
              ret->errCode = COMMAND_OBJECT_NOT_EXIST;  // domena neexistujea
              LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
            }
          else
            // zahaj transakci
          if( PQsql.BeginTransaction() )
            {


              if( PQsql.SELECT( "DOMAIN", "id", id ) )
                {

                  clid = atoi( PQsql.GetFieldValueName( "ClID", 0 ) );
                  ex = get_time_t( PQsql.GetFieldValueName( "ExDate", 0 ) );    // datum a cas  expirace domeny
                  PQsql.FreeSelect();
                }

              if( ex != curExpDate )
                {
                  LOG( WARNING_LOG, "curExpDate is not same as ExDate" );
                  ret->errCode = COMMAND_PARAMETR_ERROR;
                }
              else
                {
                  // zpracuj  pole statusu
                  status.Make( PQsql.GetStatusFromTable( "DOMAIN", id ) );

                  if( status.Test( STATUS_RENEW ) )
                    {
                      LOG( WARNING_LOG, "status RenewProhibited" );
                      ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                      stat = false;
                    }
                  else          // status je OK


                  if( clid != regID )
                    {
                      LOG( WARNING_LOG, "bad autorization not  client of domain [%s]", fqdn );
                      ret->errCode = COMMAND_AUTOR_ERROR;       // spatna autorizace
                    }
                  else
                    {
                      // preved datum a cas expirace prodluz tim cas platnosti domeny
                      t = expiry_time( ex, period );
                      exDate = t;       // datum a cas prodlouzeni domeny
                      get_timestamp( t, exDateStr );

                      //  uloz do historie
                      if( PQsql.MakeHistory() )
                        {


                          if( PQsql.SaveHistory( "domain_contact_map", "domainID", id ) )       // uloz kontakty
                            {
                              if( PQsql.SaveHistory( "Domain", "id", id ) )     // uloz zaznam
                                {
                                  // zmena platnosti domeny
                                  PQsql.UPDATE( "DOMAIN" );
                                  PQsql.SET( "ExDate", exDateStr );
                                  PQsql.WHEREID( id );
                                  if( PQsql.EXEC() ) ret->errCode = COMMAND_OK;
                                  else ret->errCode = COMMAND_FAILED;
                                }
                            }

                          if( ret->errCode == COMMAND_OK )
                            {
                              if( PQsql.SaveHistory( "enumval", "domainID", id ) )
                                {
                                  if( valExpDate )      // zmena extension
                                    {
                                      get_timestamp( valExpDate, valexpiryDate );
                                      LOG( NOTICE_LOG, "change valExpDate %s ", valexpiryDate );

                                      PQsql.UPDATE( "enumval" );
                                      PQsql.SET( "ExDate", valexpiryDate );
                                      PQsql.WHERE( "domainID", id );

                                      if( PQsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;
                                    }

                                }
                            }



                        }

                    }
                }


              // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
              if( ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
              else PQsql.RollbackTransaction();
            }


          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }


return ret;
}


/***********************************************************************
 *
 * FUNCTION:    DomainTransfer
 *
 * DESCRIPTION: prevod domeny  ze stavajiciho na noveho registratora
 *              a ulozeni zmen do historie
 * PARAMETERS:  fqdn - plnohodnotny nazev domeny
 *             //  registrant - vlastnik domeny 
 *              authInfo - autentifikace heslem 
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 *
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::DomainTransfer( const char *fqdn, const char *authInfo, CORBA::Long clientID, const char *clTRID )
{
ccReg::Response * ret;
PQ PQsql;
Status status;
int regID = 0, clID = 0, id;  //   registrantid , contactid;

ret = new ccReg::Response;

// default
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "DomainTransfer: clientID -> %d clTRID [%s] fqdn  [%s]  ", clientID, clTRID, fqdn );


  if( PQsql.OpenDatabase( database ) )
    {

      if( PQsql.BeginAction( clientID, EPP_DomainTransfer, ( char * ) clTRID ) )
        {

          // pokud domena existuje
          if( ( id = PQsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", ( char * ) fqdn ) ) == 0 )
            {
              ret->errCode = COMMAND_OBJECT_NOT_EXIST;
              LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
            }
          else if( PQsql.BeginTransaction() )
            {
              // get  registrator ID
              regID = PQsql.GetLoginRegistrarID( clientID );
              // client contaktu
              clID = PQsql.GetNumericFromTable( "DOMAIN", "clID", "id", id );

              if( regID != clID )       // transfer nemuze delat stavajici client
                {
                  // zpracuj  pole statusu
                  status.Make( PQsql.GetStatusFromTable( "DOMAIN", id ) );

                  if( status.Test( STATUS_TRANSFER ) )
                    {
                      LOG( WARNING_LOG, "status TransferProhibited" );
                      ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;

                    }
                  else
                    {
                      // autentifikace
                      if( PQsql.AuthTable( "DOMAIN", ( char * ) authInfo, id ) == false )
                        {
                          ret->errCode = COMMAND_AUTOR_ERROR;   // spatna autorizace
                          LOG( WARNING_LOG, "autorization error bad authInfo [%s] ", authInfo );
                        }
                      else
                        {
                          //  uloz do historie
                          if( PQsql.MakeHistory() )
                            {
                              if( PQsql.SaveHistory( "Domain", "id", id ) )     // uloz zaznam
                                {
                                  // zmena registratora
                                  PQsql.UPDATE( "DOMAIN" );
                                  PQsql.SET( "TrDate", "now" );
                                  PQsql.SET( "ClID", regID );
                                  PQsql.WHEREID( id );
                                  if( PQsql.EXEC() ) ret->errCode = COMMAND_OK;  // nastavit OK                                  
                                  else ret->errCode = COMMAND_FAILED;
                                }
                            }

                        }

                    }

                }
              // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
              if( ret->errCode == COMMAND_OK ) PQsql.CommitTransaction();    // pokud uspesne nainsertovalo
              else PQsql.RollbackTransaction();
            }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( PQsql.EndAction( ret->errCode ) );
        }


      ret->errMsg = CORBA::string_dup( PQsql.GetErrorMessage( ret->errCode ) );

      PQsql.Disconnect();
    }

if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

return ret;
}

