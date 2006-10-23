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
#include "reason.h"
		
// prace se status flagy
#include "status.h"

// log
#include "log.h"
//
// Example implementational code for IDL interface ccReg::EPP
//
ccReg_EPP_i::ccReg_EPP_i(ccReg::Admin_ptr _admin , ccReg::Whois_ptr _whois ) : admin(_admin) ,  whois(_whois) {

}
ccReg_EPP_i::~ccReg_EPP_i(){
 
}

ccReg::Admin_ptr 
ccReg_EPP_i::getAdmin()
{
  return ccReg::Admin::_duplicate(admin);
}


ccReg::Whois_ptr 
ccReg_EPP_i::getWhois()
{
  return ccReg::Whois::_duplicate(whois);
}

// test spojeni na databazi
bool ccReg_EPP_i::TestDatabaseConnect(char *db)
{
DB  DBsql;

// zkopiruj pri vytvoreni instance
strcpy( database , db ); // retezec na  pripojeni k Postgres SQL

if(  DBsql.OpenDatabase( database ) )
{
LOG( NOTICE_LOG ,  "successfully  connect to DATABASE" );
DBsql.Disconnect();
return true;
}
else
{
LOG( ERROR_LOG , "can not connect to DATABASE" );
return false;
}

}

// ziskani cisla verze plus datum cas
char* ccReg_EPP_i::version(ccReg::timestamp_out datetime)
{
char *version;
time_t t;
char dateStr[MAX_DATE];

// casova znacka
t = time(NULL);

// char str[64];
version =  new char[128];

sprintf( version , "SVN %s BUILD %s %s" , SVERSION , __DATE__ , __TIME__ );
LOG( NOTICE_LOG , "get version %s" , version );

// aktualnidatum a cas
// vrat casove razitko

get_rfc3339_timestamp( t , dateStr  );
datetime =  CORBA::string_dup( dateStr );


return version;
}



// parse extension
void  ccReg_EPP_i::GetValExpDateFromExtension( char *valexpDate , const ccReg::ExtensionList& ext )
{
int len , i ;
const ccReg::ENUMValidationExtension * enumVal;

strcpy( valexpDate , "" );


  len = ext.length();
  if( len > 0 )
    {
      LOG( DEBUG_LOG, "extension length %d", (int ) ext.length() );
      for( i = 0; i < len; i++ )
        {
          if( ext[i] >>= enumVal )
            {
              strcpy( valexpDate, enumVal->valExDate );
              LOG( DEBUG_LOG, "enumVal %s ", valexpDate );
            }
          else
            {
              LOG( ERROR_LOG, "Unknown value extension[%d]", i );
              break;
            }

        }
    }

}


// DISCLOSE
/*
 info
A)  pokud je policy VSE ZOBRAZ, pak flag bude nastaven na DISCL_HIDE a polozky z databaze, ktere
    maji hodnotu 'false' budou mit hodnotu 'true', zbytek 'false'. Pokud ani jedna polozka nebude
    mit nastaveno 'true', vrati se flag DISCL_EMPTY (hodnoty polozek nejsou nepodstatny)

B)  pokud je policy VSE SKRYJ, pak flag bude nastaven na DISCL_DISPLAY a polozky z databaze, ktere
    maji hodnotu 'true' budou mit hodnotu 'true', zbytek 'false'.  Pokud ani jedna polozka nebude
    mit nastaveno 'true', vrati se flag DISCL_EMPTY (hodnoty polozek nejsou nepodstatny)
*/

// db parametr true ci false z DB
bool ccReg_EPP_i::get_DISCLOSE( bool db )
{
if( DefaultPolicy() )
  {
    if( db == false  )  return true;
    else return false;
  }
else
  {
    if( db == true )  return true;
    else return false;
  }

}

/*
1)   pokud je flag DISCL_HIDE a defaultni policy je VSE ZOBRAZ, pak se do databaze ulozi 'true'
     pro idl polozky s hodnotou 'false' a 'false' pro idl polozky s hodnotou 'true'

2)   pokud je flag DISCL_DISPLAY a defaultni policy je VSE ZOBRAZ, pak se do databaze ulozi ke vsem   polozkam 'true'

3)   pokud je flag DISCL_HIDE a defaultni policy je VSE SKRYJ, pak se do databaze ulozi ke vsem   polozkam 'false'

4)   pokud je flag DISCL_DISPLAY a defaultni policy je VSE SKRYJ, pak se do databaze ulozi 'true'
     pro idl polozky s hodnotou 'true' a 'false' pro idl polozky s hodnotou 'false'

5)   pokud je flag DISCL_EMPTY a defaultni policy je VSE ZOBRAZ, pak se do  databaze ulozi vsude 'true'
6)    pokud je flag DISCL_EMPTY a defaultni policy je VSE SKRYJ, pak se do  databaze ulozi vsude 'false'

update to samy jako create s tim ze pokud je flag:    DISCL_EMPTY tak se databaze neaktualizuje (bez ohledu na politiku serveru)
*/


// pro update
// d parametr nastaveni disclose pri update
char  ccReg_EPP_i::update_DISCLOSE( bool  d   ,  ccReg::Disclose flag )
{

if( flag ==  ccReg::DISCL_EMPTY ) return ' '; // nic nemeni v databazi
else 
 {
     if(  setvalue_DISCLOSE( d , flag ) ) return 't' ;
     else return 'f' ;
  }

}




// pro create
bool  ccReg_EPP_i::setvalue_DISCLOSE( bool  d   ,  ccReg::Disclose flag )
{

switch( flag )
{
case ccReg::DISCL_DISPLAY:
    if( DefaultPolicy() )  return true; // 2
    else // 4
      {
        if( d ) return true ;
         else return false ;
      }

case ccReg::DISCL_HIDE:
     if( DefaultPolicy() )
       {
           // 1
           if( d ) return false ;
           else return true ;
       }
     else  return true ; // 3

case ccReg::DISCL_EMPTY:
     // pouzij policy servru
     if( DefaultPolicy() )   return true; // 5
     else  return false ; // 6
}


// default
return false;
}



// ZONE

int  ccReg_EPP_i::loadZones()  // load zones
{
int rows=0 , i;
DB DBsql;

LOG( NOTICE_LOG, "LOAD zones" );
zone = new ccReg::Zones;

if( DBsql.OpenDatabase( database ) )
{


   if( DBsql.ExecSelect("select * from zone order by id") )
     {
       rows = DBsql.GetSelectRows();
       max_zone = rows;
 
       LOG( NOTICE_LOG, "Max zone  -> %d "  , max_zone );

      
       zone->length( rows );

       for( i = 0 ; i < rows ; i ++ )
          {
             (*zone)[i].fqdn=CORBA::string_dup( DBsql.GetFieldValueName( "fqdn" , i )   );
             (*zone)[i].ex_period_min= atoi( DBsql.GetFieldValueName( "ex_period_min" , i ));  
             (*zone)[i].ex_period_max=  atoi(  DBsql.GetFieldValueName( "ex_period_max" , i ));  
             (*zone)[i].val_period=  atoi(  DBsql.GetFieldValueName( "val_period" , i ));  
             (*zone)[i].dots_max=  atoi(   DBsql.GetFieldValueName( "dots_max" , i ) );  
             (*zone)[i].enum_zone =  DBsql.GetFieldBooleanValueName( "enum_zone" , i );
              LOG( NOTICE_LOG, "Get ZONE %d fqdn [%s] ex_period_min %d ex_period_max %d val_period %d dots_max %d  enum_zone %d" , i+1 ,
          ( char *)  (*zone)[i].fqdn , (*zone)[i].ex_period_min , (*zone)[i].ex_period_max , (*zone)[i].val_period , (*zone)[i].dots_max ,  (*zone)[i].enum_zone );  
          }

      DBsql.FreeSelect();
     }


DBsql.Disconnect();
}

if( rows == 0 ) zone->length( rows );

return rows;
}

  // parametry zone
int  ccReg_EPP_i::GetZoneExPeriodMin(int z)
{

if( z > 0 && z <=  (int )  zone->length() )
{
 LOG(LOG_DEBUG , "GetZoneExPreriodMin zone %d -> %d" , z , (*zone)[z-1].ex_period_min );
 return (*zone)[z-1].ex_period_min;
} 
else
return 0;
}

int  ccReg_EPP_i::GetZoneExPeriodMax(int z)
{

if( z > 0 && z <= (int )  zone->length() )
{
 LOG(LOG_DEBUG , "GetZoneExPreriodMax zone %d -> %d" , z , (*zone)[z-1].ex_period_max );
 return (*zone)[z-1].ex_period_max;
}
else
return 0;
}

int  ccReg_EPP_i::GetZoneValPeriod(int z)
{

if( z > 0 && z <=  (int ) zone->length() )
{
 LOG(LOG_DEBUG , "GetZoneValPreriod zone %d -> %d" , z , (*zone)[z-1].val_period );
 return (*zone)[z-1].val_period;
}
else
return 0;

}

bool ccReg_EPP_i::GetZoneEnum(int z)
{

if( z > 0 && z <= (int ) zone->length() )
{
  LOG(LOG_DEBUG , "GetZoneEnum zone %d -> %d" , z , (*zone)[z-1].enum_zone );
 return (*zone)[z-1].enum_zone;
}
else return 0;
}

int  ccReg_EPP_i::GetZoneDotsMax( int z) 
{
if( z > 0 && z <=  (int )  zone->length() )
{
 LOG(LOG_DEBUG , "GetZoneDotsMax zone %d -> %d" , z , (*zone)[z-1].dots_max );
 return (*zone)[z-1].dots_max ;
}
else
return 0;


}

char * ccReg_EPP_i::GetZoneFQDN( int z)
{

if( z > 0 && z <= (int ) zone->length() )
{
 LOG( LOG_DEBUG , "GetZoneFQDN zone %d -> [%s]" , z , (char *) (*zone)[z-1].fqdn );
 return (*zone)[z-1].fqdn;
}
else return "";

}

int ccReg_EPP_i::getZone( const char *fqdn )
{
return getZZ( fqdn , true );
}

int ccReg_EPP_i::getZoneMax( const char *fqdn )
{
return getZZ( fqdn , false );
}


