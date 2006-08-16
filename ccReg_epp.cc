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
#include "dbsql.h"

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
ccReg_EPP_i::ccReg_EPP_i(ccReg::Admin_ptr _admin) : admin(_admin) {

}
ccReg_EPP_i::~ccReg_EPP_i(){
 
}

ccReg::Admin_ptr 
ccReg_EPP_i::getAdmin()
{
  return ccReg::Admin::_duplicate(admin);
}

// test spojeni na databazi
bool ccReg_EPP_i::TestDatabaseConnect(char *db)
{
DB  DBsql;

// zkopiruj pri vytvoreni instance
strcpy( database , db ); // retezec na  pripojeni k Postgres SQL

if(  DBsql.OpenDatabase( database ) )
{
LOG( NOTICE_LOG ,  "succefuly connect to:  [%s]" , database );
DBsql.Disconnect();
return true;
}
else
{
LOG( ERROR_LOG , "can not connect to database: [%s]" , database );
return false;
}

}

//   Methods corresponding to IDL attributes and operations
char* ccReg_EPP_i::version()
{
char *version;
// char str[64];
version =  new char[128];

sprintf( version , "SVN %s BUILD %s %s" , SVERSION , __DATE__ , __TIME__ );
LOG( NOTICE_LOG , "get version %s" , version );

return version;
}


// DISCLOSE
//  podpora prace s  disclose parametrem pres enum definici
ccReg::Disclose ccReg_EPP_i::get_DISCLOSE( bool d )
{
if( d ) return ccReg::DISCL_DISPLAY;
else return ccReg::DISCL_HIDE;
}

char ccReg_EPP_i::set_DISCLOSE(  ccReg::Disclose d )
{
switch( d )
{
case ccReg::DISCL_DISPLAY:
    return 't' ;
case ccReg::DISCL_HIDE:
    return 'f' ;
default:  
    return ' ' ;
}

}


