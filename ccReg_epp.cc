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
		
#include "log.h"

// MailerManager is connected in constructor
#include "register/auth_info.h"
#include "register/domain.h"
#include "register/contact.h"
#include "register/nsset.h"
#include <memory>
#include "tech_check.h"

//
// Example implementational code for IDL interface ccReg::EPP
//
ccReg_EPP_i::ccReg_EPP_i(MailerManager *_mm, NameService *_ns ) :
  mm(_mm), ns(_ns) {

}
ccReg_EPP_i::~ccReg_EPP_i(){

}

void ccReg_EPP_i::ServerInternalError()
{
  LOG( ALERT_LOG, "ServerInternalError");

 throw ccReg::EPP::ServerIntError( "server internal error" );
} 


void ccReg_EPP_i::CreateSession(int max , long wait)
{
int i;


LOG( DEBUG_LOG , "SESSION CREATE max %d wait %ld" , max , wait );

session = new Session[max];
numSession=0; // pocet aktivnich session
maxSession=max; // maximalni pocet pripustnych session
maxWaitClient=wait; // prodleva
for( i = 0 ; i < max ; i ++ )
  {
    session[i].clientID=0;
    session[i].registrarID=0;
    session[i].language =0;
    session[i].timestamp=0;
  }

}
 // session
bool ccReg_EPP_i::LoginSession( int loginID , int registrarID , int language )
{
int i;

GarbageSesion();

if( numSession < maxSession )
{
// najdi prvni uvolnenou session
for( i = 0 ; i <  maxSession ; i ++ )
  {
   if( session[i].clientID== 0 )
     {
       LOG( DEBUG_LOG , "SESSION  login  clientID %d registrarID %d lang %d" , loginID , registrarID ,  language );
       session[i].clientID=loginID;
       session[i].registrarID=registrarID;
       session[i].language = language;
       session[i].timestamp=( long long ) time(NULL);
       numSession++;
       LOG( DEBUG_LOG , "SESSION  num %d numSession %d timespatmp %lld"  , i , numSession  , session[i].timestamp );
       return true;
     }
  }
}
else
{
LOG( ALERT_LOG , "SESSION MAX_CLIENTS %d"  , maxSession );
}

return false;
}

bool  ccReg_EPP_i::LogoutSession( int loginID )
{
int i;

GarbageSesion();

for( i = 0 ; i < maxSession  ; i ++ )
{
   if( session[i].clientID== loginID )
     {
       session[i].clientID=0;
       session[i].registrarID=0;
       session[i].language =0;
       session[i].timestamp=0;
       numSession--;
       LOG( DEBUG_LOG , "SESSION LOGOUT %d numSession %d"  , i , numSession );
       return true;
     }
}

LOG( ALERT_LOG , "SESSION LOGOUT UNKNOW loginID %d" , loginID );
   
return false;
}

void ccReg_EPP_i::GarbageSesion()
{
int i ;
long long  t;

LOG( DEBUG_LOG , "SESSION GARBAGE" );
t = ( long long )  time(NULL);

for( i = 0 ; i < maxSession ; i ++ )
{

  if( session[i].clientID )
  {
   LOG( DEBUG_LOG , "SESSION  maxWait %lld time %lld timestamp session[%d].timestamp  %lld" ,   maxWaitClient , t , i ,  session[i].timestamp );

   // garbage collector
   if(  t >  session[i].timestamp  +  maxWaitClient )
     {
          LOG( ERROR_LOG , "SESSION[%d] TIMEOUT %lld GARBAGE" ,  i ,  session[i].timestamp);
          session[i].clientID=0;
          session[i].registrarID=0;
          session[i].language =0;
          session[i].timestamp=0;
          numSession--;
     }
  }

}

}


int  ccReg_EPP_i::GetRegistrarID( int clientID )
{
int regID=0;
int i ;

LOG( DEBUG_LOG , "SESSION GetRegistrarID %d" ,   clientID );


for( i = 0 ; i < maxSession ; i ++ )
{

   if( session[i].clientID==clientID )
     {
           session[i].timestamp=  ( long long )  time(NULL);
           LOG( DEBUG_LOG , "SESSION[%d] loginID %d -> regID %d" , i ,  clientID , session[i].registrarID );
           LOG( DEBUG_LOG , "SESSION[%d] TIMESTMAP %lld" ,   i ,  session[i].timestamp );
           regID = session[i].registrarID ; 
      }

  
}

return regID;
}



int  ccReg_EPP_i::GetRegistrarLang( int clientID )
{
int i;

for( i = 0 ; i < numSession ; i ++ )
  {
   if( session[i].clientID==clientID ) 
     {
        LOG( DEBUG_LOG , "SESSION[%d]  loginID %d -> lang %d" ,  i ,  clientID , session[i].language );
        return session[i].language ;
      }
  }

return 0;
}

// test spojeni na databazi
bool ccReg_EPP_i::TestDatabaseConnect(const char *db)
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


int  ccReg_EPP_i:: LoadReasonMessages()
{
DB  DBsql;
int i , rows;

if(  DBsql.OpenDatabase(  database ) )
{
rows=0;
if( DBsql.ExecSelect("SELECT id , reason , reason_cs FROM enum_reason order by id;" ) )
 {
   rows = DBsql.GetSelectRows();
   ReasonMsg = new Mesg(  rows );
   for( i = 0 ; i < rows ; i ++ ) ReasonMsg->AddMesg( atoi(  DBsql.GetFieldValue( i , 0 )  ),  DBsql.GetFieldValue( i , 1 ) , DBsql.GetFieldValue( i , 2) );
   DBsql.FreeSelect();
 }

DBsql.Disconnect();
}
else return -1;

return rows;
}



int ccReg_EPP_i::LoadErrorMessages()
{
DB  DBsql;
int i , rows;

if(  DBsql.OpenDatabase(  database ) )
{
rows=0;
if( DBsql.ExecSelect("SELECT id , status , status_cs FROM enum_error order by id;" ) )
 {
   rows = DBsql.GetSelectRows();
   ErrorMsg = new Mesg(  rows );
   for( i = 0 ; i < rows ; i ++ ) ErrorMsg->AddMesg(  atoi( DBsql.GetFieldValue( i , 0 ) ) ,  DBsql.GetFieldValue( i , 1 ) , DBsql.GetFieldValue( i , 2));
   DBsql.FreeSelect();
 }

DBsql.Disconnect();
}
else return -1;

return rows;
}


char * ccReg_EPP_i::GetReasonMessage(int err , int lang )
{
if( lang == LANG_CS ) return ReasonMsg->GetMesg_CS( err );
else return ReasonMsg->GetMesg( err );
}

char * ccReg_EPP_i::GetErrorMessage(int err , int lang )
{
if( lang  == LANG_CS ) return ErrorMsg->GetMesg_CS( err );
else return ErrorMsg->GetMesg( err );
}
 


void ccReg_EPP_i::SetErrorReason(  ccReg::Response *ret , int errCode ,  ccReg::ErrorSpec reasonCode ,  int reasonMsg , const char *value , int lang )
{
unsigned int seq;
// delka sequence
seq =   ret->errors.length();
ret->errCode = errCode;
ret->errors.length(  seq+1 );
ret->errors[seq].code = reasonCode ;    // spatne zadany neznamy country code
ret->errors[seq].value = CORBA::string_dup( value);
ret->errors[seq].reason = CORBA::string_dup( GetReasonMessage( reasonMsg , lang  ) );

LOG( WARNING_LOG, "SetErrorReason: code %d value [%s] msg [%s] " , reasonCode , value , (const char * )GetReasonMessage( reasonMsg , lang  ) );
}

void ccReg_EPP_i::SetReasonUnknowCC( ccReg::Response *ret ,  const char *value , int lang )
{
LOG( WARNING_LOG, "Reason: unknown country code: %s"  , value );
SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::contact_cc , REASON_MSG_COUNTRY_NOTEXIST ,  value ,  lang );
}


void ccReg_EPP_i::SetReasonContactHandle( ccReg::Response *ret ,  const char *handle , int id ,   int lang )
{
 

if( id < 0 ) 
  {
      LOG( WARNING_LOG, "bad format of contact [%s]" , handle );
      SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::contact_handle ,  REASON_MSG_BAD_FORMAT_CONTACT_HANDLE , handle , lang );
  }
else  if( id == 0 )
        {
                 LOG( WARNING_LOG, "contact handle [%s] NOT_EXIST", handle );
                 ret->errCode = COMMAND_OBJECT_NOT_EXIST;
         }

}



void ccReg_EPP_i::SetReasonNSSetHandle( ccReg::Response *ret ,  const char *handle , int id ,   int lang )
{
 

if( id < 0 ) 
  {
      LOG( WARNING_LOG, "bad format of nsset  [%s]" , handle );
      SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::nsset_handle ,  REASON_MSG_BAD_FORMAT_NSSET_HANDLE , handle , lang );
  }
else  if( id == 0 )
        {
                 LOG( WARNING_LOG, "nsset handle [%s] NOT_EXIST", handle );
                 ret->errCode = COMMAND_OBJECT_NOT_EXIST;
         }

}


void ccReg_EPP_i::SetReasonDomainFQDN(  ccReg::Response *ret , const char *fqdn ,  int zone , int lang  )
{

LOG( WARNING_LOG, "domain in zone %s" , (const char * )  fqdn );
if( zone == 0 )
    SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::domain_fqdn , REASON_MSG_NOT_APPLICABLE_DOMAIN  , fqdn ,   lang );
else if ( zone < 0 )
      SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::domain_fqdn , REASON_MSG_BAD_FORMAT_FQDN  , fqdn ,   lang );

}


void ccReg_EPP_i::SetReasonDomainNSSet(  ccReg::Response *ret , const char * nsset_handle , int  nssetid , int  lang)
{


if( nssetid < 0 )
  {
      LOG( WARNING_LOG, "bad format of domain nsset  [%s]" , nsset_handle );
      SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::domain_nsset ,  REASON_MSG_BAD_FORMAT_NSSET_HANDLE , nsset_handle , lang );
  }
else  if( nssetid == 0 )
        {
          LOG( WARNING_LOG, " domain nsset not exist [%s]" , nsset_handle );
          SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::domain_nsset ,  REASON_MSG_NSSET_NOTEXIST , nsset_handle , lang );
        }

}

void ccReg_EPP_i::SetReasonDomainRegistrant( ccReg::Response *ret , const char * contact_handle , int   contactid , int  lang)
{
if( contactid < 0 )
  {
      LOG( WARNING_LOG, "bad format of registrant  [%s]" , contact_handle );
      SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::domain_registrant ,  REASON_MSG_BAD_FORMAT_CONTACT_HANDLE , contact_handle , lang );
  }
else  if( contactid == 0 )
        {
          LOG( WARNING_LOG, " domain registrant not exist [%s]" , contact_handle );
          SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::domain_registrant ,  REASON_MSG_REGISTRANT_NOTEXIST ,  contact_handle , lang );
        }
}


void ccReg_EPP_i::SetReasonProtectedPeriod( ccReg::Response *ret , const char *value , int lang  )
{
LOG( WARNING_LOG, "object [%s] in history period" , value );
SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::contact_handle ,  REASON_MSG_PROTECTED_PERIOD , value ,  lang );
}




void ccReg_EPP_i::SetReasonContactMap( ccReg::Response *ret ,  ccReg::ErrorSpec reasonCode , const char *handle , int id ,  int lang ,  bool tech_or_admin )
{

if( id < 0 )
  {
     LOG( WARNING_LOG, "bad format of Contact %s" , (const char *) handle );
     SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR ,  reasonCode , REASON_MSG_BAD_FORMAT_CONTACT_HANDLE , handle , lang );
  }
else if( id == 0 )
       {
          LOG( WARNING_LOG, " Contact %s not exist" ,  (const char *)  handle  );
          if( tech_or_admin )  SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , reasonCode , REASON_MSG_TECH_NOTEXIST  , handle  , lang );
          else   SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , reasonCode , REASON_MSG_ADMIN_NOTEXIST  , handle  , lang );
       }

}

void ccReg_EPP_i::SetReasonNSSetTech( ccReg::Response *ret , const char * handle  , int  techID ,  int lang  )
{
SetReasonContactMap(  ret , ccReg::nsset_tech , handle , techID ,lang , true);
}

void ccReg_EPP_i::SetReasonNSSetTechADD( ccReg::Response *ret , const char * handle  , int  techID ,  int lang  )
{
SetReasonContactMap(  ret , ccReg::nsset_tech_add , handle , techID,lang , true);
}

void ccReg_EPP_i::SetReasonNSSetTechREM( ccReg::Response *ret , const char * handle  , int  techID ,  int lang  )
{
SetReasonContactMap(  ret , ccReg::nsset_tech_rem, handle , techID,lang , true);
}


void ccReg_EPP_i::SetReasonDomainAdmin( ccReg::Response *ret , const char * handle  , int  adminID ,  int lang  )
{
SetReasonContactMap(  ret , ccReg::domain_admin , handle , adminID ,lang , false);
}

void ccReg_EPP_i::SetReasonDomainAdminADD( ccReg::Response *ret , const char * handle  , int  adminID ,  int lang  )
{
SetReasonContactMap(  ret , ccReg::domain_admin_add , handle , adminID ,lang , false );
}

void ccReg_EPP_i::SetReasonDomainAdminREM( ccReg::Response *ret , const char * handle  , int  adminID ,  int lang  )
{
SetReasonContactMap(  ret , ccReg::domain_admin_rem , handle , adminID ,lang , false );
}


void ccReg_EPP_i::SetReasonNSSetTechExistMap(   ccReg::Response *ret , const char * handle  ,  int lang  )
{
 LOG( WARNING_LOG, "Tech Contact [%s] exist in contact map table"  , (const char *) handle );
 SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::nsset_tech_add ,  REASON_MSG_TECH_EXIST  , handle , lang );
}

void ccReg_EPP_i::SetReasonNSSetTechNotExistMap(   ccReg::Response *ret , const char * handle  ,  int lang  )
{
 LOG( WARNING_LOG, "Tech Contact [%s] notexist in contact map table"  , (const char *) handle );
 SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::nsset_tech_rem ,  REASON_MSG_TECH_NOTEXIST  , handle , lang );
}


void ccReg_EPP_i::SetReasonDomainAdminExistMap(   ccReg::Response *ret , const char * handle  ,  int lang  )
{
 LOG( WARNING_LOG, "Admin Contact [%s] exist in contact map table"  , (const char *) handle );
 SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::domain_admin_add ,  REASON_MSG_ADMIN_EXIST  , handle , lang );
}

void ccReg_EPP_i::SetReasonDomainAdminNotExistMap(   ccReg::Response *ret , const char * handle  ,  int lang  )
{
 LOG( WARNING_LOG, "Admin Contact [%s] notexist in contact map table"  , (const char *) handle );
 SetErrorReason( ret ,  COMMAND_PARAMETR_ERROR , ccReg::domain_admin_rem ,  REASON_MSG_ADMIN_NOTEXIST  , handle , lang );
}

 

  // nacita tabulku zemi enum_country z databaze
int  ccReg_EPP_i::LoadCountryCode()
{
DB  DBsql;
int i , rows;

if(  DBsql.OpenDatabase(  database) )
{
rows=0;
if( DBsql.ExecSelect("SELECT id FROM enum_country order by id;" ) )
 {
   rows = DBsql.GetSelectRows();
   CC = new CountryCode( rows );
   for( i = 0 ; i < rows ; i ++ ) CC->AddCode(   DBsql.GetFieldValue( i , 0 ) );
   DBsql.FreeSelect();
 }

DBsql.Disconnect();
}
else return -1;

return rows;
}

