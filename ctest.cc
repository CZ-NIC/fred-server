//  implementing IDL interfaces for file ccReg.idl
// autor Petr Blaha petr.blaha@nic.cz

#include <fstream.h>
#include <iostream.h>

#include <stdlib.h> 
#include <string.h>
#include <time.h>

#include "ctest.h"


// pmocne funkce
#include "util.h"

#include "action.h"    // kody funkci do ccReg
#include "response.h"  // vracene chybove kody
		
// prace se status flagy
#include "status.h"

// log
#include "log.h"


ccReg_EPP_test::ccReg_EPP_test(const char *db)
{
// zkopiruj pri vytvoreni instance
strcpy( database , db ); // retezec na  pripojeni k Postgres SQL
}
// test spojeni na databazi
bool ccReg_EPP_test::DatabaseConnect()
{

if(  DBsql.OpenDatabase( database ) )
{
LOG( NOTICE_LOG ,  "succefuly connect to:  [%s]" , database );
return true;
}
else
{
LOG( ERROR_LOG , "can not connect to database: [%s]" , database );
return false;
}

}


void ccReg_EPP_test::DatabaseDisconnect()
{
LOG( NOTICE_LOG ,  "Disconnec to:  [%s]" , database );
DBsql.Disconnect();
}


int  ccReg_EPP_test::ContactDelete(const char* handle , long clientID, const char* clTRID , const char* XML )
{
int ret_errCode;
// DB DBsql;
Status status;
char HANDLE[64];
int regID=0 , id ,  crID =0  ;



ret_errCode=COMMAND_FAILED;


LOG( NOTICE_LOG ,  "ContactDelete: clientID -> %d clTRID [%s] handle [%s] " , clientID , clTRID , handle );


#ifndef PERNAMENT
   if( DBsql.OpenDatabase( database ) )
#endif
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
                  ret_errCode = COMMAND_OBJECT_NOT_EXIST;      // pokud objekt neexistuje
                }
              else 
                {
                  // get  registrator ID
                  regID = DBsql.GetLoginRegistrarID( clientID );
                  // client contaktu ktery ho vytvoril
                  crID = DBsql.GetNumericFromTable( "CONTACT", "crID", "handle", ( char * ) HANDLE );


                  if( regID != crID )   // pokud neni tvurcem kontaktu 
                    {
                      LOG( WARNING_LOG, "bad autorization not  creator of handle [%s]", handle );
                      ret_errCode = COMMAND_AUTOR_ERROR; // spatna autorizace
                    }                               
                  else                                                                                           
                    {
                      // zpracuj  pole statusu
                      status.Make( DBsql.GetStatusFromTable( "CONTACT", id ) );

                      if( status.Test( STATUS_DELETE ) )
                        {
                          LOG( WARNING_LOG, "status DeleteProhibited" );
                          ret_errCode = COMMAND_STATUS_PROHIBITS_OPERATION;
                        }
                      else // status je OK
                        {
                          // test na vazbu do tabulky domain domain_contact_map a nsset_contact_map
                          if( DBsql.TestContactRelations( id ) )        // kontakt nemuze byt smazan ma vazby  
                            {
                              LOG( WARNING_LOG, "test contact handle [%s] relations: PROHIBITS_OPERATION", handle );
                              ret_errCode = COMMAND_PROHIBITS_OPERATION;
                            }
                          else
                            {
                              //  uloz do historie
                              if( DBsql.MakeHistory() )
                                {
                                  if( DBsql.SaveHistory( "Contact", "id", id ) ) // uloz zaznam
                                    {
                                      if( DBsql.DeleteFromTable( "CONTACT", "id", id ) ) ret_errCode = COMMAND_OK;      // pokud usmesne smazal
                                    }
                                }


                            }

                        }

                    }


                }

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret_errCode );
            }

          }
          // zapis na konec action
     //     ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret_errCode ) );
         LOG( NOTICE_LOG , "svTRID %s" ,      DBsql.EndAction( ret_errCode ) );
        }


   //   ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret_errCode ) );

#ifndef PERNAMENT
      DBsql.Disconnect();
#endif
    }

return ret_errCode;
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