int ccReg_EPP_i::getZZ( const char *fqdn , bool compare )
{
int max , i ;
int  len  , slen , l ;

max = (int ) zone->length();

len = strlen(  fqdn );

for(  i = 0 ; i < max ; i ++ )
   {
  
        slen = strlen(  (char *) (*zone)[i].fqdn );
        l = len - slen ;

        if( l > 0 )
         {
          if( fqdn[l-1] == '.' ) // case less comapare
             {
                if( compare ) 
                  {
                     if(  strncasecmp(  fqdn+l ,  (char *) (*zone)[i].fqdn  , slen ) == 0 ) return i +1; // zaradi do zony                  
                  }
                 else return l -1 ; // vraci konec nazvu                        
             }
         }

   }

// vrat max konec nazvu domeny bez tecky
if( compare == false )
{
for( l = len-1 ; l > 0 ; l -- )
  {
    if(  fqdn[l] == '.' )  return l-1;   // vraci konec nazvu domeny 
  }
}

return 0;
}


bool ccReg_EPP_i::testFQDN( const char *fqdn )
{
char FQDN[64];

if( getFQDN( FQDN ,  fqdn ) > 0  ) return true;
else return false;
}

int ccReg_EPP_i::getFQDN( char *FQDN , const char *fqdn )
{
int i , len , max ;
int z;
int dot=0 , dot_max;
bool en;
z = getZone( fqdn  );
max = getZoneMax( fqdn  ); // konec nazvu

len = strlen( fqdn);

FQDN[0] = 0;

LOG( LOG_DEBUG ,  "getFQDN [%s] zone %d max %d" , fqdn  , z , max );

// maximalni delka
if( len > 63 ) { LOG( LOG_DEBUG ,  "out ouf maximal length %d" , len ); return -1; }
if( max == 0 ) { LOG( LOG_DEBUG ,  "minimal length" ); return -1;}

// test double dot .. and double --
for( i = 1 ; i <  len ; i ++ )
{

if( fqdn[i] == '.' && fqdn[i-1] == '.' )
  {
   LOG( LOG_DEBUG ,  "double \'.\' not allowed" );
   return -1;
  }

if( fqdn[i] == '-' && fqdn[i-1] == '-' )
  {
   LOG( LOG_DEBUG ,  "double  \'-\' not allowed" );
   return -1;
  }

}


if( fqdn[0] ==  '-' )
{
    LOG( LOG_DEBUG ,  "first \'-\' not allowed" );
    return -1;
}




for( i = 0 ; i <=  max ; i ++ )
{
   if( fqdn[i] == '.' ) dot ++ ;
}

// test pocet tecek
dot_max = GetZoneDotsMax(z);

if( dot > dot_max )
{
    LOG( LOG_DEBUG ,  "too much %d dots max %d" , dot   , dot_max  );
    return -1;
}


en =  GetZoneEnum( z );



         for( i = 0 ; i <  max ; i ++ )
            {
              
              // TEST pro eunum zone a ccTLD
              if(  en )
                {
                 if(  isdigit( fqdn[i] )  ||  fqdn[i] == '.' ||  fqdn[i] == '-' ) FQDN[i] = fqdn[i];
                 else {  LOG( LOG_DEBUG ,  "character  %c not allowed"  , fqdn[i] );  FQDN[0] = 0 ;  return -1; }
                }
               else  
                {
                      // TEST povolene znaky
                     if( isalnum( fqdn[i]  ) ||  fqdn[i] == '-' ||  fqdn[i] == '.' ) FQDN[i] = tolower( fqdn[i] ) ;
                     else {  LOG( LOG_DEBUG ,  "character  %c not allowed"  , fqdn[i] );  FQDN[0] = 0 ;  return -1; }
                }


            }


            // PREVOD konce na mala  pismena
            for( i = max ; i < len ; i ++ )   FQDN[i] = tolower( fqdn[i] );
            FQDN[i] = 0 ; // ukocit


   LOG( LOG_DEBUG ,  "zone %d -> FQDN [%s]" , z ,  FQDN );
   return z;

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

LOG( NOTICE_LOG, "GetTransaction: clientID -> %d clTRID [%s] ",  (int )  clientID, clTRID );

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

LOG( NOTICE_LOG, "PollAcknowledgement: clientID -> %d clTRID [%s] msgID -> %d", (int) clientID, clTRID,   (int ) msgID );

  if( DBsql.OpenDatabase( database ) )
    {

      // get  registrator ID
      regID = DBsql.GetLoginRegistrarID( clientID );

      if( DBsql.BeginAction( clientID, EPP_PollAcknowledgement, clTRID , XML ) )
        {

          // test msg ID and clientID
          sprintf( sqlString, "SELECT * FROM MESSAGE WHERE id=%d AND clID=%d;",  (int ) msgID  , regID );
          rows = 0;
          if( DBsql.ExecSelect( sqlString ) )
            {
              rows = DBsql.GetSelectRows();
              DBsql.FreeSelect();
            }
          else
            ret->errCode = COMMAND_FAILED;

              if( rows == 0 )
                {
                  LOG( ERROR_LOG, "unknown msgID %d", (int)  msgID );
                  ret->errors.length( 1 );
                  ret->errCode = COMMAND_PARAMETR_ERROR;
                  ret->errors[0].code = ccReg::pollAck_msgID;   // spatna msg ID
                  ret->errors[0].value <<= msgID;
                  ret->errors[0].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_UNKNOW_MSGID) );
                }
          else  

          if( rows == 1 )       // pokud tam ta zprava existuje
            {
              // oznac zpravu jako prectenou  
              sprintf( sqlString, "UPDATE MESSAGE SET seen='t' WHERE id=%d AND clID=%d;", (int ) msgID, regID );

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
                          LOG( NOTICE_LOG, "PollAcknowledgement: newmsgID -> %d count -> %d", (int ) newmsgID, count );
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

ccReg::Response * ccReg_EPP_i::PollRequest( CORBA::Long & msgID, CORBA::Short & count, ccReg::timestamp_out qDate, CORBA::String_out mesg, CORBA::Long clientID, const char *clTRID , const char* XML )
{
DB DBsql;
char sqlString[1024];
char dateStr[MAX_DATE];
ccReg::Response * ret;
int regID;
int rows;
ret = new ccReg::Response;


//vyprazdni
qDate =  CORBA::string_dup( "" );
count = 0;
msgID = 0;
mesg = CORBA::string_dup( "" );       // prazdna hodnota

ret->errCode = 0;
ret->errors.length( 0 );


LOG( NOTICE_LOG, "PollRequest: clientID -> %d clTRID [%s] msgID %d",  (int ) clientID, clTRID, (int ) msgID );

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
                  // prevede cas s postgres na rfc3339 cas s offsetem casove zony
                  get_dateStr( dateStr , DBsql.GetFieldValueName("CrDate" , 0 ) ); 
                  qDate =  CORBA::string_dup( dateStr );
                  msgID = atoi( DBsql.GetFieldValueName( "ID", 0 ) );
                  mesg = CORBA::string_dup( DBsql.GetFieldValueName( "message", 0 ) );
                  ret->errCode = COMMAND_ACK_MESG;      // zpravy jsou ve fronte
                  LOG( NOTICE_LOG, "PollRequest: msgID -> %d count -> %d mesg [%s]",  (int )  msgID, count, CORBA::string_dup( mesg ) );
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
 * FUNCTION:    ClientCredit
 *
 * DESCRIPTION: informacee o vysi creditu  prihlaseneho registratora
 * PARAMETERS:  clientID - id pripojeneho klienta
 *              clTRID - cislo transakce klienta
 *        OUT:  credit - vyse creditu v halirich
 *       
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ClientCredit(ccReg::price& credit, CORBA::Long clientID, const char* clTRID, const char* XML)
{
DB DBsql;
ccReg::Response * ret;
int regID;

ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "ClientCredit: clientID -> %d clTRID [%s]",  (int )  clientID, clTRID );


  if( DBsql.OpenDatabase( database ) )
    {
      if( DBsql.BeginAction( clientID, EPP_ClientCredit, clTRID , XML  ) )
        {

           // get  registrator ID
           regID = DBsql.GetLoginRegistrarID( clientID );

           // vyse creditu registratora prevedena na halire
           credit  =  get_price( DBsql.GetRegistrarCredit( regID)  );

           ret->errCode = COMMAND_OK;   


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

LOG( NOTICE_LOG, "ClientLogout: clientID -> %d clTRID [%s]",  (int )  clientID, clTRID );


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
                        LOG( NOTICE_LOG, "GET clientID  -> %d",  (int )  clientID );

                        ret->errCode = COMMAND_OK; // zikano clinetID OK

                        // nankonec zmena komunikacniho  jazyka pouze na cestinu
                        if( lang == ccReg::CS )
                          {
                            LOG( NOTICE_LOG, "SET LANG to CS" ); 

                            DBsql.UPDATE( "Login" );
                            DBsql.SSET( "lang" , "cs"  );
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
unsigned long i , len;
int  zone ;
char HANDLE[64] , FQDN[64];
ret = new ccReg::Response;

a = new ccReg::CheckResp;


ret->errCode=0;
ret->errors.length(0);

len = chck.length();
a->length(len);

LOG( NOTICE_LOG ,  "OBJECT %d  Check: clientID -> %d clTRID [%s] " , act  ,  (int )  clientID , clTRID );


if( DBsql.OpenDatabase( database ) )
{

  if( DBsql.BeginAction( clientID , act ,  clTRID , XML  ) )
  {

    for( i = 0 ; i < len ; i ++ )
     {
      switch(act)
            {
                case EPP_ContactCheck:
                     if( get_HANDLE( HANDLE , chck[i] )  )
                         {
                            if( DBsql.CheckContact( HANDLE ) )
                              {
                                a[i].avail  =  ccReg::Exist ;    // objekt existuje
                                a[i].reason =  CORBA::string_dup(   DBsql.GetReasonMessage( REASON_MSG_CONTACT_EXIST ) );
                                LOG( NOTICE_LOG ,  "contact %s exist not Avail" , (const char * ) chck[i] );
                              }
                             else
                              {
                               if( DBsql.TestContactHandleHistory(  HANDLE  , DefaultContactHandlePeriod()  ) ) 
                                 {
                                     a[i].avail =  ccReg::DelPeriod;    // ochrana lhuta
                                     a[i].reason =  CORBA::string_dup(    DBsql.GetReasonMessage( REASON_MSG_CONTACT_HISTORY ) );  // v ochrane lhute
                                     LOG( NOTICE_LOG ,  "contact %s in delete period" ,(const char * ) chck[i] );
                                 }
                                else
                                 {
                                     a[i].avail =  ccReg::NotExist;    // objekt ne existuje
                                     a[i].reason =  CORBA::string_dup( "");  // free
                                     LOG( NOTICE_LOG ,  "contact %s not exist  Avail" ,(const char * ) chck[i] );
                                  }
                              }

                         }
                       else
                         {
                            LOG( NOTICE_LOG ,  "bad format %s of contact handle"  , (const char * ) chck[i] );
                            a[i].avail = ccReg::BadFormat;    // spatny format
                            a[i].reason =  CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_BAD_FORMAT_CONTACT_HANDLE  ));
                         }

                        break;

                  case EPP_NSsetCheck:
                       if( get_HANDLE( HANDLE , chck[i] )  )
                         {
                            if( DBsql.CheckNSSet( HANDLE ) )
                              {
                                a[i].avail = ccReg::Exist;    // objekt existuje
                                a[i].reason =  CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_NSSET_EXIST )  );
                                LOG( NOTICE_LOG ,  "nsset %s exist not Avail" , (const char * ) chck[i] );
                              }
                             else
                              {
                               if( DBsql.TestNSSetHandleHistory( HANDLE ,  DefaultDomainNSSetPeriod()  ) )
                                 {
                                     a[i].avail =  ccReg::DelPeriod;    // ochrana lhuta
                                     a[i].reason =  CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_NSSET_HISTORY )  );  // v ochrane lhute
                                     LOG( NOTICE_LOG ,  "nsset %s in delete period" ,(const char * ) chck[i] );
                                 }
                                else
                                 {
                                      a[i].avail =  ccReg::NotExist;    // objekt ne existuje
                                      a[i].reason =  CORBA::string_dup( "");  // free
                                      LOG( NOTICE_LOG ,  "nsset %s not exist  Avail" ,(const char * ) chck[i] );
                                  }
                              }

                         }
                       else
                         {
                            LOG( NOTICE_LOG ,  "bad format %s of nsset handle"  , (const char * ) chck[i] );
                            a[i].avail = ccReg::BadFormat;    // spatny format
                            a[i].reason =  CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_BAD_FORMAT_NSSET_HANDLE ) );
                         }

                        break;
                  case EPP_DomainCheck:
                       if( (  zone =  getFQDN( FQDN , chck[i] )  ) > 0  )
                         {
                            if( DBsql.CheckDomain( FQDN , zone , GetZoneEnum( zone )  ) )
                              {
                                a[i].avail = ccReg::Exist;    // objekt existuje
                                a[i].reason =  CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_FQDN_EXIST ) );
                                LOG( NOTICE_LOG ,  "domain %s exist not Avail" , (const char * ) chck[i] );
                              }
                             else
                              {

                               if( DBsql.TestDomainFQDNHistory( FQDN , DefaultDomainFQDNPeriod() ) )
                                 {
                                     a[i].avail =  ccReg::DelPeriod;    // objekt byl smazan je v historri a ma ochranou lhutu
                                     a[i].reason =  CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_FQDN_HISTORY ) );  // v ochrane lhute
                                     LOG( NOTICE_LOG ,  "domain %s in delete period" ,(const char * ) chck[i] );
                                 }
                                else
                                 {
                                     a[i].avail =  ccReg::NotExist;    // objekt ne existuje
                                     a[i].reason =  CORBA::string_dup( "");  // free
                                     LOG( NOTICE_LOG ,  "domain %s not exist  Avail" ,(const char * ) chck[i] );
                                  }
                              }

                         }
                       else
                         {
                           if( zone < 0 )   
                             {
                               LOG( NOTICE_LOG ,  "bad format %s of fqdn"  , (const char * ) chck[i] );
                               a[i].avail = ccReg::BadFormat;    // spatny format
                               a[i].reason =  CORBA::string_dup( DBsql.GetReasonMessage(REASON_MSG_BAD_FORMAT_FQDN ) );
                             }
                           else
                             {
                               LOG( NOTICE_LOG ,  "not applicable %s"  , (const char * ) chck[i] );
                               a[i].avail = ccReg::NotApplicable;    // nepouzitelna domena neni v zone
                               a[i].reason =  CORBA::string_dup( DBsql.GetReasonMessage(REASON_MSG_NOT_APPLICABLE_FQDN ) );
                             }
                         }
                        break;
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
char dateStr[MAX_DATE];
int  clid , crid , upid , regID;
int len , i  , ssn;