bool  ccReg_EPP_i::TestCountryCode( const char *cc )
{
LOG( NOTICE_LOG ,  "CCREG:: TestCountryCode  [%s]" , cc );

// kod zeme neni zadan
if( strlen( cc ) == 0 ) return true; 
else
{
 if( strlen(cc ) == 2 )  // dvojmismeny kod zeme
  {
   LOG( NOTICE_LOG ,  "TestCountryCode [%s]" , cc);
   return CC->TestCountryCode( cc ); 
  }
else return false;
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

get_rfc3339_timestamp( t , dateStr , false );
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
/*
bool ccReg_EPP_i::is_null( const char *str )
{
// nastavovani NULL hodnoty
    if( strcmp( str  , ccReg::SET_NULL_VALUE  ) ==  0 || str[0] == 0x8 )return true;
    else return false;

}
*/
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
             (*zone)[i].id=atoi( DBsql.GetFieldValueName( "id" , i )); 
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

unsigned  int ccReg_EPP_i::GetZoneLength() 
{
return  zone->length() ;
}
unsigned int  ccReg_EPP_i::GetZoneID( unsigned int z ) 
{
if( z <  zone->length() ) return  (*zone)[z].id;
else return 0;
}

  // parametry zone
int  ccReg_EPP_i::GetZoneExPeriodMin(int id)
{
unsigned int  z;

for ( z = 0 ; z < zone->length() ; z ++ )
    {
         if(  (*zone)[z].id == id )  return (*zone)[z].ex_period_min;
    }

return 0;
}

int  ccReg_EPP_i::GetZoneExPeriodMax(int id)
{
unsigned int z;

for ( z = 0 ; z < zone->length() ; z ++ )
    {
         if(  (*zone)[z].id == id ) return (*zone)[z].ex_period_max;
    }

return 0;
}

int  ccReg_EPP_i::GetZoneValPeriod(int id)
{
unsigned int z;

for ( z = 0 ; z < zone->length() ; z ++ )
    {
         if(  (*zone)[z].id == id ) return   (*zone)[z].val_period ;
    }

return 0;
}

bool ccReg_EPP_i::GetZoneEnum(int id)
{
unsigned int z;

for ( z = 0 ; z < zone->length() ; z ++ )
    {
        if(  (*zone)[z].id == id ) return  (*zone)[z].enum_zone;
    }

return false;
}


int  ccReg_EPP_i::GetZoneDotsMax( int id) 
{
unsigned int z;

for ( z = 0 ; z < zone->length() ; z ++ )
    {
         if(  (*zone)[z].id == id ) return  (*zone)[z].dots_max ;
    }

return 0;
}

char * ccReg_EPP_i::GetZoneFQDN( int id)
{
unsigned int z;
for ( z = 0 ; z < zone->length() ; z ++ )
    {
         if(  (*zone)[z].id == id ) return  (char *) (*zone)[z].fqdn ;
    }

return "";
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
                     if(  strncasecmp(  fqdn+l ,  (char *) (*zone)[i].fqdn  , slen ) == 0 ) return (*zone)[i].id ; // zaradi do zony  vraci ID
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

                 // test double numbers
                 if(  isdigit( fqdn[i] ) && isdigit( fqdn[i+1] ) )  
                 {LOG( LOG_DEBUG ,  "double digit [%c%c] not allowed"  , fqdn[i] , fqdn[i+1] ); FQDN[0] = 0 ;  return -1; }

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
 * FUNCTION:    SaveOutXML
 *
 * DESCRIPTION: uklada vystupni XML podle 
 *              vygenerovane  server transaction ID
 *
 * PARAMETERS:  svTRID - cislo transakce klienta
 *              XML - xml vystupni string z mod_eppd
 *
 * RETURNED:    true if succes save
 *
 ***********************************************************************/


CORBA::Boolean ccReg_EPP_i::SaveOutXML(const char* svTRID, const char* XML)
{
DB DBsql;
int ok;

if( DBsql.OpenDatabase( database ) )
    {
       if( DBsql.BeginTransaction() )
       {

         if( DBsql.SaveXMLout( svTRID,  XML) ) ok=CMD_OK;
         else ok=0; 

         DBsql.QuitTransaction( ok );
      }    

      DBsql.Disconnect();     
   }
else ServerInternalError();

// default
return true; // TODO
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




ccReg::Response * ccReg_EPP_i::GetTransaction(CORBA::Short errCode, const ccReg::Error& errors , CORBA::Long clientID , const char* clTRID)
{
DB DBsql;
ccReg::Response * ret;
ret = new ccReg::Response;
int i , len;
len = errors.length();
// default
ret->errCode = 0;
ret->errors.length( len );

LOG( NOTICE_LOG, "GetTransaction: clientID -> %d clTRID [%s] ",  (int )  clientID, clTRID );
LOG( NOTICE_LOG, "GetTransaction:  errCode %d",  (int ) errCode  );

// prekopiruj strukturu pro Honzu 
for( i = 0 ;i < len ; i ++ )
   {
        ret->errors[i].code = errors[i].code;
        ret->errors[i].value =  CORBA::string_dup(  errors[i].value ) ;

      switch(  errors[i].code  )
      {
       case ccReg::poll_msgID_missing:
            ret->errors[i].reason = CORBA::string_dup( GetReasonMessage(   REASON_MSG_POLL_MSGID_MISSING ,  GetRegistrarLang( clientID ) ) );
            break;
       case ccReg::contact_identtype_missing:
            ret->errors[i].reason = CORBA::string_dup( GetReasonMessage(  REASON_MSG_CONTACT_IDENTTYPE_MISSING , GetRegistrarLang( clientID ) ) );
            break;
       case ccReg::transfer_op:
            ret->errors[i].reason = CORBA::string_dup( GetReasonMessage(  REASON_MSG_TRANSFER_OP , GetRegistrarLang( clientID ) ) );
            break;
       default:
            ret->errors[i].reason = CORBA::string_dup(  "GetTransaction not specified  msg" );
       }

      LOG( NOTICE_LOG, "return reason msg: errors[%d] code %d value %s  message %s\n" , i ,   ret->errors[i].code , ( char * )   ret->errors[i].value  , (char *)   ret->errors[i].reason  );
     }


  if( DBsql.OpenDatabase( database ) )
    {
      if( errCode > 0 )
        {
           DBsql.BeginAction( clientID, EPP_UnknowAction,  clTRID , "" ) ;
            
              // chybove hlaseni bere z clienta 
              ret->errCode = errCode;
              // zapis na konec action
              // zapis na konec action
              ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
              ret->errMsg = CORBA::string_dup( GetErrorMessage( ret->errCode  ,  GetRegistrarLang( clientID ) ) );

              LOG( NOTICE_LOG, "GetTransaction: svTRID [%s] errCode -> %d msg [%s] ", ( char * ) ret->svTRID, ret->errCode, ( char * ) ret->errMsg );


       }

      DBsql.Disconnect();
    }

  if( ret->errCode == 0 ) ServerInternalError();


return ret;
}

/*
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


ccReg::Response* ccReg_EPP_i::PollAcknowledgement(const char* msgID, CORBA::Short& count, CORBA::String_out newmsgID, CORBA::Long clientID, const char* clTRID, const char* XML)
{
DB DBsql;
ccReg::Response * ret;
char sqlString[1024];
int regID, rows;

ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );
count = 0;
newmsgID =  CORBA::string_dup( "");

LOG( NOTICE_LOG, "PollAcknowledgement: clientID -> %d clTRID [%s] msgID -> %s", (int) clientID, clTRID,    msgID );
if(  ( regID = GetRegistrarID( clientID ) ) )
  if( DBsql.OpenDatabase( database ) )
    {


      if( DBsql.BeginAction( clientID, EPP_PollAcknowledgement, clTRID , XML ) )
        {

          // test msg ID and clientID
          sprintf( sqlString, "SELECT id FROM MESSAGE WHERE id=%s AND seen='f'  AND clID=%d;",   msgID  , regID );
          rows = 0;
          if( DBsql.ExecSelect( sqlString ) )
            {
              rows = DBsql.GetSelectRows();
              DBsql.FreeSelect();
            }
          else  ret->errCode = COMMAND_FAILED;

          if( rows == 0 )
            {
                  LOG( ERROR_LOG, "unknown msgID %s",   msgID );
                  SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::poll_msgID , REASON_MSG_MSGID_NOTEXIST , msgID , GetRegistrarLang( clientID ) );
            }
          else if( rows == 1 )       // pokud tam ta zprava existuje
              {
              // oznac zpravu jako prectenou
              sprintf( sqlString, "UPDATE MESSAGE SET seen='t' WHERE id=%s AND clID=%d;",  msgID, regID );

                 
              if( DBsql.ExecSQL( sqlString ) )
                {
                   ret->errCode = COMMAND_OK;     
                  // zjisteni dalsi messages 
                  sprintf( sqlString, "SELECT id FROM MESSAGE WHERE clID=%d AND seen='f' AND exDate > 'now()' ;", regID );
                  if( DBsql.ExecSelect( sqlString ) )
                    {
                     
                      rows = DBsql.GetSelectRows();   // pocet zprav
                      if( rows > 0 )    // pokud jsou nejake zpravy ve fronte
                        {
                          count = rows; // pocet dalsich zprav
                          newmsgID =   CORBA::string_dup(  DBsql.GetFieldValue( 0, 0 ) ) ;
                          LOG( NOTICE_LOG, "PollAcknowledgement: newmsgID -> %s count -> %d", (const char *) newmsgID, count );
                        }

                     DBsql.FreeSelect();
                    }

                }             


            }

          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

       ret->errMsg = CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

      DBsql.Disconnect();
    }

  if( ret->errCode == 0 ) ServerInternalError();


  return ret;
}





/***********************************************************************
 *
 * FUNCTION:    PollRequest
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

ccReg::Response* ccReg_EPP_i::PollRequest(CORBA::String_out msgID, CORBA::Short& count, ccReg::timestamp_out qDate, CORBA::String_out mesg, CORBA::Long clientID, const char* clTRID, const char* XML)
{
DB DBsql;
char sqlString[1024];
ccReg::Response * ret;
int regID;
int rows;
ret = new ccReg::Response;



//vyprazdni
qDate =  CORBA::string_dup( "" );
count = 0;
msgID = CORBA::string_dup( "" );
mesg = CORBA::string_dup( "" );       // prazdna hodnota

ret->errCode = 0;
ret->errors.length( 0 );


LOG( NOTICE_LOG, "PollRequest: clientID -> %d clTRID [%s]",  (int ) clientID,  clTRID);

if(  ( regID = GetRegistrarID( clientID ) ) )
  if( DBsql.OpenDatabase( database ) )
    {

      // get  registrator ID
      if( (  DBsql.BeginAction( clientID, EPP_PollAcknowledgement,  clTRID , XML )  ) )
        {

         LOG( NOTICE_LOG, "PollRequest: registrarID %d" , regID );
           
          // vypsani zprav z fronty
          sprintf( sqlString, "SELECT *  FROM MESSAGE  WHERE clID=%d AND seen='f' AND exDate > 'now()' ;", regID );

          if( DBsql.ExecSelect( sqlString ) )
            {
              rows = DBsql.GetSelectRows();   // pocet zprav

              if( rows > 0 )    // pokud jsou nejake zpravy ve fronte
                {
                  count = rows;
                  // prevede cas s postgres na rfc3339 cas s offsetem casove zony
                  qDate =  CORBA::string_dup( DBsql.GetFieldDateTimeValueName("CrDate" , 0 )  );
                  msgID =   CORBA::string_dup(  DBsql.GetFieldValueName( "ID", 0 ) );
                  mesg = CORBA::string_dup( DBsql.GetFieldValueName( "message", 0 ) );
                  ret->errCode = COMMAND_ACK_MESG;      // zpravy jsou ve fronte
                  LOG( NOTICE_LOG, "PollRequest: msgID -> %s count -> %d mesg [%s]",  (const char * )   msgID, count, CORBA::string_dup( mesg ) );
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


      ret->errMsg = CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

      DBsql.Disconnect();
    }

  if( ret->errCode == 0 ) ServerInternalError();
  

return ret;
}






/***********************************************************************
 *
 * FUNCTION:    ClientCredit
 *
 * DESCRIPTION: informacee o vysi creditu  prihlaseneho registratora
 * PARAMETERS:  clientID - id pripojeneho klienta
 *              clTRID - cislo transakce klienta
 *        OUT:  credit - vyse creditu v halirich pro kazdou zonu
 *       
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/

ccReg::Response* ccReg_EPP_i::ClientCredit( ccReg::ZoneCredit_out credit, CORBA::Long clientID , const char* clTRID, const char* XML)
{
DB DBsql;
ccReg::Response * ret;
int regID;
long price;
unsigned int z , seq , zoneID;

ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

credit  = new ccReg::ZoneCredit;
credit->length(0);
seq=0;

LOG( NOTICE_LOG, "ClientCredit: clientID -> %d clTRID [%s]",  (int )  clientID, clTRID );

if(  ( regID = GetRegistrarID( clientID ) ) )
  if( DBsql.OpenDatabase( database ) )
    {
      if( ( DBsql.BeginAction( clientID, EPP_ClientCredit, clTRID , XML  )  ) )
        {
 

          for( z = 0 ; z < GetZoneLength() ; z ++ )
          {
            zoneID = GetZoneID(z);
           // vyse creditu registratora prevedena na halire
            price  = DBsql.GetRegistrarCredit( regID , zoneID );

// vzdy neco vracet i kdyz je credit nulovy            if( price >  0)
             {
                credit->length(seq+1);             
                credit[seq].price = price;
                credit[seq].zone_fqdn =  CORBA::string_dup(  GetZoneFQDN(zoneID ) );
                seq++;   
             } 

          }

           ret->errCode = COMMAND_OK;   


          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

      DBsql.Disconnect();
    }

  if( ret->errCode == 0 ) ServerInternalError();

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
int lang;


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
           lang = GetRegistrarLang( clientID ); // zapamatovat si jazyk klienta

         if(  LogoutSession(  clientID ) )// logout session
          {

          if( DBsql.EXEC() )  ret->errCode = COMMAND_LOGOUT;      // uspesne oddlaseni
          else ret->errCode = COMMAND_FAILED;

          } else ret->errCode = COMMAND_FAILED;


          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }
      else
        ret->errCode = COMMAND_FAILED;


        ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , lang )  );

      DBsql.Disconnect();
    }
 
 if( ret->errCode == 0 ) ServerInternalError();

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
int regID=0, id=0;
int language=0;
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


   // dotaz na ID registratora 
    if( ( regID = DBsql.GetNumericFromTable( "REGISTRAR", "id", "handle", ( char * ) ClID ) ) == 0 )
      {
        LOG( NOTICE_LOG, "bad username [%s]", ClID ); // spatne username 
        ret->errCode = COMMAND_AUTH_ERROR;
      }
    else
            
        // ziskej heslo z databaza a  provnej heslo a pokud neni spravne vyhod chybu
        
       if(  !DBsql.TestRegistrarACL( regID ,  passwd ,   certID ) )
         {
                LOG( NOTICE_LOG, "password [%s]  or certID [%s]  not accept", passwd ,  certID  );
                ret->errCode = COMMAND_AUTH_ERROR;
          }
        else
        
          if( DBsql.BeginTransaction() )
            {


                    id = DBsql.GetSequenceID( "login" ); // ziskani id jako sequence

                    // zapis do logovaci tabulky 

                    DBsql.INSERT( "Login" );
                    DBsql.INTO( "id" );
                    DBsql.INTO( "registrarid" );
                    DBsql.INTO( "logintrid" );
                    DBsql.VALUE( id );
                    DBsql.VALUE( regID );
                    DBsql.VALUE( clTRID );
   
                    if( DBsql.EXEC() )    // pokud se podarilo zapsat do tabulky
                      {
                        clientID = id;
                        LOG( NOTICE_LOG, "GET clientID  -> %d",  (int )  clientID );

                        
                        // nankonec zmena komunikacniho  jazyka pouze na cestinu
                        if( lang == ccReg::CS )
                          {
                            LOG( NOTICE_LOG, "SET LANG to CS" ); 

                            DBsql.UPDATE( "Login" );
                            DBsql.SSET( "lang" , "cs"  );
                            DBsql.WHEREID( clientID  );
                            language=1;
                            if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;    // pokud se nezdarilo
                          }



                           // zmena hesla pokud je nejake nastaveno
                           if( strlen( newpass ) )
                             {
                               LOG( NOTICE_LOG, "change password  [%s]  to  newpass [%s] ", passwd, newpass );


                               DBsql.UPDATE( "REGISTRARACL" );
                               DBsql.SET( "password" , newpass );         
                               DBsql.WHERE( "registrarid" , regID  );

                               if( DBsql.EXEC() ==  false ) ret->errCode = COMMAND_FAILED;   // pokud se nezdarilo
                             }


                         if(  ret->errCode == 0 )
                           {
                             if(  LoginSession(  clientID  , regID ,  language ) ) ret->errCode = COMMAND_OK;
                             else 
                              {
                                  clientID=0; // zadny klient pro insert
                                  ret->errCode =COMMAND_MAX_SESSION_LIMIT; // prekrocen maximalni pocet spojeni
                              }
                           }


                      }

              // ukoci transakco 
              DBsql.QuitTransaction( ret->errCode );
          }

          // zapis do tabuky action a vrat svTRID
         if(  DBsql.BeginAction( clientID, EPP_ClientLogin, clTRID , XML )  )
          {
               ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
           
         

             ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );
          }
         else ServerInternalError();

      

    DBsql.Disconnect();
  }

  if( ret->errCode == 0 ) ServerInternalError();

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
//int  zone ;
// char  FQDN[64];
// int id;

Register::NameIdPair caConflict;
Register::Domain::CheckAvailType caType;
Register::Contact::Manager::CheckAvailType cType;
Register::NSSet::Manager::CheckAvailType nType;

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
                           try {
                                std::auto_ptr<Register::Contact::Manager> cman( Register::Contact::Manager::create(&DBsql)  );

                                LOG( NOTICE_LOG ,  "contact checkAvail handle [%s]"  ,  (const char * ) chck[i] );

                                cType = cman->checkAvail(  ( const char * )  chck[i]  , caConflict );
                                LOG( NOTICE_LOG ,  "contact type %d" , cType );
                              }
                     catch (...) {
                               LOG( WARNING_LOG, "cannot run Register::Contact::checkAvail");
                                ret->errCode=COMMAND_FAILED;
                            }

                     switch( cType )
                          {
                             case Register::Contact::Manager::CA_INVALID_HANDLE:
                             a[i].avail = ccReg::BadFormat;    // spatny format
                             a[i].reason =  CORBA::string_dup( GetReasonMessage( REASON_MSG_INVALID_FORMAT , GetRegistrarLang( clientID ) ));                   
                             LOG( NOTICE_LOG ,  "bad format %s of contact handle"  , (const char * ) chck[i] );
                             break;
                             case Register::Contact::Manager::CA_REGISTRED:
                             a[i].avail  =  ccReg::Exist ;    // objekt existuje
                             a[i].reason =  CORBA::string_dup(   GetReasonMessage( REASON_MSG_REGISTRED , GetRegistrarLang( clientID ) )  );
                             LOG( NOTICE_LOG ,  "contact %s exist not Avail" , (const char * ) chck[i] );
                             break;

                             case Register::Contact::Manager::CA_PROTECTED:
                             a[i].avail =  ccReg::DelPeriod;    // ochrana lhuta
                             a[i].reason =  CORBA::string_dup( GetReasonMessage( REASON_MSG_PROTECTED_PERIOD , GetRegistrarLang( clientID ) ) );  // v ochrane lhute
                             LOG( NOTICE_LOG ,  "contact %s in delete period" ,(const char * ) chck[i] );
                             break;
                             case Register::Contact::Manager::CA_FREE:
                             a[i].avail =  ccReg::NotExist;    // objekt ne existuje
                             a[i].reason =  CORBA::string_dup( "");  // free
                             LOG( NOTICE_LOG ,  "contact %s not exist  Avail" ,(const char * ) chck[i] );
                             break;
                         }

 

/* OLD
                     id =  DBsql.GetContactID( chck[i] );
                     if( id >= 0   )
                         {
                            if( id >  0   )
                              {
                                a[i].avail  =  ccReg::Exist ;    // objekt existuje
                                a[i].reason =  CORBA::string_dup(   GetReasonMessage( REASON_MSG_REGISTRED , GetRegistrarLang( clientID ) )  );
                                LOG( NOTICE_LOG ,  "contact %s exist not Avail" , (const char * ) chck[i] );
                              }
                             else
                              {
                               if( DBsql.TestContactHandleHistory(  chck[i]  , DefaultContactHandlePeriod()  ) ) 
                                 {
                                     a[i].avail =  ccReg::DelPeriod;    // ochrana lhuta
                                     a[i].reason =  CORBA::string_dup( GetReasonMessage( REASON_MSG_PROTECTED_PERIOD , GetRegistrarLang( clientID ) ) );  // v ochrane lhute
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
                            a[i].reason =  CORBA::string_dup( GetReasonMessage( REASON_MSG_INVALID_FORMAT , GetRegistrarLang( clientID ) ));
                         }
*/
                        break;

                  case EPP_NSsetCheck:

                           try {
                                std::auto_ptr<Register::NSSet::Manager> nman( Register::NSSet::Manager::create(&DBsql)  );

                                LOG( NOTICE_LOG ,  "nsset checkAvail handle [%s]"  ,  (const char * ) chck[i] );

                                nType = nman->checkAvail(  ( const char * )  chck[i]  , caConflict );
                                LOG( NOTICE_LOG ,  "contact type %d" , cType );
                              }
                     catch (...) {
                               LOG( WARNING_LOG, "cannot run Register::NSSet::checkAvail");
                                ret->errCode=COMMAND_FAILED;
                            }

                     switch( nType )
                          {
                             case Register::NSSet::Manager::CA_INVALID_HANDLE:
                             a[i].avail = ccReg::BadFormat;    // spatny format
                             a[i].reason =  CORBA::string_dup( GetReasonMessage( REASON_MSG_INVALID_FORMAT , GetRegistrarLang( clientID ) ));                   
                             LOG( NOTICE_LOG ,  "bad format %s of nsset handle"  , (const char * ) chck[i] );
                             break;
                             case Register::NSSet::Manager::CA_REGISTRED:
                             a[i].avail  =  ccReg::Exist ;    // objekt existuje
                             a[i].reason =  CORBA::string_dup(   GetReasonMessage( REASON_MSG_REGISTRED , GetRegistrarLang( clientID ) )  );
                             LOG( NOTICE_LOG ,  "nsset %s exist not Avail" , (const char * ) chck[i] );
                             break;

                             case Register::NSSet::Manager::CA_PROTECTED:
                             a[i].avail =  ccReg::DelPeriod;    // ochrana lhuta
                             a[i].reason =  CORBA::string_dup( GetReasonMessage( REASON_MSG_PROTECTED_PERIOD , GetRegistrarLang( clientID ) ) );  // v ochrane lhute
                             LOG( NOTICE_LOG ,  "nsset %s in delete period" ,(const char * ) chck[i] );
                             break;
                             case Register::NSSet::Manager::CA_FREE:
                             a[i].avail =  ccReg::NotExist;    // objekt ne existuje
                             a[i].reason =  CORBA::string_dup( "");  // free
                             LOG( NOTICE_LOG ,  "nsset %s not exist  Avail" ,(const char * ) chck[i] );
                             break;
                         }

/*
                        id =  DBsql.GetNSSetID( chck[i] );

                       if( id >=0   )
                         {
                            if( id > 0  )
                              {
                                a[i].avail = ccReg::Exist;    // objekt existuje
                                a[i].reason =  CORBA::string_dup( GetReasonMessage( REASON_MSG_REGISTRED , GetRegistrarLang( clientID ) )  );
                                LOG( NOTICE_LOG ,  "nsset %s exist not Avail" , (const char * ) chck[i] );
                              }
                             else
                              {
                               if( DBsql.TestNSSetHandleHistory( chck[i]  ,  DefaultDomainNSSetPeriod()  ) )
                                 {
                                     a[i].avail =  ccReg::DelPeriod;    // ochrana lhuta
                                     a[i].reason =  CORBA::string_dup( GetReasonMessage( REASON_MSG_PROTECTED_PERIOD , GetRegistrarLang( clientID ) )  );  // v ochrane lhute
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
                            a[i].reason =  CORBA::string_dup(  GetReasonMessage( REASON_MSG_INVALID_FORMAT , GetRegistrarLang( clientID ) ) );
                         }
*/

                        break;

                  case EPP_DomainCheck:

                           try {
                              std::auto_ptr<Register::Zone::Manager> zm( Register::Zone::Manager::create(&DBsql)  );
                              std::auto_ptr<Register::Domain::Manager> dman( Register::Domain::Manager::create(&DBsql,zm.get())  );

                                LOG( NOTICE_LOG ,  "domain checkAvail fqdn [%s]"  ,  (const char * ) chck[i] );
                              
                                caType = dman->checkAvail(  ( const char * )  chck[i] , caConflict);
                                LOG( NOTICE_LOG ,  "domain type %d" , caType );
                              }
                     catch (...) {
                               LOG( WARNING_LOG, "cannot run Register::Domain::checkAvail");
                                ret->errCode=COMMAND_FAILED;
                            }

                     switch( caType )
                          {
                             case Register::Domain::CA_INVALID_HANDLE:
                             case Register::Domain::CA_BAD_LENGHT:
                                  a[i].avail = ccReg::BadFormat;    // spatny format
                                  a[i].reason =  CORBA::string_dup( GetReasonMessage(REASON_MSG_INVALID_FORMAT , GetRegistrarLang( clientID ) ) );
                                  LOG( NOTICE_LOG ,  "bad format %s of fqdn"  , (const char * ) chck[i] );
                                  break;
                             case Register::Domain::CA_REGISTRED:
                             case Register::Domain::CA_CHILD_REGISTRED:
                             case Register::Domain::CA_PARENT_REGISTRED:
                                  a[i].avail = ccReg::Exist;    // objekt existuje
                                  a[i].reason =  CORBA::string_dup(  GetReasonMessage( REASON_MSG_REGISTRED , GetRegistrarLang( clientID ) ) );
                                  LOG( NOTICE_LOG ,  "domain %s exist not Avail" , (const char * ) chck[i] );                                              
                                  break;
                             case Register::Domain::CA_BLACKLIST: // TODO
                             case Register::Domain::CA_AVAILABLE:
                                  a[i].avail =  ccReg::NotExist;    // objekt ne existuje
                                  a[i].reason =  CORBA::string_dup( "");  // free
                                  LOG( NOTICE_LOG ,  "domain %s not exist  Avail" ,(const char * ) chck[i]  );
                                  break;
                             case Register::Domain::CA_BAD_ZONE:
                                   a[i].avail = ccReg::NotApplicable;    // nepouzitelna domena neni v zone
                                   a[i].reason =  CORBA::string_dup( GetReasonMessage(REASON_MSG_NOT_APPLICABLE_DOMAIN , GetRegistrarLang( clientID ) ) );
                                   LOG( NOTICE_LOG ,  "not applicable %s"  , (const char * ) chck[i] );
                      }                 

 /*
#      CA_INVALID_HANDLE, ///< bad formed handle
#      CA_BAD_ZONE, ///< domain outside of register
#      CA_BAD_LENGHT, ///< domain longer then acceptable
#      CA_PROTECTED, ///< domain temporary protected for registration
#      CA_BLACKLIST, ///< registration blocked in blacklist TODO
#      CA_REGISTRED, ///< domain registred
#      CA_PARENT_REGISTRED, ///< parent already registred
#      CA_CHILD_REGISTRED, ///< child already registred
#      CA_AVAILABLE ///< domain is available
*/
/* OLD ver
                       if( (  zone =  getFQDN( FQDN , chck[i] )  ) > 0  )
                         {
                            if( ( id = DBsql.GetDomainID( FQDN , GetZoneEnum( zone ) ) ) > 0  )
                              {
                                a[i].avail = ccReg::Exist;    // objekt existuje
                                a[i].reason =  CORBA::string_dup(  GetReasonMessage( REASON_MSG_REGISTRED , GetRegistrarLang( clientID ) ) );
                                LOG( NOTICE_LOG ,  "domain %s exist not Avail" , (const char * ) chck[i] );
                              }
                             else
                              {

                               if( DBsql.TestDomainFQDNHistory( FQDN , DefaultDomainFQDNPeriod() ) )
                                 {
                                     a[i].avail =  ccReg::DelPeriod;    // objekt byl smazan je v historri a ma ochranou lhutu
                                     a[i].reason =  CORBA::string_dup(  GetReasonMessage( REASON_MSG_PROTECTED_PERIOD , GetRegistrarLang( clientID ) ) );  // v ochrane lhute
                                     LOG( NOTICE_LOG ,  "domain %s in delete period" ,(const char * ) chck[i] );
                                 }
                                else
                                 {
                                     a[i].avail =  ccReg::NotExist;    // objekt ne existuje
                                     a[i].reason =  CORBA::string_dup( "");  // free
                                     LOG( NOTICE_LOG ,  "domain %s not exist  Avail" ,(const char * ) chck[i]  );
                                  }
                              }

                         }
                       else
                         {
                           if( zone < 0 )   
                             {
                               LOG( NOTICE_LOG ,  "bad format %s of fqdn"  , (const char * ) chck[i] );
                               a[i].avail = ccReg::BadFormat;    // spatny format
                               a[i].reason =  CORBA::string_dup( GetReasonMessage(REASON_MSG_INVALID_FORMAT , GetRegistrarLang( clientID ) ) );
                             }
                           else
                             {
                               LOG( NOTICE_LOG ,  "not applicable %s"  , (const char * ) chck[i] );
                               a[i].avail = ccReg::NotApplicable;    // nepouzitelna domena neni v zone
                               a[i].reason =  CORBA::string_dup( GetReasonMessage(REASON_MSG_NOT_APPLICABLE_DOMAIN , GetRegistrarLang( clientID ) ) );
                             }
                         }
*/

                        break;


           }
     }


      // comand OK
     if( ret->errCode == 0 ) ret->errCode=COMMAND_OK; // vse proslo OK zadna chyba

      // zapis na konec action
      ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) ) ;
  }