int ccReg_EPP_test::ContactCreate( const char *handle, const ContactChange & c, timestamp & crDate,  long clientID, const char *clTRID , const char* XML )
{
// DB DBsql;
char createDate[32];
char roid[64];
char HANDLE[64]; // handle na velka pismena
// ccReg::Response * ret;
int ret_errCode;
int regID, id;
int i , len;
time_t now;

crDate = 0;


LOG( NOTICE_LOG, "ContactCreate: clientID -> %d clTRID [%s] handle [%s]", clientID, clTRID, handle );
LOG( NOTICE_LOG, "ContactCreate: Disclose Name %d Org %d Add %d Tel %d Fax %d Email %d" ,
 c.DiscloseName  , c.DiscloseOrganization , c.DiscloseAddress , c.DiscloseTelephone , c.DiscloseFax , c.DiscloseEmail );

#ifndef PERNAMENT
   if( DBsql.OpenDatabase( database ) )
#endif
    {
      if( DBsql.BeginAction( clientID, EPP_ContactCreate,  clTRID ,  XML ) )
        {

       // preved handle na velka pismena
       if( get_HANDLE( HANDLE , handle ) == false )  // spatny format handlu
         {

            ret_errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "bad format  of handle[%s]" , handle );
/*
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::contactCreate_handle; 
            ret->errors[0].value <<= CORBA::string_dup( handle );
            ret->errors[0].reason = CORBA::string_dup( "bad format of handle" );
*/
        }
        else 
        {
          if( DBsql.BeginTransaction() )      // zahajeni transakce
            {
              // test zdali kontakt uz existuje
              if( DBsql.CheckObject( "CONTACT",  "handle", handle  ) )
                {
                  LOG( WARNING_LOG, "object handle [%s] EXIST", handle  );
                  ret_errCode = COMMAND_OBJECT_EXIST;  // je uz zalozena
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
                      DBsql.INTO( "status" );

                      DBsql.INTOVAL( "Name", c.Name );
                      DBsql.INTOVAL( "Organization", c.Organization );
/*
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
*/

                      if( c.DiscloseName ==  1 ) DBsql.INTO( "DiscloseName" );
                      if( c.DiscloseOrganization == 1  ) DBsql.INTO( "DiscloseOrganization" );
                      if( c.DiscloseAddress == 1 ) DBsql.INTO( "DiscloseAddress" );
                      if( c.DiscloseTelephone == 1 ) DBsql.INTO( "DiscloseTelephone" );
                      if( c.DiscloseFax == 1 ) DBsql.INTO( "DiscloseFax" );
                      if( c.DiscloseEmail == 1 )DBsql.INTO( "DiscloseEmail" );

                      DBsql.VALUE( id );
                      DBsql.VALUE( roid );
                      DBsql.VALUE( HANDLE );
                      DBsql.VALUE( createDate );
                      DBsql.VALUE( regID );
                      DBsql.VALUE( "{ 1 }" );   // OK status


                      DBsql.VAL( c.Name );
                      DBsql.VAL( c.Organization );
/*
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
*/

                      if( c.DiscloseName == 1 ) DBsql.VALUE( "t" );
                      if( c.DiscloseOrganization ==  1 ) DBsql.VALUE( "t" );
                      if( c.DiscloseAddress ==  1 ) DBsql.VALUE( "t" );
                      if( c.DiscloseTelephone ==  1 ) DBsql.VALUE( "t" );
                      if( c.DiscloseFax ==  1 ) DBsql.VALUE( "t" );
                      if( c.DiscloseEmail == 1 ) DBsql.VALUE( "t" );




                      // pokud se podarilo insertovat
                      if( DBsql.EXEC() )      //   ret->errCode = COMMAND_OK;
                        {       //  uloz do historie
                          if( DBsql.MakeHistory() )
                            {
                              if( DBsql.SaveHistory( "Contact", "id", id ) )    // uloz zaznam
                                {
                                  ret_errCode = COMMAND_OK;    // pokud se ulozilo do Historie
                                }
                            }
                        }

                    }
                  else          // neplatny kod zeme  
                    {
                      ret_errCode = COMMAND_PARAMETR_ERROR;
                      LOG( WARNING_LOG, "unknow country code" );
/*
                      ret->errors.length( 1 );
                      ret->errors[0].code = ccReg::contactCreate_cc;    // spatne zadany neznamy country code
                      ret->errors[0].value <<= CORBA::string_dup( c.CC );
                      ret->errors[0].reason = CORBA::string_dup( "unknow country code" );
*/
                    }
                }

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret_errCode );
            }
          }
          // zapis na konec action
   //        ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
         LOG( NOTICE_LOG , "svTRID %s" ,      DBsql.EndAction( ret_errCode ) );

        }

//       ret->errMsg = CORBA::string_dup( DBsql.GetErrorMessage( ret->errCode ) );      
#ifndef PERNAMENT
      DBsql.Disconnect();
#endif
   }


if( ret_errCode == 0 )
  {
    ret_errCode = COMMAND_FAILED;
//    ret->svTRID = CORBA::string_dup( "" );    // prazdna hodnota
//    ret->errMsg = CORBA::string_dup( "" );
  }

return ret_errCode ;
}