c = new ccReg::Contact;
// inicializace pro pripad neuspesneho hledani
c->DiscloseName = ccReg::DISCL_EMPTY;
c->DiscloseOrganization = ccReg::DISCL_EMPTY;
c->DiscloseAddress = ccReg::DISCL_EMPTY;
c->DiscloseTelephone = ccReg::DISCL_EMPTY;
c->DiscloseFax  = ccReg::DISCL_EMPTY;
c->DiscloseEmail = ccReg::DISCL_EMPTY;
c->SSNtype = ccReg::EMPTY; 

ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ContactInfo: clientID -> %d clTRID [%s] handle [%s] " ,  (int )  clientID , clTRID , handle );
 
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
        convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("CrDate" , 0 ) ); // datum a cas vytvoreni
	c->CrDate= CORBA::string_dup( dateStr );
	convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
        c->UpDate= CORBA::string_dup( dateStr ); 
	convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("TrDate" , 0 ) ); // datum a cas transferu
        c->TrDate= CORBA::string_dup( dateStr );
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





        // DiscloseFlag nastaveni podle defaul policy servru

        if( DefaultPolicy() ) c->DiscloseFlag = ccReg::DISCL_HIDE;
        else c->DiscloseFlag =   ccReg::DISCL_DISPLAY;




        // nastavuje tru jen ty co se lisy od default policy pres get_DISCLOSE
        c->DiscloseName =  get_DISCLOSE(  DBsql.GetFieldBooleanValueName( "DiscloseName" , 0 )   );
        c->DiscloseOrganization =  get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseOrganization" , 0 ) );
        c->DiscloseAddress =get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseAddress" , 0 ) );
        c->DiscloseTelephone = get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseTelephone" , 0 ) );
        c->DiscloseFax  = get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseFax" , 0 ) );
        c->DiscloseEmail = get_DISCLOSE( DBsql.GetFieldBooleanValueName( "DiscloseEmail" , 0  ) );

      // pokud neni nic nastaveno na true vrat flag empty
      if( !c->DiscloseName && !c->DiscloseOrganization && !c->DiscloseAddress && !c->DiscloseTelephone && !c->DiscloseFax && !c->DiscloseEmail )
               c->DiscloseFlag =   ccReg::DISCL_EMPTY;




    
    
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
            ret->errors[0].reason = CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_BAD_FORMAT_CONTACT_HANDLE )  );
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
c->CrDate=CORBA::string_dup(""); // datum a cas vytvoreni
c->UpDate=CORBA::string_dup(""); // datum a cas zmeny
c->TrDate=CORBA::string_dup(""); // dattum a cas transferu
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

LOG( NOTICE_LOG ,  "ContactDelete: clientID -> %d clTRID [%s] handle [%s] " , (int )  clientID , clTRID , handle );



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
int regID ,  clID , id ;
bool remove_update_flag = false ;
int  i , seq;
Status status;

seq=0;
ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "ContactUpdate: clientID -> %d clTRID [%s] handle [%s] ", (int ) clientID, clTRID, handle );
LOG( NOTICE_LOG, "Discloseflag %d: Disclose Name %d Org %d Add %d Tel %d Fax %d Email %d" , c.DiscloseFlag ,
 c.DiscloseName  , c.DiscloseOrganization , c.DiscloseAddress , c.DiscloseTelephone , c.DiscloseFax , c.DiscloseEmail );