ret->errMsg =  CORBA::string_dup(   GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  ) ;

DBsql.Disconnect();
}

  if( ret->errCode == 0 ) ServerInternalError();


 
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
// Status status;
ccReg::Response *ret;
// char HANDLE[64]; // handle na velka pismena
// char dateStr[MAX_DATE];
int  clid , crid , upid , regID ;
int  ssn , id , slen;
int s , snum ;
char streetStr[32];

c = new ccReg::Contact;
// inicializace pro pripad neuspesneho hledani
c->DiscloseName = ccReg::DISCL_EMPTY;
c->DiscloseOrganization = ccReg::DISCL_EMPTY;
c->DiscloseAddress = ccReg::DISCL_EMPTY;
c->DiscloseTelephone = ccReg::DISCL_EMPTY;
c->DiscloseFax  = ccReg::DISCL_EMPTY;
c->DiscloseEmail = ccReg::DISCL_EMPTY;
c->identtype = ccReg::EMPTY; 

ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ContactInfo: clientID -> %d clTRID [%s] handle [%s] " ,  (int )  clientID , clTRID , handle );

if(  (regID = GetRegistrarID( clientID ) ) )
 if( DBsql.OpenDatabase( database ) )
  {

  
  if(   DBsql.BeginAction( clientID , EPP_ContactInfo ,  clTRID  , XML )  )
   {
 

  if( (id = DBsql.GetContactID( handle ) ) <= 0  )  SetReasonContactHandle( ret , handle , id , GetRegistrarLang( clientID ) );
  else if( DBsql.BeginTransaction() )
  {
    if( DBsql.SELECTOBJECTID( "CONTACT" ,"handle"  ,  id )  )
      {


     //        id =   DBsql.GetFieldNumericValueName("ID" , 0 ); // ID kontaktu pro ziskani vazeb
        clid =  DBsql.GetFieldNumericValueName("ClID" , 0 ); 
        crid =  DBsql.GetFieldNumericValueName("CrID" , 0 ); 
        upid =  DBsql.GetFieldNumericValueName("UpID" , 0 ); 




	c->ROID=CORBA::string_dup( DBsql.GetFieldValueName("ROID" , 0 ) ); // ROID     

	c->CrDate= CORBA::string_dup( DBsql.GetFieldDateTimeValueName("CrDate" , 0 ) );
        c->UpDate= CORBA::string_dup(  DBsql.GetFieldDateTimeValueName("UpDate" , 0 ) ); 
        c->TrDate= CORBA::string_dup(   DBsql.GetFieldDateTimeValueName("TrDate" , 0 )  );


	c->handle=CORBA::string_dup( DBsql.GetFieldValueName("Name" , 0 ) ); // handle

	c->Organization=CORBA::string_dup( DBsql.GetFieldValueName("Organization" , 0 )); // nazev organizace
       
        for( s = 0 , snum =0  ; s < 3 ; s ++ )
        {
        sprintf( streetStr , "Street%d" , s +1);           
        if(  DBsql.IsNotNull( 0 ,  DBsql.GetNameField(  streetStr ) ) ) snum ++ ;
        }

        c->Streets.length( snum );
         for( s = 0   ; s < 3 ; s ++ )
          {
            sprintf( streetStr , "Street%d" , s +1);           
           if(  DBsql.IsNotNull( 0 ,  DBsql.GetNameField(  streetStr ) ) )
           {
            c->Streets.length( snum + 1 );
            c->Streets[snum]=CORBA::string_dup( DBsql.GetFieldValueName(  streetStr, 0 ) ); // adresa
            snum ++;
            }
          }

	c->City=CORBA::string_dup( DBsql.GetFieldValueName("City" , 0 ) );  // obec
	c->StateOrProvince=CORBA::string_dup( DBsql.GetFieldValueName("StateOrProvince"  , 0 ));
	c->PostalCode=CORBA::string_dup(DBsql.GetFieldValueName("PostalCode" , 0 )); // PSC
	c->Telephone=CORBA::string_dup( DBsql.GetFieldValueName("Telephone" , 0 ));
	c->Fax=CORBA::string_dup(DBsql.GetFieldValueName("Fax" , 0 ));
	c->Email=CORBA::string_dup(DBsql.GetFieldValueName("Email" , 0 ));
	c->NotifyEmail=CORBA::string_dup(DBsql.GetFieldValueName("NotifyEmail" , 0 )); // upozornovaci email
        c->CountryCode=CORBA::string_dup(  DBsql.GetFieldValueName("Country" , 0 )  ); // vracet pouze ISO kod


	c->VAT=CORBA::string_dup(DBsql.GetFieldValueName("VAT" , 0 )); // DIC
	c->ident=CORBA::string_dup(DBsql.GetFieldValueName("SSN" , 0 )); // SSN

        ssn = DBsql.GetFieldNumericValueName("SSNtype" , 0 ) ;



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

        // jmeno kontaktu
	c->Name=CORBA::string_dup( DBsql.GetValueFromTable(  "Contact" , "Name"  , "id" , id )   ); // jmeno nebo nazev kontaktu


       if( DBsql.TestContactRelations( id ) )  slen = 2;
       else slen=1;

        c->stat.length(slen);

        c->stat[0].value = CORBA::string_dup( "ok" );         
        c->stat[0].text = CORBA::string_dup( "Contact is OK" ); 

       if( slen > 1 )
        {
          c->stat[1].value = CORBA::string_dup( "linked" );         
          c->stat[1].text = CORBA::string_dup( "Contact is admin or tech" ); 
        }

              
        // identifikator registratora
        c->CrID =  CORBA::string_dup(  DBsql.GetRegistrarHandle( crid ) );
        c->UpID =  CORBA::string_dup(  DBsql.GetRegistrarHandle( upid ) );
        c->ClID =  CORBA::string_dup(  DBsql.GetRegistrarHandle( clid ) );


        // type SSN EMPTY , RC , OP , PASS , ICO
        
        switch( ssn )
        {
         case 1:
                  c->identtype = ccReg::RC;
                  break;
         case 2:
                  c->identtype = ccReg::OP;
                  break;
         case 3:
                  c->identtype = ccReg::PASS;
                  break;
         case 4:
                  c->identtype = ccReg::ICO;
                  break;
         case 5:
                  c->identtype = ccReg::MPSV;
                  break;
         default:
                 c->identtype = ccReg::EMPTY;
                  break;
        }

        ret->errCode=COMMAND_OK; // VAS OK


      } else ret->errCode=COMMAND_FAILED; // select neprosel

     // konec transakce commit ci rollback
      DBsql.QuitTransaction( ret->errCode );
    }


     // zapis na konec action
     ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
  }
        



ret->errMsg =  CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )   );

DBsql.Disconnect();
}


if( ret->errCode == 0 ) ServerInternalError();
 

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
// Status status;
DB DBsql;
int regID , id ;

ret = new ccReg::Response;


ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ContactDelete: clientID -> %d clTRID [%s] handle [%s] " , (int )  clientID , clTRID , handle );