bool ccReg_EPP_i::test_DISCLOSE(  ccReg::Disclose d )
{
if( d == ccReg::DISCL_DISPLAY ) return true;
else return false;
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
DB DBsql;
ccReg::Response * ret;
ret = new ccReg::Response;

// default
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "GetTransaction: clientID -> %d clTRID [%s] ", clientID, clTRID );

  if( DBsql.OpenDatabase( database ) )
    {
      if( errCode > 0 )
        {
          if( DBsql.BeginAction( clientID, EPP_UnknowAction,  clTRID , "" ) )
            {
              // chybove hlaseni bere z clienta 
              ret->errCode = errCode;
              // zapis na konec action
              // zapis na konec action
              ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
              ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

              LOG( NOTICE_LOG, "GetTransaction: svTRID [%s] errCode -> %d msg [%s] ", ( char * ) ret->svTRID, ret->errCode, ( char * ) ret->errMsg );
            }
          else 
           {
              ret->errCode = errCode;
             ret->svTRID = CORBA::string_dup( "NULL");
            ret->errMsg =   CORBA::string_dup( "ERROR" );
                 LOG(  ERROR_LOG , "GetTransaction: error BeginAction");          
           }
        }

      DBsql.Disconnect();
    }

  if( ret->errCode == 0 )
    {
      LOG( LOG_DEBUG, "GetTransaction: error empty");
      ret->errCode = 0;         // obecna chyba
      ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
      ret->errMsg = CORBA::string_dup( "" );
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


ccReg::Response * ccReg_EPP_i::PollAcknowledgement( CORBA::Long msgID, CORBA::Short & count, CORBA::Long & newmsgID, CORBA::Long clientID, const char *clTRID , const char* XML )
{
DB DBsql;
ccReg::Response * ret;
char sqlString[1024];
int regID, rows;

ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );
count = 0;
newmsgID = 0;

LOG( NOTICE_LOG, "PollAcknowledgement: clientID -> %d clTRID [%s] msgID -> %d", clientID, clTRID, msgID );

  if( DBsql.OpenDatabase( database ) )
    {

      // get  registrator ID
      regID = DBsql.GetLoginRegistrarID( clientID );

      if( DBsql.BeginAction( clientID, EPP_PollAcknowledgement, clTRID , XML ) )
        {

          // test msg ID and clientID
          sprintf( sqlString, "SELECT * FROM MESSAGE WHERE id=%d AND clID=%d;", msgID  , regID );
          rows = 0;
          if( DBsql.ExecSelect( sqlString ) )
            {
              rows = DBsql.GetSelectRows();
              if( rows == 0 )
                {
                  LOG( ERROR_LOG, "unknow msgID %d", msgID );
                  ret->errors.length( 1 );
                  ret->errCode = COMMAND_PARAMETR_ERROR;
                  ret->errors[0].code = ccReg::pollAck_msgID;   // spatna msg ID
                  ret->errors[0].value <<= msgID;
                  ret->errors[0].reason = CORBA::string_dup( "unknow msgID" );
                }
              DBsql.FreeSelect();
            }
          else
            ret->errCode = COMMAND_FAILED;

          if( rows == 1 )       // pokud tam ta zprava existuje
            {
              // oznac zpravu jako prectenou  
              sprintf( sqlString, "UPDATE MESSAGE SET seen='t' WHERE id=%d AND clID=%d;", msgID, regID );

              if( DBsql.ExecSQL( sqlString ) )
                {
                  // zjisteni dalsi ID zpravy ve fronte
                  sprintf( sqlString, "SELECT id  FROM MESSAGE  WHERE clID=%d AND seen='f' AND exDate > 'now()' ;", regID );
                  if( DBsql.ExecSelect( sqlString ) )
                    {
                      ret->errCode = COMMAND_OK; // prikaz splnen
                      rows = DBsql.GetSelectRows();   // pocet zprav
                      if( rows > 0 )    // pokud jsou nejake zpravy ve fronte
                        {
                          count = rows; // pocet dalsich zprav
                          newmsgID = atoi( DBsql.GetFieldValue( 0, 0 ) );
                          LOG( NOTICE_LOG, "PollAcknowledgement: newmsgID -> %d count -> %d", newmsgID, count );
                        }

                      DBsql.FreeSelect();
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
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }
      else
        ret->errCode = COMMAND_FAILED;

      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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

ccReg::Response * ccReg_EPP_i::PollRequest( CORBA::Long & msgID, CORBA::Short & count, ccReg::timestamp & qDate, CORBA::String_out mesg, CORBA::Long clientID, const char *clTRID , const char* XML )
{
DB DBsql;
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

  if( DBsql.OpenDatabase( database ) )
    {

      // get  registrator ID
      regID = DBsql.GetLoginRegistrarID( clientID );


      if( DBsql.BeginAction( clientID, EPP_PollAcknowledgement,  clTRID , XML ) )
        {

          // vypsani zprav z fronty
          sprintf( sqlString, "SELECT *  FROM MESSAGE  WHERE clID=%d AND seen='f' AND exDate > 'now()' ;", regID );

          if( DBsql.ExecSelect( sqlString ) )
            {
              rows = DBsql.GetSelectRows();   // pocet zprav 

              if( rows > 0 )    // pokud jsou nejake zpravy ve fronte
                {
                  count = rows;
                  qDate = get_time_t( DBsql.GetFieldValueName( "CrDate", 0 ) );
                  msgID = atoi( DBsql.GetFieldValueName( "ID", 0 ) );
                  mesg = CORBA::string_dup( DBsql.GetFieldValueName( "message", 0 ) );
                  ret->errCode = COMMAND_ACK_MESG;      // zpravy jsou ve fronte
                  LOG( NOTICE_LOG, "PollRequest: msgID -> %d count -> %d mesg [%s]", msgID, count, CORBA::string_dup( mesg ) );
                }
              else
                ret->errCode = COMMAND_NO_MESG; // zadne zpravy ve fronte

              DBsql.FreeSelect();
            }
          else
            ret->errCode = COMMAND_FAILED;

          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );

        }
      else
        ret->errCode = COMMAND_FAILED;


      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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

ccReg::Response * ccReg_EPP_i::ClientLogout( CORBA::Long clientID, const char *clTRID , const char* XML )
{
DB DBsql;
ccReg::Response * ret;


ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "ClientLogout: clientID -> %d clTRID [%s]", clientID, clTRID );


  if( DBsql.OpenDatabase( database ) )
    {
      if( DBsql.BeginAction( clientID, EPP_ClientLogout, clTRID , XML  ) )
        {

           DBsql.UPDATE( "Login" );
           DBsql.SET( "logoutdate" , "now" );
           DBsql.SET( "logouttrid" , clTRID );
           DBsql.WHEREID( clientID );   


          if( DBsql.EXEC() )  ret->errCode = COMMAND_LOGOUT;      // uspesne oddlaseni
          else ret->errCode = COMMAND_FAILED;


          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }
      else
        ret->errCode = COMMAND_FAILED;


      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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


ccReg::Response * ccReg_EPP_i::ClientLogin( const char *ClID, const char *passwd, const char *newpass, 
                                            const char *clTRID, const char* XML, 
                                            CORBA::Long & clientID, const char *certID, ccReg::Languages lang )
{
DB DBsql;
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


if( DBsql.OpenDatabase( database ) )
  {

    if( DBsql.BeginTransaction() )
      {

        // dotaz na ID registratora 
        if( ( roid = DBsql.GetNumericFromTable( "REGISTRAR", "id", "handle", ( char * ) ClID ) ) == 0 )
          {
            LOG( NOTICE_LOG, "bad username [%s]", ClID ); // spatne username 
            ret->errCode = COMMAND_AUTH_ERROR;
          }
        else
          {


             // ziskej heslo z databaza a  provnej heslo a pokud neni spravne vyhod chybu
            if( strcmp(  DBsql.GetValueFromTable("REGISTRARACL" , "password" , "registrarid" ,  roid )  , passwd )  )
              {
                LOG( NOTICE_LOG, "bad password [%s] not accept", passwd );
                ret->errCode = COMMAND_AUTH_ERROR;
              }
            else
             //  porovnej certifika 
            {
             if( strcmp(  DBsql.GetValueFromTable("REGISTRARACL" , "cert" ,  "registrarid" ,  roid )  ,  certID )  )
              {
                LOG( NOTICE_LOG, "bad certID [%s] not accept", certID );
                ret->errCode = COMMAND_AUTH_ERROR;
               }
            else
             {
                LOG( NOTICE_LOG, "password [%s] accept", passwd );

                // zmena hesla pokud je nejake nastaveno
                if( strlen( newpass ) )
                  {
                    LOG( NOTICE_LOG, "change password  [%s]  to  newpass [%s] ", passwd, newpass );


                    DBsql.UPDATE( "REGISTRARACL" );
                    DBsql.SET( "password" , newpass );         
                    DBsql.WHERE( "registrarid" , roid  );

                    if( DBsql.EXEC() == false )
                      {
                        ret->errCode = COMMAND_FAILED;
                        change = false;
                      }
                  }

                if( change ) // pokud projde zmena defaulte je nastava true
                  {
                    id = DBsql.GetSequenceID( "login" ); // ziskani id jako sequence

                    // zapis do logovaci tabulky 

                    DBsql.INSERT( "Login" );
                    DBsql.INTO( "id" );
                    DBsql.INTO( "registrarid" );
                    DBsql.INTO( "logintrid" );
                    DBsql.VALUE( id );
                    DBsql.VALUE( roid );
                    DBsql.VALUE( clTRID );
   
                    if( DBsql.EXEC() )    // pokud se podarilo zapsat do tabulky
                      {
                        clientID = id;
                        LOG( NOTICE_LOG, "GET clientID  -> %d", clientID );

                        ret->errCode = COMMAND_OK; // zikano clinetID OK

                        // nankonec zmena komunikacniho  jazyka pouze na cestinu
                        if( lang == ccReg::CS )
                          {
                            LOG( NOTICE_LOG, "SET LANG to CS" ); 

                            DBsql.UPDATE( "Login" );
                            DBsql.SET( "lang" , "cs"  );
                            DBsql.WHEREID( clientID  );

                            if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;    // pokud se nezdarilo
                          }

                      }
                    else ret->errCode = COMMAND_FAILED;

                  }

              }
            }
          }

        // probehne action pro svrTrID   musi byt az na mkonci pokud zna clientID
        if( DBsql.BeginAction( clientID, EPP_ClientLogin, clTRID , XML ) )
          {
            // zapis na konec action
            ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
          }


        ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

        // konec transakce commit ci rollback
        DBsql.QuitTransaction( ret->errCode );

      }

    DBsql.Disconnect();
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


ccReg::Response* ccReg_EPP_i::ObjectCheck( short act , char * table , char *fname , const ccReg::Check& chck , ccReg::CheckResp_out  a, CORBA::Long clientID, const char* clTRID , const char* XML )
{
DB DBsql;
ccReg::Response *ret;
int  len , av ;
char HANDLE[64] , FQDN[64];
long unsigned int i;
ret = new ccReg::Response;

a = new ccReg::CheckResp;


ret->errCode=0;
ret->errors.length(0);

len = chck.length();
a->length(len);

LOG( NOTICE_LOG ,  "OBJECT %d  Check: clientID -> %d clTRID [%s] " , act  , clientID , clTRID );
 

if( DBsql.OpenDatabase( database ) )
{

  if( DBsql.BeginAction( clientID , act ,  clTRID , XML  ) )
  {
 
    for( i = 0 ; i < len ; i ++ )
     { 
      switch(act)
            {
                  case EPP_ContactCheck:
                  case EPP_NSsetCheck:
                       if( get_HANDLE( HANDLE ,  chck[i] ) ==  false )
                         {
                            a[i].avail = ccReg::BadFormat;    // spatny format
                            a[i].reason =  CORBA::string_dup( "bad format  of handle" );

                         }
                        break;
                  case EPP_DomainCheck:
                       if( get_FQDN( FQDN ,  chck[i] ) ==  false )
                         {
                            a[i].avail = ccReg::BadFormat;    // spatny format
                            a[i].reason =  CORBA::string_dup( "bad format  of domain" );
                         }
                        break;
            
            }
     
      if(   a[i].avail != ccReg::BadFormat )
      {
      switch( DBsql.CheckObject( table , fname , chck[i] ) )
           {
             case 1:
                       a[i].avail = ccReg::Exist;    // objekt existuje
                       a[i].reason =  CORBA::string_dup( "object exist not Avail" );
                       LOG( NOTICE_LOG ,  "object %s exist not Avail" , (const char * ) chck[i] );
                       break;
             case 0:
                       a[i].avail =  ccReg::NotExist;    // objekt ne existuje
                       a[i].reason =  CORBA::string_dup( "");  // free
                       LOG( NOTICE_LOG ,  "object %s not exist  Avail" ,(const char * ) chck[i] );
                       break; 
             default: // error
                      ret->errCode=COMMAND_FAILED;
                      break;             
            }
      }
  
     }
    
     
      // comand OK
     if( ret->errCode == 0 ) ret->errCode=COMMAND_OK; // vse proslo OK zadna chyba 

      // zapis na konec action
      ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) ) ;    
  }

ret->errMsg =  CORBA::string_dup(   DBsql.GetErrorMessage(  ret->errCode  ) ) ;
 
DBsql.Disconnect();
}


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;    // obecna chyba
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }


 
return ret;
}


ccReg::Response* ccReg_EPP_i::ContactCheck(const ccReg::Check& handle, ccReg::CheckResp_out  a, CORBA::Long clientID, const char* clTRID , const char* XML )
{
return ObjectCheck( EPP_ContactCheck , "CONTACT"  , "handle" , handle , a , clientID , clTRID , XML);
}

ccReg::Response* ccReg_EPP_i::NSSetCheck(const ccReg::Check& handle, ccReg::CheckResp_out  a, CORBA::Long clientID, const char* clTRID , const char* XML )
{
return ObjectCheck( EPP_NSsetCheck ,  "NSSET"  , "handle" , handle , a ,  clientID , clTRID , XML);
}


ccReg::Response*  ccReg_EPP_i::DomainCheck(const ccReg::Check& fqdn, ccReg::CheckResp_out  a, CORBA::Long clientID, const char* clTRID , const char* XML )
{
return ObjectCheck(  EPP_DomainCheck , "DOMAIN"  , "fqdn" ,   fqdn , a ,  clientID , clTRID , XML);
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

ccReg::Response* ccReg_EPP_i::ContactInfo(const char* handle, ccReg::Contact_out c , CORBA::Long clientID, const char* clTRID , const char* XML )
{
DB DBsql;
Status status;
ccReg::Response *ret;
char HANDLE[64]; // handle na velka pismena
int id , clid , crid , upid , regID;
int actionID=0 , ssn ;
int len , i  , s ;

c = new ccReg::Contact;
ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ContactInfo: clientID -> %d clTRID [%s] handle [%s] " , clientID , clTRID , handle );
 
if( DBsql.OpenDatabase( database ) )
{

  
if( DBsql.BeginAction( clientID , EPP_ContactInfo ,  clTRID  , XML ) )
  {
   // get  registrator ID
   regID = DBsql.GetLoginRegistrarID( clientID );
 
 if( get_HANDLE( HANDLE , handle ) ) // preved a otestuj na velka pismena
 {
  if( DBsql.SELECTONE( "CONTACT" , "HANDLE" , HANDLE )  )
  {
  if( DBsql.GetSelectRows() == 1 )
    {

        clid =  atoi( DBsql.GetFieldValueName("ClID" , 0 ) ); 
        crid =  atoi( DBsql.GetFieldValueName("CrID" , 0 ) ); 
        upid =  atoi( DBsql.GetFieldValueName("UpID" , 0 ) ); 



        status.Make(  DBsql.GetFieldValueName("status" , 0 ) ) ; // status


	c->handle=CORBA::string_dup( DBsql.GetFieldValueName("handle" , 0 ) ); // handle
	c->ROID=CORBA::string_dup( DBsql.GetFieldValueName("ROID" , 0 ) ); // ROID     
	c->CrDate= get_time_t( DBsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
	c->UpDate= get_time_t( DBsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
	c->TrDate= get_time_t( DBsql.GetFieldValueName("TrDate" , 0 ) ); // datum a cas transferu
	c->Name=CORBA::string_dup( DBsql.GetFieldValueName("Name" , 0 )  ); // jmeno nebo nazev kontaktu
	c->Organization=CORBA::string_dup( DBsql.GetFieldValueName("Organization" , 0 )); // nazev organizace
	c->Street1=CORBA::string_dup( DBsql.GetFieldValueName("Street1" , 0 ) ); // adresa
	c->Street2=CORBA::string_dup( DBsql.GetFieldValueName("Street2" , 0 ) ); // adresa

	c->Street3=CORBA::string_dup( DBsql.GetFieldValueName("Street3" , 0 ) ); // adresa
	c->City=CORBA::string_dup( DBsql.GetFieldValueName("City" , 0 ) );  // obec
	c->StateOrProvince=CORBA::string_dup( DBsql.GetFieldValueName("StateOrProvince"  , 0 ));
	c->PostalCode=CORBA::string_dup(DBsql.GetFieldValueName("PostalCode" , 0 )); // PSC
	c->Telephone=CORBA::string_dup( DBsql.GetFieldValueName("Telephone" , 0 ));
	c->Fax=CORBA::string_dup(DBsql.GetFieldValueName("Fax" , 0 ));
	c->Email=CORBA::string_dup(DBsql.GetFieldValueName("Email" , 0 ));
	c->NotifyEmail=CORBA::string_dup(DBsql.GetFieldValueName("NotifyEmail" , 0 )); // upozornovaci email
  //      strncpy( countryCode ,  DBsql.GetFieldValueName("Country" , 0 ) , 2 ); // 2 mistny ISO kod zeme
    //    countryCode[2] = 0;
        c->CountryCode=CORBA::string_dup(  DBsql.GetFieldValueName("Country" , 0 )  ); // vracet pouze ISO kod


	c->VAT=CORBA::string_dup(DBsql.GetFieldValueName("VAT" , 0 )); // DIC
	c->SSN=CORBA::string_dup(DBsql.GetFieldValueName("SSN" , 0 )); // SSN

        ssn =   atoi( DBsql.GetFieldValueName("SSNtype" , 0 ) );



        if( regID == clid ) // pokud je registrator clientem obdrzi autentifikaci
           c->AuthInfoPw = CORBA::string_dup( DBsql.GetFieldValueName("AuthInfoPw" , 0 ) ); // autentifikace
         else  c->AuthInfoPw = CORBA::string_dup( "" ); // jinak prazdny retezec


        c->DiscloseName = get_DISCLOSE(  DBsql.GetFieldBooleanValueName( "DiscloseName" , 0 ) );
        c->DiscloseOrganization =  get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseOrganization" , 0 ) );
        c->DiscloseAddress =get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseAddress" , 0 ) );
        c->DiscloseTelephone = get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseTelephone" , 0 ) );
        c->DiscloseFax  = get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseFax" , 0 ) );
        c->DiscloseEmail = get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseEmail" , 0  ) );


    
    
        // free select
	DBsql.FreeSelect();


        // zpracuj pole statusu
        len =  status.Length();
        c->stat.length(len);

        for( i = 0 ; i < len  ; i ++)
           {
              c->stat[i] = CORBA::string_dup( status.GetStatusString(  status.Get(i)  ) );
           }

              
        // identifikator registratora
        c->CrID =  CORBA::string_dup(  DBsql.GetRegistrarHandle( crid ) );
        c->UpID =  CORBA::string_dup(  DBsql.GetRegistrarHandle( upid ) );
        c->ClID =  CORBA::string_dup(  DBsql.GetRegistrarHandle( clid ) );


        // type SSN EMPTY , RC , OP , PASS , ICO

        switch( ssn )
        {
         case 1:
                  c->SSNtype = ccReg::RC;
                  break;
         case 2:
                  c->SSNtype = ccReg::OP;
                  break;
         case 3:
                  c->SSNtype = ccReg::PASS;
                  break;
         case 4:
                  c->SSNtype = ccReg::ICO;
                  break;
         case 5:
                  c->SSNtype = ccReg::MPSV;
                  break;
         default:
                 c->SSNtype = ccReg::EMPTY;
                  break;
        }
        ret->errCode=COMMAND_OK; // VASE OK


/*
         // kod zeme cesky
        if( DBsql.GetClientLanguage() == LANG_CS ) c->Country=CORBA::string_dup( DBsql.GetCountryNameCS( countryCode ) );
	else c->Country=CORBA::string_dup( DBsql.GetCountryNameEN( countryCode ) );
*/

     }
    else 
     {
      DBsql.FreeSelect();
      LOG( WARNING_LOG  ,  "object handle [%s] NOT_EXIST" , handle );
      ret->errCode =  COMMAND_OBJECT_NOT_EXIST;
     }    

   }

  }
 else
  {
            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "bad format of contact [%s]" , handle );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::contactInfo_handle;
            ret->errors[0].value <<= CORBA::string_dup( handle );
            ret->errors[0].reason = CORBA::string_dup( "bad format contact handle" );
  }

 
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) ) ;

}

ret->errMsg =  CORBA::string_dup(   DBsql.GetErrorMessage(  ret->errCode  ) );

DBsql.Disconnect();
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
c->ClID=CORBA::string_dup(""); 
c->CrDate=0; // datum a cas vytvoreni
c->UpDate=0; // datum a cas zmeny
c->TrDate=0; // dattum a cas transferu
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
c->SSNtype =  ccReg::EMPTY ;
c->AuthInfoPw=CORBA::string_dup(""); // heslo
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


ccReg::Response* ccReg_EPP_i::ContactDelete(const char* handle , CORBA::Long clientID, const char* clTRID , const char* XML )
{
ccReg::Response *ret;
DB DBsql;
Status status;
char HANDLE[64];
int regID , id ,  clID ;

ret = new ccReg::Response;


ret->errCode=COMMAND_FAILED;
ret->svTRID = CORBA::string_alloc(32); //  server transaction
ret->svTRID = CORBA::string_dup(""); // prazdna hodnota
ret->errMsg = CORBA::string_alloc(64);
ret->errMsg = CORBA::string_dup("");
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ContactDelete: clientID -> %d clTRID [%s] handle [%s] " , clientID , clTRID , handle );



  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_ContactDelete,  clTRID , XML ) )
        {

       // preved handle na velka pismena
       if( get_HANDLE( HANDLE , handle )  )  
         {
          if( DBsql.BeginTransaction() )
            {

              if( ( id = DBsql.GetNumericFromTable( "CONTACT", "id", "handle", ( char * ) HANDLE ) ) == 0 )
                {
                  LOG( WARNING_LOG, "object handle [%s] NOT_EXIST", handle );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;      // pokud objekt neexistuje
                }
              else 
                {
                  // get  registrator ID
                  regID = DBsql.GetLoginRegistrarID( clientID );
                  // klient kontaku
                  clID = DBsql.GetNumericFromTable( "CONTACT", "ClID", "id", id );



                  if( regID != clID )   // pokud neni klientem
                    {
                      LOG( WARNING_LOG, "bad autorization not  creator of handle [%s]", handle );
                      ret->errCode = COMMAND_AUTOR_ERROR; // spatna autorizace
                    }                               
                  else                                                                                           
                    {
                      // zpracuj  pole statusu
                      status.Make( DBsql.GetStatusFromTable( "CONTACT", id ) );

                      if( status.Test( STATUS_DELETE ) )
                        {
                          LOG( WARNING_LOG, "status DeleteProhibited" );
                          ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                        }
                      else // status je OK
                        {
                          // test na vazbu do tabulky domain domain_contact_map a nsset_contact_map
                          if( DBsql.TestContactRelations( id ) )        // kontakt nemuze byt smazan ma vazby  
                            {
                              LOG( WARNING_LOG, "test contact handle [%s] relations: PROHIBITS_OPERATION", handle );
                              ret->errCode = COMMAND_PROHIBITS_OPERATION;
                            }
                          else
                            {
                              //  uloz do historie
                              if( DBsql.MakeHistory() )
                                {
                                  if( DBsql.SaveHistory( "Contact", "id", id ) ) // uloz zaznam
                                    {
                                      if( DBsql.DeleteFromTable( "CONTACT", "id", id ) ) ret->errCode = COMMAND_OK;      // pokud usmesne smazal
                                    }
                                }


                            }

                        }

                    }


                }

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }

          }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }


      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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
                                              CORBA::Long clientID, const char *clTRID , const char* XML )
{
ccReg::Response * ret;
DB DBsql;
char statusString[128] , HANDLE[64];
int regID = 0,  clID = 0, id , num ;
bool remove_update_flag = false ;
int len, i , seq;
Status status;

seq=0;
ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "ContactUpdate: clientID -> %d clTRID [%s] handle [%s] ", clientID, clTRID, handle );

// nacti status flagy
  for( i = 0; i < status_add.length(); i++ )
    {
      LOG( NOTICE_LOG, "status_add [%s] -> %d",  (const char *)  status_add[i]  ,  status.GetStatusNumber( status_add[i] )  ); 
      status.PutAdd( status.GetStatusNumber( status_add[i] ) ); // pridej status flag
    }

  for( i = 0; i < status_rem.length(); i++ )
    {
      LOG( NOTICE_LOG, "status_rem [%s] -> %d ",   (const char *) status_rem[i]  , status.GetStatusNumber( status_rem[i] )  );
      status.PutRem(  status.GetStatusNumber( status_rem[i] ) );       
      if( status.GetStatusNumber( status_rem[i] ) ==  STATUS_clientUpdateProhibited ) 
        {
           LOG( NOTICE_LOG, "remove STATUS_clientUpdateProhibited");
           remove_update_flag=true;          
        }
    }




  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_ContactUpdate, clTRID , XML ) )
        {

       // preved handle na velka pismena
       if( get_HANDLE( HANDLE , handle )  )  
         {


          if( DBsql.BeginTransaction() )      // zahajeni transakce
            {
              if( ( id = DBsql.GetNumericFromTable( "CONTACT", "id", "handle", ( char * ) HANDLE ) ) == 0 )
                {
                  LOG( WARNING_LOG, "object handle [%s] NOT_EXIST", handle );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;
                }
              // pokud kontakt existuje 
              else
                {
                  // get  registrator ID
                  regID = DBsql.GetLoginRegistrarID( clientID );
                  // client contaktu
                  clID = DBsql.GetNumericFromTable( "CONTACT", "clID", "id", id );

                  if( clID != regID )
                    {
                        LOG( WARNING_LOG, "bad autorization not  client of contact [%s]", handle );
                        ret->errCode = COMMAND_AUTOR_ERROR;     // spatna autorizace
                    }
                  else
                   {

                      if( DBsql.TestCountryCode( c.CC ) )       // test kodu zeme pokud je nastavena
                        {

                          // zjisti  pole statusu
                          status.Make( DBsql.GetStatusFromTable( "CONTACT", id ) );


                           
                          // neplati kdyz je UpdateProhibited v remove  status flagu
                          if( status.Test( STATUS_UPDATE ) && remove_update_flag == false )
                            {
                              LOG( WARNING_LOG, "status UpdateProhibited" );
                              ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                            }
                          else  // status je OK
                            {
                               
                                  //  uloz do historie
                                  if( DBsql.MakeHistory() )
                                    {
                                      if( DBsql.SaveHistory( "Contact", "id", id ) )    // uloz zaznam
                                        {

                                          // pridany status
                                          for( i = 0; i < status.AddLength(); i++ )
                                            {
                                                  if( status.Add(i) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::contactUpdate_status_add;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_add[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup( "can not add status flag" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                    }
                                            }

                                          // zruseny status flagy
                                         for( i = 0; i < status.RemLength(); i++ )
                                             {
                                              
                                                  if( status.Rem(i) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::contactUpdate_status_rem;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_rem[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup( "can not remove status flag" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                    }
                                            }


                                         if( ret->errCode == 0 )
                                         {


                                          // zahaj update
                                          DBsql.UPDATE( "Contact" );

                                          // pridat zmenene polozky 
                                          if( remove_update_flag == false )
                                          {
                                          DBsql.SET( "Name", c.Name );
                                          DBsql.SET( "Organization", c.Organization );
                                          DBsql.SET( "Street1", c.Street1 );
                                          DBsql.SET( "Street2", c.Street2 );
                                          DBsql.SET( "Street3", c.Street3 );
                                          DBsql.SET( "City", c.City );
                                          DBsql.SET( "StateOrProvince", c.StateOrProvince );
                                          DBsql.SET( "PostalCode", c.PostalCode );
                                          DBsql.SET( "Country", c.CC );
                                          DBsql.SET( "Telephone", c.Telephone );
                                          DBsql.SET( "Fax", c.Fax );
                                          DBsql.SET( "Email", c.Email );
                                          DBsql.SET( "NotifyEmail", c.NotifyEmail );
                                          DBsql.SET( "VAT", c.VAT );
                                          DBsql.SET( "SSN", c.SSN );
                                          if(  c.SSNtype > ccReg::EMPTY )  DBsql.SET( "SSNtype" , c.SSNtype ); // typ ssn
                                          // heslo
                                          DBsql.SET( "AuthInfoPw", c.AuthInfoPw ); 


                                          //  Disclose parametry
                                          DBsql.SETBOOL( "DiscloseName", set_DISCLOSE( c.DiscloseName) );
                                          DBsql.SETBOOL( "DiscloseOrganization", set_DISCLOSE( c.DiscloseOrganization ) );
                                          DBsql.SETBOOL( "DiscloseAddress", set_DISCLOSE(  c.DiscloseAddress ) );
                                          DBsql.SETBOOL( "DiscloseTelephone",  set_DISCLOSE(  c.DiscloseTelephone ) );
                                          DBsql.SETBOOL( "DiscloseFax",  set_DISCLOSE( c.DiscloseFax ) );
                                          DBsql.SETBOOL( "DiscloseEmail", set_DISCLOSE( c.DiscloseEmail ) );
                                          }
                                          // datum a cas updatu  plus kdo zmenil zanzma na konec
                                          DBsql.SET( "UpDate", "now" );
                                          DBsql.SET( "UpID", regID );

                                          //  vygeneruj  novy status string array
                                          status.Array( statusString );
                                          DBsql.SET( "status", statusString );

                                          // podminka na konec 
                                          DBsql.WHEREID( id );

                                          if( DBsql.EXEC() ) ret->errCode = COMMAND_OK;
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
                          ret->errors[0].value <<= CORBA::string_dup( c.CC );
                          ret->errors[0].reason = CORBA::string_dup( "unknow country code" );
                        }

                    }


                }

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }

          }

          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }


      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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
                                              ccReg::timestamp & crDate, CORBA::Long clientID, const char *clTRID , const char* XML )
{
DB DBsql;
char createDate[32];
char roid[64];
char HANDLE[64]; // handle na velka pismena
ccReg::Response * ret;
int regID, id;
int i , len;
time_t now;
// default
ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

crDate = 0;


LOG( NOTICE_LOG, "ContactCreate: clientID -> %d clTRID [%s] handle [%s]", clientID, clTRID, handle );
LOG( NOTICE_LOG, "ContactCreate: Disclose Name %d Org %d Add %d Tel %d Fax %d Email %d" ,
 c.DiscloseName  , c.DiscloseOrganization , c.DiscloseAddress , c.DiscloseTelephone , c.DiscloseFax , c.DiscloseEmail );


  if( DBsql.OpenDatabase( database ) )
    {
      if( DBsql.BeginAction( clientID, EPP_ContactCreate,  clTRID ,  XML ) )
        {

       // preved handle na velka pismena
       if( get_CONTACTHANDLE( HANDLE , handle ) == false )  // spatny format handlu
         {

            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "bad format  of handle[%s]" , handle );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::contactCreate_handle; 
            ret->errors[0].value <<= CORBA::string_dup( handle );
            ret->errors[0].reason = CORBA::string_dup( "bad format of handle use CID:CONTACT" );
        }
        else 
        {
          if( DBsql.BeginTransaction() )      // zahajeni transakce
            {
              // test zdali kontakt uz existuje
              if( DBsql.CheckObject( "CONTACT",  "handle", handle  ) )
                {
                  LOG( WARNING_LOG, "object handle [%s] EXIST", handle  );
                  ret->errCode = COMMAND_OBJECT_EXIST;  // je uz zalozena
                }
              else              // pokud kontakt nexistuje
                {
                  // test zdali country code je existujici zeme
                  if( DBsql.TestCountryCode( c.CC ) )
                    {
                      // datum vytvoreni kontaktu
                      now = time( NULL );
                      crDate = now;
                      get_timestamp( now, createDate );

                      id = DBsql.GetSequenceID( "contact" );
                      // get  registrator ID
                      regID = DBsql.GetLoginRegistrarID( clientID );

                      // vytvor roid kontaktu
                      get_roid( roid, "C", id );

                      DBsql.INSERT( "CONTACT" );
                      DBsql.INTO( "id" );
                      DBsql.INTO( "roid" );
                      DBsql.INTO( "handle" );
                      DBsql.INTO( "CrDate" );
                      DBsql.INTO( "CrID" );
                      DBsql.INTO( "ClID" );
                      DBsql.INTO( "status" );

                      DBsql.INTOVAL( "Name", c.Name );
                      DBsql.INTOVAL( "Organization", c.Organization );
                      DBsql.INTOVAL( "Street1", c.Street1 );
                      DBsql.INTOVAL( "Street2", c.Street2 );
                      DBsql.INTOVAL( "Street3", c.Street3 );
                      DBsql.INTOVAL( "City", c.City );
                      DBsql.INTOVAL( "StateOrProvince", c.StateOrProvince );
                      DBsql.INTOVAL( "PostalCode", c.PostalCode );
                      DBsql.INTOVAL( "Country", c.CC );
                      DBsql.INTOVAL( "Telephone", c.Telephone );
                      DBsql.INTOVAL( "Fax", c.Fax );
                      DBsql.INTOVAL( "Email", c.Email );
                      DBsql.INTOVAL( "NotifyEmail", c.NotifyEmail );
                      DBsql.INTOVAL( "VAT", c.VAT );
                      DBsql.INTOVAL( "SSN", c.SSN );
                      if(  c.SSNtype > ccReg::EMPTY ) DBsql.INTO( "SSNtype");
                      DBsql.INTOVAL( "AuthInfoPw", c.AuthInfoPw );


                      if( c.DiscloseName ==  ccReg::DISCL_DISPLAY ) DBsql.INTO( "DiscloseName" );
                      if( c.DiscloseOrganization == ccReg::DISCL_DISPLAY  ) DBsql.INTO( "DiscloseOrganization" );
                      if( c.DiscloseAddress == ccReg::DISCL_DISPLAY ) DBsql.INTO( "DiscloseAddress" );
                      if( c.DiscloseTelephone == ccReg::DISCL_DISPLAY ) DBsql.INTO( "DiscloseTelephone" );
                      if( c.DiscloseFax == ccReg::DISCL_DISPLAY ) DBsql.INTO( "DiscloseFax" );
                      if( c.DiscloseEmail == ccReg::DISCL_DISPLAY )DBsql.INTO( "DiscloseEmail" );

                      DBsql.VALUE( id );
                      DBsql.VALUE( roid );
                      DBsql.VALUE( HANDLE );
                      DBsql.VALUE( createDate );
                      DBsql.VALUE( regID );
                      DBsql.VALUE( regID );
                      DBsql.VALUE( "{ 1 }" );   // OK status


                      DBsql.VAL( c.Name );
                      DBsql.VAL( c.Organization );
                      DBsql.VAL( c.Street1 );
                      DBsql.VAL( c.Street2 );
                      DBsql.VAL( c.Street3 );
                      DBsql.VAL( c.City );
                      DBsql.VAL( c.StateOrProvince );
                      DBsql.VAL( c.PostalCode );
                      DBsql.VAL( c.CC );
                      DBsql.VAL( c.Telephone );
                      DBsql.VAL( c.Fax );
                      DBsql.VAL( c.Email );
                      DBsql.VAL( c.NotifyEmail );
                      DBsql.VAL( c.VAT );
                      DBsql.VAL( c.SSN );
                      if(  c.SSNtype > ccReg::EMPTY ) DBsql.VALUE(   c.SSNtype );
                      DBsql.VAL( c.AuthInfoPw  );


                      if( test_DISCLOSE( c.DiscloseName ) ) DBsql.VALUE( "t" );
                      if( test_DISCLOSE( c.DiscloseOrganization ) ) DBsql.VALUE( "t" );
                      if( test_DISCLOSE( c.DiscloseAddress ) ) DBsql.VALUE( "t" );
                      if( test_DISCLOSE( c.DiscloseTelephone ) ) DBsql.VALUE( "t" );
                      if( test_DISCLOSE( c.DiscloseFax ) ) DBsql.VALUE( "t" );
                      if( test_DISCLOSE( c.DiscloseEmail ) ) DBsql.VALUE( "t" );



                      // pokud se podarilo insertovat
                      if( DBsql.EXEC() )      //   ret->errCode = COMMAND_OK;
                        {       //  uloz do historie
                          if( DBsql.MakeHistory() )
                            {
                              if( DBsql.SaveHistory( "Contact", "id", id ) )    // uloz zaznam
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
                      ret->errors[0].value <<= CORBA::string_dup( c.CC );
                      ret->errors[0].reason = CORBA::string_dup( "unknow country code" );
                    }
                }

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }
          }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );      
      DBsql.Disconnect();
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
 * FUNCTION:    ContactTransfer
 *
 * DESCRIPTION: prevod kontactu od puvodniho na noveho registratora
 *              a ulozeni zmen do historie
 * PARAMETERS:  handle - identifikator kontaktu
 *              authInfo - autentifikace heslem
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 *
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response* ccReg_EPP_i::ContactTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID , const char* XML )
{
ccReg::Response *ret;
DB DBsql;
char HANDLE[64];
Status status;
int regID=0 , clID=0 , id , contactid;

ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ContactTransfer: clientID -> %d clTRID [%s] handle [%s] authInfo [%s] " , clientID , clTRID , handle , authInfo );

if( DBsql.OpenDatabase( database ) )
{

if( DBsql.BeginAction( clientID , EPP_ContactTransfer ,  clTRID , XML  ) )
 {

  // preved handle na velka pismena
  if( get_HANDLE( HANDLE , handle )  )
  {
  if( DBsql.BeginTransaction() )
  {
 
   // pokud domena neexistuje
  if( (id = DBsql.GetNumericFromTable(  "CONTACT"  , "id" , "handle" , (char * ) HANDLE ) ) == 0 ) 
    {
        LOG( WARNING_LOG  ,  "object [%s] NOT_EXIST" ,  handle );
      ret->errCode= COMMAND_OBJECT_NOT_EXIST;
    }
  else
  {
   // get  registrator ID
   regID =   DBsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  DBsql.GetNumericFromTable(  "CONTACT"  , "clID" , "id" , id );



  if( regID == clID )       // transfer nemuze delat stavajici client
    {
      LOG( WARNING_LOG, "client can not transfer contact %s" , handle );
      ret->errCode =  COMMAND_NOT_ELIGIBLE_FOR_TRANSFER;
    }
   else
  {

                  // zpracuj  pole statusu
                  status.Make( DBsql.GetStatusFromTable( "CONTACT", id ) );

                  if( status.Test( STATUS_TRANSFER ) )
                    {
                      LOG( WARNING_LOG, "status TransferProhibited" );
                      ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                    }
                  else
                    {

   if(  DBsql.AuthTable(  "CONTACT"  , (char *)authInfo , id )  == false  ) // pokud prosla autentifikace 
     {       
        LOG( WARNING_LOG , "autorization failed");
        ret->errCode = COMMAND_AUTOR_ERROR; // spatna autorizace
     }
    else
     {
         //  uloz do historie
       if( DBsql.MakeHistory() )
        {
          if( DBsql.SaveHistory( "CONTACT" , "id" , id ) ) // uloz zaznam
           { 


                // zmena registratora
                DBsql.UPDATE( "CONTACT");
                DBsql.SET( "TrDate" , "now" );
                DBsql.SET( "clID" , regID );
                DBsql.WHEREID( id ); 
                if(   DBsql.EXEC() )  ret->errCode = COMMAND_OK; // nastavit OK                                  
                else  ret->errCode = COMMAND_FAILED;
           }

       }
     }
    }
    }
   }
    // konec transakce commit ci rollback
    DBsql.QuitTransaction( ret->errCode );
   }

  }
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode  ) ) ;
}


ret->errMsg =  CORBA::string_dup(   DBsql.GetErrorMessage(  ret->errCode  ) ) ;

DBsql.Disconnect();
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

ccReg::Response* ccReg_EPP_i::NSSetInfo(const char* handle, ccReg::NSSet_out n, 
                          CORBA::Long clientID, const char* clTRID ,  const char* XML)
{
DB DBsql;
Status status;
char HANDLE[64];
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
 


if( DBsql.OpenDatabase( database ) )
{

if( DBsql.BeginAction( clientID , EPP_NSsetInfo , clTRID , XML  ) )
 {


if( get_HANDLE( HANDLE , handle ) ) 
 {

   // get  registrator ID
   regID = DBsql.GetLoginRegistrarID( clientID );

  if(  DBsql.SELECTONE( "NSSET" , "HANDLE" , HANDLE ) )
  {
  if( DBsql.GetSelectRows() == 1 )
    {
 
        nssetid = atoi( DBsql.GetFieldValueName("ID" , 0 ) );
        clid = atoi( DBsql.GetFieldValueName("ClID" , 0 ) );
        crid = atoi( DBsql.GetFieldValueName("CrID" , 0 ) );
        upid = atoi( DBsql.GetFieldValueName("UpID" , 0 ) );

        status.Make(  DBsql.GetFieldValueName("status" , 0 ) ); 

        n->ROID=CORBA::string_dup( DBsql.GetFieldValueName("ROID" , 0 ) ); // ROID
        n->handle=CORBA::string_dup( DBsql.GetFieldValueName("handle" , 0 ) ); // ROID
        n->CrDate= get_time_t( DBsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
        n->UpDate= get_time_t( DBsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
        n->TrDate= get_time_t(DBsql.GetFieldValueName("TrDate" , 0 ) );  // datum a cas transferu

        if( regID == clid ) // pokud je registrator clientem obdrzi autentifikaci
           n->AuthInfoPw = CORBA::string_dup( DBsql.GetFieldValueName("AuthInfoPw" , 0 ) ); // autentifikace
         else  n->AuthInfoPw = CORBA::string_dup( "" ); // jinak prazdny retezec

        ret->errCode=COMMAND_OK;


        // free select
        DBsql.FreeSelect();



        // zpracuj pole statusu
        len =  status.Length();
        n->stat.length(len);
        for( i = 0 ; i <  len  ; i ++)
           {
              n->stat[i] = CORBA::string_dup( status.GetStatusString(  status.Get(i)  ) );
           }


        n->ClID =  CORBA::string_dup( DBsql.GetRegistrarHandle( clid ) );
        n->CrID =  CORBA::string_dup( DBsql.GetRegistrarHandle( upid ) );
        n->UpID =  CORBA::string_dup( DBsql.GetRegistrarHandle( crid ) );

        // dotaz na DNS servry  na tabulky host
        if(   DBsql.SELECTONE( "HOST" , "nssetid" , nssetid  ) )
          {  
             len =  DBsql.GetSelectRows();
            
               
             n->dns.length(len);
 
             for( i = 0 ; i < len ; i ++)   
                {
                     
                   // fqdn DNS servru nazev  
                   n->dns[i].fqdn = CORBA::string_dup(  DBsql.GetFieldValueName("fqdn" , i ) );
 
                   // pole adres
                   strcpy( adres , DBsql.GetFieldValueName("ipaddr" , i ) );

                   // zpracuj pole adres
                   ilen =  get_array_length( adres );
                   n->dns[i].inet.length(ilen); // sequence ip adres
                   for( j = 0 ; j < ilen ; j ++)
                      {
                        get_array_value( adres , adr , j );
                        n->dns[i].inet[j] =CORBA::string_dup( adr );
                      }


                }

             DBsql.FreeSelect();
          } else ret->errCode=COMMAND_FAILED;
 



        // dotaz na technicke kontakty
        if(  DBsql.SELECTCONTACTMAP( "nsset"  , nssetid ) )
          {
               len =  DBsql.GetSelectRows(); // pocet technickych kontaktu
               n->tech.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) n->tech[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );

               DBsql.FreeSelect();
          } else ret->errCode=COMMAND_FAILED;


     }
   else
    {
     // free select
    DBsql.FreeSelect();
    LOG( WARNING_LOG  ,  "object handle [%s] NOT_EXIST" , handle );
    ret->errCode=COMMAND_OBJECT_NOT_EXIST;
    }

   } else ret->errCode=COMMAND_FAILED;


       
  }
 else
  {
            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "bad format of nsset [%s]" , handle );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::nssetInfo_handle;
            ret->errors[0].value <<= CORBA::string_dup( handle );
            ret->errors[0].reason = CORBA::string_dup( "bad format nsset handle" );
  }
 

   // zapis na konec action
   ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode  ) ) ;
 }

ret->errMsg =  CORBA::string_dup(   DBsql.GetErrorMessage(  ret->errCode  ) ) ;

DBsql.Disconnect();
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

ccReg::Response* ccReg_EPP_i::NSSetDelete(const char* handle, CORBA::Long clientID, const char* clTRID , const char* XML )
{
ccReg::Response *ret;
DB DBsql;
Status status;
char HANDLE[64];
int regID , id , clID = 0;
bool stat , del;


ret = new ccReg::Response;


ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetDelete: clientID -> %d clTRID [%s] handle [%s] " , clientID , clTRID , handle );


  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_NSsetDelete, clTRID , XML ) )
        {
       // preved handle na velka pismena
       if( get_HANDLE( HANDLE , handle )  )  
        {
          if( DBsql.BeginTransaction() )      // zahajeni transakce
            {

        


              // pokud NSSET existuje 
              if( ( id = DBsql.GetNumericFromTable( "NSSET", "id", "handle", ( char * ) HANDLE ) ) == 0 )
                {
                  LOG( WARNING_LOG, "object handle [%s] NOT_EXIST", handle );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;      // pokud objekt neexistuje
                }
              else
                {
                  // get  registrator ID
                  regID = DBsql.GetLoginRegistrarID( clientID );
                  clID = DBsql.GetNumericFromTable( "NSSET", "ClID", "id", id );

                  if( regID != clID )   // pokud neni klientem 
                    {
                      LOG( WARNING_LOG, "bad autorization not client of nsset [%s]", handle );
                      ret->errCode = COMMAND_AUTOR_ERROR;       // spatna autorizace
                    }
                  else
                    {
                      // zpracuj  pole statusu
                      status.Make( DBsql.GetStatusFromTable( "NSSET", id ) );

                      if( status.Test( STATUS_DELETE ) )
                        {
                          LOG( WARNING_LOG, "status DeleteProhibited" );
                          ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                          stat = false;
                        }
                      else      // status je OK
                        {
                          // test na vazbu do tabulky domain jestli existuji vazby na  nsset
                          if( DBsql.TestNSSetRelations( id ) )  //  nemuze byt smazan

                            {
                              LOG( WARNING_LOG, "database relations" );
                              ret->errCode = COMMAND_PROHIBITS_OPERATION;
                              del = false;
                            }
                          else
                            {
                              //  uloz do historie
                              if( DBsql.MakeHistory() )
                                {
                                  if( DBsql.SaveHistory( "nsset_contact_map", "nssetid", id ) ) // historie tech kontakty
                                    {
                                      // na zacatku vymaz technicke kontakty
                                      if( DBsql.DeleteFromTable( "nsset_contact_map", "nssetid", id ) )
                                        {

                                          if( DBsql.SaveHistory( "HOST", "nssetid", id ) )
                                            {
                                              // vymaz nejdrive podrizene hosty
                                              if( DBsql.DeleteFromTable( "HOST", "nssetid", id ) )
                                                {
                                                  if( DBsql.SaveHistory( "NSSET", "id", id ) )
                                                    {
                                                      // vymaz NSSET nakonec
                                                      if( DBsql.DeleteFromTable( "NSSET", "id", id ) ) ret->errCode = COMMAND_OK;   // pokud vse OK
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

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }

          }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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
                                            ccReg::timestamp & crDate, CORBA::Long clientID, const char *clTRID , const char* XML )
{
DB DBsql;
char Array[512] , NAME[256];
char createDate[32];
char HANDLE[64]; // handle na velka pismena
char roid[64];
ccReg::Response * ret;
int regID, id, techid;
int i, len, j , l , seq , zone ;
time_t now;

ret = new ccReg::Response;
// default
ret->errCode = 0;
ret->errors.length( 0 );
crDate = 0;
seq=0;

LOG( NOTICE_LOG, "NSSetCreate: clientID -> %d clTRID [%s] handle [%s]  authInfoPw [%s]", clientID, clTRID, handle , authInfoPw  );

  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_NSsetCreate,  clTRID  , XML) )
        {

       // preved handle na velka pismena
       if( get_NSSETHANDLE( HANDLE , handle ) == false )  // spatny format handlu
         {

            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "bad format of handle[%s]" ,  handle);
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::nssetCreate_handle;
            ret->errors[0].value <<= CORBA::string_dup( handle );
            ret->errors[0].reason = CORBA::string_dup( "bad format of handle use NSSID:NSSET" );
        }
        else
     {
      if( DBsql.BeginTransaction() )      // zahaj transakci
        {
 
        // prvni test zdali nsset uz existuje          
        if(  DBsql.CheckObject( "NSSET",  "handle", handle  )   )
         {
               LOG( WARNING_LOG, "nsset handle [%s] EXIST", HANDLE );
               ret->errCode = COMMAND_OBJECT_EXIST;  // je uz zalozen
        }
        else                  // pokud nexistuje 
       {

              // get  registrator ID
              regID = DBsql.GetLoginRegistrarID( clientID );


              // ID je cislo ze sequence
              id = DBsql.GetSequenceID( "nsset" );

              // vytvor roid nssetu
              get_roid( roid, "N", id );

              // datum a cas vytvoreni
              now = time( NULL );
              crDate = now;
              get_timestamp( now, createDate );



              DBsql.INSERT( "NSSET" );
              DBsql.INTO( "id" );
              DBsql.INTO( "roid" );
              DBsql.INTO( "handle" );
              DBsql.INTO( "CrDate" );
              DBsql.INTO( "CrID" );
              DBsql.INTO( "ClID" );
              DBsql.INTO( "status" );
              DBsql.INTOVAL( "authinfopw", authInfoPw );

              DBsql.VALUE( id );
              DBsql.VALUE( roid );
              DBsql.VALUE( HANDLE  );
              DBsql.VALUE( createDate );
              DBsql.VALUE( regID );
              DBsql.VALUE( regID );
              DBsql.VALUE( "{ 1 }" );   // status OK
              DBsql.VAL( authInfoPw );

              // zapis nejdrive nsset 
              if( DBsql.EXEC() )
                {


                  // zapis technicke kontakty 
                  for( i = 0; i <  tech.length() ;  i++ )
                    {

                      // preved handle na velka pismena
                      if( get_HANDLE( HANDLE , tech[i] ) == false )  // spatny format handlu
                        {
                          LOG( WARNING_LOG, "NSSetCreate: unknow tech Contact " , (const char *)  tech[i] );
                          ret->errors.length( seq +1 );
                          ret->errors[seq].code = ccReg::nssetCreate_tech;
                          ret->errors[seq].value <<= CORBA::string_dup(  tech[i] );
                          ret->errors[seq].reason = CORBA::string_dup( "unknow tech contact" );
                          seq++;
                          ret->errCode = COMMAND_PARAMETR_ERROR;
                        }
                      else
                      { 
                      techid = DBsql.GetNumericFromTable( "Contact", "id", "handle", HANDLE );
                      if( techid )
                        {
                          LOG( NOTICE_LOG, "NSSetCreate: create tech Contact " , (const char *)  tech[i] );
                          DBsql.INSERT( "nsset_contact_map" );
                          DBsql.VALUE( id );
                          DBsql.VALUE( techid );

                          if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;
                        }
                      else
                        {
                          LOG( WARNING_LOG, "NSSetCreate: unknow tech Contact " , (const char *)  tech[i]  );                          
                          ret->errors.length( seq +1 );
                          ret->errors[seq].code = ccReg::nssetCreate_tech;
                          ret->errors[seq].value <<= CORBA::string_dup(  tech[i] );
                          ret->errors[seq].reason = CORBA::string_dup( "unknow tech contact" );
                          seq++;                                 
                          // TODO error value 
                          ret->errCode = COMMAND_PARAMETR_ERROR;
                        }
                      }
                    }

                  // zapis do tabulky hostu
                  for( i = 0; i < dns.length() ; i++ )
                    {
     

                      // preved sequenci adres
                      strcpy( Array, " { " );
                      for( j = 0; j < dns[i].inet.length(); j++ )
                        {
                          if( j > 0 ) strcat( Array, " , " );

                          if( TestInetAddress( dns[i].inet[j] ) )
                            {
                                 for( l = 0 ; l < j  ; l ++ )
                                    {
                                         if( strcmp( dns[i].inet[l] ,   dns[i].inet[j] ) == 0 )
                                            {
                                               LOG( WARNING_LOG, "NSSetCreate: duplicity host address %s " , (const char *) dns[i].inet[j]  );
                                               ret->errors.length( seq +1 );
                                               ret->errors[seq].code = ccReg::nssetCreate_ns_addr;
                                               ret->errors[seq].value <<= CORBA::string_dup(   dns[i].inet[j]  );
                                               ret->errors[seq].reason = CORBA::string_dup( "duplicity host address" );
                                               seq++;
                                               ret->errCode = COMMAND_PARAMETR_ERROR;
                                           }
                                    }
                                     
                              strcat( Array,  dns[i].inet[j]  );
                            }
                          else
                            {
                                  LOG( WARNING_LOG, "NSSetCreate: bad host address %s " , (const char *) dns[i].inet[j]  );
                                  ret->errors.length( seq +1 );
                                  ret->errors[seq].code = ccReg::nssetCreate_ns_addr;
                                  ret->errors[seq].value <<= CORBA::string_dup(   dns[i].inet[j]  );
                                  ret->errors[seq].reason = CORBA::string_dup( "bad host address" );
                                  seq++;
                                  ret->errCode = COMMAND_PARAMETR_ERROR;
                            }

                        }
                      strcat( Array, " } " );



                      // test DNS hostu
                      if( TestDNSHost( dns[i].fqdn ) )
                        {
                       LOG( NOTICE_LOG ,  "NSSetCreate: DNS Host %s [%s] ",   (const char *)  dns[i].fqdn   , Array    );

                       convert_hostname(  NAME , dns[i].fqdn );
 

                        zone = get_zone( NAME  , true ); // cislo zony kam patri
 
                        if( zone == 0 && dns[i].inet.length() > 0 ) // neni v definovanych zonach a obsahuje zaznam ip adresy
                        {
                            for( j = 0 ; j < dns[i].inet.length() ; j ++ )
                               {

                                    LOG( WARNING_LOG, "NSSetCreate:  ipaddr  glue not allowed %s " , (const char *) dns[i].inet[j]   );
                                    ret->errors.length( seq +1 );
                                    ret->errors[seq].code = ccReg::nssetCreate_ns_addr;
                                    ret->errors[seq].value <<= CORBA::string_dup(   dns[i].inet[j]  ); // staci vratit prvni zaznam
                                    ret->errors[seq].reason = CORBA::string_dup( "not glue ipaddr allowed" );
                                    seq++;
                                }
                                    ret->errCode = COMMAND_PARAMETR_ERROR;
 
                        }
                       else
                        {
                          // HOST informace pouze ipaddr a fqdn 
                          DBsql.INSERT( "HOST" );
                          DBsql.INTO( "NSSETID" );
                          DBsql.INTO( "fqdn" );
                          DBsql.INTO( "ipaddr" );
                          DBsql.VALUE( id );

                          DBsql.VALUE( NAME );
                          DBsql.VALUE( Array );
                          if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED; 
                        }


                                                    
                        }
                     else 
                       {
                                  LOG( WARNING_LOG, "NSSetCreate: bad host name %s " , (const char *)  dns[i].fqdn  );
                                  ret->errors.length( seq +1 );
                                  ret->errors[seq].code = ccReg::nssetCreate_ns_name;
                                  ret->errors[seq].value <<= CORBA::string_dup(  dns[i].fqdn  );
                                  ret->errors[seq].reason = CORBA::string_dup( "bad host name" );
                                  seq++;
                                  // TODO error value
                                  ret->errCode = COMMAND_PARAMETR_ERROR;

                       }

                    }

                }


              //  uloz do historie
              if( ret->errCode == 0 )   // pokud zatim neni zadna chyba 
                {
                  if( DBsql.MakeHistory() )
                    {
                      if( DBsql.SaveHistory( "nsset_contact_map", "nssetid", id ) )     // historie tech kontakty
                        {

                          if( DBsql.SaveHistory( "HOST", "nssetid", id ) )
                            {
                              //  uloz podrizene hosty
                              if( DBsql.SaveHistory( "NSSET", "id", id ) ) ret->errCode = COMMAND_OK;
                            }
                        }
                    }          

                }

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }


            }
            }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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



ccReg::Response* ccReg_EPP_i::NSSetUpdate(const char* handle , const char* authInfo_chg, 
                                          const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem,
                                          const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem,
                                          const ccReg::Status& status_add, const ccReg::Status& status_rem, 
                                          CORBA::Long clientID, const char* clTRID , const char* XML )
{
ccReg::Response *ret;
DB DBsql;
Status status;
bool  check;
char  Array[512] ,  statusString[128] , HANDLE[64] , NAME[256];
int regID=0 , clID=0 , id ,nssetid ,  techid  , hostID;
int i , j , l  , seq , zone ;
int hostNum;
bool remove_update_flag=false;

ret = new ccReg::Response;
seq=0;
ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetUpdate: clientID -> %d clTRID [%s] handle [%s] authInfo_chg  [%s] " , clientID , clTRID , handle  , authInfo_chg);

// nacti status flagy
  for( i = 0; i < status_add.length(); i++ )
    {
      LOG( NOTICE_LOG, "status_add [%s] -> %d",  (const char *)  status_add[i]  ,  status.GetStatusNumber( status_add[i] )  );
      status.PutAdd( status.GetStatusNumber( status_add[i] ) ); // pridej status flag
    }

  for( i = 0; i < status_rem.length(); i++ )
    {
      LOG( NOTICE_LOG, "status_rem [%s] -> %d ",   (const char *) status_rem[i]  , status.GetStatusNumber( status_rem[i] )  );
      status.PutRem(  status.GetStatusNumber( status_rem[i] ) );
      if( status.GetStatusNumber( status_rem[i] ) ==  STATUS_clientUpdateProhibited )
        {
           LOG( NOTICE_LOG, "remove STATUS_clientUpdateProhibited");
           remove_update_flag=true;
        }
    }



if( DBsql.OpenDatabase( database ) )
  {

    if( DBsql.BeginAction( clientID, EPP_NSsetUpdate,  clTRID , XML) )
      {

       // preved handle na velka pismena
       if( get_HANDLE( HANDLE , handle )  )
        {

        if( ret->errCode == 0 )
          {
          if( DBsql.BeginTransaction() )
            {
            // pokud   neexistuje
            if( ( id = DBsql.GetNumericFromTable( "NSSET", "id", "handle", ( char * ) HANDLE ) ) == 0 )
              {
                LOG( WARNING_LOG, "object [%s] NOT_EXIST", handle );
                ret->errCode = COMMAND_OBJECT_NOT_EXIST;
              }
            else
              {
                    // get  registrator ID
                    regID = DBsql.GetLoginRegistrarID( clientID );
                    // client contaktu
                    clID = DBsql.GetNumericFromTable( "NSSET", "clID", "id", id );

                    if( clID != regID )
                      {
                        LOG( WARNING_LOG, "bad autorization not  client of nsset [%s]", handle );
                        ret->errCode = COMMAND_AUTOR_ERROR;     // spatna autorizace
                      }
                    else
                      {
                        // zpracuj  pole statusu
                        status.Make( DBsql.GetStatusFromTable( "NSSET", id ) );

                        if( status.Test( STATUS_UPDATE ) && remove_update_flag== false)
                          {
                            LOG( WARNING_LOG, "status UpdateProhibited" );
                            ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                          }
                        else
                          {
                            //  uloz do historie
                            if( DBsql.MakeHistory() )
                              {
                                if( DBsql.SaveHistory( "NSSET", "id", id ) )    // uloz zaznam
                                  {

                                          // pridany status
                                          for( i = 0; i < status.AddLength(); i++ )
                                            {
                                                  if( status.Add(i) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::nssetUpdate_status_add;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_add[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup( "can not add status flag" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                    }
                                            }

                                          // zruseny status flagy
                                         for( i = 0; i < status.RemLength(); i++ )
                                             {
                                              
                                                  if( status.Rem(i) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::nssetUpdate_status_rem;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_rem[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup( "can not remove status flag" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                    }
                                            }





                                    if( ret->errCode == 0 )
                                    {

                                    //  vygeneruj  novy status string array
                                    status.Array( statusString );




                                    // zmenit zaznam o domene
                                    DBsql.UPDATE( "NSSET" );                                   
                                    DBsql.SET( "UpDate", "now" );
                                    DBsql.SET( "UpID", regID );
                                    DBsql.SET( "status", statusString );
                                    if( remove_update_flag == false ) DBsql.SET( "AuthInfoPw", authInfo_chg );    // zmena autentifikace  
                                    DBsql.WHEREID( id );


                                    if( DBsql.EXEC() )
                                      {
                                        ret->errCode = COMMAND_OK;      // nastavit uspesne
                                       
                                        if( DBsql.SaveHistory( "nsset_contact_map", "nssetID", id ) )   // uloz do historie tech kontakty
                                         {
                                           if( remove_update_flag == false ) 
                                           {                                           
                                            // pridat tech kontakty                      
                                            for( i = 0; i < tech_add.length(); i++ )
                                              {
                                               if( get_HANDLE( HANDLE , tech_add[i]  )  )
                                                 techid = DBsql.GetNumericFromTable( "Contact", "id", "handle", HANDLE );
                                                else techid = 0 ;

                                                check = DBsql.CheckContactMap( "nsset", id, techid );
                                                if( techid && !check )
                                                  {
                                                    LOG( NOTICE_LOG ,  "add techid ->%d [%s]" ,  techid ,  (const char *) tech_add[i] );
                                                    DBsql.INSERT( "nsset_contact_map" );
                                                    DBsql.VALUE( id );
                                                    DBsql.VALUE( techid );
                                                    if( !DBsql.EXEC() )
                                                      {
                                                        ret->errCode = COMMAND_FAILED;
                                                        break;
                                                      }
                                                  }
                                                else
                                                  {
                                                    if( techid == 0 )
                                                      {
                                                        LOG( WARNING_LOG, "add tech Contact [%s] not exist" , (const char *) tech_add[i] );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_tech_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  tech_add[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup( "unknow add tech contact" );
                                                        seq++;
                                                      }
                                                    if( check )  
                                                      {
                                                        LOG( WARNING_LOG, "add tech Contact [%s] exist in contact map table"  , (const char *) tech_add[i] );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_tech_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  tech_add[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup( "tech contact exist in contact map" );
                                                        seq++;
                                                      }                                                

                                                    ret->errCode = COMMAND_PARAMETR_ERROR;                                            
                                                  }
                                                
                                              }

                                            // vymaz  tech kontakty
                                            for( i = 0; i < tech_rem.length(); i++ )
                                              {
                                                 if( get_HANDLE( HANDLE , tech_rem[i] ) ) 
                                                    techid = DBsql.GetNumericFromTable( "Contact", "id", "handle", HANDLE );
                                                 else techid = 0 ;

                                                check = DBsql.CheckContactMap( "nsset", id, techid );

                                                if( techid && check )
                                                  {
                                                     LOG( NOTICE_LOG ,  "rem techid ->%d [%s]" ,  techid , (const char *) tech_rem[i]  ); 
                                                    if( !DBsql.DeleteFromTableMap( "nsset", id, techid ) )
                                                      {
                                                        ret->errCode = COMMAND_FAILED;
                                                        break;
                                                      }
                                                  }
                                                else
                                                  {
                                                    if( techid == 0 )
                                                      {
                                                        LOG( WARNING_LOG, "rem tech Contact [%s]  not exist"  , (const char *) tech_rem[i] );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_tech_rem;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  tech_rem[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup( "unknow rem tech contact" );
                                                        seq++;
                                                      }
                                                    if( !check )  
                                                      {
                                                       LOG( WARNING_LOG, "rem tech Contact [%s] not in contact map table" , (const char *) tech_rem[i] );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_tech_rem;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  tech_rem[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup( "tech contact not exist in contact map" );
                                                        seq++;
                                                      }
                                                   ret->errCode = COMMAND_PARAMETR_ERROR;
                                                  }
                                              }
                                             }
                                          }

                                        if( DBsql.SaveHistory( "host", "nssetID", id ) )        // uloz do historie hosty
                                          {
                                           if( remove_update_flag == false )
                                           {

                                            // pridat DNS HOSTY
                                            for( i = 0; i < dns_add.length(); i++ )
                                              {



                                                // vytvor pole inet adres
                                                strcpy( Array, "{ " );
                                              for( j = 0; j < dns_add[i].inet.length(); j++ )
                                                {
                                                    if( j > 0 ) strcat( Array, " , " );

                                                    if( TestInetAddress( dns_add[i].inet[j] ) )  
                                                      {
                                                         for( l = 0 ; l < j ; l ++ )
                                                         {
                                                          if( strcmp( dns_add[i].inet[l] ,   dns_add[i].inet[j] ) == 0 )
                                                            {
                                                                LOG( WARNING_LOG, "NSSetUpdate: duplicity host address %s " , (const char *) dns_add[i].inet[j]  );
                                                                ret->errors.length( seq +1 );
                                                                ret->errors[seq].code = ccReg::nssetUpdate_ns_addr_add;
                                                                ret->errors[seq].value <<= CORBA::string_dup(   dns_add[i].inet[j]  );
                                                                ret->errors[seq].reason = CORBA::string_dup( "duplicity host address" );
                                                                seq++;
                                                               ret->errCode = COMMAND_PARAMETR_ERROR;
                                                             }
                                                          }

                                                      strcat( Array,  dns_add[i].inet[j]  );
                                                    }
                                                    else
                                                      {
                                                        LOG( WARNING_LOG, "NSSetUpdate: bad add host address %s " , (const char *)  dns_add[i].inet[j] );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_ns_addr_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(    dns_add[i].inet[j] );
                                                        ret->errors[seq].reason = CORBA::string_dup( "bad host address" );
                                                        seq++;
                                                        ret->errCode = COMMAND_PARAMETR_ERROR;
                                                      }
                                                 }

                                              
                                                 strcat( Array, " } " );
 







                                                // test DNS hostu
                                                if( TestDNSHost( dns_add[i].fqdn  ) )
                                                {
                                                 LOG( NOTICE_LOG ,  "NSSetUpdate: add dns [%s] %s" , (const char * ) dns_add[i].fqdn  , Array );                                


                                                 convert_hostname(  NAME , dns_add[i].fqdn );

                                                 zone = get_zone( NAME , true ); // cislo zony kam patri

                                               if( zone == 0 && dns_add[i].inet.length() > 0 ) // neni v definovanych zonach a obsahuje zaznam ip adresy
                                                 {
                                                   for( j = 0 ; j < dns_add[i].inet.length() ; j ++ )
                                                   {
                                                    LOG( WARNING_LOG, "NSSetUpdate:  ipaddr  glue not allowed %s " , (const char *) dns_add[i].inet[j]   );
                                                    ret->errors.length( seq +1 );
                                                    ret->errors[seq].code = ccReg::nssetUpdate_ns_addr_add;
                                                    ret->errors[seq].value <<= CORBA::string_dup(   dns_add[i].inet[j]  ); 
                                                    ret->errors[seq].reason = CORBA::string_dup( "not glue ipaddr allowed" );
                                                    seq++;
                                                   }
                                                   ret->errCode = COMMAND_PARAMETR_ERROR;             
                                                }
                                              else
                                               {

                                                   if(  DBsql.CheckHost( NAME , id )  ) // uz exstuje nelze pridat
                                                     {
                                                        LOG( WARNING_LOG, "NSSetUpdate:  host name %s exist" , (const char *)  dns_add[i].fqdn );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_ns_name_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(   dns_add[i].fqdn  );
                                                        ret->errors[seq].reason = CORBA::string_dup( "host name exist" );
                                                        seq++;
                                                        ret->errCode = COMMAND_PARAMETR_ERROR;
                                                     }
                                                     else
                                                     {
                                                         DBsql.INSERT( "HOST" );
                                                         DBsql.INTO( "nssetid" );
                                                         DBsql.INTO( "fqdn" );
                                                         DBsql.INTO( "ipaddr" );
                                                         DBsql.VALUE( id );
                                                         DBsql.VALUE( NAME );
                                                         DBsql.VALUE( Array );
                                                         if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;
                                                      }
                                                 }
                                             }
                                             else
                                                      {
                                                        LOG( WARNING_LOG, "NSSetUpdate: bad add host name %s " , (const char *)  dns_add[i].fqdn );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_ns_name_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(     dns_add[i].fqdn  );
                                                        ret->errors[seq].reason = CORBA::string_dup( "bad host address" );
                                                        seq++;
                                                        ret->errCode = COMMAND_PARAMETR_ERROR;
                                                      }
                                          
                                            
                                                  

                                              }

                                            // smazat DNS HOSTY
                                            for( i = 0; i < dns_rem.length(); i++ )
                                              {
                                               LOG( NOTICE_LOG ,  "NSSetUpdate:  delete  host  [%s] " , (const char *)   dns_rem[i].fqdn );
                                               
                                               if( TestDNSHost( dns_rem[i].fqdn  ) )
                                                 {
                                                        convert_hostname(  NAME , dns_rem[i].fqdn );
                                                        if( ( hostID = DBsql.CheckHost( NAME , id )  ) == 0 )
                                                        {
                                                        
                                                        LOG( WARNING_LOG, "NSSetUpdate:  host  [%s] not in table" ,  (const char *)   dns_rem[i].fqdn );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_ns_name_rem;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  dns_rem[i].fqdn );
                                                        ret->errors[seq].reason = CORBA::string_dup( "host is not in table" );
                                                        seq++;
                                                        ret->errCode = COMMAND_PARAMETR_ERROR;   
                                                       }
                                                       else // smaz pokud zaznam existuje
                                                       {
                                                             LOG( NOTICE_LOG ,  "NSSetUpdate: Delete hostID %d" , hostID );
                                                             if( !DBsql.DeleteFromTable("HOST" , "id" ,  hostID  ) ){ret->errCode = COMMAND_FAILED; break; }
                                                       }
                                                        

                                                }
                                               else
                                                {
                                                        LOG( WARNING_LOG, "NSSetUpdate:  bad  rem  host [%s]" ,  (const char *)   dns_rem[i].fqdn );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_ns_name_rem;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  dns_rem[i].fqdn );
                                                        ret->errors[seq].reason = CORBA::string_dup( "bad host name" );
                                                        seq++;
                                                        ret->errCode = COMMAND_PARAMETR_ERROR;
                                                 }
  
                                               // TEST pocet dns hostu
                                                hostNum = DBsql.GetNSSetNum( id );
                                                LOG(NOTICE_LOG, "NSSetUpdate:  hostNum %d" , hostNum );

                                                if( hostNum ==  0 )
                                                  {
                                                    for( i = 0; i < dns_rem.length(); i++ )
                                                      {
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code =  ccReg::nssetUpdate_ns_name_rem; 
                                                        ret->errors[seq].value <<= CORBA::string_dup(  dns_rem[i].fqdn );
                                                        ret->errors[seq].reason = CORBA::string_dup( "can not remove DNS host" );
                                                        seq++; 
                                                      }
                                                    ret->errCode = COMMAND_PARAMETR_VALUE_POLICY_ERROR;
                                                  }

                                               if( hostNum >  9 ) // maximalni pocet
                                                  {
                                                    for( i = 0; i < dns_add.length(); i++ )
                                                      {
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code =  ccReg::nssetUpdate_ns_name_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  dns_add[i].fqdn );
                                                        ret->errors[seq].reason = CORBA::string_dup( "can not add DNS host" );
                                                        seq++;
                                                      }
                                                    ret->errCode = COMMAND_PARAMETR_VALUE_POLICY_ERROR;
                                                  }
 
              
                                              }

                                          }
                                        }

                                      }
                                    else ret->errCode = COMMAND_FAILED; // spatny SQL update
                                   
                                    }


                                  }



                              }



                          }

                      }
                    // konec transakce commit ci rollback
                    DBsql.QuitTransaction( ret->errCode );
                  }
              }

          }
        }
        // zapis na konec action
        ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) ); 
      }

    ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

    DBsql.Disconnect();
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


ccReg::Response* ccReg_EPP_i::NSSetTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID , const char* XML )
{
ccReg::Response *ret;
DB DBsql;
char HANDLE[64];
Status status;
int regID=0 , clID=0 , id , contactid;

ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetTransfer: clientID -> %d clTRID [%s] handle [%s] authInfo [%s] " , clientID , clTRID , handle , authInfo );

if( DBsql.OpenDatabase( database ) )
{

if( DBsql.BeginAction( clientID , EPP_NSsetTransfer ,  clTRID , XML  ) )
 {

  // preved handle na velka pismena
  if( get_HANDLE( HANDLE , handle )  )
  {
  if( DBsql.BeginTransaction() )
  {
 
   // pokud domena neexistuje
  if( (id = DBsql.GetNumericFromTable(  "NSSET"  , "id" , "handle" , (char * ) HANDLE ) ) == 0 ) 
    {
        LOG( WARNING_LOG  ,  "object [%s] NOT_EXIST" ,  handle );
      ret->errCode= COMMAND_OBJECT_NOT_EXIST;
    }
  else
  {
   // get  registrator ID
   regID =   DBsql.GetLoginRegistrarID( clientID);
   // client contaktu
   clID  =  DBsql.GetNumericFromTable(  "NSSET"  , "clID" , "id" , id );



  if( regID == clID )       // transfer nemuze delat stavajici client
    {
      LOG( WARNING_LOG, "client can not transfer NSSET %s" , handle );
      ret->errCode =  COMMAND_NOT_ELIGIBLE_FOR_TRANSFER;
    }
   else
  {

                  // zpracuj  pole statusu
                  status.Make( DBsql.GetStatusFromTable( "NSSET", id ) );

                  if( status.Test( STATUS_TRANSFER ) )
                    {
                      LOG( WARNING_LOG, "status TransferProhibited" );
                      ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                    }
                  else
                    {

   if(  DBsql.AuthTable(  "NSSET"  , (char *)authInfo , id )  == false  ) // pokud prosla autentifikace 
     {       
        LOG( WARNING_LOG , "autorization failed");
        ret->errCode = COMMAND_AUTOR_ERROR; // spatna autorizace
     }
    else
     {
         //  uloz do historie
       if( DBsql.MakeHistory() )
        {
                      if( DBsql.SaveHistory( "nsset_contact_map", "nssetid", id ) )     // historie tech kontakty
                        {
                          if( DBsql.SaveHistory( "HOST", "nssetid", id ) ) // historie hostu
                           {
          if( DBsql.SaveHistory( "NSSET" , "id" , id ) ) // uloz zaznam
           { 


                // zmena registratora
                DBsql.UPDATE( "NSSET");
                DBsql.SET( "TrDate" , "now" );
                DBsql.SET( "clID" , regID );
                DBsql.WHEREID( id ); 
                if(   DBsql.EXEC() )  ret->errCode = COMMAND_OK; // nastavit OK                                  
                else  ret->errCode = COMMAND_FAILED;
           }
              }
                 }

       }
     }
    }
    }
   }
    // konec transakce commit ci rollback
    DBsql.QuitTransaction( ret->errCode );
   }

  }
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode  ) ) ;
}


ret->errMsg =  CORBA::string_dup(   DBsql.GetErrorMessage(  ret->errCode  ) ) ;

DBsql.Disconnect();
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

ccReg::Response* ccReg_EPP_i::DomainInfo(const char* fqdn, ccReg::Domain_out d , CORBA::Long clientID, const char* clTRID ,  const char* XML)
{
DB DBsql;
Status status;
ccReg::timestamp valexDate;
ccReg::ENUMValidationExtension *enumVal;
ccReg::Response *ret;
char FQDN[64];
int id , clid , crid ,  upid , regid ,nssetid , regID;
int i , len ;

d = new ccReg::Domain;
ret = new ccReg::Response;




// default
ret->errCode=COMMAND_FAILED;
ret->errors.length(0);


LOG( NOTICE_LOG ,  "DomainInfo: clientID -> %d clTRID [%s] fqdn  [%s] " , clientID , clTRID  , fqdn );


d->ext.length(0); // extension

if( DBsql.OpenDatabase( database ) )
{

if( DBsql.BeginAction( clientID , EPP_DomainInfo , clTRID , XML  ) )
 {

  // preved fqd na  mala pismena a otestuj to
  if(  get_FQDN( FQDN , fqdn )  )  // spatny format navu domeny
   {
   
   // get  registrator ID
   regID = DBsql.GetLoginRegistrarID( clientID );


  if(  DBsql.SELECTONE( "DOMAIN" , "fqdn" , FQDN )  )
  {
    if( DBsql.GetSelectRows() == 1 )
      {
        id = atoi( DBsql.GetFieldValueName("id" , 0 ) );
        clid = atoi( DBsql.GetFieldValueName("ClID" , 0 ) ); 
        crid = atoi( DBsql.GetFieldValueName("CrID" , 0 ) ); 
        upid = atoi( DBsql.GetFieldValueName("UpID" , 0 ) ); 
        regid = atoi( DBsql.GetFieldValueName("registrant" , 0 ) ); 
        nssetid = atoi( DBsql.GetFieldValueName("nsset" , 0 ) );  

        status.Make(  DBsql.GetFieldValueName("status" , 0 ) ) ; // status

	d->CrDate= get_time_t( DBsql.GetFieldValueName("CrDate" , 0 ) )  ; // datum a cas vytvoreni
	d->UpDate= get_time_t( DBsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
	d->TrDate= get_time_t( DBsql.GetFieldValueName("TrDate" , 0 ) ); // datum a cas transferu
	d->ExDate= get_time_t( DBsql.GetFieldValueName("ExDate" , 0 ) ); //  datum a cas expirace

	d->ROID=CORBA::string_dup( DBsql.GetFieldValueName("roid" , 0 )  ); // jmeno nebo nazev kontaktu
	d->name=CORBA::string_dup( DBsql.GetFieldValueName("fqdn" , 0 )  ); // jmeno nebo nazev kontaktu


        if( regID == clid ) // pokud je registrator clientem obdrzi autentifikaci
           d->AuthInfoPw = CORBA::string_dup( DBsql.GetFieldValueName("AuthInfoPw" , 0 ) ); // autentifikace
         else  d->AuthInfoPw = CORBA::string_dup( "" ); // jinak prazdny retezec
 


    
        ret->errCode=COMMAND_OK;

    
        // free select
	DBsql.FreeSelect();
        
        // zpracuj pole statusu
        len =  status.Length();
        d->stat.length(len);

        for( i = 0 ; i < len ; i ++)
           {
              d->stat[i] = CORBA::string_dup( status.GetStatusString(  status.Get(i)  ) );
           }


        d->ClID = CORBA::string_dup( DBsql.GetRegistrarHandle( clid ) );
        d->CrID = CORBA::string_dup( DBsql.GetRegistrarHandle( crid ) );
        d->UpID = CORBA::string_dup( DBsql.GetRegistrarHandle( upid ) );

        // vlastnik domeny
        d->Registrant=CORBA::string_dup( DBsql.GetValueFromTable( "CONTACT" , "handle" , "id" , regid ) );


        //  handle na nsset
        d->nsset=CORBA::string_dup( DBsql.GetValueFromTable( "NSSET" , "handle", "id" , nssetid ) );
    

        // dotaz na admin kontakty
        // dotaz na technicke kontakty
        if(  DBsql.SELECTCONTACTMAP( "domain"  , id ) )
          {
               len =  DBsql.GetSelectRows(); // pocet technickych kontaktu
               d->admin.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) 
                 {
                   d->admin[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );
                 }
               DBsql.FreeSelect();
           }else ret->errCode=COMMAND_FAILED;


       // uloz extension pokud existuje
        if( DBsql.SELECTONE( "enumval" , "domainID" , id ) )
          {    
            if( DBsql.GetSelectRows() == 1 )
              {
                valexDate =  get_time_t( DBsql.GetFieldValueName("ExDate" , 0 ) );

                enumVal = new ccReg::ENUMValidationExtension;
                enumVal->valExDate = valexDate ;
                d->ext.length(1); // preved na  extension
                d->ext[0] <<= enumVal;
                LOG( NOTICE_LOG , "enumValExtension ExDate %d" ,   valexDate );
              }

           DBsql.FreeSelect();
         } else ret->errCode=COMMAND_FAILED;
   

     }
   else
    {
     // free select
    DBsql.FreeSelect();
    LOG( WARNING_LOG  ,  "domain [%s] NOT_EXIST" , fqdn );
    ret->errCode =  COMMAND_OBJECT_NOT_EXIST;

    }

   }

  }
 else
  {
            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "bad format of fqdn[%s]" , fqdn );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::domainInfo_fqdn;
            ret->errors[0].value <<= CORBA::string_dup( fqdn );
            ret->errors[0].reason = CORBA::string_dup( "bad format of fqdn" );
  }


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode  ) );

 }

ret->errMsg =  CORBA::string_dup(   DBsql.GetErrorMessage(  ret->errCode  ) ) ;

DBsql.Disconnect();
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

ccReg::Response* ccReg_EPP_i::DomainDelete(const char* fqdn , CORBA::Long clientID, const char* clTRID , const char* XML)
{
ccReg::Response *ret;
DB DBsql;
Status status;
char FQDN[64];
int regID , clID , id;
bool stat;
ret = new ccReg::Response;


// default
ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "DomainDelete: clientID -> %d clTRID [%s] fqdn  [%s] " , clientID , clTRID  , fqdn );


  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_DomainDelete,  clTRID  , XML) )
        {


      // preved fqd na  mala pismena a otestuj to
       if(  get_FQDN( FQDN , fqdn )   )  // spatny format navu domeny
         {

          if( DBsql.BeginTransaction() )
            {
              if( ( id = DBsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", ( char * ) FQDN ) ) == 0 )
                {
                  LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;
                }
              else
                {

                  regID = DBsql.GetLoginRegistrarID( clientID );        // aktivni registrator
                  clID = DBsql.GetNumericFromTable( "DOMAIN", "ClID", "id", id );       // client objektu

                  if( regID != clID )
                    {
                      LOG( WARNING_LOG, "bad autorization not client of fqdn [%s]", fqdn );
                      ret->errCode = COMMAND_AUTOR_ERROR;       // spatna autorizace
                    }
                  else
                    {
                      // zpracuj  pole statusu
                      status.Make( DBsql.GetStatusFromTable( "DOMAIN", id ) );

                      if( status.Test( STATUS_DELETE ) )
                        {
                          LOG( WARNING_LOG, "status DeleteProhibited" );
                          ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                          stat = false;
                        }
                      else      // status je OK
                        {
                          //  uloz do historie
                          if( DBsql.MakeHistory() )
                            {
                              if( DBsql.SaveHistory( "domain_contact_map", "domainID", id ) )
                                {                                 
                                      if( DBsql.DeleteFromTable( "domain_contact_map", "domainID", id ) )
                                        {
                                           if( DBsql.SaveHistory( "enumval", "domainID", id ) ) 
                                             {
                                               if( DBsql.DeleteFromTable( "enumval", "domainID", id ) )      // enumval extension
                                                {
 
                                                 if( DBsql.SaveHistory( "DOMAIN", "id", id ) )
                                                    {
                                                       if( DBsql.DeleteFromTable( "DOMAIN", "id", id ) )  
                                                        ret->errCode = COMMAND_OK; // pokud usmesne smazal
                                                    }
                                                 }
                                              }
                                        }
                                    
                                }
                            }
                        }
                    }

                }

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }

          }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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
ccReg::Response * ccReg_EPP_i::DomainUpdate( const char *fqdn, const char *registrant_chg, const char *authInfo_chg, const char *nsset_chg,
                                             const ccReg::AdminContact & admin_add, const ccReg::AdminContact & admin_rem,
                                             const ccReg::Status & status_add, const ccReg::Status & status_rem,
                                             CORBA::Long clientID, const char *clTRID,  const char* XML ,  const ccReg::ExtensionList & ext )
{
ccReg::Response * ret;
DB DBsql;
Status status;
const ccReg::ENUMValidationExtension * enumVal;
bool stat, check;
char FQDN[64] , HANDLE[64];
char valexpiryDate[32];
char statusString[128];
int regID = 0, clID = 0, id, nssetid, contactid, adminid;
int i, len, slen, j , seq , zone;
bool remove_update_flag=false;
time_t valExpDate = 0;

ret = new ccReg::Response;
seq=0;
ret->errCode = 0;
ret->errors.length( 0 );



LOG( NOTICE_LOG, "DomainUpdate: clientID -> %d clTRID [%s] fqdn  [%s] , registrant_chg  [%s] authInfo_chg [%s]  nsset_chg [%s] ",
       clientID, clTRID, fqdn, registrant_chg, authInfo_chg, nsset_chg );


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


// nacti status flagy
  for( i = 0; i < status_add.length(); i++ )
    {
      LOG( NOTICE_LOG, "status_add [%s] -> %d",  (const char *)  status_add[i]  ,  status.GetStatusNumber( status_add[i] )  );
      status.PutAdd( status.GetStatusNumber( status_add[i] ) ); // pridej status flag
    }

  for( i = 0; i < status_rem.length(); i++ )
    {
      LOG( NOTICE_LOG, "status_rem [%s] -> %d ",   (const char *) status_rem[i]  , status.GetStatusNumber( status_rem[i] )  );
      status.PutRem(  status.GetStatusNumber( status_rem[i] ) );
      if( status.GetStatusNumber( status_rem[i] ) ==  STATUS_clientUpdateProhibited )
        {
           LOG( NOTICE_LOG, "remove STATUS_clientUpdateProhibited");
           remove_update_flag=true;
        }
    }




  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_DomainUpdate, clTRID , XML ) )
        {


      // preved fqd na  mala pismena a otestuj to
       if( ( zone = get_FQDN( FQDN , fqdn ) ) == 0 )  // spatny format navu domeny
         {
            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "bad format of fqdn[%s]" , fqdn );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::domainUpdate_fqdn;
            ret->errors[0].value <<= CORBA::string_dup( fqdn );
            ret->errors[0].reason = CORBA::string_dup( "bad format of fqdn" );
        }

          if( ret->errCode == 0 )
            {
             if( DBsql.BeginTransaction() )
               {

              // pokud domena existuje
              if( ( id = DBsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", ( char * ) FQDN ) ) == 0 )
                {
                  LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;
                }
              else
                {
                      // get  registrator ID
                      regID = DBsql.GetLoginRegistrarID( clientID );
                      // client contaktu
                      clID = DBsql.GetNumericFromTable( "DOMAIN", "clID", "id", id );

                      if( clID != regID )
                        {
                          LOG( WARNING_LOG, "bad autorization not  client of domain [%s]", fqdn );
                          ret->errCode = COMMAND_AUTOR_ERROR;   // spatna autorizace

                        }
                      else
                        {
                          // zpracuj  pole statusu
                          status.Make( DBsql.GetStatusFromTable( "DOMAIN", id ) );

                          if( status.Test( STATUS_UPDATE )  && remove_update_flag == false )
                            {
                              LOG( WARNING_LOG, "status UpdateProhibited" );
                              ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                            }
                          else
                            {
                                 // test validate
                                                     if(  TestValidityExpDate(  valExpDate  , DBsql.GetValPreriod( zone ) )  ==  false )
                                                       {
                                                             LOG( WARNING_LOG, "bad validity exp date" );
                                                             ret->errors.length( seq +1);
                                                             ret->errors[seq].code = ccReg::domainUpdate_ext_valDate;
                                                             ret->errors[seq].value <<=   valExpDate;
                                                             ret->errors[seq].reason = CORBA::string_dup( "bad valExpDate");
                                                              seq++;
                                                          ret->errCode = COMMAND_PARAMETR_ERROR;
                                    }
                                                else

                              //  uloz do historie
                              if( DBsql.MakeHistory() )
                                {
                                  if( DBsql.SaveHistory( "Domain", "id", id ) ) // uloz zaznam
                                    {



                                      if( strlen( nsset_chg ) )
                                        {
                                         if(  get_HANDLE( HANDLE , nsset_chg ) ) 
                                           {
                                            if( ( nssetid = DBsql.GetNumericFromTable( "NSSET", "id", "handle", HANDLE ) ) == 0 )
                                            {
                                              LOG( WARNING_LOG, "nsset %s not exist", nsset_chg );

                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::domainUpdate_nsset;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  nsset_chg );
                                                      ret->errors[seq].reason = CORBA::string_dup( "nsset not exist" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;

                                            }
 
                                          }
                                           else 
                                          {
                                                      LOG( WARNING_LOG, "nsset %s bad handle" , nsset_chg );
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::domainUpdate_nsset;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  nsset_chg );
                                                      ret->errors[seq].reason = CORBA::string_dup( "bad nsset handle" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;

                                          }
                                        }
                                      else nssetid = 0;    // nemenim nsset;


                                      if( strlen( registrant_chg ) )
                                        {
                                          if(  get_HANDLE( HANDLE , registrant_chg ) )
                                           {

                                          if( ( contactid = DBsql.GetNumericFromTable( "CONTACT", "id", "handle", HANDLE ) ) == 0 )
                                            {
                                              LOG( WARNING_LOG, "registrant %s not exist", registrant_chg );
                                                    
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::domainUpdate_registrant;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  registrant_chg );
                                                      ret->errors[seq].reason = CORBA::string_dup( "registrant not exist" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;

                                            }
                                           }
                                         else
                                            {
                                              LOG( WARNING_LOG, "registrant %s bad handle", registrant_chg );

                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::domainUpdate_registrant;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  registrant_chg );
                                                      ret->errors[seq].reason = CORBA::string_dup( "bad handle registrant" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;

                                            }

                                        }
                                      else contactid = 0;  // nemenim vlastnika


                                         // pridany status
                                          for( i = 0; i < status.AddLength(); i++ )
                                            {
                                                  if( status.Add( i ) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::domainUpdate_status_add;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_add[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup( "can not add status flag" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                    }
                                            }

                                          // zruseny status flagy
                                         for( i = 0; i < status.RemLength(); i++ )
                                             {
                                                  if( status.Rem( i ) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::domainUpdate_status_rem;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_rem[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup( "can not remove status flag" );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                    }
                                            }

                                      if( ret->errCode == 0 )
                                      { 


                                      //  vygeneruj  novy status string array
                                      status.Array( statusString );


                                      // zmenit zaznam o domene
                                      DBsql.UPDATE( "DOMAIN" );
                                      DBsql.SET( "status", statusString );                                                      
                                      DBsql.SET( "UpDate", "now" );
                                      DBsql.SET( "UpID", regID );
                                      if( !remove_update_flag  )
                                      {
                                      if( nssetid )  DBsql.SET( "nsset", nssetid );    // zmena nssetu
                                      if( contactid ) DBsql.SET( "registrant", contactid );     // zmena drzitele domeny
                                      DBsql.SET( "AuthInfoPw", authInfo_chg );  // zmena autentifikace
                                      }
                                      DBsql.WHEREID( id );


                                      if( DBsql.EXEC() )
                                        {
                                          ret->errCode = COMMAND_OK;    // nastavit uspesne

                                          if( DBsql.SaveHistory( "enumval", "domainID", id ) )  // uloz do historie 
                                                 {

                                               if( !remove_update_flag  )
                                               {
                                              // zmena extension
                                              if( valExpDate > 0 )

                                                {
                                                     get_timestamp( valExpDate, valexpiryDate );
                                                     LOG( NOTICE_LOG, "change valExpDate %s ", valexpiryDate );
                                                     DBsql.UPDATE( "enumval" );
                                                     DBsql.SET( "ExDate", valexpiryDate );
                                                     DBsql.WHERE( "domainID", id );
 
                                                     if( !DBsql.EXEC() )  ret->errCode = COMMAND_FAILED; 
                                                }
                                                }
                                            }

                                          if( DBsql.SaveHistory( "domain_contact_map", "domainID", id ) )       // uloz do historie admin kontakty
                                            {
                                              if( !remove_update_flag  )
                                               {

                                              // pridat admin kontakty                      
                                              for( i = 0; i < admin_add.length(); i++ )
                                                {
                                                  if(  get_HANDLE( HANDLE , admin_add[i]  ) )
                                                  adminid = DBsql.GetNumericFromTable( "Contact", "id", "handle", HANDLE );
                                                  else adminid = 0;

                                                  check = DBsql.CheckContactMap( "domain", id, adminid );

                                                  if( adminid && !check )
                                                    {
                                                      //  LOG( NOTICE_LOG ,  "add admin  id ->%d [%s]" ,  adminid , admin_add[i] );
                                                      DBsql.INSERT( "domain_contact_map" );
                                                      DBsql.VALUE( id );                                                    
                                                      DBsql.VALUE( adminid );
                                                      
                                                      if( !DBsql.EXEC() ) { ret->errCode = COMMAND_FAILED; break; }
                                                    }
                                                  else
                                                    {
                                                      // if( adminid == 0 ) LOG( WARNING_LOG , "contact handle [%s] not exist" , admin_add[i]  );
                                                      //if( check ) LOG( WARNING_LOG , "Admin contact [%s] exist in cotact map table" ,  admin_add[i]  );

                                                     if( adminid  == 0 )
                                                      {
                                                        LOG( WARNING_LOG, "add admin-c not exist" );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::domainUpdate_admin_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  admin_add[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup( "unknow add admin contact" );
                                                        seq++;
                                                      }
                                                    if( check )
                                                      {
                                                        LOG( WARNING_LOG, "add tech Contact exist in contact map table" );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::domainUpdate_admin_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  admin_add[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup( "admin contact exist in contact map" );
                                                        seq++;
                                                      }

                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                      break;
                                                    }

                                                }

                                              // vymaz  admin kontakty
                                              for( i = 0; i < admin_rem.length(); i++ )
                                                {
                                                  if(  get_HANDLE( HANDLE ,  admin_rem[i] ) )
                                                  adminid = DBsql.GetNumericFromTable( "Contact", "id", "handle", HANDLE );
                                                  else adminid = 0;

                                                  check = DBsql.CheckContactMap( "domain", id, adminid );

                                                  if( adminid && check )
                                                    {
                                                      //  LOG( NOTICE_LOG ,  "rem admin  -> %d [%s]" ,  adminid , admin_rem[i]  ); 
                                                      if( !DBsql.DeleteFromTableMap( "domain", id, adminid ) )
                                                        {
                                                          ret->errCode = COMMAND_FAILED;
                                                          break;
                                                        }
                                                    }
                                                  else
                                                    {
                                                      // if( adsminid == 0 ) LOG( WARNING_LOG , "contact handle [%s] not exist" , tech_rem[i] );
                                                      // if( check == false  ) LOG( WARNING_LOG , "Tech contact [%s]  not exist" , tech_rem[i] );

                                                   if( adminid  == 0 )
                                                      {
                                                        LOG( WARNING_LOG, "rem admin-c not exist" );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::domainUpdate_admin_rem;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  admin_rem[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup( "unknow rem admin contact" );
                                                        seq++;
                                                      }
                                                    if( !check )
                                                      {
                                                        LOG( WARNING_LOG, "rem admin Contac not exist in contact map table" );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::domainUpdate_admin_rem;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  admin_rem[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup( "admin contact not exist in contact map" );
                                                        seq++;
                                                      }

                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                      break;
                                                    }
                                                }

                                               }
                                            }



                                        }
                                     else ret->errCode = COMMAND_FAILED; // spatny SQL update
                                    }
                                  }
                                }
                            }
                        }
                      // konec transakce commit ci rollback
                      DBsql.QuitTransaction( ret->errCode );
                    }
                }

            }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );                                        
        }

      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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
                                             CORBA::Long clientID, const char *clTRID,  const  char* XML , const ccReg::ExtensionList & ext )
{
DB DBsql;
const ccReg::ENUMValidationExtension * enumVal;
char expiryDate[32], valexpiryDate[32], createDate[32];
char roid[64] , FQDN[64] , HANDLE[64];
ccReg::Response * ret;
int contactid, regID, nssetid, adminid, id;
int i, len, s, zone , seq;
bool insert = true;
time_t t, valExpDate;

ret = new ccReg::Response;


// default
valExpDate = 0;
seq=0;
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



  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_DomainCreate, clTRID , XML ) )
        {


      // preved fqd na  mala pismena a otestuj to
       if( ( zone = get_FQDN( FQDN , fqdn ) ) == 0 )  // spatny format navu domeny
         {
            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "bad format of fqdn[%s]" , fqdn );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::domainCreate_fqdn;
            ret->errors[0].value <<= CORBA::string_dup( fqdn );
            ret->errors[0].reason = CORBA::string_dup( "bad format of fqdn" );
        }
      else
       {
          if( DBsql.BeginTransaction() )
            {


          //  test zdali domena uz existuje                   
          if( DBsql.CheckObject( "DOMAIN" , "fqdn" , fqdn )   )
            {
              ret->errCode = COMMAND_OBJECT_EXIST;      // je uz zalozena
              LOG( WARNING_LOG, "domain  [%s] EXIST", fqdn );
            }
          else // pokud domena nexistuje             
          {  
              id = DBsql.GetSequenceID( "domain" );     // id domeny

              // vytvor roid domeny
              get_roid( roid, "D", id );



             // get  registrator ID
             regID = DBsql.GetLoginRegistrarID( clientID );
             // nsset
            if( get_HANDLE( HANDLE , nsset ) == false )
              {
                      LOG( WARNING_LOG, "bad nsset handle %s", nsset );
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::domainCreate_nsset;
                      ret->errors[seq].value <<= CORBA::string_dup( nsset );
                      ret->errors[seq].reason = CORBA::string_dup( "bad handle nsset format" );
                      seq++;
                      ret->errCode = COMMAND_PARAMETR_ERROR;

              }
            else 
            if( (nssetid = DBsql.GetNumericFromTable( "NSSET", "id", "handle", HANDLE  ) ) == 0 )
              {
                      LOG( WARNING_LOG, "unknow nsset handle %s", nsset );
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::domainCreate_nsset;
                      ret->errors[seq].value <<= CORBA::string_dup( nsset );
                      ret->errors[seq].reason = CORBA::string_dup( "unknow nsset" );
                      seq++;
                      ret->errCode = COMMAND_PARAMETR_ERROR;
               }
              
             // nsset
            if( get_HANDLE( HANDLE , Registrant ) == false )
              {
                      LOG( WARNING_LOG, "bad registrant handle %s", Registrant );
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::domainCreate_registrant;
                      ret->errors[seq].value <<= CORBA::string_dup( Registrant );
                      ret->errors[seq].reason = CORBA::string_dup( "bad handle  Registrant format" );
                      seq++;
                      ret->errCode = COMMAND_PARAMETR_ERROR;

              }
           else                    
           if( ( contactid = DBsql.GetNumericFromTable( "CONTACT", "id", "handle", HANDLE ) ) == 0 )
              {
                      LOG( WARNING_LOG, "unknow Registrant handle %s", Registrant );
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::domainCreate_registrant;
                      ret->errors[seq].value <<= CORBA::string_dup( Registrant );
                      ret->errors[seq].reason = CORBA::string_dup( "unknow Registrant" );
                      seq++;
                      ret->errCode = COMMAND_PARAMETR_ERROR;
               }



             // nastaveni defaultni periody
             if( period == 0 )
               {
                 period = DBsql.GetExPreriodMin( zone );
                 LOG( NOTICE_LOG, "get defualt peridod %d month  for zone   %d ", period , zone  );

               }

             if(  TestPeriodyInterval( period  ,   DBsql.GetExPreriodMin( zone )  ,  DBsql.GetExPreriodMax( zone )  )  == false )
              {
                  LOG( WARNING_LOG, "bad period interval" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainCreate_period;
                  ret->errors[seq].value <<=  period;
                  ret->errors[seq].reason = CORBA::string_dup( "bad periody interval");
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;
               }
            if(  TestValidityExpDate(  valExpDate  , DBsql.GetValPreriod( zone ) )  ==  false )
              {
                  LOG( WARNING_LOG, "bad validity exp date" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainCreate_ext_valDate;
                  ret->errors[seq].value <<=   valExpDate;
                  ret->errors[seq].reason = CORBA::string_dup( "bad valExpDate");
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;

              }


        if( DBsql.UpdateCredit(  regID ,  id  ,  zone ,  period ,  EPP_DomainCreate )  == false )
          {
               ret->errCode =  COMMAND_BILLING_FAILURE;
          }

                        if(  ret->errCode == 0  )
                        {
                          t = time( NULL );
                          crDate = t;   // datum a cas vytvoreni objektu
                          exDate = expiry_time( t, period );    // datum a cas expirace o pulnoci
                          // preved datum a cas expirace prodluz tim cas platnosti domeny
                          get_timestamp( exDate, expiryDate );
                          get_timestamp( crDate, createDate );


                          DBsql.INSERT( "DOMAIN" );
                          DBsql.INTO( "zone" );
                          DBsql.INTO( "id" );
                          DBsql.INTO( "roid" );
                          DBsql.INTO( "fqdn" );
                          DBsql.INTO( "CrDate" );
                          DBsql.INTO( "Exdate" );
                          DBsql.INTO( "ClID");
                          DBsql.INTO( "CrID" );
                          DBsql.INTO( "status" );
                          DBsql.INTO( "Registrant" );
                          DBsql.INTO( "nsset");
                          DBsql.INTOVAL( "authinfopw" , AuthInfoPw);
                                                                  
                                                                  
                          DBsql.VALUE( zone );
                          DBsql.VALUE( id );
                          DBsql.VALUE( roid );
                          DBsql.VALUE( FQDN );
                          DBsql.VALUE( createDate );
                          DBsql.VALUE( expiryDate );
                          DBsql.VALUE( regID );
                          DBsql.VALUE( regID );
                          DBsql.VALUE( "{ 1 }" ); // status OK
                          DBsql.VALUE( contactid );
                          DBsql.VALUE( nssetid );
                          DBsql.VAL(  AuthInfoPw);   

                          // pokud se insertovalo do tabulky
                          if( DBsql.EXEC() )
                            {

                              // pridej enum  extension
                              if( valExpDate )
                                {
                                  get_timestamp( valExpDate, valexpiryDate );
                                  DBsql.INSERT( "enumval" );
                                  DBsql.VALUE( id );
                                  DBsql.VALUE( valexpiryDate ); 
                                  if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;;
                                }

                              // zapis admin kontakty
                              len = admin.length();
                              if( len > 0  )
                                {
                                  for( i = 0; i < len; i++ )
                                    {
                                     // nsset
                                      if( get_HANDLE( HANDLE , admin[i] ) == false )
                                        {
                                          LOG( WARNING_LOG, "DomainCreate: bad tech Contact " );
                                          ret->errors.length( seq +1 );
                                          ret->errors[seq].code = ccReg::domainCreate_admin;
                                          ret->errors[seq].value <<= CORBA::string_dup(  admin[i] );
                                          ret->errors[seq].reason = CORBA::string_dup( "bad admin contact" );
                                          seq++;
                                          ret->errCode = COMMAND_PARAMETR_ERROR;
 
                                       }
                                      else 
                                      {
                                      adminid = DBsql.GetNumericFromTable( "Contact", "id", "handle", HANDLE  );

                                      if( adminid )
                                       {
                                          DBsql.INSERT( "domain_contact_map" );
                                          DBsql.VALUE( id );
                                          DBsql.VALUE( adminid );
                                         // pokud se nepodarilo pridat do tabulky                                     
                                         if( DBsql.EXEC() == false )   ret->errCode = COMMAND_FAILED;
                                       }   
                                      else
                                      {
                                          LOG( WARNING_LOG, "DomainCreate: unknow tech Contact " );
                                          ret->errors.length( seq +1 );
                                          ret->errors[seq].code = ccReg::domainCreate_admin;
                                          ret->errors[seq].value <<= CORBA::string_dup(  admin[i] );
                                          ret->errors[seq].reason = CORBA::string_dup( "unknow admin contact" );
                                          seq++;
                                         ret->errCode = COMMAND_PARAMETR_ERROR;
                                      }
                                     } 
                                    }
                                }

                              if( ret->errCode == 0 )   // pokud zadna chyba uloz do historie
                                {
                                  //  uloz do historie
                                  if( DBsql.MakeHistory() )
                                    {
                                      if( DBsql.SaveHistory( "enumval", "domainID", id ) )
                                        {
                                          if( DBsql.SaveHistory( "domain_contact_map", "domainID", id ) )
                                            {
                                              if( DBsql.SaveHistory( "DOMAIN", "id", id ) ) ret->errCode = COMMAND_OK;
                                            }
                                        }
                                    }
                                }

                            

                      }                        

                    

                }

              // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }
   }
 }

          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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
                                            const char *clTRID, const  char* XML , const ccReg::ExtensionList & ext )
{
  DB DBsql;
  Status status;
  const ccReg::ENUMValidationExtension * enumVal;
  char exDateStr[24], valexpiryDate[32];
  char FQDN[64]; 
  ccReg::Response * ret;
  int clid, regID, id, len, i , zone , seq;
  bool stat;
  time_t ex = 0, t, valExpDate = 0 , now ;

  ret = new ccReg::Response;

  t = curExpDate;
// aktualni cas
   now = time(NULL);
// default
  exDate = 0;
  seq = 0;

// default
  ret->errCode = 0;
  ret->errors.length( 0 );


  LOG( NOTICE_LOG, "DomainRenew: clientID -> %d clTRID [%s] fqdn  [%s] period %d month", clientID, clTRID, fqdn, period );


  len = ext.length();

  if( len > 0 )
    {
      LOG( NOTICE_LOG, "extension length %d", ext.length() );
      for( i = 0; i < len; i++ )
        {
          if( ext[i] >>= enumVal )
            {
              valExpDate = enumVal->valExDate;
              LOG( NOTICE_LOG, "enumVal %d ", valExpDate );
            }
          else
            {
              LOG( ERROR_LOG, "Unknown value extension[%d]", i );
              break;
            }

        }
    }



  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_DomainRenew,  clTRID , XML ) )
        {

       if(  ( zone = get_FQDN( FQDN , fqdn ) ) > 0 ) 
         {
            // zahaj transakci
          if( DBsql.BeginTransaction() )
            {

          regID = DBsql.GetLoginRegistrarID( clientID );        // aktivni registrator
          if( ( id = DBsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", ( char * ) FQDN ) ) == 0 )
            // prvni test zdali domena  neexistuje 
            {
              ret->errCode = COMMAND_OBJECT_NOT_EXIST;  // domena neexistujea
              LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
            }
          else
            {


              if( DBsql.SELECTONE( "DOMAIN", "id", id ) )
                {

                  clid = atoi( DBsql.GetFieldValueName( "ClID", 0 ) );
                  ex = get_time_t( DBsql.GetFieldValueName( "ExDate", 0 ) );    // datum a cas  expirace domeny
                  DBsql.FreeSelect();
                }

              if( ex != curExpDate )
                {
                  LOG( WARNING_LOG, "curExpDate is not same as ExDate" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainRenew_curExpDate;
                  ret->errors[seq].value <<=  curExpDate;
                  ret->errors[seq].reason = CORBA::string_dup( "curExpDate is not ExDate");
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;
                }
                  
             // nastaveni defaultni periody           
             if( period == 0 ) 
               {
                 period = DBsql.GetExPreriodMin( zone );
                 LOG( NOTICE_LOG, "get defualt peridod %d month  for zone   %d ", period , zone  );

               }
  
             if(  TestPeriodyInterval( period  ,   DBsql.GetExPreriodMin( zone )  ,  DBsql.GetExPreriodMax( zone )  )  == false ) 
              {
                  LOG( WARNING_LOG, "bad period interval" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainRenew_period;
                  ret->errors[seq].value <<=  period;
                  ret->errors[seq].reason = CORBA::string_dup( "bad periody interval");
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;                 
               }
            if(  TestValidityExpDate(  valExpDate  , DBsql.GetValPreriod( zone ) )  ==  false )
              {
                  LOG( WARNING_LOG, "bad validity exp date" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainRenew_ext_valDate;
                  ret->errors[seq].value <<=   valExpDate;
                  ret->errors[seq].reason = CORBA::string_dup( "bad valExpDate");
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;

              }

        if( DBsql.UpdateCredit(  regID ,  id  ,  zone ,   period ,  EPP_DomainRenew )  == false )
          {
               ret->errCode =  COMMAND_BILLING_FAILURE;
          }

               if(  ret->errCode == 0 )
                 {
                  // zpracuj  pole statusu
                  status.Make( DBsql.GetStatusFromTable( "DOMAIN", id ) );

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
                      if( DBsql.MakeHistory() )
                        {

                        if( DBsql.SaveHistory( "enumval",  "domainID", id ) ) // uloz extension 
                          {
                          if( DBsql.SaveHistory( "domain_contact_map", "domainID", id ) )       // uloz kontakty
                            {
                              if( DBsql.SaveHistory( "Domain", "id", id ) )     // uloz zaznam
                                {

                                  if( valExpDate )      // zmena extension
                                    {
                                      get_timestamp( valExpDate, valexpiryDate );
                                      LOG( NOTICE_LOG, "change valExpDate %s ", valexpiryDate );

                                      DBsql.UPDATE( "enumval" );
                                      DBsql.SET( "ExDate", valexpiryDate );
                                      DBsql.WHERE( "domainID", id );

                                      if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;
                                    }

                                   if( ret->errCode == 0 ) // pokud je OK
                                   {
                                  // zmena platnosti domeny
                                  DBsql.UPDATE( "DOMAIN" );
                                  DBsql.SET( "ExDate", exDateStr );
                                  DBsql.WHEREID( id );
                                  if( DBsql.EXEC() ) ret->errCode = COMMAND_OK;
                                  else ret->errCode = COMMAND_FAILED;
                                  }
                                }
                            }
                          }

                           }


                        }

                    }
                }


              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }


          }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
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
 *              authInfo - autentifikace heslem 
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 *
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response * ccReg_EPP_i::DomainTransfer( const char *fqdn, const char *authInfo, 
                                 CORBA::Long clientID, const char *clTRID , const  char* XML  )
{
ccReg::Response * ret;
DB DBsql;
char FQDN[64];
Status status;
int regID = 0, clID = 0, id;  //   registrantid , contactid;

ret = new ccReg::Response;

// default
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "DomainTransfer: clientID -> %d clTRID [%s] fqdn  [%s]  ", clientID, clTRID, fqdn );


  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_DomainTransfer, clTRID , XML  ) )
        {
      if(  get_FQDN( FQDN , fqdn )    )  // spatny format navu domeny
         {

if( DBsql.BeginTransaction() )
 {
          // pokud domena existuje
          if( ( id = DBsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", ( char * ) FQDN ) ) == 0 )
            {
              ret->errCode = COMMAND_OBJECT_NOT_EXIST;
              LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
            }
          else 
            {
              // get  registrator ID
              regID = DBsql.GetLoginRegistrarID( clientID );
              // client contaktu
              clID = DBsql.GetNumericFromTable( "DOMAIN", "clID", "id", id );

              if( regID == clID )       // transfer nemuze delat stavajici client
                {
                  LOG( WARNING_LOG, "client can not transfer domain %s", fqdn );
                  ret->errCode = COMMAND_NOT_ELIGIBLE_FOR_TRANSFER;
                }
              else
                {
                  // zpracuj  pole statusu
                  status.Make( DBsql.GetStatusFromTable( "DOMAIN", id ) );

                  if( status.Test( STATUS_TRANSFER ) )
                    {
                      LOG( WARNING_LOG, "status TransferProhibited" );
                      ret->errCode = COMMAND_STATUS_PROHIBITS_OPERATION;

                    }
                  else
                    {
                      // autentifikace
                      if( DBsql.AuthTable( "DOMAIN", ( char * ) authInfo, id ) == false )
                        {
                          ret->errCode = COMMAND_AUTOR_ERROR;   // spatna autorizace
                          LOG( WARNING_LOG, "autorization error bad authInfo [%s] ", authInfo );
                        }
                      else
                        {
                          //  uloz do historie
                          if( DBsql.MakeHistory() )
                            {
                        if( DBsql.SaveHistory( "enumval",  "domainID", id ) ) // uloz extension
                          {
                          if( DBsql.SaveHistory( "domain_contact_map", "domainID", id ) )       // uloz kontakty
                            {

                              if( DBsql.SaveHistory( "Domain", "id", id ) )     // uloz zaznam
                                {
                                  // zmena registratora
                                  DBsql.UPDATE( "DOMAIN" );
                                  DBsql.SET( "TrDate", "now" );
                                  DBsql.SET( "ClID", regID );
                                  DBsql.WHEREID( id );
                                  if( DBsql.EXEC() ) ret->errCode = COMMAND_OK;  // nastavit OK                                  
                                  else ret->errCode = COMMAND_FAILED;
                                }
                              }
                             }
                            }

                        }

                    }
                    }
                }
              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }
          }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }


      ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );

      DBsql.Disconnect();
    }


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

return ret;
}



ccReg::RegistrarList* ccReg_EPP_i::getRegistrars()
{
int rows = 0 , i ;
DB DBsql;
ccReg::RegistrarList *reglist;

reglist = new ccReg::RegistrarList;

if( DBsql.OpenDatabase( database ) )
  {
   if( DBsql.ExecSelect( "SELECT * FROM REGISTRAR;"  ) )
     {
       rows = DBsql.GetSelectRows();
       LOG( NOTICE_LOG, "getRegistrars: num -> %d",  rows );
       reglist->length( rows );

       for( i = 0 ; i < rows ; i ++ )
          {    
             //  *(reglist[i]) = new ccReg::Registrar;   

             (*reglist)[i].id = atoi( DBsql.GetFieldValueName("ID" , i ) );
             (*reglist)[i].handle=CORBA::string_dup( DBsql.GetFieldValueName("handle" , i ) ); // handle
             (*reglist)[i].name=CORBA::string_dup( DBsql.GetFieldValueName("name" , i ) ); 
             (*reglist)[i].organization=CORBA::string_dup( DBsql.GetFieldValueName("organization" , i ) ); 
             (*reglist)[i].street1=CORBA::string_dup( DBsql.GetFieldValueName("street1" , i ) );
             (*reglist)[i].street2=CORBA::string_dup( DBsql.GetFieldValueName("street2" , i ) );
             (*reglist)[i].street3=CORBA::string_dup( DBsql.GetFieldValueName("street3" , i ) );
             (*reglist)[i].city=CORBA::string_dup( DBsql.GetFieldValueName("city" , i ) );
             (*reglist)[i].stateorprovince=CORBA::string_dup( DBsql.GetFieldValueName("stateorprovince" , i ) );
             (*reglist)[i].postalcode=CORBA::string_dup( DBsql.GetFieldValueName("postalcode" , i ) );
             (*reglist)[i].country=CORBA::string_dup( DBsql.GetFieldValueName("country" , i ) );
             (*reglist)[i].telephone=CORBA::string_dup( DBsql.GetFieldValueName("telephone" , i ) );
             (*reglist)[i].fax=CORBA::string_dup( DBsql.GetFieldValueName("fax" , i ) );
             (*reglist)[i].email=CORBA::string_dup( DBsql.GetFieldValueName("email" , i ) );
             (*reglist)[i].url=CORBA::string_dup( DBsql.GetFieldValueName("url" , i ) );

          }

       DBsql.FreeSelect();
     }

    DBsql.Disconnect();
  }

if( rows == 0 ) reglist->length( 0 ); // nulova delka

return  reglist;
}

ccReg::Registrar* ccReg_EPP_i::getRegistrarByHandle(const char* handle)
{
DB DBsql;
ccReg::Registrar *reg;
char sqlString[128];
bool find=false;

reg = new ccReg::Registrar ;

if( DBsql.OpenDatabase( database ) )
  { 
    sprintf( sqlString , "SELECT * FROM REGISTRAR WHERE handle=\'%s\';" , handle );
    LOG( NOTICE_LOG, "getRegistrar: num -> %s", handle );

   if( DBsql.ExecSelect( sqlString ) )
   {
        if( DBsql.GetSelectRows() == 1 )
         {
 
             reg->id =  atoi( DBsql.GetFieldValueName("ID" , 0 ) );
             reg->handle=CORBA::string_dup( DBsql.GetFieldValueName("handle" , 0 ) ); // handle
             reg->name=CORBA::string_dup( DBsql.GetFieldValueName("name" , 0 ) );
             reg->organization=CORBA::string_dup( DBsql.GetFieldValueName("organization" , 0 ) );
             reg->street1=CORBA::string_dup( DBsql.GetFieldValueName("street1" , 0 ) );
             reg->street2=CORBA::string_dup( DBsql.GetFieldValueName("street2" , 0 ) );
             reg->street3=CORBA::string_dup( DBsql.GetFieldValueName("street3" , 0 ) );
             reg->city=CORBA::string_dup( DBsql.GetFieldValueName("city" , 0 ) );
             reg->stateorprovince=CORBA::string_dup( DBsql.GetFieldValueName("stateorprovince" , 0 ) );
             reg->postalcode=CORBA::string_dup( DBsql.GetFieldValueName("postalcode" , 0 ) );
             reg->country=CORBA::string_dup( DBsql.GetFieldValueName("country" , 0 ) );
             reg->telephone=CORBA::string_dup( DBsql.GetFieldValueName("telephone" , 0 ) );
             reg->fax=CORBA::string_dup( DBsql.GetFieldValueName("fax" , 0 ) );
             reg->email=CORBA::string_dup( DBsql.GetFieldValueName("email" , 0 ) );
             reg->url=CORBA::string_dup( DBsql.GetFieldValueName("url" , 0 ) );

            find = true;
         }
     
    DBsql.FreeSelect();
    }

    DBsql.Disconnect();
  }

 
if( find == false )
{
reg->id =  0;
reg->handle=CORBA::string_dup( ""  ); // handle
reg->name=CORBA::string_dup( ""  );
reg->organization=CORBA::string_dup( "" );
reg->street1=CORBA::string_dup( "" );
reg->street2=CORBA::string_dup( "" );
reg->street3=CORBA::string_dup( "" );
reg->city=CORBA::string_dup( "" );
reg->stateorprovince=CORBA::string_dup( "" );
reg->postalcode=CORBA::string_dup( "" );
reg->country=CORBA::string_dup( "" );
reg->telephone=CORBA::string_dup( "" );
reg->fax=CORBA::string_dup( "" );
reg->email=CORBA::string_dup( "" );
reg->url=CORBA::string_dup( "" );
}

return reg;
}



// primitivni vypis
ccReg::Lists*  ccReg_EPP_i::ObjectList( char* table , char *fname )
{
DB DBsql;
ccReg::Lists *list;
int rows =0, i;

list = new ccReg::Lists;


if( DBsql.OpenDatabase( database ) )
  {
   DBsql.SELECTFROM( fname , table );


   if( DBsql.SELECT() )
     {
       rows = DBsql.GetSelectRows();
       LOG( NOTICE_LOG, "List: %s  num -> %d",  table , rows );
       list->length( rows );

       for( i = 0 ; i < rows ; i ++ )
          {
             (*list)[i]=CORBA::string_dup( DBsql.GetFieldValue(  i , 0 )  ); 
          }

      DBsql.FreeSelect();
     }
    DBsql.Disconnect();

  }

if( rows == 0 ) list->length( 0 ); // nulova delka

return list;
}

ccReg::Lists* ccReg_EPP_i::ListRegistrar()
{
return ObjectList( "REGISTRAR" , "handle" );
}
ccReg::Lists* ccReg_EPP_i::ListDomain()
{
return ObjectList( "DOMAIN" , "fqdn" );
}
ccReg::Lists* ccReg_EPP_i::ListContact()
{
return ObjectList( "CONTACT" , "handle" );
}
ccReg::Lists* ccReg_EPP_i::ListNSSet()
{
return ObjectList( "NSSET" , "handle" );
}


ccReg::RegObjectType ccReg_EPP_i::getRegObjectType(const char* objectName)
{
DB DBsql;
char sqlString[128];
int zone , id ;

if( DBsql.OpenDatabase( database ) )
  {
  if( ( id =  DBsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", (char *) objectName ) )  > 0  )
    {
      zone =  DBsql.GetNumericFromTable( "DOMAIN", "zone" , "id" ,  id ) ;
      switch( zone )
            {
               case ZONE_CZ:
                              return ccReg::CZ_DOMAIN;
               case ZONE_ENUM:
                              return ccReg::ENUM_DOMAIN;
               
            }      
    
    }
   else
   {
   if(  DBsql.GetNumericFromTable( "CONTACT", "id", "handle", (char *) objectName ) ) return ccReg::CONTACT_HANDLE;
   else 
        if(  DBsql.GetNumericFromTable( "NSSET", "id", "handle", (char *) objectName ) ) return ccReg::NSSET_HANDLE;
   }

    DBsql.Disconnect();  
 }


// deafult
return  ccReg::NONE;
}
 

// primitivni vypis
ccReg::Response*  ccReg_EPP_i::FullList(short act , const char *table , char *fname  ,  ccReg::Lists_out  list ,   CORBA::Long clientID, const char* clTRID, const char* XML )
{
DB DBsql;
int rows =0, i;
ccReg::Response * ret;
int regID;
ret = new ccReg::Response;

// default
ret->errors.length( 0 );
ret->errCode =0 ; // default

list = new ccReg::Lists;

LOG( NOTICE_LOG ,  "LIST %d  clientID -> %d clTRID [%s] " , act  , clientID , clTRID );

if( DBsql.OpenDatabase( database ) )
{
  if( DBsql.BeginAction( clientID , act ,  clTRID , XML  ) )
  {

    // get  registrator ID
    regID = DBsql.GetLoginRegistrarID( clientID );

   DBsql.SELECTFROM( fname , table );
   DBsql.WHERE( "ClID" , regID );

   if( DBsql.SELECT() )
     {
       rows = DBsql.GetSelectRows();
        
       LOG( NOTICE_LOG, "Full List: %s  num -> %d ClID %d",  table , rows  ,  regID );
       list->length( rows );

       for( i = 0 ; i < rows ; i ++ )
          {
             (*list)[i]=CORBA::string_dup( DBsql.GetFieldValue(  i , 0 )  ); 
          }

      DBsql.FreeSelect();
     }



      // comand OK
     if( ret->errCode == 0 ) ret->errCode=COMMAND_OK; // vse proslo OK zadna chyba

      // zapis na konec action
      ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) ) ;
  }

ret->errMsg =  CORBA::string_dup(   DBsql.GetErrorMessage(  ret->errCode  ) ) ;

DBsql.Disconnect();
}


if( ret->errCode == 0 )
  {
    ret->errCode = COMMAND_FAILED;    // obecna chyba
    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
    ret->errMsg = CORBA::string_dup( "" );
  }

return ret;
}


ccReg::Response* ccReg_EPP_i::ContactList(ccReg::Lists_out contacts, CORBA::Long clientID, const char* clTRID, const char* XML)
{
return FullList( EPP_ListContact , "CONTACT"  , "HANDLE" , contacts ,  clientID,  clTRID,  XML);
}



ccReg::Response* ccReg_EPP_i::NSSetList(ccReg::Lists_out nssets, CORBA::Long clientID, const char* clTRID, const char* XML)
{
return FullList( EPP_ListNSset  , "NSSET"  , "HANDLE" , nssets ,  clientID,  clTRID,  XML);
}


ccReg::Response* ccReg_EPP_i::DomainList(ccReg::Lists_out domains, CORBA::Long clientID, const char* clTRID, const char* XML)
{
return FullList( EPP_ListDomain , "DOMAIN"  , "fqdn" , domains ,  clientID,  clTRID,  XML);
}