// nacti status flagy
  for( i = 0; i < (int ) status_add.length(); i++ )
    {
      LOG( NOTICE_LOG, "status_add [%s] -> %d",  (const char *)  status_add[i]  ,  status.GetStatusNumber( status_add[i] )  ); 
      status.PutAdd( status.GetStatusNumber( status_add[i] ) ); // pridej status flag
    }

  for( i = 0; i <  (int ) status_rem.length(); i++ )
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
                                                      ret->errors[seq].reason =
                                                      CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_CAN_NOT_ADD_STATUS )   );
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
                                                      ret->errors[seq].reason = 
                                                      CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_CAN_NOT_REM_STATUS )   );
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
                                          DBsql.SETBOOL( "DiscloseName", update_DISCLOSE( c.DiscloseName , c.DiscloseFlag ) );
                                          DBsql.SETBOOL( "DiscloseOrganization", update_DISCLOSE( c.DiscloseOrganization , c.DiscloseFlag ) );
                                          DBsql.SETBOOL( "DiscloseAddress", update_DISCLOSE(  c.DiscloseAddress, c.DiscloseFlag  ) );
                                          DBsql.SETBOOL( "DiscloseTelephone",  update_DISCLOSE(  c.DiscloseTelephone , c.DiscloseFlag ) );
                                          DBsql.SETBOOL( "DiscloseFax",  update_DISCLOSE( c.DiscloseFax , c.DiscloseFlag ) );
                                          DBsql.SETBOOL( "DiscloseEmail", update_DISCLOSE( c.DiscloseEmail, c.DiscloseFlag  ) );

                                          }
                                          // datum a cas updatu  plus kdo zmenil zanzma na konec
                                          DBsql.SSET( "UpDate", "now" );
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
                          LOG( WARNING_LOG, "unknown country code" );
                          ret->errors.length( 1 );
                          ret->errors[0].code = ccReg::contactUpdate_cc;        // spatne zadany neznamy country code
                          ret->errors[0].value <<= CORBA::string_dup( c.CC );
                          ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_UNKNOW_COUNTRY ) );

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
                                              ccReg::timestamp_out crDate, CORBA::Long clientID, const char *clTRID , const char* XML )
{
DB DBsql;
char pass[PASS_LEN+1];
char dateStr[MAX_DATE];
char roid[64];
char HANDLE[64]; // handle na velka pismena
ccReg::Response * ret;
int regID, id;
// default
ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

crDate = CORBA::string_dup( "" ); 


LOG( NOTICE_LOG, "ContactCreate: clientID -> %d clTRID [%s] handle [%s]", (int ) clientID, clTRID, handle );
LOG( NOTICE_LOG, "Discloseflag %d: Disclose Name %d Org %d Add %d Tel %d Fax %d Email %d" , c.DiscloseFlag ,
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
            ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_BAD_FORMAT_CONTACT_HANDLE ) );
        }
        else 
        {
          if( DBsql.BeginTransaction() )      // zahajeni transakce
            {
              // test zdali kontakt uz existuje
              if( DBsql.CheckContact( HANDLE  ) )
                {
                  LOG( WARNING_LOG, "object handle [%s] EXIST", handle  );
                  ret->errCode = COMMAND_OBJECT_EXIST;  // je uz zalozena
                }
              else              // pokud kontakt nexistuje
                {
                    // test jestli neni ve smazanych kontaktech
                   if( DBsql.TestContactHandleHistory( HANDLE , DefaultContactHandlePeriod() ) )
                     {

                       ret->errCode = COMMAND_PARAMETR_ERROR;
                       LOG( WARNING_LOG, "handle[%s] was deleted" , handle );
                       ret->errors.length( 1 );
                       ret->errors[0].code = ccReg::contactCreate_handle;
                       ret->errors[0].value <<= CORBA::string_dup( handle );
                       ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_CONTACT_HISTORY  ) );
                     }
                  else


                  // test zdali country code je existujici zeme
                  if( DBsql.TestCountryCode( c.CC ) )
                    {
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
                      DBsql.INTO( "AuthInfoPw" );


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

                      // disclose se vzdy zapisou but t nebo f
                      DBsql.INTO( "DiscloseName" );
                      DBsql.INTO( "DiscloseOrganization" );
                      DBsql.INTO( "DiscloseAddress" );
                      DBsql.INTO( "DiscloseTelephone" );
                      DBsql.INTO( "DiscloseFax" );
                      DBsql.INTO( "DiscloseEmail" );

                      DBsql.VALUE( id );
                      DBsql.VALUE( roid );
                      DBsql.VALUE( HANDLE );
                      DBsql.VALUENOW();
                      DBsql.VALUE( regID );
                      DBsql.VALUE( regID );
                      DBsql.VALUE( "{ 1 }" );   // OK status



                     if( strlen ( c.AuthInfoPw ) == 0 )
                       {
                          random_pass(  pass  ); // autogenerovane heslo pokud se heslo nezada
                          DBsql.VVALUE( pass );
                        }
                      else DBsql.VALUE( c.AuthInfoPw );



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

                      // zapis disclose podle DiscloseFlag a DefaultPolicy servru
                      DBsql.VALUE( setvalue_DISCLOSE( c.DiscloseName , c.DiscloseFlag )  );
                      DBsql.VALUE( setvalue_DISCLOSE( c.DiscloseOrganization , c.DiscloseFlag ) );
                      DBsql.VALUE( setvalue_DISCLOSE( c.DiscloseAddress , c.DiscloseFlag ) );
                      DBsql.VALUE( setvalue_DISCLOSE( c.DiscloseTelephone , c.DiscloseFlag ) );
                      DBsql.VALUE( setvalue_DISCLOSE( c.DiscloseFax , c.DiscloseFlag ) );
                      DBsql.VALUE( setvalue_DISCLOSE( c.DiscloseEmail , c.DiscloseFlag  ) );




                      // pokud se podarilo insertovat
                      if( DBsql.EXEC() )    
                        {     
                          // datum a cas vytvoreni
                         convert_rfc3339_timestamp( dateStr ,   DBsql.GetValueFromTable( "CONTACT", "CrDate" , "id" , id ) ); 
                         crDate= CORBA::string_dup( dateStr );

                          //  uloz do historie
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
                      LOG( WARNING_LOG, "unknown country code" );
                      ret->errors.length( 1 );
                      ret->errors[0].code = ccReg::contactCreate_cc;    // spatne zadany neznamy country code
                      ret->errors[0].value <<= CORBA::string_dup( c.CC );
                      ret->errors[0].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_UNKNOW_COUNTRY )   );
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
char pass[PASS_LEN+1];
Status status;
int regID , clID , id;

ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ContactTransfer: clientID -> %d clTRID [%s] handle [%s] authInfo [%s] " , (int ) clientID , clTRID , handle , authInfo );

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

                // pri prevodu autogeneruj nove heslo
                random_pass(  pass  );

                // zmena registratora
                DBsql.UPDATE( "CONTACT");
                DBsql.SSET( "TrDate" , "now" );
                DBsql.SSET( "AuthInfoPw" , pass );
                DBsql.SET( "ClID" , regID );
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
char dateStr[MAX_DATE];
ccReg::Response *ret;
int clid , crid , upid , nssetid , regID;
int i , j  ,ilen , len ;

ret = new ccReg::Response;
n = new ccReg::NSSet;

// default
ret->errCode = 0;
ret->errors.length(0);
LOG( NOTICE_LOG ,  "NSSetInfo: clientID -> %d clTRID [%s] handle [%s] " , (int ) clientID , clTRID , handle );
 


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
        convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("CrDate" , 0 ) ); // datum a cas vytvoreni
        n->CrDate= CORBA::string_dup( dateStr );
        convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
        n->UpDate= CORBA::string_dup( dateStr );
        convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("TrDate" , 0 ) ); // datum a cas transferu
        n->TrDate= CORBA::string_dup( dateStr );

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
            ret->errors[0].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_BAD_FORMAT_NSSET_HANDLE )  );
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
n->CrDate=CORBA::string_dup( "" ); // datum vytvoreni
n->UpDate=CORBA::string_dup( "" ); // datum zmeny
n->TrDate=CORBA::string_dup( "" ); // datum transferu
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