if(  ( regID = GetRegistrarID( clientID ) ) )

  if( DBsql.OpenDatabase( database ) )
    {
      if( (  DBsql.BeginAction( clientID, EPP_ContactDelete,  clTRID , XML )  ))
        {

 
         if( (id = DBsql.GetContactID( handle ) ) <= 0  )  SetReasonContactHandle( ret , handle , id , GetRegistrarLang( clientID ) );      
          else if( DBsql.BeginTransaction() )
          {


                  if( !DBsql.TestObjectClientID( id , regID  )   )   // pokud neni klientem
                    {
                      LOG( WARNING_LOG, "bad autorization not  creator of handle [%s]", handle );
                      ret->errCode = COMMAND_AUTOR_ERROR; // spatna autorizace
                    }                               
                  else                                                                                           
                    
                          // test na vazbu do tabulky domain domain_contact_map a nsset_contact_map
                          if( DBsql.TestContactRelations( id ) )        // kontakt nemuze byt smazan ma vazby  
                            {
                              LOG( WARNING_LOG, "test contact handle [%s] relations: PROHIBITS_OPERATION", handle );
                              ret->errCode = COMMAND_PROHIBITS_OPERATION;
                            }
                          else
                            {
                               if(  DBsql.SaveObjectDelete( id  ) ) // uloz do tabulky smazanych objektu
                                 {
                                       if(  DBsql.DeleteContactObject( id ) ) ret->errCode = COMMAND_OK;      // pokud usmesne smazal
                                 }

                            }

                        

                    


                

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }


          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }


      ret->errMsg = CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

      DBsql.Disconnect();
    }


if( ret->errCode == 0 ) ServerInternalError();

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
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response * ccReg_EPP_i::ContactUpdate( const char *handle, const ccReg::ContactChange & c, 
                                              CORBA::Long clientID, const char *clTRID , const char* XML )
{
ccReg::Response * ret;
DB DBsql;
int regID ,  id ;
int s , snum;
char streetStr[10];

ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

LOG( NOTICE_LOG, "ContactUpdate: clientID -> %d clTRID [%s] handle [%s] ", (int ) clientID, clTRID, handle );
LOG( NOTICE_LOG, "Discloseflag %d: Disclose Name %d Org %d Add %d Tel %d Fax %d Email %d" , c.DiscloseFlag ,
 c.DiscloseName  , c.DiscloseOrganization , c.DiscloseAddress , c.DiscloseTelephone , c.DiscloseFax , c.DiscloseEmail );



if(  ( regID = GetRegistrarID( clientID ) ) )

  if( DBsql.OpenDatabase( database ) )
    {

      if( (  DBsql.BeginAction( clientID, EPP_ContactUpdate, clTRID , XML ) ) )
        {

 
         if( (id = DBsql.GetContactID( handle ) ) <= 0  )  SetReasonContactHandle( ret , handle , id , GetRegistrarLang( clientID ) );
         else  if( DBsql.BeginTransaction() )      // zahajeni transakce
            {

                  if( !DBsql.TestObjectClientID( id , regID  )  )
                    {
                        LOG( WARNING_LOG, "bad autorization not  client of contact [%s]", handle );
                        ret->errCode = COMMAND_AUTOR_ERROR;     // spatna autorizace
                    }
                  else                   

                      if( !TestCountryCode( c.CC ) ) SetReasonUnknowCC( ret , c.CC , GetRegistrarLang( clientID ) );
                      else
                          if( DBsql.ObjectUpdate(id , regID  , c.AuthInfoPw )  ) // update OBJECT tabulky 
                             {

                                          // zahaj update
                                          DBsql.UPDATE( "Contact" );

                                          DBsql.SET( "Name", c.Name );
                                          DBsql.SET( "Organization", c.Organization );
                                          snum = c.Streets.length();
                                          for( s = 0 ; s < 3  ; s ++ )
                                             {
                                               sprintf( streetStr , "Street%d" , s +1);
                                               if( s < snum ) DBsql.SET(  streetStr , c.Streets[s] );
                                               else DBsql.SET( streetStr , "\b" ); // SET NULL by hack
                                             }

                                          DBsql.SET( "City", c.City );
                                          DBsql.SET( "StateOrProvince", c.StateOrProvince );
                                          DBsql.SET( "PostalCode", c.PostalCode);
                                          DBsql.SET( "Country", c.CC );
                                          DBsql.SET( "Telephone", c.Telephone );
                                          DBsql.SET( "Fax", c.Fax );
                                          DBsql.SET( "Email", c.Email );
                                          DBsql.SET( "NotifyEmail", c.NotifyEmail );
                                          DBsql.SET( "VAT", c.VAT );
                                          DBsql.SET( "SSN", c.ident );
                                          if(  c.identtype > ccReg::EMPTY )  DBsql.SET( "SSNtype" , c.identtype ); // typ ssn

                                          // heslo je ve spolecne tabulce object 
                                          // DBsql.SET( "AuthInfoPw", c.AuthInfoPw ); 



                                          //  Disclose parametry
                                          DBsql.SETBOOL( "DiscloseName", update_DISCLOSE( c.DiscloseName , c.DiscloseFlag ) );
                                          DBsql.SETBOOL( "DiscloseOrganization", update_DISCLOSE( c.DiscloseOrganization , c.DiscloseFlag ) );
                                          DBsql.SETBOOL( "DiscloseAddress", update_DISCLOSE(  c.DiscloseAddress, c.DiscloseFlag  ) );
                                          DBsql.SETBOOL( "DiscloseTelephone",  update_DISCLOSE(  c.DiscloseTelephone , c.DiscloseFlag ) );
                                          DBsql.SETBOOL( "DiscloseFax",  update_DISCLOSE( c.DiscloseFax , c.DiscloseFlag ) );
                                          DBsql.SETBOOL( "DiscloseEmail", update_DISCLOSE( c.DiscloseEmail, c.DiscloseFlag  ) );



                                          // podminka na konec 
                                          DBsql.WHEREID( id );

                                          if( DBsql.EXEC() )  if(   DBsql.SaveContactHistory( id ) )  ret->errCode = COMMAND_OK;

                             }

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );            
           }
          

          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }


      ret->errMsg = CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );


      DBsql.Disconnect();
    }


if( ret->errCode == 0 ) ServerInternalError();

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
ccReg::Response * ret;
int regID, id;
int s , snum;
char streetStr[10];

// default
ret = new ccReg::Response;
ret->errCode = 0;
ret->errors.length( 0 );

crDate = CORBA::string_dup( "" ); 


LOG( NOTICE_LOG, "ContactCreate: clientID -> %d clTRID [%s] handle [%s]", (int ) clientID, clTRID, handle );
LOG( NOTICE_LOG, "Discloseflag %d: Disclose Name %d Org %d Add %d Tel %d Fax %d Email %d" , c.DiscloseFlag ,
 c.DiscloseName  , c.DiscloseOrganization , c.DiscloseAddress , c.DiscloseTelephone , c.DiscloseFax , c.DiscloseEmail );


if(  ( regID = GetRegistrarID( clientID ) ) )

  if( DBsql.OpenDatabase( database ) )
    {
      if( (  DBsql.BeginAction( clientID, EPP_ContactCreate,  clTRID ,  XML )  ))
        {
 
         if( (id = DBsql.GetContactID( handle ) ) < 0 ) SetReasonContactHandle( ret , handle , id , GetRegistrarLang( clientID ) ); 
           else if( id ) 
               {
                   LOG( WARNING_LOG  ,  "object [%s] EXIST" ,  handle );
                   ret->errCode= COMMAND_OBJECT_EXIST;
               }             
             else   if( DBsql.BeginTransaction() )      // zahajeni transakce
              {
                    // test jestli neni ve smazanych kontaktech
                  if( DBsql.TestContactHandleHistory( handle , DefaultContactHandlePeriod() ) ) SetReasonProtectedPeriod( ret , handle , GetRegistrarLang( clientID ) );
                  else
                  // test zdali country code je existujici zeme
                  if( !TestCountryCode( c.CC ) ) SetReasonUnknowCC( ret , c.CC , GetRegistrarLang( clientID ) );
                  else
                    {
                    //  id = DBsql.GetSequenceID( "contact" );

                      id= DBsql.CreateObject( "C" ,  regID , handle ,  c.AuthInfoPw );


                      DBsql.INSERT( "CONTACT" );
                      DBsql.INTO( "id" );


                      DBsql.INTOVAL( "Name", c.Name );
                      DBsql.INTOVAL( "Organization", c.Organization );

                      snum = c.Streets.length();
                      for( s = 0 ; s < snum  ; s ++ )
                         {
                           sprintf( streetStr , "Street%d" , s +1);
                           DBsql.INTOVAL(  streetStr, c.Streets[s] );
                         }


                      DBsql.INTOVAL( "City", c.City );
                      DBsql.INTOVAL( "StateOrProvince", c.StateOrProvince );
                      DBsql.INTOVAL( "PostalCode", c.PostalCode );
                      DBsql.INTOVAL( "Country", c.CC );
                      DBsql.INTOVAL( "Telephone", c.Telephone );
                      DBsql.INTOVAL( "Fax", c.Fax );
                      DBsql.INTOVAL( "Email", c.Email );
                      DBsql.INTOVAL( "NotifyEmail", c.NotifyEmail );
                      DBsql.INTOVAL( "VAT", c.VAT );
                      DBsql.INTOVAL( "SSN", c.ident );
                      if(  c.identtype > ccReg::EMPTY ) DBsql.INTO( "SSNtype");

                      // disclose se vzdy zapisou but t nebo f
                      DBsql.INTO( "DiscloseName" );
                      DBsql.INTO( "DiscloseOrganization" );
                      DBsql.INTO( "DiscloseAddress" );
                      DBsql.INTO( "DiscloseTelephone" );
                      DBsql.INTO( "DiscloseFax" );
                      DBsql.INTO( "DiscloseEmail" );

                      DBsql.VALUE( id );

                      DBsql.VAL( c.Name );
                      DBsql.VAL( c.Organization );
                      snum = c.Streets.length();
                      for( s = 0 ; s < snum  ; s ++ )  
                         {
                           sprintf( streetStr , "Street%d" , s +1);
                           DBsql.VAL(  c.Streets[s] );
                         }

                      DBsql.VAL( c.City );
                      DBsql.VAL( c.StateOrProvince );
                      DBsql.VAL( c.PostalCode );
                      DBsql.VAL( c.CC );
                      DBsql.VAL( c.Telephone );
                      DBsql.VAL( c.Fax );
                      DBsql.VAL( c.Email );
                      DBsql.VAL( c.NotifyEmail );
                      DBsql.VAL( c.VAT );
                      DBsql.VAL( c.ident );
                      if(  c.identtype > ccReg::EMPTY ) DBsql.VALUE(   c.identtype );

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
                          // zjisti datum a cas vytvoreni objektu
                          crDate= CORBA::string_dup( DBsql.GetObjectCrDateTime( id )  );
                          if(   DBsql.SaveContactHistory( id ) )   // pokud se ulozilo do Historie
                              if ( DBsql.SaveObjectCreate( id ) )   ret->errCode = COMMAND_OK;    // pokud ulozilo crhistoryID 
                        }                    
                   }

                

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            
          }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );
      DBsql.Disconnect();
   }



if( ret->errCode == 0 ) ServerInternalError();

return ret;
}




/***********************************************************************
 *
 * FUNCTION:    ObjectTransfer
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


ccReg::Response* ccReg_EPP_i::ObjectTransfer(short act ,  const char*table ,  const char*fname , const char* name,
                                           const char* authInfo, CORBA::Long clientID, const char* clTRID , const char* XML )
{
ccReg::Response *ret;
DB DBsql;
char FQDN[64];
char pass[PASS_LEN+1];
int regID  , id;
int zone;

ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "ObjectContact: act %d  clientID -> %d clTRID [%s] object [%s] authInfo [%s] " , act  ,  (int ) clientID , clTRID , name , authInfo );
if(  (regID = GetRegistrarID( clientID ) ) )
 if( DBsql.OpenDatabase( database ) )
 {

 if( (  DBsql.BeginAction( clientID , act  ,  clTRID , XML  )  ) )
 {
 

   switch( act )
   {
   case EPP_ContactTransfer:
       if( (id = DBsql.GetContactID( name ) ) <= 0 ) SetReasonContactHandle( ret , name , id , GetRegistrarLang( clientID ) );
       break;

   case EPP_NSsetTransfer:
       if( (id = DBsql.GetNSSetID( name ) ) <= 0 ) SetReasonNSSetHandle( ret , name , id , GetRegistrarLang( clientID ) );
       break;
   case EPP_DomainTransfer:
      // preved fqd na  mala pismena a otestuj to
       if(  ( zone = getFQDN( FQDN , name ) ) <= 0  ) SetReasonDomainFQDN( ret , name , zone , GetRegistrarLang( clientID ) );
       else
         {
               if( ( id = DBsql.GetDomainID( FQDN ,  GetZoneEnum( zone ) ) )   == 0 )
                {
                   LOG( WARNING_LOG  ,  "domain [%s] NOT_EXIST" ,  name );
                   ret->errCode= COMMAND_OBJECT_NOT_EXIST;
               }

               
          }
         break;

    }

   

   
    if( DBsql.BeginTransaction()  )
      {
 
               if(  DBsql.TestObjectClientID( id , regID  )  )       // transfer nemuze delat stavajici client
                 {
                   LOG( WARNING_LOG, "client can not transfer  object %s" , name );
                   ret->errCode =  COMMAND_NOT_ELIGIBLE_FOR_TRANSFER;
                 }
               else
                 {

                    if(  DBsql.AuthTable(   "OBJECT"  , (char *)authInfo , id )  == false  ) // pokud prosla autentifikace 
                      {       
                         LOG( WARNING_LOG , "autorization failed");
                         ret->errCode = COMMAND_AUTOR_ERROR; // spatna autorizace
                      }
                    else
                      {
                       


                              // pri prevodu autogeneruj nove heslo
                              random_pass(  pass  );
 
                              // zmena registratora
                              DBsql.UPDATE( "OBJECT" );
                              DBsql.SSET( "TrDate" , "now" );
                              DBsql.SSET( "AuthInfoPw" , pass );
                              DBsql.SET( "ClID" , regID );
                              DBsql.WHEREID( id ); 

                              if(   DBsql.EXEC() )  
                               {
                                 switch( act )
                                  {
                                    case EPP_ContactTransfer:
                                     if(   DBsql.SaveContactHistory(  id ) ) ret->errCode = COMMAND_OK;
                                      break;
                                    case EPP_NSsetTransfer:
                                     if(   DBsql.SaveNSSetHistory(  id ) ) ret->errCode = COMMAND_OK;
                                      break;
                                   case EPP_DomainTransfer:
                                     if(   DBsql.SaveDomainHistory(  id ) ) ret->errCode = COMMAND_OK;
                                      break;

                                    
                                  }
                               }
                       
                      
                      }
                 }

                     
         
    // konec transakce commit ci rollback
    DBsql.QuitTransaction( ret->errCode );
   }

  
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode  ) ) ;
 }


ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

DBsql.Disconnect();
}


if( ret->errCode == 0 ) ServerInternalError();

return ret;
}



ccReg::Response* ccReg_EPP_i::ContactTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID , const char* XML )
{
return ObjectTransfer( EPP_ContactTransfer , "CONTACT" ,  "handle" , handle, authInfo,  clientID, clTRID , XML );
}

ccReg::Response* ccReg_EPP_i::NSSetTransfer(const char* handle, const char* authInfo, CORBA::Long clientID, const char* clTRID , const char* XML )
{
return ObjectTransfer( EPP_NSsetTransfer , "NSSET" ,  "handle" , handle, authInfo,  clientID, clTRID , XML );
}

ccReg::Response* ccReg_EPP_i::DomainTransfer(const char* fqdn, const char* authInfo, CORBA::Long clientID, const char* clTRID , const char* XML )
{
return ObjectTransfer( EPP_DomainTransfer , "DOMAIN" , "fqdn" , fqdn, authInfo,  clientID, clTRID , XML );
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
// char HANDLE[64];
// char  adres[1042] , adr[128] ;
// char dateStr[MAX_DATE];
ccReg::Response *ret;
int hostID[10]; // TODO define max hostID
int clid ,  crid , upid , nssetid , regID;
int i , j  ,ilen , len , slen ;

ret = new ccReg::Response;
n = new ccReg::NSSet;

// default
ret->errCode = 0;
ret->errors.length(0);
LOG( NOTICE_LOG ,  "NSSetInfo: clientID -> %d clTRID [%s] handle [%s] " , (int ) clientID , clTRID , handle );
 

if(  (regID = GetRegistrarID( clientID ) ) )

if( DBsql.OpenDatabase( database ) )
{

if( ( DBsql.BeginAction( clientID , EPP_NSsetInfo , clTRID , XML  )  ))
 {
 


 if( (nssetid = DBsql.GetNSSetID( handle ) ) <= 0  )  SetReasonNSSetHandle( ret , handle , nssetid , GetRegistrarLang( clientID ) ); 
 else if( DBsql.BeginTransaction() )
  {
     if(  DBsql.SELECTOBJECTID( "NSSET" , "HANDLE" , nssetid ) )
     {
 
//        nssetid = DBsql.GetFieldNumericValueName("ID" , 0 );
        clid =  DBsql.GetFieldNumericValueName("ClID" , 0 );
        crid =  DBsql.GetFieldNumericValueName("CrID" , 0 );
        upid =  DBsql.GetFieldNumericValueName("UpID" , 0 );


        n->ROID=CORBA::string_dup( DBsql.GetFieldValueName("ROID" , 0 ) ); // ROID
        n->handle=CORBA::string_dup( DBsql.GetFieldValueName("name" , 0 ) ); // ROID
        n->CrDate= CORBA::string_dup( DBsql.GetFieldDateTimeValueName("CrDate" , 0 ) );
        n->UpDate= CORBA::string_dup(  DBsql.GetFieldDateTimeValueName("UpDate" , 0 ) );
        n->TrDate= CORBA::string_dup(  DBsql.GetFieldDateTimeValueName("TrDate" , 0 ) );

        // uroven tech testu
        n->level =  DBsql.GetFieldNumericValueName("checklevel" , 0 );

        if( regID == clid ) // pokud je registrator clientem obdrzi autentifikaci
           n->AuthInfoPw = CORBA::string_dup( DBsql.GetFieldValueName("AuthInfoPw" , 0 ) ); // autentifikace
         else  n->AuthInfoPw = CORBA::string_dup( "" ); // jinak prazdny retezec

        ret->errCode=COMMAND_OK;


        // free select
        DBsql.FreeSelect();





       if( DBsql.TestNSSetRelations( nssetid ) )  slen = 2;
       else slen=1;

        n->stat.length(slen);
        n->stat[0].value = CORBA::string_dup( "ok" );
        n->stat[0].text = CORBA::string_dup( "NSSet is OK" );


       if( slen > 1 )
        {
          n->stat[1].value = CORBA::string_dup( "linked" );
          n->stat[1].text = CORBA::string_dup( "NSSet is linked with domains" );
        }



        n->ClID =  CORBA::string_dup( DBsql.GetRegistrarHandle( clid ) );
        n->CrID =  CORBA::string_dup( DBsql.GetRegistrarHandle( crid ) );
        n->UpID =  CORBA::string_dup( DBsql.GetRegistrarHandle( upid ) );

        // dotaz na DNS servry  na tabulky host
        if(   DBsql.SELECTONE( "HOST" , "nssetid" , nssetid  ) )
          {  
             len =  DBsql.GetSelectRows();
            
               
             n->dns.length(len);
 
             for( i = 0 ; i < len ; i ++)   
                {                     
                   // fqdn DNS servru nazev  
                   n->dns[i].fqdn = CORBA::string_dup(  DBsql.GetFieldValueName("fqdn" , i ) );
                   hostID[i] =  DBsql.GetFieldNumericValueName("id"  , i ); // ziskej hostID 
                }
             DBsql.FreeSelect();

               // ziskej vypid ipadres pro jednotlive HOSTY
                for( i = 0 ; i < len ; i ++)
                 {
                      if(   DBsql.SELECTONE( "HOST_ipaddr_map" , "hostID" ,  hostID[i] ) )
                       {
                           ilen = DBsql.GetSelectRows(); // pocet ip adres
                           n->dns[i].inet.length(ilen); // sequence ip adres
                           for( j = 0 ; j < ilen ; j ++)                      
                               n->dns[i].inet[j] =CORBA::string_dup( DBsql.GetFieldValueName("ipaddr" , j ) );
                          DBsql.FreeSelect();
                       }
                    else n->dns[i].inet.length(0); // nulova delka pokud nejsou zadany ip adresy
                  }
 

          } else ret->errCode=COMMAND_FAILED;
 



        // dotaz na technicke kontakty
        if(  DBsql.SELECTCONTACTMAP( "nsset"  , nssetid ) )
          {
               len =  DBsql.GetSelectRows(); // pocet technickych kontaktu
               n->tech.length(len); // technicke kontaktry handle
               for( i = 0 ; i < len ; i ++) n->tech[i] = CORBA::string_dup( DBsql.GetFieldValue( i , 0 )  );

               DBsql.FreeSelect();
          } else ret->errCode=COMMAND_FAILED;

     

     } else ret->errCode=COMMAND_FAILED;

    // konec transakce commit ci rollback
    DBsql.QuitTransaction( ret->errCode );

    }else ret->errCode=COMMAND_FAILED;

       
   // zapis na konec action
   ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode  ) ) ;
 }

ret->errMsg = CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

DBsql.Disconnect();
}


if( ret->errCode == 0 ) ServerInternalError();


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
int regID , id ;
bool  del;


ret = new ccReg::Response;


ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetDelete: clientID -> %d clTRID [%s] handle [%s] " , (int ) clientID , clTRID , handle );

if(  ( regID = GetRegistrarID( clientID ) ) )

  if( DBsql.OpenDatabase( database ) )
    {

      if( (  DBsql.BeginAction( clientID, EPP_NSsetDelete, clTRID , XML )  ))
        {
 
          if( (  id = DBsql.GetNSSetID( handle ) ) <= 0 )  SetReasonNSSetHandle( ret , handle , id , GetRegistrarLang( clientID ) );
          else if( DBsql.BeginTransaction() )      // zahajeni transakce
            {

                  if(  !DBsql.TestObjectClientID( id , regID  )  )   // pokud neni klientem 
                    {
                      LOG( WARNING_LOG, "bad autorization not client of nsset [%s]", handle );
                      ret->errCode = COMMAND_AUTOR_ERROR;       // spatna autorizace
                    }
                  else
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
                              if(  DBsql.SaveObjectDelete( id  ) ) // uloz do tabulky smazanych objektu
                                {
                                     if( DBsql.DeleteNSSetObject( id )  ) ret->errCode = COMMAND_OK;   // pokud vse OK 
                                }
                            }                        
                    }
                

              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );
            }

          


          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg = CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

      DBsql.Disconnect();
    }


if( ret->errCode == 0 ) ServerInternalError();

 
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
 *              level - tech check  level
 *        OUT:  crDate - datum vytvoreni objektu
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/


ccReg::Response * ccReg_EPP_i::NSSetCreate( const char *handle, const char *authInfoPw, 
                                            const ccReg::TechContact & tech, const ccReg::DNSHost & dns,CORBA::Short level,
                                            ccReg::timestamp_out crDate, CORBA::Long clientID,  const char *clTRID , const char* XML ) 
{ 
DB DBsql; 
char  NAME[256] ; // handle na velka pismena 
// char dateStr[MAX_DATE];
ccReg::Response * ret; 
int regID, id, techid, hostID;  
unsigned int  i , j ;
ret = new ccReg::Response;
// default
ret->errCode = 0;
ret->errors.length( 0 );
crDate = CORBA::string_dup( "" );

LOG( NOTICE_LOG, "NSSetCreate: clientID -> %d clTRID [%s] handle [%s]  authInfoPw [%s]", (int ) clientID, clTRID, handle , authInfoPw  );
LOG( NOTICE_LOG, "NSSetCreate: tech check level %d" , (int) level );

if(  ( regID = GetRegistrarID( clientID ) ) )

  if( DBsql.OpenDatabase( database ) )
    {

      if( (  DBsql.BeginAction( clientID, EPP_NSsetCreate,  clTRID  , XML)  ))
        {
 

       if( (  id = DBsql.GetNSSetID( handle ) ) <   0 )  SetReasonNSSetHandle( ret , handle , id , GetRegistrarLang( clientID ) );
        else if( id )
               {
                   LOG( WARNING_LOG  ,  "object [%s] EXIST" ,  handle );
                   ret->errCode= COMMAND_OBJECT_EXIST;
               }

      else    if( DBsql.BeginTransaction() )      // zahaj transakci
        {
 

             // test jestli neni ve smazanych kontaktech
         if( DBsql.TestNSSetHandleHistory( handle ,  DefaultDomainNSSetPeriod()  ) ) SetReasonProtectedPeriod( ret , handle , GetRegistrarLang( clientID ) );
         else 
          {
             // Test tech kontaktu 

                  // test tech kontaktu
                 if(  tech.length() == 0 ) 
                   {
                     ret->errCode = COMMAND_PARAMETR_MISSING ;                                       
                     LOG( WARNING_LOG, "NSSetCreate: not any tech Contact " );
/*  TODO
                      ret->errors.length( seq +1 );
                      ret->errors[seq].code = ccReg::nssetCreate_tech;
                      ret->errors[seq].value = CORBA::string_dup( "tech contact"  ); // TODO ???
                      ret->errors[seq].reason = CORBA::string_dup(  GetReasonMessage( REASON_MSG_NOT_ANY_TECH  , GetRegistrarLang( clientID ) ) );
                      seq++;
*/
                      ret->errCode = COMMAND_PARAMETR_MISSING ; // musi byt alespon jeden nsset;
                   }
                 else
                 {
                 
                    // test tech kontaktu
                     for( i = 0; i <   tech.length() ;  i++ )
                      {
                         if( ( techid = DBsql.GetContactID( tech[i] ) ) <= 0 )   SetReasonNSSetTech( ret , tech[i] , techid ,  GetRegistrarLang( clientID )  ); 
                      }

                  }
         
               LOG( DEBUG_LOG ,  "NSSetCreate:  dns.length %d" , (int ) dns.length() );
             // test DNS hostu
               if(  dns.length() < 2  ) // musi zadat minimalne dva dns hosty
                 {
                
                      if( dns.length() == 1 )
                        {
                          LOG( WARNING_LOG, "NSSetCreate: minimal two dns host create one %s"  , (const char *)  dns[0].fqdn   );    
                          SetErrorReason( ret , COMMAND_PARAMETR_VALUE_POLICY_ERROR , ccReg::nsset_dns_name , REASON_MSG_MIN_TWO_DNS_SERVER , dns[0].fqdn , GetRegistrarLang( clientID ) );
                        }
                      else
                        {
                          LOG( WARNING_LOG, "NSSetCreate: minimal two dns DNS hosts" );
/* TODO
                          ret->errors.length( seq +1 );
                          ret->errors[seq].code = ccReg::nssetCreate_ns_addr;
                          ret->errors[seq].value = CORBA::string_dup( "not any dns host"  ); // TODO VALUE ?? 
                          ret->errors[seq].reason = CORBA::string_dup( GetReasonMessage(  REASON_MSG_MIN_DNS ,  GetRegistrarLang( clientID )) );
                          seq++;
*/
                          ret->errCode = COMMAND_PARAMETR_MISSING;
                        }
 
                }
               else
               {
                  // test dns hostu
                  for( i = 0; i <   dns.length() ; i++ )
                    {
     
                      LOG( DEBUG_LOG , "NSSetCreate: test host %s" ,  (const char *)  dns[i].fqdn  );
                      // preved sequenci adres
                      for( j = 0; j <  dns[i].inet.length(); j++ )
                        {
                          LOG( DEBUG_LOG , "NSSetCreate: test inet[%d] = %s " , j ,   (const char *) dns[i].inet[j]   );
                          if( TestInetAddress( dns[i].inet[j] )  == false )
                            {
                                  LOG( WARNING_LOG, "NSSetCreate: bad host address %s " , (const char *) dns[i].inet[j]  );
                                  SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_addr , REASON_MSG_BAD_IP_ADDRESS, dns[i].inet[j] , GetRegistrarLang( clientID ) );
                            }

                        }



                      // test DNS hostu
                     if( TestDNSHost( dns[i].fqdn ) == false )
                       {

                          LOG( WARNING_LOG, "NSSetCreate: bad host name %s " , (const char *)  dns[i].fqdn  );
                         SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_name , REASON_MSG_BAD_DNS_NAME ,  dns[i].fqdn , GetRegistrarLang( clientID ) );
                       }
                     else
                       {
                          LOG( NOTICE_LOG ,  "NSSetCreate: test DNS Host %s",   (const char *)  dns[i].fqdn       );
                          convert_hostname(  NAME , dns[i].fqdn );

 
                          if( getZone( dns[i].fqdn  ) == 0   && dns[i].inet.length() > 0 ) // neni v definovanych zonach a obsahuje zaznam ip adresy
                          {
                            for( j = 0 ; j <  dns[i].inet.length() ; j ++ )
                               {

                                    LOG( WARNING_LOG, "NSSetCreate:  ipaddr  glue not allowed %s " , (const char *) dns[i].inet[j]   );
                                    SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_addr , REASON_MSG_IP_GLUE_NOT_ALLOWED ,  dns[i].inet[j] ,  GetRegistrarLang( clientID ) );
                                } 
                           }                                                    
                       }  

                   } // konec cyklu

                }
 

            if( ret->errCode == 0 )
            {

              id= DBsql.CreateObject( "N" ,  regID , handle ,  authInfoPw );

              // zapis to tabulky nsset
              DBsql.INSERT( "NSSET" );
              DBsql.INTO( "id" );
              if( level >= 0 ) DBsql.INTO( "checklevel" );
              DBsql.VALUE( id );
              if( level >= 0 ) DBsql.VALUE( level );

              // zapis nejdrive nsset 
              if( ! DBsql.EXEC() ) ret->errCode = COMMAND_FAILED;
              else 
              {

                  // zjisti datum a cas vytvoreni objektu
                  crDate= CORBA::string_dup( DBsql.GetObjectCrDateTime( id )  );

                  // zapis technicke kontakty
                  for( i = 0; i <   tech.length() ;  i++ )
                    {
                      techid = DBsql.GetContactID( tech[i] );
                      LOG( DEBUG_LOG, "NSSetCreate: add tech Contact %s id %d " , (const char *)  tech[i]  , techid);
                      if(  !DBsql.AddContactMap( "nsset" , id  , techid  ) ) { ret->errCode = COMMAND_FAILED; break; }
                    }



                 // zapis DNS hosty


              // zapis do tabulky hostu
                  for( i = 0; i < dns.length() ; i++ )
                    {


                      // preved nazev domeny
                       LOG( NOTICE_LOG ,  "NSSetCreate: DNS Host %s ",   (const char *)  dns[i].fqdn      );
                       convert_hostname(  NAME , dns[i].fqdn );


                          // ID je cislo ze sequence
                          hostID = DBsql.GetSequenceID( "host" );




                          // HOST informace pouze ipaddr a fqdn
                          DBsql.INSERT( "HOST" );
                          DBsql.INTO( "ID" );
                          DBsql.INTO( "NSSETID" );
                          DBsql.INTO( "fqdn" );
                          DBsql.VALUE( hostID );
                          DBsql.VALUE( id );
                          DBsql.VVALUE( NAME );
                          if( DBsql.EXEC()  )
                            {

                              // preved sequenci adres
                              for( j = 0; j <  dns[i].inet.length(); j++ )
                                 {
                                    LOG( NOTICE_LOG ,  "NSSetCreate: IP address hostID  %d [%s] ",  hostID  , (const char *)  dns[i].inet[j]   );

                                   // HOST_IPADDR ulozeni IP adress DNS hostu
                                   DBsql.INSERT( "HOST_IPADDR_map" );
                                   DBsql.INTO( "HOSTID" );
                                   DBsql.INTO( "NSSETID" );
                                   DBsql.INTO( "ipaddr" );
                                   DBsql.VALUE( hostID );
                                   DBsql.VALUE( id ); // nsset id
                                   DBsql.VVALUE( dns[i].inet[j]  );

                                   if( DBsql.EXEC() == false ) {  ret->errCode = COMMAND_FAILED; break ; }

                                }

                            }
                           else {  ret->errCode = COMMAND_FAILED; break; }

                      
                     }   // konec cyklu host 


                  //  uloz do historie POKUD vse OK
                 if(  ret->errCode !=  COMMAND_FAILED  )  
                  if( DBsql.SaveNSSetHistory( id ) )
                         if ( DBsql.SaveObjectCreate( id ) )  ret->errCode = COMMAND_OK; 
                } // konec exec

           

           }


        }
       // konec transakce commit ci rollback
      DBsql.QuitTransaction( ret->errCode );
     }


     // zapis na konec action
     ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
     }

      ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

      DBsql.Disconnect();
    }

if( ret->errCode == 0 ) ServerInternalError();

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
 *              level - tech check level
 *              clientID - id pripojeneho klienta 
 *              clTRID - cislo transakce klienta
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/



ccReg::Response* ccReg_EPP_i::NSSetUpdate(const char* handle , const char* authInfo_chg, 
                                          const ccReg::DNSHost& dns_add, const ccReg::DNSHost& dns_rem,
                                          const ccReg::TechContact& tech_add, const ccReg::TechContact& tech_rem, CORBA::Short level,
                                          CORBA::Long clientID, const char* clTRID , const char* XML )
{
ccReg::Response *ret;
DB DBsql;
char   NAME[256] , REM_NAME[256];
int regID , nssetID ,  techid  , hostID;
unsigned int i , j , k , l ;
int hostNum , techNum;
bool findRem; // test zdali se pridavany host zaroven delete 
ret = new ccReg::Response;

ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "NSSetUpdate: clientID -> %d clTRID [%s] handle [%s] authInfo_chg  [%s] " , (int ) clientID , clTRID , handle  , authInfo_chg);
LOG( NOTICE_LOG,   "NSSetUpdate: tech check level %d" , (int) level );
 
if(  (regID = GetRegistrarID( clientID ) ) ) 