LOG( NOTICE_LOG ,  "NSSetDelete: clientID -> %d clTRID [%s] handle [%s] " , (int ) clientID , clTRID , handle );


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
                                            ccReg::timestamp_out crDate, CORBA::Long clientID, const char *clTRID , const char* XML ) 
{ 
DB DBsql; 
char Array[512] , NAME[256] ,   HANDLE[64]; // handle na velka pismena 
char pass[PASS_LEN+1];
char dateStr[MAX_DATE];
char roid[64]; ccReg::Response * ret; int regID, id, techid; 
int  i , j ,  seq ;
ret = new ccReg::Response;
// default
ret->errCode = 0;
ret->errors.length( 0 );
crDate = CORBA::string_dup( "" );
seq=0;

LOG( NOTICE_LOG, "NSSetCreate: clientID -> %d clTRID [%s] handle [%s]  authInfoPw [%s]", (int ) clientID, clTRID, handle , authInfoPw  );

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
            ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_BAD_FORMAT_NSSET_HANDLE )  );
        }
        else
     {
      if( DBsql.BeginTransaction() )      // zahaj transakci
        {
 
        // prvni test zdali nsset uz existuje          
        if(  DBsql.CheckNSSet( HANDLE )   )
         {
               LOG( WARNING_LOG, "nsset handle [%s] EXIST", HANDLE );
               ret->errCode = COMMAND_OBJECT_EXIST;  // je uz zalozen
        }
        else                  // pokud nexistuje 
       {

                    // test jestli neni ve smazanych kontaktech
                   if( DBsql.TestNSSetHandleHistory( HANDLE ,  DefaultDomainNSSetPeriod()  ) )
                     {
                       ret->errCode = COMMAND_PARAMETR_ERROR;
                       LOG( WARNING_LOG, "handle[%s] was deleted" , handle );
                       ret->errors.length( 1 );
                       ret->errors[0].code = ccReg::nssetCreate_handle;
                       ret->errors[0].value <<= CORBA::string_dup( handle );
                       ret->errors[0].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_NSSET_HISTORY ) );
                     }
                  else

             // Test tech kontaktu 

                 if(  tech.length() == 0 )
                   {
                      LOG( WARNING_LOG, "NSSetCreate: not any tech Contact "  );
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::nssetCreate_tech;
                      ret->errors[seq].value <<= CORBA::string_dup( "tech contact"  );
                      ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_NOT_ANY_TECH ) );
                      seq++;
                      ret->errCode = COMMAND_PARAMETR_MISSING ; // musi byt alespon jeden nsset;
                   }
                 else
                 {
                 
                  // zapis technicke kontakty 
                  for( i = 0; i < (int)  tech.length() ;  i++ )
                    {

                      // preved handle na velka pismena
                      if( get_HANDLE( HANDLE , tech[i] ) == false )  // spatny format handlu
                        {
                          LOG( WARNING_LOG, "NSSetCreate: unknown tech Contact %s" , (const char *)  tech[i] );
                          ret->errors.length( seq +1 );
                          ret->errors[seq].code = ccReg::nssetCreate_tech;
                          ret->errors[seq].value <<= CORBA::string_dup(  tech[i] );
                          ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_BAD_FORMAT_CONTACT_HANDLE  ) );
                          seq++;
                          ret->errCode = COMMAND_PARAMETR_ERROR;
                        }
                      else
                        { 
                          techid = DBsql.GetNumericFromTable( "Contact", "id", "handle", HANDLE );
                          if( techid == 0 )
                          {
                          LOG( WARNING_LOG, "NSSetCreate: unknown tech Contact %s" , (const char *)  tech[i]  );                          
                          ret->errors.length( seq +1 );
                          ret->errors[seq].code = ccReg::nssetCreate_tech;
                          ret->errors[seq].value <<= CORBA::string_dup(  tech[i] );
                          ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_UNKNOW_TECH ) );
                          seq++;                                 
                          ret->errCode = COMMAND_PARAMETR_ERROR;
                          }
                        }
                    }

                  }
         
           LOG( DEBUG_LOG ,  "NSSetCreate:  dns.length %d" , (int ) dns.length() );
             // test DNS hostu
               if(  dns.length() < 2  ) // musi zadat minimalne dva dns hosty
                 {
                 
                      if( dns.length() == 1 )
                        {
                          LOG( WARNING_LOG, "NSSetCreate: minimal two dns host create one %s"  , (const char *)  dns[0].fqdn   );    
                          ret->errors.length( seq +1 );
                          ret->errors[seq].code = ccReg::nssetCreate_ns_addr;
                          ret->errors[seq].value <<= CORBA::string_dup( dns[0].fqdn   );
                          ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_MIN_DNS )  );
                          seq++;
                          ret->errCode = COMMAND_PARAMETR_VALUE_POLICY_ERROR;
                        }
                      else
                        {
                          LOG( WARNING_LOG, "NSSetCreate: minimal two dns DNS hosts"     );
                          ret->errors.length( seq +1 );
                          ret->errors[seq].code = ccReg::nssetCreate_ns_addr;
                          ret->errors[seq].value <<= CORBA::string_dup( "not any dns host"  ); // TODO VALUE ?? 
                          ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(  REASON_MSG_MIN_DNS ) );
                          seq++;
                          ret->errCode = COMMAND_PARAMETR_MISSING;
                        }
 
                  }
                  else
                  {
                  // test dns hostu
                  for( i = 0; i < (int)  dns.length() ; i++ )
                    {
     
                      LOG( DEBUG_LOG , "NSSetCreate: test host %s" ,  (const char *)  dns[i].fqdn  );
                      // preved sequenci adres
                      for( j = 0; j < (int ) dns[i].inet.length(); j++ )
                        {
                           LOG( DEBUG_LOG , "NSSetCreate: test inet[%d] = %s " , j ,   (const char *) dns[i].inet[j]   );
                          if( TestInetAddress( dns[i].inet[j] )  == false )
                            {
                                  LOG( WARNING_LOG, "NSSetCreate: bad host address %s " , (const char *) dns[i].inet[j]  );
                                  ret->errors.length( seq +1 );
                                  ret->errors[seq].code = ccReg::nssetCreate_ns_addr;
                                  ret->errors[seq].value <<= CORBA::string_dup(   dns[i].inet[j]  );
                                  ret->errors[seq].reason = CORBA::string_dup(   DBsql.GetReasonMessage( REASON_MSG_BAD_IP_ADDRESS )  );
                                  seq++;
                                  ret->errCode = COMMAND_PARAMETR_ERROR;
                            }

                        }



                      // test DNS hostu
                     if( TestDNSHost( dns[i].fqdn ) )
                      {
                       LOG( NOTICE_LOG ,  "NSSetCreate: test DNS Host %s",   (const char *)  dns[i].fqdn       );

                       convert_hostname(  NAME , dns[i].fqdn );
 

 
                        if( getZone( dns[i].fqdn  ) == 0   && dns[i].inet.length() > 0 ) // neni v definovanych zonach a obsahuje zaznam ip adresy
                          {
                            for( j = 0 ; j < (int )  dns[i].inet.length() ; j ++ )
                               {

                                    LOG( WARNING_LOG, "NSSetCreate:  ipaddr  glue not allowed %s " , (const char *) dns[i].inet[j]   );
                                    ret->errors.length( seq +1 );
                                    ret->errors[seq].code = ccReg::nssetCreate_ns_addr;
                                    ret->errors[seq].value <<= CORBA::string_dup(   dns[i].inet[j]  ); // staci vratit prvni zaznam
                                    ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_IP_GLUE_NOT_ALLOWED ) );
                                    seq++;
                                }
                                    ret->errCode = COMMAND_PARAMETR_ERROR;
 
                           }

                                                    
                        }
                       else 
                          {
                                  LOG( WARNING_LOG, "NSSetCreate: bad host name %s " , (const char *)  dns[i].fqdn  );
                                  ret->errors.length( seq +1 );
                                  ret->errors[seq].code = ccReg::nssetCreate_ns_name;
                                  ret->errors[seq].value <<= CORBA::string_dup(  dns[i].fqdn  );
                                  ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_BAD_DNS_NAME ) );
                                  seq++;
                                  ret->errCode = COMMAND_PARAMETR_ERROR;

                          }

                       }
 
                    }
                  }

            if( ret->errCode == 0 )
            {
              // get  registrator ID
              regID = DBsql.GetLoginRegistrarID( clientID );


              // ID je cislo ze sequence
              id = DBsql.GetSequenceID( "nsset" );

              // vytvor roid nssetu
              get_roid( roid, "N", id );

              // preved znova nsset handle
              get_NSSETHANDLE( HANDLE , handle );

              DBsql.INSERT( "NSSET" );
              DBsql.INTO( "id" );
              DBsql.INTO( "roid" );
              DBsql.INTO( "handle" );
              DBsql.INTO( "CrDate" );
              DBsql.INTO( "CrID" );
              DBsql.INTO( "ClID" );
              DBsql.INTO( "status" );
              DBsql.INTO( "authinfopw");

              DBsql.VALUE( id );
              DBsql.VVALUE( roid ); // neni potreba escape
              DBsql.VVALUE( HANDLE  );
              DBsql.VALUENOW(); // aktualni cas current_timestamp
              DBsql.VALUE( regID );
              DBsql.VALUE( regID );
              DBsql.VVALUE( "{ 1 }" );   // status OK


              if( strlen ( authInfoPw ) == 0 )
                {
                  random_pass(  pass  ); // autogenerovane heslo pokud se heslo nezada
                  DBsql.VVALUE( pass );
                }
              else DBsql.VALUE( authInfoPw ); 
 

              // zapis nejdrive nsset 
              if( DBsql.EXEC() )
                {
                  // datum a cas vytvoreni
                  convert_rfc3339_timestamp( dateStr ,   DBsql.GetValueFromTable(  "NSSET", "CrDate" , "id" , id ) );
                  crDate= CORBA::string_dup( dateStr );

                  // zapis technicke kontakty
                  for( i = 0; i <  (int ) tech.length() ;  i++ )
                    {

                      // preved handle na velka pismena
                      if( get_HANDLE( HANDLE , tech[i] )  )  // spatny format handlu
                      {
                      techid = DBsql.GetNumericFromTable( "Contact", "id", "handle", HANDLE );
                      if( techid )
                        {
                          LOG( NOTICE_LOG, "NSSetCreate: create tech Contact %s" , (const char *)  tech[i] );
                          DBsql.INSERT( "nsset_contact_map" );
                          DBsql.VALUE( id );
                          DBsql.VALUE( techid );

                          if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;
                        }
                      }
                    }


                 // zapis DNS hosty


              // zapis do tabulky hostu
                  for( i = 0; i < (int) dns.length() ; i++ )
                    {


                      // preved sequenci adres
                      strcpy( Array, " { " );
                      for( j = 0; j < (int) dns[i].inet.length(); j++ )
                        {
                          if( j > 0 ) strcat( Array, " , " );

                          if( TestInetAddress( dns[i].inet[j] ) )
                            {
                               strcat( Array,  dns[i].inet[j]  );
                            }

                        }
                      strcat( Array, " } " );



                      // preved nazev domeny 
                       LOG( NOTICE_LOG ,  "NSSetCreate: DNS Host %s [%s] ",   (const char *)  dns[i].fqdn   , Array    );
                       convert_hostname(  NAME , dns[i].fqdn );



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
int regID , clID , id ,  techid  , hostID;
int  j , l  , seq ;
unsigned int i;
int hostNum , techNum;
bool remove_update_flag=false;

ret = new ccReg::Response;
seq=0;
ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetUpdate: clientID -> %d clTRID [%s] handle [%s] authInfo_chg  [%s] " , (int ) clientID , clTRID , handle  , authInfo_chg);

// nacti status flagy
  for( i = 0; i <  status_add.length(); i++ )
    {
      LOG( NOTICE_LOG, "status_add [%s] -> %d",  (const char *)  status_add[i]  ,  status.GetStatusNumber( status_add[i] )  );
      status.PutAdd( status.GetStatusNumber( status_add[i] ) ); // pridej status flag
    }

  for( i = 0; i <  status_rem.length(); i++ )
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
                                          for( i = 0; i < (unsigned int ) status.AddLength(); i++ )
                                            {
                                                  if( status.Add(i) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::nssetUpdate_status_add;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_add[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_CAN_NOT_ADD_STATUS ) );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                    }
                                            }

                                          // zruseny status flagy
                                         for( i = 0; i < (unsigned int )   status.RemLength(); i++ )
                                             {
                                              
                                                  if( status.Rem(i) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::nssetUpdate_status_rem;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_rem[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_CAN_NOT_REM_STATUS ) );
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
                                    DBsql.SSET( "UpDate", "now" );
                                    DBsql.SET( "UpID", regID );
                                    DBsql.SSET( "status", statusString );
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
                                                        ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_UNKNOW_TECH )  );
                                                        seq++;
                                                      }
                                                    if( check )  
                                                      {
                                                        LOG( WARNING_LOG, "add tech Contact [%s] exist in contact map table"  , (const char *) tech_add[i] );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_tech_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  tech_add[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_TECH_EXIST )  );
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
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_UNKNOW_TECH )  );
                                                        seq++;
                                                      }
                                                    if( !check )  
                                                      {
                                                       LOG( WARNING_LOG, "rem tech Contact [%s] not in contact map table" , (const char *) tech_rem[i] );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::nssetUpdate_tech_rem;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  tech_rem[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_TECH_NOTEXIST ) );
                                                        seq++;
                                                      }
                                                   ret->errCode = COMMAND_PARAMETR_ERROR;
                                                  }
                                              }

                                               // TEST pocet tech kontaktu
                                                techNum = DBsql.GetNSSetContacts( id );
                                                LOG(NOTICE_LOG, "NSSetUpdate: tech Contact  %d" , techNum );

                                               if( techNum == 0  ) // musi zustat alespon jeden tech kontact po update
                                                 {

                                                    for( i = 0; i < tech_rem.length(); i++ )
                                                      {
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code =  ccReg::nssetUpdate_tech_rem;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  tech_rem[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_CAN_NOT_REMOVE_TECH ));
                                                        seq++;
                                                      }
                                                    ret->errCode = COMMAND_PARAMETR_VALUE_POLICY_ERROR;

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
                                              for( j = 0; j < (int ) dns_add[i].inet.length(); j++ )
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
                                                                ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_REASON_MSG_DUPLICITY_IP_ADDRESS ) );
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
                                                        ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_BAD_IP_ADDRESS ) );
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


                                               if( getZone( dns_add[i].fqdn ) == 0     && (int )  dns_add[i].inet.length() > 0 ) // neni v definovanych zonach a obsahuje zaznam ip adresy
                                                 {
                                                   for( j = 0 ; j < (int )  dns_add[i].inet.length() ; j ++ )
                                                   {
                                                    LOG( WARNING_LOG, "NSSetUpdate:  ipaddr  glue not allowed %s " , (const char *) dns_add[i].inet[j]   );
                                                    ret->errors.length( seq +1 );
                                                    ret->errors[seq].code = ccReg::nssetUpdate_ns_addr_add;
                                                    ret->errors[seq].value <<= CORBA::string_dup(   dns_add[i].inet[j]  ); 
                                                    ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_IP_GLUE_NOT_ALLOWED )  );
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
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_DNS_NAME_EXIST )  );
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
                                                        ret->errors[seq].reason = CORBA::string_dup(   DBsql.GetReasonMessage( REASON_MSG_BAD_DNS_NAME  ) );
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
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(  REASON_MSG_DNS_NAME_NOTEXIST ) );
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
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_BAD_DNS_NAME  )  );
                                                        seq++;
                                                        ret->errCode = COMMAND_PARAMETR_ERROR;
                                                 }
  
                                               // TEST pocet dns hostu
                                                hostNum = DBsql.GetNSSetHosts( id );
                                                LOG(NOTICE_LOG, "NSSetUpdate:  hostNum %d" , hostNum );

                                                if( hostNum <  2 ) // musi minimalne dva DNS hosty zustat
                                                  {
                                                    for( i = 0; i < dns_rem.length(); i++ )
                                                      {
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code =  ccReg::nssetUpdate_ns_name_rem; 
                                                        ret->errors[seq].value <<= CORBA::string_dup(  dns_rem[i].fqdn );
                                                        ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_CAN_NOT_REM_DNS ) );
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
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(  REASON_MSG_CAN_NOT_ADD_DNS )  );
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
char pass[PASS_LEN+1];
char HANDLE[64];
Status status;
int regID , clID , id ;

ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetTransfer: clientID -> %d clTRID [%s] handle [%s] authInfo [%s] " , (int ) clientID , clTRID , handle , authInfo );

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


                // pri prevodu autogeneruj nove heslo
                random_pass(  pass  );

                // zmena registratora
                DBsql.UPDATE( "NSSET");
                DBsql.SSET( "TrDate" , "now" );
                DBsql.SSET( "AuthInfoPw" , pass );
                DBsql.SET( "ClID" , regID );
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
ccReg::ENUMValidationExtension *enumVal;
ccReg::Response *ret;
char FQDN[64];
char dateStr[64];
int id , clid , crid ,  upid , regid ,nssetid , regID , zone ;
int i , len ;