if( DBsql.OpenDatabase( database ) )
  {

  if( (  DBsql.BeginAction( clientID, EPP_NSsetUpdate,  clTRID , XML)  ) )
   {

 
     if( (  nssetID = DBsql.GetNSSetID( handle ) ) <= 0 )  SetReasonNSSetHandle( ret , handle , nssetID , GetRegistrarLang( clientID ) );
     else  if( DBsql.BeginTransaction() )
       {
            // client contaktu 
           if( !DBsql.TestObjectClientID( nssetID , regID  )  )
            {
                LOG( WARNING_LOG, "bad autorization not  client of nsset [%s]", handle );
                ret->errCode = COMMAND_AUTOR_ERROR;     // spatna autorizace
             }
           else
             {

              
               // test  ADD tech kontaktu
               for( i = 0; i < tech_add.length(); i++ )
                  {
                    if( ( techid = DBsql.GetContactID( tech_add[i] ) ) <= 0 )   SetReasonNSSetTechADD( ret , tech_add[i] , techid ,  GetRegistrarLang( clientID )  );
                    else  if(  DBsql.CheckContactMap( "nsset", nssetID , techid ) ) SetReasonNSSetTechExistMap(  ret , tech_add[i]  , GetRegistrarLang( clientID ) );

                    LOG( NOTICE_LOG ,  "ADD  tech  techid ->%d [%s]" ,  techid ,  (const char *) tech_add[i] );
                  }


                // test REM tech kontaktu
                for( i = 0; i < tech_rem.length(); i++ )
                   { 
                     
                    if( ( techid = DBsql.GetContactID( tech_rem[i] ) ) <= 0 ) SetReasonNSSetTechREM( ret , tech_rem[i] , techid ,  GetRegistrarLang( clientID )  );
                    else  if( !DBsql.CheckContactMap( "nsset", nssetID , techid ) ) SetReasonNSSetTechNotExistMap(  ret , tech_rem[i]  , GetRegistrarLang( clientID ) );

                    LOG( NOTICE_LOG ,  "REM  tech  techid ->%d [%s]" ,  techid ,  (const char *) tech_rem[i] );
                     
                   }





                // pridat DNS HOSTY TEST IP adresy a nazvy DNS HOSTU
                for( i = 0; i < dns_add.length(); i++ )
                {

                                                // test DNS hostu
                    if( TestDNSHost( dns_add[i].fqdn  ) == false )
                     {
                          LOG( WARNING_LOG, "NSSetUpdate: bad add host name %s " , (const char *)  dns_add[i].fqdn );
                          SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_name_add , REASON_MSG_BAD_DNS_NAME ,  dns_add[i].fqdn , GetRegistrarLang( clientID ) );
                     }
                    else
                     {
                        LOG( NOTICE_LOG ,  "NSSetUpdate: add dns [%s]" , (const char * ) dns_add[i].fqdn );   
  
                        convert_hostname(  NAME , dns_add[i].fqdn ); // preved na mala pismena
                        // neni v definovanych zonach a obsahuje zaznam ip adresy 
                        if( getZone( dns_add[i].fqdn ) == 0     && (int )  dns_add[i].inet.length() > 0 ) 
                          {
                             for( j = 0 ; j <   dns_add[i].inet.length() ; j ++ )
                                {
                                   LOG( WARNING_LOG, "NSSetUpdate:  ipaddr  glue not allowed %s " , (const char *) dns_add[i].inet[j]   );
                                   SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_addr , REASON_MSG_IP_GLUE_NOT_ALLOWED ,  dns_add[i].inet[j] , GetRegistrarLang( clientID ) );
                                 }
                           }
                         else                                              
                          {
                 
                             if(  DBsql.GetHostID( NAME , nssetID )  ) // uz exstuje nelze pridat
                               {
                                 // TEST jestli nahodou neni DNS host mezi  dns_rem[i].fqdn
                                  findRem=false;
                                  for( k = 0; k < dns_rem.length(); k++ )
                                     {
                                       convert_hostname(  REM_NAME , dns_rem[k].fqdn );
                                       if( strcmp( NAME , REM_NAME ) == 0 ) 
                                         { 
                                           LOG( NOTICE_LOG ,"NSSetUpdate: add HOST %s find remove host %s" , NAME , REM_NAME );
                                           findRem=true; break; 
                                         }                                    
                                     } 

                                  if(  !findRem ) // pokud neni ve vymazavanych dns hostech
                                    {
                                      LOG( WARNING_LOG, "NSSetUpdate:  host name %s exist" , (const char *)  dns_add[i].fqdn );
                                      SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_name_add , REASON_MSG_DNS_NAME_EXIST ,  dns_add[i].fqdn , GetRegistrarLang( clientID ) );
                                    }

                                } 
                                                
                            }

                       }

                        // TEST IP adres 
                        for( j = 0; j <  dns_add[i].inet.length(); j++ )
                          {

                             if( TestInetAddress( dns_add[i].inet[j] ) )  
                               {
                                 for( l = 0 ; l < j ; l ++ ) // test na duplicitu
                                     {
                                       if( strcmp( dns_add[i].inet[l] ,   dns_add[i].inet[j] ) == 0 )
                                         {
                                           LOG( WARNING_LOG, "NSSetUpdate: duplicity host address %s " , (const char *) dns_add[i].inet[j]  );
                                           SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_addr , REASON_MSG_DUPLICITY_DNS_ADDRESS ,  dns_add[i].inet[j] , GetRegistrarLang( clientID ) );
                                         }
                                     }

                               }
                             else // spatna IP adresa
                               {
                                   LOG( WARNING_LOG, "NSSetUpdate: bad add host address %s " , (const char *)  dns_add[i].inet[j] );
                                   SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_addr ,  REASON_MSG_BAD_IP_ADDRESS ,  dns_add[i].inet[j] ,  GetRegistrarLang( clientID ) );
                               }
                           }                                            
   
                                                                                                                                 
                } // konec cyklu 





               // test pro  DNS HOSTY na smazani jestli ej spravny format a jestli existuji
                for( i = 0; i < dns_rem.length(); i++ )
                {
                   LOG( NOTICE_LOG ,  "NSSetUpdate:  delete  host  [%s] " , (const char *)   dns_rem[i].fqdn );
                                               
                    if( TestDNSHost( dns_rem[i].fqdn  ) == false )
                      {
                          LOG( WARNING_LOG, "NSSetUpdate: bad rem host name %s " , (const char *)  dns_rem[i].fqdn );
                          SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_name_rem , REASON_MSG_BAD_DNS_NAME ,  dns_add[i].fqdn , GetRegistrarLang( clientID ) );
                       }
                     else
                       {
                           convert_hostname(  NAME , dns_rem[i].fqdn );
                           if( ( hostID = DBsql.GetHostID( NAME , nssetID )  ) == 0 )
                             {                                                        
                                 LOG( WARNING_LOG, "NSSetUpdate:  host  [%s] not in table" ,  (const char *)   dns_rem[i].fqdn );
                                 SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::nsset_dns_name_rem , REASON_MSG_DNS_NAME_NOTEXIST  ,  dns_rem[i].fqdn , GetRegistrarLang( clientID ) );
                             }
                       }
                  }


                // pokud neni zadna chyba v parametru proved update
               if( ret->errCode == 0  ) 
                 if( DBsql.ObjectUpdate( nssetID , regID  , authInfo_chg )  )
                   {

                     // update tech testu
                     if( level >= 0 )
                       {
                                      LOG( NOTICE_LOG, "update nsset check level %d ", (int ) level  );
                                      DBsql.UPDATE( "nsset" );
                                      DBsql.SET( "checklevel", level );
                                      DBsql.WHERE( "id", nssetID );
                                      if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;
                       }
 
                                            //-------- TECH kontakty

                                            // pridat tech kontakty                      
                                            for( i = 0; i < tech_add.length(); i++ )
                                              {
                                                techid = DBsql.GetContactID( tech_add[i]  );
                                                
                                                LOG( NOTICE_LOG ,  "INSERT add techid ->%d [%s]" ,  techid ,  (const char *) tech_add[i] );
                                                if(  !DBsql.AddContactMap( "nsset" , nssetID  , techid  ) ) { ret->errCode = COMMAND_FAILED; break; } 
                                                                                                
                                              }

                                            // vymaz  tech kontakty
                                            for( i = 0; i < tech_rem.length(); i++ )
                                              {
                                                techid = DBsql.GetContactID( tech_rem[i]   );


                                                   LOG( NOTICE_LOG ,  "DELETE rem techid ->%d [%s]" ,  techid , (const char *) tech_rem[i]  ); 
                                                  if( !DBsql.DeleteFromTableMap( "nsset", nssetID  , techid ) ) { ret->errCode = COMMAND_FAILED;  break; }
                                 
                                              }

                                               //--------- TEST pocet tech kontaktu po ADD a REM
                                               // pouze kdyz se nejake tech kontakty mazzou
                                              if(  tech_rem.length() > 0 )
                                              {
                                                techNum = DBsql.GetNSSetContacts( nssetID );
                                                LOG(NOTICE_LOG, "NSSetUpdate: tech Contact  %d" , techNum );

                                               if( techNum == 0  ) // musi zustat alespon jeden tech kontact po update
                                                 {

                                                    for( i = 0; i < tech_rem.length(); i++ ) // vsecny REM tech kontakty jsou chyba
                                                      {
                                                       SetErrorReason( ret , COMMAND_PARAMETR_VALUE_POLICY_ERROR , ccReg::nsset_tech_rem , REASON_MSG_CAN_NOT_REMOVE_TECH ,   tech_rem[i] , GetRegistrarLang( clientID ) );
                                                      }
                                                 }
                                              }


                                            // smazat DNS HOSTY  PRVNI FAZE
                                            for( i = 0; i < dns_rem.length(); i++ )
                                              {
                                               LOG( NOTICE_LOG ,  "NSSetUpdate:  delete  host  [%s] " , (const char *)   dns_rem[i].fqdn );

                                                      convert_hostname(  NAME , dns_rem[i].fqdn );
                                                      hostID = DBsql.GetHostID( NAME , nssetID );
                                                      LOG( NOTICE_LOG ,  "DELETE  hostID %d" , hostID );
                                                      if( !DBsql.DeleteFromTable("HOST" , "id" ,  hostID  ) )  ret->errCode = COMMAND_FAILED;
                                                       else   if(  !DBsql.DeleteFromTable("HOST_IPADDR_map"  , "hostID" ,  hostID )  )ret->errCode = COMMAND_FAILED;
                                              }


                                             
                                         

                                        //-------- DNS HOSTY pridat DRUHA FAZE

                                          for( i = 0; i < dns_add.length(); i++ )
                                             {

                                             convert_hostname(  NAME , dns_add[i].fqdn ); // preved na mala pismena
                                                    
                                                     // ID je cislo ze sequence
                                                      hostID = DBsql.GetSequenceID( "host" );

                                                         // HOST informace pouze ipaddr a fqdn
                                                         DBsql.INSERT( "HOST" );
                                                         DBsql.INTO( "ID" ); 
                                                         DBsql.INTO( "nssetid" );
                                                         DBsql.INTO( "fqdn" );
                                                         DBsql.VALUE( hostID ); 
                                                         DBsql.VALUE( nssetID ); 
                                                         DBsql.VALUE( NAME );
                                                         if( DBsql.EXEC())  // pridej IP adresey
                                                           {
                                                                // preved sequenci adres
                                                                for( j = 0; j < dns_add[i].inet.length(); j++ )
                                                                     {
                                                                       LOG( NOTICE_LOG ,  "insert  IP address hostID  %d [%s] ",  hostID  , (const char *)  dns_add[i].inet[j]   );

                                                                       // HOST_IPADDR ulozeni IP adress DNS hostu
                                                                       DBsql.INSERT( "HOST_IPADDR_map" );
                                                                       DBsql.INTO( "HOSTID" );
                                                                       DBsql.INTO( "NSSETID" );
                                                                       DBsql.INTO( "ipaddr" );
                                                                       DBsql.VALUE( hostID );
                                                                       DBsql.VALUE( nssetID ); 
                                                                       DBsql.VVALUE( dns_add[i].inet[j]  );

                                                                      if( DBsql.EXEC() == false ) {  ret->errCode = COMMAND_FAILED; break ; }

                                                                      }

                                                           }
                                                          else  { ret->errCode = COMMAND_FAILED; break ; } 
                                                           
                                                             
                                              }
                                                



                                               //------- TEST pocet dns hostu
                                                // pouze kdyz je s nimi manipulovano 
                                               if(   dns_rem.length() > 0 ||  dns_add.length() > 0 )
                                              {
                                                hostNum = DBsql.GetNSSetHosts( nssetID );
                                                LOG(NOTICE_LOG, "NSSetUpdate:  hostNum %d" , hostNum );

                                                if( hostNum <  2 ) // musi minimalne dva DNS hosty zustat
                                                  {
                                                    for( i = 0; i < dns_rem.length(); i++ )
                                                      {
                                                      // vechny REM DNS hosy jsou chyba
                                                       SetErrorReason( ret , COMMAND_PARAMETR_VALUE_POLICY_ERROR , ccReg::nsset_dns_name_rem , REASON_MSG_CAN_NOT_REM_DNS  , dns_rem[i].fqdn , GetRegistrarLang( clientID ) );
                                                      }
                                                  }

                                               if( hostNum >  9 ) // maximalni pocet
                                                  {
                                                    for( i = 0; i < dns_add.length(); i++ )
                                                      {
                                                        // vsechny ADD dns host jsou chyba
                                                       SetErrorReason( ret , COMMAND_PARAMETR_VALUE_POLICY_ERROR , ccReg::nsset_dns_name_add , REASON_MSG_CAN_NOT_ADD_DNS  , dns_add[i].fqdn , GetRegistrarLang( clientID ) );
                                                      }
                                                  }

                                                }


                                                // uloz history na konci update  nejdrive pred update pokud nedoslo k chybe 
                                                if( ret->errCode == 0 )  if( DBsql.SaveNSSetHistory( nssetID ) ) ret->errCode = COMMAND_OK;      // nastavit uspesne jako default
                                       
                             
                      }          
                        

              }                                           
            // konec transakce commit ci rollback
            DBsql.QuitTransaction( ret->errCode );
          }
       // zapis na konec action
        ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) ); 
      }

    ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

    DBsql.Disconnect();
  }

if( ret->errCode == 0 ) ServerInternalError();


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
ccReg::ENUMValidationExtension *enumVal;
ccReg::Response *ret;
char FQDN[64];
// char dateStr[64];
int id , clid , crid ,  upid , regid ,nssetid , regID , zone ;
int i , len ;

d = new ccReg::Domain;
ret = new ccReg::Response;




// default
ret->errCode=0;
ret->errors.length(0);


LOG( NOTICE_LOG ,  "DomainInfo: clientID -> %d clTRID [%s] fqdn  [%s] " , (int ) clientID , clTRID  , fqdn );


d->ext.length(0); // extension

if(  ( regID = GetRegistrarID( clientID ) ) )

if( DBsql.OpenDatabase( database ) )
{

if( (  DBsql.BeginAction( clientID , EPP_DomainInfo , clTRID , XML  ) ) )
 {
 


      // preved fqd na  mala pismena a otestuj to
       // spatny format navu domeny
    if( ( zone = getFQDN( FQDN , fqdn ) ) <= 0 ) SetReasonDomainFQDN( ret , fqdn , zone , GetRegistrarLang( clientID ) );
   else
   if( DBsql.BeginTransaction() )      // zahajeni transakce
    {
   
        if( ( id = DBsql.GetDomainID( FQDN  ,   GetZoneEnum( zone )  ) ) == 0 )
          {
            LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
           ret->errCode = COMMAND_OBJECT_NOT_EXIST;
         }
       else 
//     if(  DBsql.SELECTDOMAIN(  FQDN  , zone , GetZoneEnum( zone ) )  )
     if(  DBsql.SELECTOBJECTID( "DOMAIN" , "fqdn" , id ) ) 
      {

 //        id =  DBsql.GetFieldNumericValueName("id" , 0 );
        clid =  DBsql.GetFieldNumericValueName("ClID" , 0 ); 
        crid =  DBsql.GetFieldNumericValueName("CrID" , 0 ); 
        upid =  DBsql.GetFieldNumericValueName("UpID" , 0 ); 
        regid = DBsql.GetFieldNumericValueName("registrant" , 0 ) ; 
        nssetid =  DBsql.GetFieldNumericValueName("nsset" , 0 ) ;  

        d->CrDate= CORBA::string_dup(  DBsql.GetFieldDateTimeValueName("CrDate" , 0 ) );
        d->UpDate= CORBA::string_dup(  DBsql.GetFieldDateTimeValueName("UpDate" , 0 ) );
        d->TrDate= CORBA::string_dup(  DBsql.GetFieldDateTimeValueName("TrDate" , 0 ) );
        d->ExDate= CORBA::string_dup(  DBsql.GetFieldDateValueName("ExDate" , 0 ) );


	d->ROID=CORBA::string_dup( DBsql.GetFieldValueName("ROID" , 0 )  ); // jmeno nebo nazev kontaktu
	d->name=CORBA::string_dup( DBsql.GetFieldValueName("name" , 0 )  ); // jmeno nebo nazev kontaktu


        if( regID == clid ) // pokud je registrator clientem obdrzi autentifikaci
           d->AuthInfoPw = CORBA::string_dup( DBsql.GetFieldValueName("AuthInfoPw" , 0 ) ); // autentifikace
         else  d->AuthInfoPw = CORBA::string_dup( "" ); // jinak prazdny retezec
 

    
        ret->errCode=COMMAND_OK;

    
        // free select
	DBsql.FreeSelect();

          // status is OK
          d->stat.length(1);
          d->stat[0].value = CORBA::string_dup( "ok" );
          d->stat[0].text = CORBA::string_dup( "Domain is OK" );


         

        d->ClID = CORBA::string_dup( DBsql.GetRegistrarHandle( clid ) );
        d->CrID = CORBA::string_dup( DBsql.GetRegistrarHandle( crid ) );
        d->UpID = CORBA::string_dup( DBsql.GetRegistrarHandle( upid ) );

        // vlastnik domeny
        if(  GetZoneEnum( zone )  )
          {
              if( regID == clid )  d->Registrant=CORBA::string_dup( DBsql.GetObjectName( regid ) );
              else  d->Registrant=CORBA::string_dup( "" ); // skryj vlastnika enum domenu 
          }
         else  d->Registrant=CORBA::string_dup( DBsql.GetObjectName( regid ) );

        //  handle na nsset
        d->nsset=CORBA::string_dup(  DBsql.GetObjectName( nssetid ) );
    

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
                enumVal->valExDate = CORBA::string_dup(  DBsql.GetFieldDateValueName( "ExDate" , 0 ) );
                d->ext.length(1); // preved na  extension
                d->ext[0] <<=  enumVal;
             }

           DBsql.FreeSelect();
         } else ret->errCode=COMMAND_FAILED;
   

     } else ret->errCode=COMMAND_FAILED;

     // konec transakce commit ci rollback
     DBsql.QuitTransaction( ret->errCode );
 

   }

  


   // zapis na konec action
   ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode  ) );

 }
      ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

DBsql.Disconnect();
}


if( ret->errCode == 0 ) ServerInternalError();


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
char FQDN[64];
int regID , id , zone;
ret = new ccReg::Response;


// default
ret->errCode=0;
ret->errors.length(0);

LOG( NOTICE_LOG ,  "DomainDelete: clientID -> %d clTRID [%s] fqdn  [%s] " , (int ) clientID , clTRID  , fqdn );

if(  (regID = GetRegistrarID( clientID ) ) )

  if( DBsql.OpenDatabase( database ) )
    {

      if( (  DBsql.BeginAction( clientID, EPP_DomainDelete,  clTRID  , XML)  ) )
        {

 

      // preved fqd na  mala pismena a otestuj to
       // spatny format navu domeny
       if( ( zone = getFQDN( FQDN , fqdn ) ) <= 0 )    SetReasonDomainFQDN( ret , fqdn , zone , GetRegistrarLang( clientID ) );
        else
         {


           if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
             }
          else  
          if( DBsql.BeginTransaction() )
            {
              if( ( id = DBsql.GetDomainID( FQDN ,   GetZoneEnum( zone )  ) ) == 0 )
                {
                  LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;
                }
              else
                {


                  if(  !DBsql.TestObjectClientID(  id , regID )  )
                    {
                      LOG( WARNING_LOG, "bad autorization not client of fqdn [%s]", fqdn );
                      ret->errCode = COMMAND_AUTOR_ERROR;       // spatna autorizace
                    }
                  else
                    {
                      if(  DBsql.SaveObjectDelete( id  ) ) // uloz do tabulky smazanych objektu
                        {

                           if(  DBsql.DeleteDomainObject( id )  ) ret->errCode = COMMAND_OK; // pokud usmesne smazal
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

      ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

      DBsql.Disconnect();
    }

if( ret->errCode == 0 ) ServerInternalError();

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
 *              clientID - id klienta
 *              clTRID - cislo transakce klienta
 *              ext - ExtensionList
 * 
 * RETURNED:    svTRID a errCode
 *
 ***********************************************************************/
ccReg::Response * ccReg_EPP_i::DomainUpdate( const char *fqdn, const char *registrant_chg, const char *authInfo_chg, const char *nsset_chg,
                                             const ccReg::AdminContact & admin_add, const ccReg::AdminContact & admin_rem,
                                             CORBA::Long clientID, const char *clTRID,  const char* XML ,  const ccReg::ExtensionList & ext )
{
ccReg::Response * ret;
DB DBsql;
char FQDN[64];
char valexpiryDate[MAX_DATE];
int regID = 0, id, nssetid, contactid, adminid;
int   seq , zone;
unsigned int i;

ret = new ccReg::Response;
seq=0;
ret->errCode = 0;
ret->errors.length( 0 );

strcpy( valexpiryDate , "" ); // default

LOG( NOTICE_LOG, "DomainUpdate: clientID -> %d clTRID [%s] fqdn  [%s] , registrant_chg  [%s] authInfo_chg [%s]  nsset_chg [%s] ext.length %ld",
      (int )  clientID, clTRID, fqdn, registrant_chg, authInfo_chg, nsset_chg , (long)ext.length() );


// parse extension
GetValExpDateFromExtension( valexpiryDate , ext );

if(  ( regID = GetRegistrarID( clientID ) ) )

if( DBsql.OpenDatabase( database ) )
  {

   if( (  DBsql.BeginAction( clientID, EPP_DomainUpdate, clTRID , XML )  ) )
     {
 

      // preved fqd na  mala pismena a otestuj to
       if( ( zone = getFQDN( FQDN , fqdn ) ) <= 0 )   SetReasonDomainFQDN( ret , fqdn , zone , GetRegistrarLang( clientID ) );
        else if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
             }
            // pokud domena existuje
             else if( ( id = DBsql.GetDomainID( FQDN ,   GetZoneEnum( zone )  ) ) == 0 )
                {
                  LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
                  ret->errCode = COMMAND_OBJECT_NOT_EXIST;
                }
              // jeslize neni client kontaktu 
              else  if(  !DBsql.TestObjectClientID( id , regID ) )
                      {
                          LOG( WARNING_LOG, "bad autorization not  client of domain [%s]", fqdn );
                          ret->errCode = COMMAND_AUTOR_ERROR;   // spatna autorizace
                       }
                      else if( DBsql.BeginTransaction() )
                           {


                      
                            // test  ADD admin kontaktu
                            for( i = 0; i < admin_add.length(); i++ )
                               {   LOG( NOTICE_LOG , "admin ADD contact %s" , (const char *) admin_add[i] );
                                    if( ( adminid = DBsql.GetContactID( admin_add[i] ) ) <= 0 ) 
                                          SetReasonDomainAdminADD( ret , admin_add[i] , adminid ,  GetRegistrarLang( clientID )  );
                                    else  if(  DBsql.CheckContactMap( "domain", id, adminid ) )
                                              SetReasonDomainAdminExistMap(  ret , admin_add[i]  , GetRegistrarLang( clientID ) );
                               }


                             // test REM admin kontaktu
                            for( i = 0; i < admin_rem.length(); i++ )
                               {
                                  LOG( NOTICE_LOG , "admin REM contact %s" , (const char *) admin_rem[i] );
                                    if( ( adminid = DBsql.GetContactID( admin_rem[i] ) ) <= 0 )
                                          SetReasonDomainAdminREM( ret , admin_rem[i] , adminid ,  GetRegistrarLang( clientID )  );
                                    else  if(  !DBsql.CheckContactMap( "domain", id, adminid ) )
                                              SetReasonDomainAdminNotExistMap(  ret , admin_rem[i]  , GetRegistrarLang( clientID ) );
                                }



                            if( strlen( nsset_chg ) == 0  ) nssetid = 0;    // nemenim nsset;
                            else if( ( nssetid = DBsql.GetNSSetID( nsset_chg ) ) <= 0 )
                                     SetReasonDomainNSSet( ret , nsset_chg , nssetid , GetRegistrarLang( clientID ) );
 



                         //  registrant
                           if( strlen( registrant_chg  ) == 0  )  contactid = 0;  // nemenim vlastnika
                           else  if( (  contactid =   DBsql.GetContactID(  registrant_chg ) ) <= 0 ) 
                                     SetReasonDomainRegistrant( ret , registrant_chg , contactid , GetRegistrarLang( clientID ) );


                          if( strlen( valexpiryDate ) )
                            {
                               // Test u enaum domen
                             if( GetZoneEnum( zone )  )
                               {
                                 if(  DBsql.TestValExDate( valexpiryDate , GetZoneValPeriod( zone ) , DefaultValExpInterval() , id  ) ==  false ) // test validace expirace
                                   {
                                      LOG( WARNING_LOG, "DomainUpdate:  validity exp date is not valid %s" , valexpiryDate  );
                                      SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::domain_ext_valDate , REASON_MSG_VALEXPDATE_NOT_VALID  , valexpiryDate  , GetRegistrarLang( clientID ) );
                                    }
                                 
                               }
                              else
                                {

                                    LOG( WARNING_LOG, "DomainUpdate: can not  validity exp date %s"   , valexpiryDate );
                                    SetErrorReason( ret , COMMAND_PARAMETR_VALUE_POLICY_ERROR , ccReg::domain_ext_valDate ,  REASON_MSG_VALEXPDATE_NOT_USED  , valexpiryDate  , GetRegistrarLang( clientID ) );

                                }            
                             }

               if( ret->errCode == 0 )
                {


                           if( DBsql.ObjectUpdate( id ,  regID  , authInfo_chg )  ) 
                             {

                              if( nssetid || contactid  )  // update domain tabulky pouze kdyz menim registratora ci nsset
                                {                                            
                                      // zmenit zaznam o domene
                                      DBsql.UPDATE( "DOMAIN" );
                                    //  DBsql.SSET( "UpDate", "now" );
                                      //DBsql.SET( "UpID", regID );
                                      if( nssetid )  DBsql.SET( "nsset", nssetid );    // zmena nssetu
                                      if( contactid ) DBsql.SET( "registrant", contactid );     // zmena drzitele domeny
                                     // DBsql.SET( "AuthInfoPw", authInfo_chg  );  // zmena autentifikace
                                      DBsql.WHEREID( id );
                                      if( !DBsql.EXEC() )  ret->errCode = COMMAND_FAILED;
                                }

                                      if( ret->errCode == 0  )
                                      {


                                             // zmena extension
                                             if( GetZoneEnum( zone ) && strlen( valexpiryDate )  > 0  )
                                               {
                                                     LOG( NOTICE_LOG, "change valExpDate %s ", valexpiryDate );
                                                     DBsql.UPDATE( "enumval" );
                                                     DBsql.SET( "ExDate", valexpiryDate );
                                                     DBsql.WHERE( "domainID", id );
 
                                                     if( !DBsql.EXEC() )  ret->errCode = COMMAND_FAILED; 
                                                }
                                            


                                              // pridat admin kontakty                      
                                              for( i = 0; i < admin_add.length(); i++ )
                                                {

                                                    adminid  = DBsql.GetContactID( admin_add[i] );
                                                    LOG( DEBUG_LOG, "DomainUpdate: add admin Contact %s id %d " , (const char *)   admin_add[i] , adminid );
                                                   if(  !DBsql.AddContactMap( "domain" , id  , adminid  ) ) { ret->errCode = COMMAND_FAILED; break; }

                                                 }
                                                

                                              // vymaz  admin kontakty
                                              for( i = 0; i < admin_rem.length(); i++ )
                                                {
                                                  if( ( adminid = DBsql.GetContactID( admin_rem[i] ) ) )
                                                    {
                                                      LOG( NOTICE_LOG ,  "delete admin  -> %d [%s]" ,  adminid , (const char * ) admin_rem[i]  ); 
                                                      if( !DBsql.DeleteFromTableMap( "domain", id, adminid ) )  { ret->errCode = COMMAND_FAILED; break; }
                                                    }

                                                 }

                                                  
                                 // uloz history nejdrive no konci update pokud nedoslo k chybe
                                     if( ret->errCode == 0 )   if( DBsql.SaveDomainHistory( id ) )   ret->errCode = COMMAND_OK;    // nastavit uspesne
                                       

                                      }


                            }
                        }
                    
               // konec transakce commit ci rollback
                DBsql.QuitTransaction( ret->errCode );
               }
                
            
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );                                        
        }

      ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

      DBsql.Disconnect();
    }

if( ret->errCode == 0 ) ServerInternalError();

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

ccReg::Response * ccReg_EPP_i::DomainCreate( const char *fqdn, const char *Registrant, const char *nsset, const char *AuthInfoPw,  const ccReg::Period_str& period,
                                             const ccReg::AdminContact & admin, ccReg::timestamp_out crDate, ccReg::timestamp_out  exDate, 
                                             CORBA::Long clientID, const char *clTRID,  const  char* XML , const ccReg::ExtensionList & ext )
{
DB DBsql;
char valexpiryDate[MAX_DATE];
char  FQDN[64];
ccReg::Response * ret;
int contactid, regID, nssetid, adminid, id;
int   zone ;
unsigned int i;
int period_count;
char periodStr[10];
ret = new ccReg::Response;

// default
strcpy( valexpiryDate , "" );

// default
ret->errCode = 0;
ret->errors.length( 0 );
crDate =  CORBA::string_dup( "" );
exDate =  CORBA::string_dup( "" );



LOG( NOTICE_LOG, "DomainCreate: clientID -> %d clTRID [%s] fqdn  [%s] ", (int )  clientID, clTRID, fqdn );
LOG( NOTICE_LOG, "DomainCreate:  Registrant  [%s]  nsset [%s]  AuthInfoPw [%s]", Registrant, nsset, AuthInfoPw);



// prepocet periody
if( period.unit == ccReg::unit_year ) {  period_count = period.count * 12; sprintf(  periodStr , "y%d" , period.count ); }
else if( period.unit == ccReg::unit_month ) {  period_count = period.count; sprintf(  periodStr , "m%d" , period.count ); }
     else period_count = 0;


LOG( NOTICE_LOG, "DomainCreate: period count %d unit %d period_count %d string [%s]" ,   period.count , period.unit ,  period_count  , periodStr);


// parse extension
GetValExpDateFromExtension( valexpiryDate , ext );

if(  ( regID = GetRegistrarID( clientID ) ) )

if( DBsql.OpenDatabase( database ) )
{

 if( (  DBsql.BeginAction( clientID, EPP_DomainCreate, clTRID , XML )  ))
   {

 
       // preved fqd na  mala pismena a otestuj to
    if(  ( zone = getFQDN( FQDN , fqdn ) ) <= 0  )  SetReasonDomainFQDN( ret , fqdn , zone , GetRegistrarLang( clientID ) );
    else 
     if( DBsql.BeginTransaction() )
      {
        if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
              }
             else if(  DBsql.GetDomainID( FQDN  ,   GetZoneEnum( zone )  )    ) // jestli existuje konrola pres id
                  {
                   LOG( WARNING_LOG, "domain  [%s] EXIST", fqdn );
                   ret->errCode = COMMAND_OBJECT_EXIST;      // je uz zalozena
                  }
                 else if( DBsql.TestDomainFQDNHistory( FQDN , DefaultDomainFQDNPeriod() ) )  SetReasonProtectedPeriod( ret , fqdn , GetRegistrarLang( clientID ));
                     else
                      {

    
                      if( strlen( nsset) == 0 ) nssetid = 0; // lze vytvorit domenu bez nssetu
                      else  if( ( nssetid = DBsql.GetNSSetID(  nsset ) ) <= 0  ) SetReasonDomainNSSet( ret , nsset , nssetid , GetRegistrarLang( clientID ) );
                    
                  

              
                       //  registrant              
                      if( (  contactid =   DBsql.GetContactID( Registrant ) ) <= 0 )
                               SetReasonDomainRegistrant( ret , Registrant , contactid , GetRegistrarLang( clientID ) );



             // nastaveni defaultni periody
             if( period_count == 0 )
               {
                 period_count = GetZoneExPeriodMin( zone );
                 LOG( NOTICE_LOG, "get defualt peridod %d month  for zone   %d ", period_count , zone  );
               }


            //  test periody
             switch(  TestPeriodyInterval( period_count  ,  GetZoneExPeriodMin( zone )  ,  GetZoneExPeriodMax( zone )  )   )
              {
               case 2:
                  LOG( WARNING_LOG, "period %d interval ot of range MAX %d MIN %d"  , period_count ,  GetZoneExPeriodMax( zone )   , GetZoneExPeriodMin( zone )  );
                  SetErrorReason( ret , COMMAND_PARAMETR_RANGE_ERROR  , ccReg::domain_period , REASON_MSG_PERIOD_RANGE ,  periodStr , GetRegistrarLang( clientID ) );
                  break;
               case 1:
                  LOG( WARNING_LOG, "period %d  interval policy error MIN %d" , period_count  ,  GetZoneExPeriodMin( zone )   );
                  SetErrorReason( ret , COMMAND_PARAMETR_VALUE_POLICY_ERROR  , ccReg::domain_period , REASON_MSG_PERIOD_POLICY ,  periodStr , GetRegistrarLang( clientID ) );
                  break;

              }


                // test datumu validace expirace
                if( strlen( valexpiryDate ) == 0 )
                  {
                    // pro enum domeny je pozadovane toto tadum
                    if( GetZoneEnum( zone ) )
                      {
                           LOG( WARNING_LOG, "DomainCreate: validity exp date MISSING"  );
                           ret->errCode = COMMAND_PARAMETR_MISSING;   
                           /* TODO
                  LOG( WARNING_LOG, "DomainCreate: not validity exp date "  );
                  ret->errors.length( seq +1 );
                  ret->errors[seq].code = ccReg::domain_ext_valDate;

                  ret->errors[seq].value = CORBA::string_dup( "not valExpDate"  ); // ??? TODO
                  ret->errors[seq].reason = CORBA::string_dup( GetReasonMessage(REASON_MSG_VALEXPDATE_REQUIRED  , GetRegistrarLang( clientID ) ) ); // TODO

                           */
                      }
                  }
                 else
                  {
                              // Test u enaum domen
                             if( GetZoneEnum( zone )  )
                               {
                                  // test bez ID domeny
                                 if(  DBsql.TestValExDate( valexpiryDate , GetZoneValPeriod( zone ) , DefaultValExpInterval() , 0  ) ==  false ) // test validace expirace
                                   {
                                      LOG( WARNING_LOG, "Validity exp date is not valid %s" , valexpiryDate  );
                                      SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::domain_ext_valDate , REASON_MSG_VALEXPDATE_NOT_VALID  , valexpiryDate  , GetRegistrarLang( clientID ) );
                                    }
                                 
                               }
                              else
                                {
                                    LOG( WARNING_LOG, "Validity exp date %s not user"   , valexpiryDate );
                                    SetErrorReason( ret , COMMAND_PARAMETR_VALUE_POLICY_ERROR , ccReg::domain_ext_valDate ,  REASON_MSG_VALEXPDATE_NOT_USED  , valexpiryDate  , GetRegistrarLang( clientID ) );

                                }
             
                  }


                 
                            // test  ADD admin kontaktu
                            for( i = 0; i < admin.length(); i++ )
                               {
                                    if( ( adminid = DBsql.GetContactID( admin[i] ) ) <= 0 )
                                            SetReasonDomainAdmin( ret , admin[i] , adminid ,  GetRegistrarLang( clientID )  );
                               }




             if(  ret->errCode == 0  ) // pokud nedoslo k chybe
               {

                        id= DBsql.CreateObject( "D" ,  regID , FQDN ,  AuthInfoPw );



                          DBsql.INSERT( "DOMAIN" );
                          DBsql.INTO( "id" );
                          DBsql.INTO( "zone" );
                          DBsql.INTO( "Exdate" );
                          DBsql.INTO( "Registrant" );
                          DBsql.INTO( "nsset" );
                                                                  
                                                                  
                          DBsql.VALUE( id );
                          DBsql.VALUE( zone );
                          DBsql.VALUEPERIOD(  period_count  ); // aktualni cas  plus interval period v mesicich
                          DBsql.VALUE( contactid );
                          if( nssetid == 0 )   DBsql.VALUENULL(); // domena bez nssetu zapsano NULL
                          else DBsql.VALUE( nssetid );

                          // pokud se insertovalo do tabulky
                          if( DBsql.EXEC() )
                            {

                                // zjisti datum a cas vytvoreni domeny
                                crDate= CORBA::string_dup(  DBsql.GetObjectCrDateTime( id )  );

                                //  vrat datum expirace jako lokalni datum 
                                exDate =  CORBA::string_dup(  DBsql.GetDomainExDate(id)  );


                                  
                              // pridej enum  extension
                              if( GetZoneEnum( zone ) && strlen( valexpiryDate) > 0  )
                                {
                                         DBsql.INSERT( "enumval" );
                                         DBsql.VALUE( id );
                                         DBsql.VALUE( valexpiryDate ); 
                                         if( DBsql.EXEC() == false ) ret->errCode = COMMAND_FAILED;
                                 }

                                 // uloz  admin kontakty
                                  for( i = 0; i < admin.length(); i++ )
                                    {
                                           adminid  = DBsql.GetContactID( admin[i] );
                                           LOG( DEBUG_LOG, "DomainCreate: add admin Contact %s id %d " , (const char *)   admin[i] , adminid );
                                          if(  !DBsql.AddContactMap( "domain" , id  , adminid  ) ) { ret->errCode = COMMAND_FAILED; break; }
 
                                     }

         
                             // zpracovani creditu a ulozeni polozky na fakturu
                              // nejprve oprerace registrace
                             if( DBsql.BillingCreateDomain( regID ,  zone ,  id  )  == false ) ret->errCode =  COMMAND_BILLING_FAILURE;
                             else                                
                                 // nasledne operace prodlouzeni domeny
                                    if( DBsql.BillingRenewDomain( regID ,  zone , id ,  period_count ,  exDate ) == false ) ret->errCode =  COMMAND_BILLING_FAILURE;
                                     else 
                                          if(  DBsql.SaveDomainHistory( id ) )
                                               if ( DBsql.SaveObjectCreate( id ) ) ret->errCode = COMMAND_OK; // pokud
                             

                            
                             } else ret->errCode = COMMAND_FAILED; // end exec

                      }                         // ret



         }              
              // pokud nebyla chyba pri insertovani do tabulky domain_contact_map
              // konec transakce commit ci rollback
         DBsql.QuitTransaction( ret->errCode );
       }

          // zapis na konec action
       ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
     }


 ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

 DBsql.Disconnect();
}

if( ret->errCode == 0 ) ServerInternalError();


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


ccReg::Response * ccReg_EPP_i::DomainRenew( const char *fqdn, const char* curExpDate,  const ccReg::Period_str& period, 
                                            ccReg::timestamp_out exDate, CORBA::Long clientID,
                                            const char *clTRID, const  char* XML , const ccReg::ExtensionList & ext )
{
  DB DBsql;
  char   valexpiryDate[MAX_DATE] ;
  char FQDN[64]; 
  ccReg::Response * ret;
  int  regID, id,  zone ;
int period_count;
char periodStr[10];

  ret = new ccReg::Response;

 
// aktualni cas

// default
  exDate =  CORBA::string_dup( "" );

   strcpy( valexpiryDate , "" );
// default
  ret->errCode = 0;
  ret->errors.length( 0 );


  LOG( NOTICE_LOG, "DomainRenew: clientID -> %d clTRID [%s] fqdn  [%s] curExpDate [%s]", (int ) clientID, clTRID, fqdn , (const char *) curExpDate ) ;




// prepocet periody
if( period.unit == ccReg::unit_year ) {  period_count = period.count * 12; sprintf(  periodStr , "y%d" , period.count ); }
else if( period.unit == ccReg::unit_month ) { period_count = period.count; sprintf(  periodStr , "m%d" , period.count ); }
     else period_count = 0;


LOG( NOTICE_LOG, "DomainRenew: period count %d unit %d period_count %d string [%s]" ,   period.count , period.unit ,  period_count  , periodStr);




// parse extension
GetValExpDateFromExtension( valexpiryDate , ext );
 

if(  ( regID = GetRegistrarID( clientID ) ) )

  if( DBsql.OpenDatabase( database ) )
    {


      if( ( DBsql.BeginAction( clientID, EPP_DomainRenew,  clTRID , XML )  ) )
        {


 

      // preved fqd na  mala pismena a otestuj to
        if(  ( zone = getFQDN( FQDN , fqdn ) ) <= 0  )   SetReasonDomainFQDN( ret , fqdn , zone , GetRegistrarLang( clientID ) );
        else
          // zahaj transakci
        if( DBsql.BeginTransaction() )
         {

          if(  DBsql.TestRegistrarZone( regID , zone ) == false )
             {
               LOG( WARNING_LOG, "Authentication error to zone: %d " , zone );
               ret->errCode =  COMMAND_AUTHENTICATION_ERROR;
             }
           else
          
              if( ( id = DBsql.GetDomainID( FQDN  ,  GetZoneEnum( zone ) ) ) == 0 )
              // prvni test zdali domena  neexistuje 
               {
                 ret->errCode = COMMAND_OBJECT_NOT_EXIST;  // domena neexistujea
                 LOG( WARNING_LOG, "domain  [%s] NOT_EXIST", fqdn );
               }
             else
             {
          
              // test ExDate
              if( TestExDate(  curExpDate  , DBsql.GetDomainExDate(id)  )  == false )
                {
                  LOG( WARNING_LOG, "curExpDate is not same as ExDate" );
                  SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::domain_curExpDate , REASON_MSG_CUREXPDATE_NOT_EXPDATE ,  curExpDate , GetRegistrarLang( clientID ) );
                }
  
             // nastaveni defaultni periody           
             if( period_count == 0 ) 
               {
                 period_count = GetZoneExPeriodMin( zone );
                 LOG( NOTICE_LOG, "get defualt peridod %d month  for zone   %d ", period_count , zone  );
                }
              
               
                  
            // vypocet periody
             switch(  TestPeriodyInterval( period_count  ,  GetZoneExPeriodMin( zone )  ,  GetZoneExPeriodMax( zone )  )   )
              {
               case 2:
                  LOG( WARNING_LOG, "period %d interval ot of range MAX %d MIN %d"  , period_count ,  GetZoneExPeriodMax( zone )   , GetZoneExPeriodMin( zone )  );
                  SetErrorReason( ret , COMMAND_PARAMETR_RANGE_ERROR  , ccReg::domain_period , REASON_MSG_PERIOD_RANGE ,  periodStr , GetRegistrarLang( clientID ) );
                  break;
               case 1:
                  LOG( WARNING_LOG, "period %d  interval policy error MIN %d" , period_count  ,  GetZoneExPeriodMin( zone )   );
                  SetErrorReason( ret , COMMAND_PARAMETR_VALUE_POLICY_ERROR  , ccReg::domain_period , REASON_MSG_PERIOD_POLICY ,  periodStr , GetRegistrarLang( clientID ) );
                  break;
               default:
                       // vypocet ExDate datum expirace
                       if( DBsql.CountExDate( id ,  period_count  ,  GetZoneExPeriodMax( zone ) ) == false )
                         {
                             LOG( WARNING_LOG, "period %d ExDate out of range" , period_count );
                             SetErrorReason( ret , COMMAND_PARAMETR_RANGE_ERROR , ccReg::domain_period , REASON_MSG_PERIOD_RANGE ,  periodStr , GetRegistrarLang( clientID ) );
                          }
                  break; 

              }

             // test validate
                          if( strlen( valexpiryDate ) )
                            {
                               // Test u enaum domen
                             if( GetZoneEnum( zone )  )
                               {
                                 if(  DBsql.TestValExDate( valexpiryDate , GetZoneValPeriod( zone ) , DefaultValExpInterval() , id  ) ==  false ) // test validace expirace
                                   {
                                      LOG( WARNING_LOG, "Validity exp date is not valid %s" , valexpiryDate  );
                                      SetErrorReason( ret , COMMAND_PARAMETR_ERROR , ccReg::domain_ext_valDate , REASON_MSG_VALEXPDATE_NOT_VALID  , valexpiryDate  , GetRegistrarLang( clientID ) );
                                    }
                                 
                               }
                              else
                                {

                                    LOG( WARNING_LOG, "Can not  validity exp date %s"   , valexpiryDate );
                                    SetErrorReason( ret , COMMAND_PARAMETR_VALUE_POLICY_ERROR , ccReg::domain_ext_valDate ,  REASON_MSG_VALEXPDATE_NOT_USED  , valexpiryDate  , GetRegistrarLang( clientID ) );

                                }            
                             }

 



               if(  ret->errCode == 0 )
                 {
                  // zpracuj  pole statusu
 // status je OK


                  if(  !DBsql.TestObjectClientID( id ,  regID  ) )
                    {
                      LOG( WARNING_LOG, "bad autorization not  client of domain [%s]", fqdn );
                      ret->errCode = COMMAND_AUTOR_ERROR;       // spatna autorizace
                    }
                  else
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

                                     // prodlouzeni platnosti domeny
                                     if( DBsql.RenewExDate( id , period_count ) ) 
                                       {
                                           //  vrat datum expirace jako lokalni datum
                                           exDate =  CORBA::string_dup(  DBsql.GetDomainExDate(id)  );

                                     // zuctovani operace prodlouzeni domeny
                                    if( DBsql.BillingRenewDomain( regID ,  zone , id ,  period_count ,  exDate ) == false )
                                     ret->errCode =  COMMAND_BILLING_FAILURE;
                                     else  if( DBsql.SaveDomainHistory( id ) )  ret->errCode = COMMAND_OK; 
                                            
                                       }
                                     else ret->errCode = COMMAND_FAILED;
                                  }

                                
                           
                        }

                    }
                }


            


              // konec transakce commit ci rollback
              DBsql.QuitTransaction( ret->errCode );

          }
          // zapis na konec action
          ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) );
        }

      ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

      DBsql.Disconnect();
    }