d = new ccReg::Domain;
ret = new ccReg::Response;




// default
ret->errCode=COMMAND_FAILED;
ret->errors.length(0);


LOG( NOTICE_LOG ,  "DomainInfo: clientID -> %d clTRID [%s] fqdn  [%s] " , (int ) clientID , clTRID  , fqdn );


d->ext.length(0); // extension

if( DBsql.OpenDatabase( database ) )
{

if( DBsql.BeginAction( clientID , EPP_DomainInfo , clTRID , XML  ) )
 {



      // preved fqd na  mala pismena a otestuj to
       // spatny format navu domeny
    if( ( zone = getFQDN( FQDN , fqdn ) ) <= 0 )
      {
            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "not in zone [%d]" , zone );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::domainInfo_fqdn;
            ret->errors[0].value <<= CORBA::string_dup( fqdn );

            if( zone == 0 ) ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_NOT_APPLICABLE_FQDN ) );
            else  ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_BAD_FORMAT_FQDN )   );
      }
     else
      {

             // get  registrator ID
            regID = DBsql.GetLoginRegistrarID( clientID );
/*
           if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
             }
          else
         ZRUSENO 
*/

   

   
    if(  DBsql.SELECTDOMAIN(  FQDN  , zone , GetZoneEnum( zone ) )  )
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

        convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("CrDate" , 0 ) ); // datum a cas vytvoreni
        d->CrDate= CORBA::string_dup( dateStr );
        convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("UpDate" , 0 ) ); // datum a cas zmeny
        d->UpDate= CORBA::string_dup( dateStr );
        convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("TrDate" , 0 ) ); // datum a cas transferu
        d->TrDate= CORBA::string_dup( dateStr );
        convert_rfc3339_timestamp( dateStr ,  DBsql.GetFieldValueName("ExDate" , 0 ) ); // datum a cas expirace
        d->ExDate= CORBA::string_dup( dateStr );


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
        if(  GetZoneEnum( zone )  )
          {
              if( regID == clid )  d->Registrant=CORBA::string_dup( DBsql.GetValueFromTable( "CONTACT" , "handle" , "id" , regid ) );
              else  d->Registrant=CORBA::string_dup( "" ); // skryj vlastnika enum domenu 
          }
         else  d->Registrant=CORBA::string_dup( DBsql.GetValueFromTable( "CONTACT" , "handle" , "id" , regid ) );

        //  handle na nsset
        d->nsset=CORBA::string_dup( DBsql.GetValueFromTable( "NSSET" , "handle", "id" , nssetid ) );
    

        // dotaz na admin kontakty

        
        if(  DBsql.SELECTCONTACTMAP( "domain"  , id ) )
          {
               len =  DBsql.GetSelectRows(); // pocet adnim kontaktu     
               if(  GetZoneEnum( zone )  &&  regID  != clid  ) len = 0 ; // zadne admin kontakty skryt 
               d->admin.length(len); // pocet admin kontaktu
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

                enumVal = new ccReg::ENUMValidationExtension;
                convert_rfc3339_date( dateStr ,  DBsql.GetFieldValueName("ExDate" , 0 ) ); // datum a cas expirace validace

                enumVal->valExDate = CORBA::string_dup( dateStr );
                d->ext.length(1); // preved na  extension
                d->ext[0] <<= enumVal;
                LOG( NOTICE_LOG , "enumValExtension ExDate %s" ,  dateStr );
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
d->UpDate= CORBA::string_dup( "" ); // datuum zmeny
d->CrDate= CORBA::string_dup( "" ); // datum vytvoreni
d->TrDate= CORBA::string_dup( "" ); // datum transferu
d->ExDate= CORBA::string_dup( "" ); // datum vyprseni
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
int regID , clID , id , zone;
bool stat;
ret = new ccReg::Response;


// default
ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "DomainDelete: clientID -> %d clTRID [%s] fqdn  [%s] " , (int ) clientID , clTRID  , fqdn );


  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_DomainDelete,  clTRID  , XML) )
        {


      // preved fqd na  mala pismena a otestuj to
       // spatny format navu domeny
       if( ( zone = getFQDN( FQDN , fqdn ) ) <= 0 )  
         {          
            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "not in zone [%d]" , zone );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::domainInfo_fqdn;
            ret->errors[0].value <<= CORBA::string_dup( fqdn );
            if( zone == 0 ) ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_NOT_APPLICABLE_FQDN ) );
            else  ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_BAD_FORMAT_FQDN )   );

         }
        else
         {

             // get  registrator ID
            regID = DBsql.GetLoginRegistrarID( clientID );

           if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
             }
          else  
          if( DBsql.BeginTransaction() )
            {
              if( ( id = DBsql.GetNumericFromTable( "DOMAIN", "id", "fqdn", ( char * ) FQDN ) ) == 0 )
                {
                  LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;
                }
              else
                {

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
bool check;
char FQDN[64] , HANDLE[64];
char valexpiryDate[MAX_DATE];
char statusString[128];
int regID = 0, clID = 0, id, nssetid, contactid, adminid;
int   seq , zone;
unsigned int i;
bool remove_update_flag=false;

ret = new ccReg::Response;
seq=0;
ret->errCode = 0;
ret->errors.length( 0 );

strcpy( valexpiryDate , "" ); // default

LOG( NOTICE_LOG, "DomainUpdate: clientID -> %d clTRID [%s] fqdn  [%s] , registrant_chg  [%s] authInfo_chg [%s]  nsset_chg [%s] ext.length %d",
      (int )  clientID, clTRID, fqdn, registrant_chg, authInfo_chg, nsset_chg , ext.length() );


// parse extension
GetValExpDateFromExtension( valexpiryDate , ext );



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
       if( ( zone = getFQDN( FQDN , fqdn ) ) <= 0 )  // spatny format navu domeny
         {
            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "not in zone %d" , zone );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::domainUpdate_fqdn;
            ret->errors[0].value <<= CORBA::string_dup( fqdn );
            if( zone == 0 ) ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_NOT_APPLICABLE_FQDN ) );
            else  ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_BAD_FORMAT_FQDN )   );

        }

             // get  registrator ID
            regID = DBsql.GetLoginRegistrarID( clientID );

           if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
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
                                                      ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_NSSET_NOTEXIST ) );
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
                                                      ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_BAD_FORMAT_NSSET_HANDLE ) );
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
                                                      ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_REGISTRANT_NOTEXIST ));
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
                                                      ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(  REASON_MSG_BAD_FORMAT_CONTACT_HANDLE )  );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;

                                            }

                                        }
                                      else contactid = 0;  // nemenim vlastnika


                                         // pridany status
                                          for( i = 0; i < (unsigned int )  status.AddLength(); i++ )
                                            {
                                                  if( status.Add( i ) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::domainUpdate_status_add;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_add[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_CAN_NOT_ADD_STATUS ) );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                    }
                                            }

                                          // zruseny status flagy
                                         for( i = 0; i <  (unsigned int ) status.RemLength(); i++ )
                                             {
                                                  if( status.Rem( i ) == false )
                                                    {
                                                      ret->errors.length( seq +1 );
                                                      ret->errors[seq].code = ccReg::domainUpdate_status_rem;
                                                      ret->errors[seq].value <<= CORBA::string_dup(  status_rem[i] );
                                                      ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_CAN_NOT_REM_STATUS )  );
                                                      seq++;
                                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                                    }
                                            }


                           // Test jestli za danono u enum domen
                             if( GetZoneEnum( zone ) )
                               {
                                 if( strlen( valexpiryDate ) > 0  )
                                 {
                                 if(  DBsql.TestValExDate( valexpiryDate , GetZoneValPeriod( zone ) , DefaultValExpInterval() , id  ) ==  false ) // test validace expirace
                                   {
                                      LOG( WARNING_LOG, "DomainUpdate: bad validity exp date" );
                                      ret->errors.length( seq +1);
                                      ret->errors[seq].code = ccReg::domainUpdate_ext_valDate;
                                      ret->errors[seq].value <<=   valexpiryDate;
                                      ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(REASON_MSG_VALEXPDATE_NOT_VALID ) );
                                      seq++;
                                      ret->errCode = COMMAND_PARAMETR_ERROR;
                                    }
                                 }
                               }

             else
             {
               if( strlen( valexpiryDate )  )  // omlyemen zadan datum validace
                {
                  LOG( WARNING_LOG, "DomainUpdate: can not  validity exp date" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainUpdate_ext_valDate;
                  ret->errors[seq].value <<=   valexpiryDate;
                  ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(REASON_MSG_VALEXPDATE_NOT_USED ) );
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
                                      DBsql.SSET( "status", statusString );                                                      
                                      DBsql.SSET( "UpDate", "now" );
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
                                             if( GetZoneEnum( zone ) )
                                               {
                                                  // zmena extension
                                                   if( strlen( valexpiryDate )  > 0 )
                                                    {
                                                     LOG( NOTICE_LOG, "change valExpDate %s ", valexpiryDate );
                                                     DBsql.UPDATE( "enumval" );
                                                     DBsql.SET( "ExDate", valexpiryDate );
                                                     DBsql.WHERE( "domainID", id );
 
                                                     if( !DBsql.EXEC() )  ret->errCode = COMMAND_FAILED; 
                                                    }
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
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_UNKNOW_ADMIN ) );
                                                        seq++;
                                                      }
                                                    if( check )
                                                      {
                                                        LOG( WARNING_LOG, "add tech Contact exist in contact map table" );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::domainUpdate_admin_add;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  admin_add[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_ADMIN_EXIST )  );
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
                                                        ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(REASON_MSG_UNKNOW_ADMIN ) );
                                                        seq++;
                                                      }
                                                    if( !check )
                                                      {
                                                        LOG( WARNING_LOG, "rem admin Contac not exist in contact map table" );
                                                        ret->errors.length( seq +1 );
                                                        ret->errors[seq].code = ccReg::domainUpdate_admin_rem;
                                                        ret->errors[seq].value <<= CORBA::string_dup(  admin_rem[i] );
                                                        ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(REASON_MSG_ADMIN_NOTEXIST )  );
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
                                             const ccReg::AdminContact & admin, ccReg::timestamp_out crDate, ccReg::timestamp_out  exDate, 
                                             CORBA::Long clientID, const char *clTRID,  const  char* XML , const ccReg::ExtensionList & ext )
{
DB DBsql;
char valexpiryDate[MAX_DATE] , dateStr[MAX_DATE];
char roid[64] , FQDN[64] , HANDLE[64];
char pass[PASS_LEN+1];
ccReg::Response * ret;
int contactid, regID, nssetid, adminid, id;
int i, len,  zone , seq;

ret = new ccReg::Response;

// default
strcpy( valexpiryDate , "" );

seq=0;
// default
ret->errCode = 0;
ret->errors.length( 0 );
crDate =  CORBA::string_dup( "" );
exDate =  CORBA::string_dup( "" );



LOG( NOTICE_LOG, "DomainCreate: clientID -> %d clTRID [%s] fqdn  [%s] ", (int )  clientID, clTRID, fqdn );
LOG( NOTICE_LOG, "DomainCreate:  Registrant  [%s]  nsset [%s]  AuthInfoPw [%s] period %d", Registrant, nsset, AuthInfoPw, period );

// parse extension
GetValExpDateFromExtension( valexpiryDate , ext );

  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_DomainCreate, clTRID , XML ) )
        {

      // preved fqd na  mala pismena a otestuj to
       if(  ( zone = getFQDN( FQDN , fqdn ) ) <= 0  )  // spatny format navu domeny
         {
            ret->errCode = COMMAND_PARAMETR_ERROR;
            LOG( WARNING_LOG, "not in zone %d" , zone );
            ret->errors.length( 1 );
            ret->errors[0].code = ccReg::domainCreate_fqdn;
            ret->errors[0].value <<= CORBA::string_dup( fqdn );
            if( zone == 0 ) ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_NOT_APPLICABLE_FQDN ) );
            else  ret->errors[0].reason = CORBA::string_dup( DBsql.GetReasonMessage(  REASON_MSG_BAD_FORMAT_FQDN )   );

        }
      else
       {
             // get  registrator ID
            regID = DBsql.GetLoginRegistrarID( clientID );

           if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
             }
          else

          if( DBsql.BeginTransaction() )
            {


          //  test zdali domena uz existuje                   
          if( DBsql.CheckDomain( FQDN , zone , GetZoneEnum( zone )  )  )
            {
              ret->errCode = COMMAND_OBJECT_EXIST;      // je uz zalozena
              LOG( WARNING_LOG, "domain  [%s] EXIST", fqdn );
            }
          else // pokud domena nexistuje             
          {  
                    // test jestli neni ve smazanych kontaktech
            if( DBsql.TestDomainFQDNHistory( FQDN , DefaultDomainFQDNPeriod() ) )
             {

                       ret->errCode = COMMAND_PARAMETR_ERROR;
                       LOG( WARNING_LOG, "handle[%s] was deleted" , fqdn );
                       ret->errors.length( 1 );
                       ret->errors[0].code = ccReg::domainCreate_fqdn;
                       ret->errors[0].value <<= CORBA::string_dup( fqdn );
                       ret->errors[0].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_FQDN_HISTORY )  );
                 }
                else
        {
              id = DBsql.GetSequenceID( "domain" );     // id domeny

              // vytvor roid domeny
              get_roid( roid, "D", id );



            if( strlen( nsset) == 0 ) nssetid = 0; // lze vytvorit domenu bez nssetu
            else
             // nsset
            if( get_HANDLE( HANDLE , nsset ) == false )
              {
                      LOG( WARNING_LOG, "bad nsset handle %s", nsset );
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::domainCreate_nsset;
                      ret->errors[seq].value <<= CORBA::string_dup( nsset );
                      ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_BAD_FORMAT_NSSET_HANDLE )  );
                      seq++;
                      ret->errCode = COMMAND_PARAMETR_ERROR;

              }
            else 
            if( (nssetid = DBsql.GetNumericFromTable( "NSSET", "id", "handle", HANDLE  ) ) == 0 )
              {
                      LOG( WARNING_LOG, "unknown nsset handle %s", nsset );
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::domainCreate_nsset;
                      ret->errors[seq].value <<= CORBA::string_dup( nsset );
                      ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_UNKNOW_NSSET  )  );
                      seq++;
                      ret->errCode = COMMAND_PARAMETR_ERROR;
               }
              
             //  registrant
            if( get_HANDLE( HANDLE , Registrant ) == false )
              {
                      LOG( WARNING_LOG, "bad registrant handle %s", Registrant );
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::domainCreate_registrant;
                      ret->errors[seq].value <<= CORBA::string_dup( Registrant );
                      ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_BAD_FORMAT_CONTACT_HANDLE )  );
                      seq++;
                      ret->errCode = COMMAND_PARAMETR_ERROR;

              }
           else                    
           if( ( contactid = DBsql.GetNumericFromTable( "CONTACT", "id", "handle", HANDLE ) ) == 0 )
              {
                      LOG( WARNING_LOG, "unknown Registrant handle %s", Registrant );
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::domainCreate_registrant;
                      ret->errors[seq].value <<= CORBA::string_dup( Registrant );
                      ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_UNKNOW_REGISTRANT )  );
                      seq++;
                      ret->errCode = COMMAND_PARAMETR_ERROR;
               }



             // nastaveni defaultni periody
             if( period == 0 )
               {
                 period = GetZoneExPeriodMin( zone );
                 LOG( NOTICE_LOG, "get defualt peridod %d month  for zone   %d ", period , zone  );

               }

             if(  TestPeriodyInterval( period  ,  GetZoneExPeriodMin( zone )  ,  GetZoneExPeriodMax( zone )  )  == false )
              {
                  LOG( WARNING_LOG, "bad period interval" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainCreate_period;
                  ret->errors[seq].value <<=  period;
                  ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_BAD_PERIOD ) );
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;
               }

            // Test jestli za danono u enum domen
            if( GetZoneEnum( zone ) )
            {
             if( strlen( valexpiryDate ) == 0 )
               {

                  LOG( WARNING_LOG, "DomainCreate: not validity exp date "  );
                  ret->errors.length( seq +1 );
                  ret->errors[seq].code = ccReg::domainCreate_ext_valDate;

                  ret->errors[seq].value <<= CORBA::string_dup( "not valExpDate"  );
                  ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage(REASON_MSG_VALEXPDATE_REQUIRED ) ); // TODO
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_MISSING ;                
               }
            else
            if(  DBsql.TestValExDate( valexpiryDate , GetZoneValPeriod( zone ) ,  DefaultValExpInterval() , 0   ) ==  false ) // test validace expirace
              {
                  LOG( WARNING_LOG, "DomainCreate: bad validity exp date" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainCreate_ext_valDate;
                  ret->errors[seq].value <<=   valexpiryDate;
                  ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_VALEXPDATE_NOT_VALID ) );
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;

              }

             }
            else 
             {
               if( strlen( valexpiryDate )  )  // omlyemen zadan datum validace
                {
                  LOG( WARNING_LOG, "DomainCreate: can not  validity exp date" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainCreate_ext_valDate;
                  ret->errors[seq].value <<=   valexpiryDate;
                  ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_VALEXPDATE_NOT_USED ));
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;
                }
             } 


                              // otestuj admin kontakty na exsitenci a spravny tvar handlu
                              len = admin.length();
                              if( len > 0  )
                                {
                                  for( i = 0; i < len; i++ )
                                    {
                                     // nsset
                                      if( get_HANDLE( HANDLE , admin[i] ) == false )
                                        {
                                          LOG( WARNING_LOG, "DomainCreate: bad admin Contact " );
                                          ret->errors.length( seq +1 );
                                          ret->errors[seq].code = ccReg::domainCreate_admin;
                                          ret->errors[seq].value <<= CORBA::string_dup(  admin[i] );
                                          ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_BAD_FORMAT_CONTACT_HANDLE ) );
                                          seq++;
                                          ret->errCode = COMMAND_PARAMETR_ERROR;
 
                                       }
                                      else 
                                      {
                                      adminid = DBsql.GetNumericFromTable( "Contact", "id", "handle", HANDLE  );

                                      if( adminid == 0 ) 
                                        {
                                          LOG( WARNING_LOG, "DomainCreate: unknown admin Contact " );
                                          ret->errors.length( seq +1 );
                                          ret->errors[seq].code = ccReg::domainCreate_admin;
                                          ret->errors[seq].value <<= CORBA::string_dup(  admin[i] );
                                          ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_UNKNOW_ADMIN ) );
                                          seq++;
                                         ret->errCode = COMMAND_PARAMETR_ERROR;
                                        }
                                      } 
                                    }
                                }

                // zpracovani creditu
               if( DBsql.UpdateCredit(  regID ,   EPP_DomainCreate  ,    zone ,  period  )  == false )  ret->errCode =  COMMAND_BILLING_FAILURE;

                        if(  ret->errCode == 0  ) // pokud nedoslo k chybe
                        {

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
                          DBsql.INTO( "nsset" );
                          DBsql.INTO( "authinfopw");
                                                                  
                                                                  
                          DBsql.VALUE( zone );
                          DBsql.VALUE( id );
                          DBsql.VVALUE( roid );
                          DBsql.VVALUE( FQDN );
                          DBsql.VALUENOW(); // aktualni cas current_timestamp
                          DBsql.VALUEPERIOD(  period  ); // aktualni cas  plus interval period v mesicich
                          DBsql.VALUE( regID );
                          DBsql.VALUE( regID );
                          DBsql.VALUE( "{ 1 }" ); // status OK
                          DBsql.VALUE( contactid );
                          if( nssetid == 0 )   DBsql.VALUENULL(); // domena bez nssetu zapsano NULL
                          else DBsql.VALUE( nssetid );

                          if( strlen ( AuthInfoPw ) == 0 )
                            {
                               random_pass(  pass  ); // autogenerovane heslo pokud se heslo nezada
                               DBsql.VVALUE( pass );
                            }
                          else DBsql.VALUE(  AuthInfoPw);   

                          // pokud se insertovalo do tabulky
                          if( DBsql.EXEC() )
                            {

                                // zjisti datum a cas vytvoreni domeny
                                convert_rfc3339_timestamp( dateStr ,   DBsql.GetValueFromTable(  "DOMAIN", "CrDate" , "id" , id ) );
                                crDate= CORBA::string_dup( dateStr );

                                //  vrat datum expirace
                                convert_rfc3339_date( dateStr ,   DBsql.GetValueFromTable( "DOMAIN", "ExDate" , "id" , id ) ); 
                                exDate =  CORBA::string_dup( dateStr );


                              // pridej enum  extension
                      if( GetZoneEnum( zone ) )
                        {
                              if( strlen( valexpiryDate) > 0  )
                                {
                                  DBsql.INSERT( "enumval" );
                                  DBsql.VALUE( id );
                                  DBsql.VALUE( valexpiryDate ); 
                                  if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;;
                                }

                         }

                                   // pridej admin kontakty
                                  for( i = 0; i < (int )   admin.length(); i++ )
                                    {
                                     // nsset
                                      if( get_HANDLE( HANDLE , admin[i] )  )
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
 *              curExpDate - datum vyprseni domeny !!! cas v GMT 00:00:00
 *              period - doba prodlouzeni v mesicich
 *        OUT:  exDate - datum a cas nove expirace domeny   
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 *              ext - ExtensionList 
 *
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response * ccReg_EPP_i::DomainRenew( const char *fqdn, const char* curExpDate, CORBA::Short period, 
                                            ccReg::timestamp_out exDate, CORBA::Long clientID,
                                            const char *clTRID, const  char* XML , const ccReg::ExtensionList & ext )
{
  DB DBsql;
  Status status;
  char expDateStr[MAX_DATE],  ExDateStr[MAX_DATE] , valexpiryDate[MAX_DATE] ;
  char FQDN[64]; 
  ccReg::Response * ret;
  int clid, regID, id,  zone , seq;
  bool stat;

  ret = new ccReg::Response;

 
// aktualni cas

// default
  exDate =  CORBA::string_dup( "" );
  seq = 0;

   strcpy( expDateStr , "" );
   strcpy( valexpiryDate , "" );
// default
  ret->errCode = 0;
  ret->errors.length( 0 );


  LOG( NOTICE_LOG, "DomainRenew: clientID -> %d clTRID [%s] fqdn  [%s] period %d month", (int ) clientID, clTRID, fqdn, period );




// parse extension
GetValExpDateFromExtension( valexpiryDate , ext );
 


  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_DomainRenew,  clTRID , XML ) )
        {

       if(  ( zone = getFQDN( FQDN , fqdn ) ) > 0 ) 
         {


             // get  registrator ID
            regID = DBsql.GetLoginRegistrarID( clientID );

           if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
             }
          else


            // zahaj transakci
          if( DBsql.BeginTransaction() )
            {

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
                  strcpy( expDateStr ,  DBsql.GetFieldValueName( "ExDate", 0 ) );    // datum a cas  expirace domeny
                  DBsql.FreeSelect();
                }

              // porovnani datumu co je uvedene v databazi se zadanym datumem
              // ex timestamp se prevadi na localtime a z toho se bere datum a porovnava se rok mesic a den
             if(  test_expiry_date( expDateStr , curExpDate ) == false )
                {
                  LOG( WARNING_LOG, "curExpDate is not same as ExDate" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainRenew_curExpDate;
                  ret->errors[seq].value <<=  curExpDate;
                  ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage( REASON_MSG_CUREXPDATE_NOT_EXPDATE ) );
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;
                }
  
             // nastaveni defaultni periody           
             if( period == 0 ) 
               {
                 period = GetZoneExPeriodMin( zone );
                 LOG( NOTICE_LOG, "get defualt peridod %d month  for zone   %d ", period , zone  );

               }

  
             if(  TestPeriodyInterval(   period  ,   GetZoneExPeriodMin( zone )  ,  GetZoneExPeriodMax( zone )  )  == false ) 
              {
                  LOG( WARNING_LOG, "bad period interval" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainRenew_period;
                  ret->errors[seq].value <<=  period;
                  ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_BAD_PERIOD ) );
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;                 
               }

              if( DBsql.GetExpDate( ExDateStr , id ,  period  ,  GetZoneExPeriodMax( zone ) )  )
                {
                    // vypocet ExDate datum expirace
                    exDate =  CORBA::string_dup( ExDateStr );                                     
                    // TODO convert to rfc3339
                }
              else
                {
                  LOG( WARNING_LOG, "bad max period interval" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainRenew_period;
                  ret->errors[seq].value <<=  period;
                  ret->errors[seq].reason = CORBA::string_dup( DBsql.GetReasonMessage( REASON_MSG_BAD_PERIOD ) );
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;
               }


             // test validate


            // Test jestli za danono u enum domen
            if( GetZoneEnum( zone ) )
            {
             if( strlen( valexpiryDate ) > 0 )
             {
              if(  DBsql.TestValExDate( valexpiryDate , GetZoneValPeriod( zone ) ,  DefaultValExpInterval() , id  ) ==  false ) // test validace expirace
                {
                  LOG( WARNING_LOG, "DomainRenew: bad validity exp date" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainRenew_ext_valDate;
                  ret->errors[seq].value <<=   valexpiryDate;
                  ret->errors[seq].reason = CORBA::string_dup(   DBsql.GetReasonMessage(REASON_MSG_VALEXPDATE_NOT_VALID ) ); // TODO
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;
                }
              }

             }
            else
             {
               if( strlen( valexpiryDate )  )  // omlyemen zadan datum validace
                {
                  LOG( WARNING_LOG, "DomainRenew: can not  validity exp date" );
                  ret->errors.length( seq +1);
                  ret->errors[seq].code = ccReg::domainRenew_ext_valDate;
                  ret->errors[seq].value <<=   valexpiryDate;
                  ret->errors[seq].reason = CORBA::string_dup(  DBsql.GetReasonMessage(REASON_MSG_VALEXPDATE_NOT_USED) ); // TODO
                  seq++;
                  ret->errCode = COMMAND_PARAMETR_ERROR;
                }
             }
 
                               // zpracovani creditu
               if( DBsql.UpdateCredit(  regID ,   EPP_DomainRenew   ,    zone ,  period  )  == false )  ret->errCode =  COMMAND_BILLING_FAILURE;

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

                      //  uloz do historie
                      if( DBsql.MakeHistory() )
                        {

                        if( DBsql.SaveHistory( "enumval",  "domainID", id ) ) // uloz extension 
                          {
                          if( DBsql.SaveHistory( "domain_contact_map", "domainID", id ) )       // uloz kontakty
                            {
                              if( DBsql.SaveHistory( "Domain", "id", id ) )     // uloz zaznam
                                {
                                if( GetZoneEnum( zone ) )
                                 {
                                  if( strlen( valexpiryDate ) > 0  )      // zmena extension
                                    {
                                      LOG( NOTICE_LOG, "change valExpDate %s ", valexpiryDate );

                                      DBsql.UPDATE( "enumval" );
                                      DBsql.SET( "ExDate", valexpiryDate );
                                      DBsql.WHERE( "domainID", id );

                                      if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;                                     
                                    }
                                   }
                                   if( ret->errCode == 0 ) // pokud je OK
                                   {
                                     // zmena platnosti domeny
                                     DBsql.UPDATE( "DOMAIN" );                                     
                                     DBsql.SET(  "ExDate", ExDateStr );
                                     DBsql.WHEREID( id );
                                     if( DBsql.EXEC() )   ret->errCode = COMMAND_OK;
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
char pass[PASS_LEN+1];
char FQDN[64];
Status status;
int regID = 0, clID = 0, id , zone;  //   registrantid , contactid;

ret = new ccReg::Response;

// default
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "DomainTransfer: clientID -> %d clTRID [%s] fqdn  [%s]  ", (int ) clientID, clTRID, fqdn );


  if( DBsql.OpenDatabase( database ) )
    {

      if( DBsql.BeginAction( clientID, EPP_DomainTransfer, clTRID , XML  ) )
        {

       if(  ( zone = getFQDN( FQDN , fqdn ) ) > 0 ) // spatny format navu domeny
         {


             // get  registrator ID
            regID = DBsql.GetLoginRegistrarID( clientID );

           if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
             }
          else


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
                                  // pri prevodu autogeneruj nove heslo
                                  random_pass(  pass  );

                                  // zmena registratora
                                  DBsql.UPDATE( "DOMAIN" );
                                  DBsql.SSET( "TrDate", "now" );
                                  DBsql.SSET( "AuthInfoPw" , pass );
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

LOG( NOTICE_LOG ,  "LIST %d  clientID -> %d clTRID [%s] " , act  , (int )  clientID , clTRID );

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