if( ret->errCode == 0 ) ServerInternalError();


return ret;
}





// primitivni vypis
ccReg::Response*  ccReg_EPP_i::FullList(short act , const char *table , char *fname  ,  ccReg::Lists_out  list ,   CORBA::Long clientID, const char* clTRID, const char* XML )
{
DB DBsql;
int rows =0, i;
ccReg::Response * ret;
int regID;
int typ;
char sqlString[128];

ret = new ccReg::Response;

// default
ret->errors.length( 0 );
ret->errCode =0 ; // default

list = new ccReg::Lists;

LOG( NOTICE_LOG ,  "LIST %d  clientID -> %d clTRID [%s] " , act  , (int )  clientID , clTRID );

if(  ( regID = GetRegistrarID( clientID ) ) )

if( DBsql.OpenDatabase( database ) )
{

  if( (   DBsql.BeginAction( clientID , act ,  clTRID , XML  )  ) )
  {

  // definovani dles typu
  switch( act )
        {
          case EPP_ListContact:
               typ=1;          
               break;
          case EPP_ListNSset:
               typ=2;          
               break;
          case EPP_ListDomain:
               typ=3;          
               break;
          default:
               typ=0;
         }



    // TODO LIST podle clienat OBJEKTU predelat na Object_registry 
   sprintf( sqlString , 
     "SELECT obr.name FROM  object_registry obr, object o "
     "WHERE obr.id=o.id AND o.clid=%d AND obr.type=%d",regID,typ 
   );


   if( DBsql.ExecSelect( sqlString) )
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

      ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

DBsql.Disconnect();
}


if( ret->errCode == 0 ) ServerInternalError();


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

ccReg::Response* ccReg_EPP_i::nssetTest(const char* handle, const char* fqdn, CORBA::Long clientID, const char* clTRID, const char* XML)
{
DB DBsql;
ccReg::Response * ret = new ccReg::Response;
int regID;
int nssetid;

LOG( NOTICE_LOG ,  "nssetTest nsset %s domain %s  clientID -> %d clTRID [%s] \n" , handle,  fqdn, (int )  clientID  , clTRID );

if( ( regID = GetRegistrarID( clientID ) ) )
if( DBsql.OpenDatabase( database ) )
{

  if( (   DBsql.BeginAction( clientID , EPP_NSsetTest ,  clTRID , XML  )  ) )
    {
      if( (nssetid = DBsql.GetNSSetID( handle ) ) <= 0  )  SetReasonNSSetHandle( ret , handle , nssetid , GetRegistrarLang( clientID ) );
      else {
        std::stringstream strid;
        strid << regID;
        std::string regHandle =
          DBsql.GetValueFromTable(
            "registrar","handle","id",strid.str().c_str()
          );
        ret->errCode=COMMAND_OK;
        TechCheckManager tc(ns);
        try {
          tc.checkFromRegistrar(regHandle,handle,fqdn);
        }
        catch (TechCheckManager::INTERNAL_ERROR) {
          LOG(ERROR_LOG,"Tech check internal error nsset [%s] fqdn  [%s] clientID -> %d clTRID [%s] " , handle  , fqdn ,  (int )  clientID , clTRID );          
        }
        catch (TechCheckManager::REGISTRAR_NOT_FOUND) {
          LOG(ERROR_LOG,"Tech check reg not found nsset [%s] fqdn [%s] clientID -> %d clTRID [%s] " , handle  , fqdn ,  (int )  clientID , clTRID );
        }
        catch (TechCheckManager::NSSET_NOT_FOUND) {
          LOG(ERROR_LOG,"Tech check nsset not found nset [%s] fqdn [%s] clientID -> %d clTRID [%s] " , handle  , fqdn ,  (int )  clientID , clTRID );          
        }
      }
      // zapis na konec action
      ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) ) ;
    }

ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

DBsql.Disconnect();
}


return ret;
}


ccReg::Response*  ccReg_EPP_i::ObjectSendAuthInfo( short act , char * table , char *fname , const char *name , CORBA::Long clientID, const char* clTRID , const char* XML )
{
DB DBsql;
int   id ,  zone;
ccReg::Response * ret;
char FQDN[64];
int regID;
ret = new ccReg::Response;

// default
ret->errors.length( 0 );
ret->errCode =0 ; // default
// autInfo[0] = 0 ;

LOG( NOTICE_LOG ,  "ObjectSendAuthInfo type %d  object [%s]  clientID -> %d clTRID [%s] " , act  , name ,  (int )  clientID , clTRID );

if( ( regID = GetRegistrarID( clientID ) ) )

if( DBsql.OpenDatabase( database ) )
{

  if( (   DBsql.BeginAction( clientID , act ,  clTRID , XML  )  ) )
  {


   switch( act )
   {
   case EPP_ContactSendAuthInfo:
       if( (id = DBsql.GetContactID( name ) ) <= 0 ) SetReasonContactHandle( ret , name , id , GetRegistrarLang( clientID ) );
       break;

   case EPP_NSSetSendAuthInfo:
       if( (id = DBsql.GetNSSetID( name ) ) <= 0 ) SetReasonNSSetHandle( ret , name , id , GetRegistrarLang( clientID ) );
       break;
   case EPP_DomainSendAuthInfo:
      // preved fqd na  mala pismena a otestuj to
       if(  ( zone = getFQDN( FQDN , name ) ) <= 0  )  SetReasonDomainFQDN( ret , name , zone , GetRegistrarLang( clientID ) );
       else if( ( id=DBsql.GetDomainID( FQDN ,GetZoneEnum( zone ) )  ) == 0 )   ret->errCode = COMMAND_OBJECT_NOT_EXIST; ; // ziskej ID domeny  pokue existuje
       break;

    }
 
     if(  ret->errCode == 0  )
       {
  
               std::auto_ptr<Register::AuthInfoRequest::Manager> airm(
                 Register::AuthInfoRequest::Manager::create(&DBsql,mm)
               );

               try {
                 LOG(  NOTICE_LOG , "createRequest objectID %d actionID %d" , 
                            id,DBsql.GetActionID() );
                 airm->createRequest(id,Register::AuthInfoRequest::RT_EPP, 
                                                     DBsql.GetActionID(),"","");
                  ret->errCode=COMMAND_OK;
                } catch (...) {
                  LOG( WARNING_LOG, "cannot create and process request");
                  ret->errCode=COMMAND_FAILED;
                }
             
      } 
      // zapis na konec action
      ret->svTRID = CORBA::string_dup( DBsql.EndAction( ret->errCode ) ) ;
  }

ret->errMsg =CORBA::string_dup( GetErrorMessage(  ret->errCode  , GetRegistrarLang( clientID ) )  );

DBsql.Disconnect();
}


if( ret->errCode == 0 ) ServerInternalError();

return ret;
}


ccReg::Response* ccReg_EPP_i::domainSendAuthInfo(const char*  fqdn, CORBA::Long clientID, const char* clTRID, const char*  XML)
{
return   ObjectSendAuthInfo(  EPP_DomainSendAuthInfo , "DOMAIN" ,  "fqdn" , fqdn , clientID , clTRID, XML);
}
ccReg::Response* ccReg_EPP_i::contactSendAuthInfo(const char* handle,  CORBA::Long clientID, const char* clTRID, const char*  XML)
{
return   ObjectSendAuthInfo(  EPP_ContactSendAuthInfo , "CONTACT" ,  "handle" , handle , clientID , clTRID, XML);
}
ccReg::Response* ccReg_EPP_i::nssetSendAuthInfo(const char* handle,  CORBA::Long clientID, const char* clTRID, const char*  XML)
{
return   ObjectSendAuthInfo(  EPP_NSSetSendAuthInfo , "NSSET" ,  "handle" , handle , clientID , clTRID, XML);
}

