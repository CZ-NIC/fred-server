#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sstream>

#include "dbsql.h"

#include "util.h"
#include "action.h"

#include "status.h"

#include "log.h"

// constructor 
DB::DB()
{ 
// nastav memory buffry
svrTRID = NULL;
memHandle=NULL;  
actionID = 0 ;
loginID = 0;
}

// uvolni memory buffry
DB::~DB()
{
if( svrTRID ) 
  {
     LOG( NOTICE_LOG , "delete svrTRID"); 
     delete[] svrTRID; 
   }

if( memHandle ) 
  { 
     LOG( NOTICE_LOG , "delete memHandle");
     delete[] memHandle;
  }

}

// zpracovani creditu
bool DB::UpdateCredit( int registrarID , int objectID ,   int zone ,  int period , int operation  )
{
char sqlString[256];

sprintf( sqlString , "INSERT INTO CREDIT (  registrarID , objectID ,  date , operationID , period ) VALUES ( %d , %d ,  'now' , %d ,  %d );" ,  registrarID  ,  objectID , operation ,  period );

// TODO odecteni kreditu
if( ExecSQL( sqlString ) ) return true;      
else return false;
}


// action
// zpracovani action
bool  DB::BeginAction(int clientID , int action ,const char *clTRID  , const char *xml  )
{
char sqlString[1024];

if( clientID == 0 ) { 
// POUZE info funkce pro PIF
if( action == EPP_ContactInfo ||  action == EPP_NSsetInfo ||   action ==  EPP_DomainInfo ) 
  { actionID = 0 ; loginID =  0 ;  return true;  }
else return false;
}
else
{
// actionID pro logovani
actionID = GetSequenceID("action");
loginID = clientID; // id klienta

// pokud neni vyplneni clientID vubec se do tabulky neuvadi, zustane tam
// defaultni hodnota NULL
std::ostringstream clientIdStr;
const char *clientIdColumn = "";
if (clientID) {
	clientIdStr << ", " << clientID;
	clientIdColumn = ", clientid";
}

if( actionID ) 
  {
  // zapis do action tabulky
   sprintf(
     sqlString , 
     "INSERT INTO ACTION ( id %s , action ,  clienttrid  ) "
     "VALUES ( %d %s, %d , \'%s\' );" ,
     clientIdColumn, actionID , clientIdStr.str().c_str()  , action  ,(char *)  clTRID
   );

   if( ExecSQL( sqlString ) ) 
     {
        if( strlen( xml ) )
           {
              LOG( DEBUG_LOG , "XML:[%s]" , xml );           
                INSERT("Action_XML");
                VALUE( actionID );
                VALUESC( xml );
                return  EXEC();
           }
        else return true;
     }
   else return false;
  }
else return false;
}

}


char * DB:: EndAction(int response )
{
char sqlString[512];
// char svrTRID[32];
int id;

if( actionID == 0 ) return "no action";
else
{
if( svrTRID == NULL ) 
 {
  LOG( SQL_LOG , "alloc svrTRID");
  svrTRID= new char[32] ; 
 }

// cislo transakce co vraci server
sprintf( svrTRID , "ccReg-%010d" , actionID );

sprintf( sqlString , "UPDATE Action SET response=%d , enddate='now()' , servertrid=\'%s\' WHERE id=%d;" ,  response  , svrTRID , actionID );

// update tabulky
id  =  actionID;
actionID = 0 ; 
LOG( SQL_LOG ,  "EndAction svrTRID: %s" ,  svrTRID );

if( ExecSQL(  sqlString ) ) return   svrTRID;
/* {

   sprintf( sqlString , "SELECT  servertrid FROM Action  WHERE id=%d;"  , id );
   if( ExecSelect( sqlString ) )
    {
      svr =  GetFieldValue( 0 , 0 );
      LOG( SQL_LOG , "GetsvrTRID %s" , svr );   
     //  FreeSelect(); 
    }
  return svr;
 }
*/

else return "svrTRID ERROR";
}
}


// vraci chybovou zpravu podle jazyka
char * DB::GetErrorMessage(int err )
{

if( GetClientLanguage() == LANG_CS ) return GetErrorMessageCS( err );
else return GetErrorMessageEN( err );

}

// vraci jazyk klienta
int DB::GetClientLanguage()
{
int lang = LANG_EN;
char sqlString[128];

sprintf( sqlString , "SELECT  lang  FROM  login  WHERE id=%d;" , loginID );

if( ExecSelect( sqlString ) )
 {
     if(  strcmp(   GetFieldValue( 0 , 0 ) , "cs" ) == 0 ) lang =LANG_CS;
     FreeSelect();
 }

return lang;
}

// zjistuje pocet hostu pro dany nsset
int DB::GetNSSetNum( int nssetID )
{
char sqlString[128];
int num=0;

sprintf( sqlString , "SELECT id FROM host  WHERE nssetID=%d;" , nssetID );

if( ExecSelect( sqlString ) )
 {
     num =  GetSelectRows();
     LOG( SQL_LOG , "nsset %d num %d" , nssetID , num ); 
     FreeSelect();
 }

return num;
}

/*
char *  DB::GetStatusString( int status )
{

char sqlString[128];
char *str;

str = new char[128];

sprintf( sqlString , "SELECT  status  FROM enum_status  WHERE id=%d;" , status );

if( ExecSelect( sqlString ) )
 {
      strcpy( str ,  GetFieldValue( 0 , 0 ) ) ;
      LOG( SQL_LOG , "GetStatustring \'%s\'  ->  %d" ,  str , status );
      FreeSelect(); 
      return str;
 }
else return "{ }" ; // prazdny status string pole
}


switch( status )
{
   case STATUS_ok:
           return "ok";
   case STATUS_inactive:
           return "inactive";
   case STATUS_linked:
           return "linked";
   case STATUS_clientDeleteProhibited:
           return "clientDeleteProhibited";
   case STATUS_serverDeleteProhibited:
           return "serverDeleteProhibited";
   case STATUS_clientHold:
           return "clientHold";
   case STATUS_serverHold:
           return "serverHold";
   case STATUS_clientRenewProhibited:
           return "clientRenewProhibited";
   case STATUS_serverRenewProhibited:
           return "serverRenewProhibited";
   case STATUS_clientTransferProhibited:
           return "clientTransferProhibited";
   case STATUS_serverTransferProhibited:
           return "serverTransferProhibited";
   case STATUS_clientUpdateProhibited:
           return "clientUpdateProhibited";
   case STATUS_serverUpdateProhibited:
           return "serverUpdateProhibited";
   default:
           return "";
}         

return "";
}


int  DB::GetStatusNumber( char  *status )
{
char sqlString[128];
char *handle;
int id =0;

sprintf( sqlString , "SELECT  id  FROM enum_status  WHERE status=\'%s\';" , status );

if( ExecSelect( sqlString ) )
 {
      handle = GetFieldValue( 0 , 0 );
      id = atoi( handle );
      LOG( SQL_LOG , "GetStatusNumeric \'%s\'  ->  (%s) %d" ,   status , handle  , id  );
      FreeSelect();
 }

return id;
}

*/

bool  DB::CheckContactMap(char * table , int id , int contactid )
{
bool ret = false;
char sqlString[128];

sprintf( sqlString , "SELECT * FROM %s_contact_map WHERE %sid=%d and contactid=%d" , table , table , id , contactid );

if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  ) ret=true; // kontakt existuje
    FreeSelect();
  }

return ret;
}

// test pri Check funkci case insensitiv
// vracena hodnota: -1 chyba 0 objekt eexistuje 1 onbjekt existuje
int DB::CheckObject( const char *table ,   const char *fname ,  const char *value )
{
char sqlString[128];
int ret = -1; // defult chyba

sprintf( sqlString , "SELECT id FROM %s  WHERE %s  ILIKE \'%s\'" , table , fname , value );

if( ExecSelect( sqlString ) )
 {
   switch( GetSelectRows() )
        {
          case 1:
                    ret = 1; //  objekt nalezen
                    break;
          case 0:
                    ret = 0; //  objekt nenalezen 
                    break;
             
        }

    FreeSelect();
  }

return ret;
}

// vraci ID hostu
int DB::CheckHost(  const char *fqdn , int nssetID )
{
char sqlString[128];
int hostID=0;

sprintf( sqlString , "SELECT id FROM HOST WHERE fqdn=\'%s\' AND nssetid=%d;" , fqdn , nssetID ); 

if( ExecSelect( sqlString ) )
 {
   if(  GetSelectRows()  == 1 )
     {
       hostID = atoi(  GetFieldValue( 0 , 0 )  );
       LOG( SQL_LOG , "CheckHost fqdn=\'%s\' nssetid=%d  -> hostID %d" , fqdn , nssetID , hostID );
     }
   
  FreeSelect();
 }

return hostID;
}

bool DB::TestNSSetRelations(int id )
{
bool ret = false;
char sqlString[128];

sprintf( sqlString , "SELECT id from DOMAIN WHERE nsset=%d;" , id );
if( ExecSelect( sqlString ) )
  {
     if(  GetSelectRows() > 0 ) ret=true;  // jestli ma domena definovany nsset 
     FreeSelect();
  }

return ret;
}

bool DB::TestContactRelations(int id )
{
bool ret = false;
char sqlString[128];

sprintf( sqlString , "SELECT id from DOMAIN WHERE Registrant=%d;" , id );
if( ExecSelect( sqlString ) ) 
  {
     if(  GetSelectRows() > 0 ) ret=true;  // nejaka domena kterou ma kontakt registrovany
     FreeSelect();
  }

sprintf( sqlString , "SELECT * from DOMAIN_CONTACT_MAP WHERE contactid=%d;" , id );
if( ExecSelect( sqlString ) )
  {
     if(  GetSelectRows() > 0 ) ret=true; // kontakt je admin kontakt nejake domeny 
     FreeSelect();
  }

sprintf( sqlString , "SELECT * from NSSET_CONTACT_MAP WHERE contactid=%d;" , id );
if( ExecSelect( sqlString ) )
  {
     if(  GetSelectRows() > 0 ) ret=true; // kontakt je tech kontakt nejakeho nssetu
     FreeSelect();
  }


return ret;
}

// potvrzeni hesla authinfopw v tabulce 
bool  DB::AuthTable(  char *table , char *auth , int id )
{
bool ret=false;
char *pass;
char sqlString[128];

sprintf( sqlString , "SELECT authinfopw from %s WHERE id=%d" , table  , id );

if( ExecSelect( sqlString ) )
  {
    if( GetSelectRows() == 1 )
      {
           pass = GetFieldValue( 0 , 0 );
           if( strcmp( pass , auth  ) == 0 )  ret = true;
      }
  
   FreeSelect();
  }

return ret;
}


// test kodu zemo
bool DB::TestCountryCode(const char *cc)
{
char sqlString[128];
bool ret = false;

if( strlen( cc )  == 0 ) return true; // test neni potreba vysledek je true

sprintf( sqlString , "SELECT id FROM enum_country WHERE id=\'%s\';" , cc );
if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() == 1   ) ret = true;
    FreeSelect();
 } 

return ret;
}

// vraci id registratora z domeny
int DB::GetClientDomainRegistrant( int clID , int contactID )
{
int regID=0;
char sqlString[128];

sprintf( sqlString , "SELECT  clID FROM DOMAIN WHERE Registrant=%d AND clID=%d" , contactID , clID );
if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() > 0   )
     {
       regID = atoi(  GetFieldValue( 0 , 0 )  );
       LOG( SQL_LOG , "Get ClientDomainRegistrant  contactID \'%d\' -> regID %d" , contactID , regID );
       FreeSelect();
     }
   }

return regID;  
}  

char *  DB::GetValueFromTable( const char *table , const char *vname , const char *fname , const char *value)
{
char sqlString[128];
int size;
// char *handle;

sprintf( sqlString , "SELECT  %s FROM %s WHERE %s=\'%s\';" , vname ,  table  ,  fname , value );

if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() == 1   ) // pokud je vracen prave jeden zaznam
     {
      size = GetValueLength( 0 , 0 );
//      handle = new char[size+1];  // alokace pameti pro retezec

      if( memHandle )
       {
          delete[] memHandle;
          LOG( SQL_LOG , "re-alloc memHandle");
          memHandle = new char[size+1];   
        }
       else { LOG( SQL_LOG , "alloc memHandle");  memHandle = new char[size+1]; } 
 
      strcpy( memHandle , GetFieldValue( 0 , 0 ) );
      LOG( SQL_LOG , "GetValueFromTable \'%s\' field %s  value  %s ->  %s" , table ,  fname , value  , memHandle );
      FreeSelect();      
      return memHandle;
     }
   else {
     FreeSelect();         	
   	 return "";
   }
  }
else return "";

}

char * DB::GetValueFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric)
{
char value[16];

sprintf( value , "%d" ,  numeric );

return GetValueFromTable( table , vname , fname , value );
}

int DB::GetNumericFromTable( const char *table , const char *vname , const char *fname , const char *value)
{
return atoi( GetValueFromTable( table , vname , fname , value )  );
}

int  DB::GetNumericFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric)
{
char value[16];

sprintf( value , "%d" ,  numeric );

return  GetNumericFromTable( table , vname , fname , value  );
}

// vymaz data z tabulky
bool  DB::DeleteFromTable(char *table , char *fname , int id )
{
char sqlString[128];

LOG( SQL_LOG , "DeleteFromTable %s fname %s id -> %d" , table , fname  , id );

sprintf(  sqlString , "DELETE FROM %s  WHERE %s=%d;" , table , fname , id );
return ExecSQL( sqlString );
}
 

// vymaz data z tabulky
bool  DB::DeleteFromTableMap(char *map ,int  id , int contactid )
{
char sqlString[128];


LOG( SQL_LOG , "DeleteFrom  %s_contact_map  id  %d contactID" , map ,id , contactid );

sprintf( sqlString , "DELETE FROM %s_contact_map WHERE  %sid=%d AND contactid=%d;" , map , map ,  id ,  contactid );

return ExecSQL( sqlString );
}



int DB::GetSequenceID( char *sequence )
{
char sqlString[128];
int id=0;


sprintf(  sqlString , "SELECT  NEXTVAL( \'%s_id_seq\'  );" , sequence );

if( ExecSelect( sqlString ) )
  {
     id = atoi(  GetFieldValue( 0 , 0 )  );
     LOG( SQL_LOG , "GetSequence \'%s\' -> ID %d" , sequence , id );
     FreeSelect();
   }

return id;
}


int DB::MakeHistory() // zapise do tabulky history
{
char sqlString[128];

historyID = 0 ;

if( actionID )
 {   
   LOG( SQL_LOG , "MakeHistory actionID -> %d " , actionID);
   historyID = GetSequenceID( "HISTORY" );
   if( historyID )
    {
     LOG( SQL_LOG , "MakeHistory actionID -> %d " , actionID); 
     sprintf(  sqlString , "INSERT INTO HISTORY ( id , action ) VALUES ( %d  , %d );" , historyID , actionID );
     if( ExecSQL(  sqlString ) ) return historyID;
    }
 }

// default
return 0;
}


// ulozi radek tabulky s id
bool DB::SaveHistory(char *table , char *fname , int id )
{
char sqlString[4096] , buf[256] ;
int i ,  row ;
bool ret= true; // default


if( historyID )
{
    LOG( SQL_LOG , "SaveHistory historyID - > %d" , historyID ); 
    sprintf(  sqlString , "SELECT * FROM %s WHERE %s=%d;" , table , fname ,  id );

    if( ExecSelect( sqlString ) )
    {
     for( row = 0 ; row <  GetSelectRows() ; row ++ )
     {

      sprintf( sqlString , "INSERT INTO %s_history ( HISTORYID   " ,  table );

       for( i = 0 ; i <  GetSelectCols() ; i ++ )
          {
           if( IsNotNull( row , i ) )
             {
              strcat( sqlString , " , " );  
              strcat( sqlString ,  GetFieldName( i ) );
             }
          }

       sprintf( buf , " ) VALUES ( %d "  ,  historyID );
       strcat( sqlString , buf ); 
 
      for( i = 0 ; i <  GetSelectCols() ; i ++ )
      {
        if( IsNotNull( row , i ) )
          {  
            sprintf( buf , " , \'%s\'" , GetFieldValue( row  , i ) );
            strcat(  sqlString , buf );
          }
       }
      strcat(  sqlString , " );" );

      if( ExecSQL(  sqlString ) == false) { ret = false ; break;  } // pokud nastane chyba
      }

     FreeSelect();
   }

}

// default 

return ret;
}

// UPDATE funkce
// SQL UPDATE funkce
void DB::UPDATE( const char * table )
{
sqlBuffer = new char[4096*4];

sprintf( sqlBuffer ,  "UPDATE %s SET  " , table );
}

void DB::SET( const char *fname , const char * value )
{

if( strlen( value ) )
 {

  strcat( sqlBuffer ,"  ");
  strcat( sqlBuffer , fname );
  strcat( sqlBuffer , "=" );

   if( value[0] == 0x8 ) // specialne vymazani pri UPDATE
   {
     strcat( sqlBuffer , "NULL" );           
   }
   else 
   {
     strcat( sqlBuffer , "'" );
     strcat( sqlBuffer , value );
     strcat( sqlBuffer , "'" );
   }

  strcat( sqlBuffer , " ," ); // carka na konec
 }

}

void DB::SET( const char *fname , int value )
{
char numStr[16];

strcat( sqlBuffer ,"  ");
strcat( sqlBuffer , fname );
strcat( sqlBuffer , "=" );
sprintf( numStr , "%d" ,  value  );
strcat( sqlBuffer , numStr );
strcat( sqlBuffer , " ," );
}

void DB::SET( const char *fname , bool  value )
{

strcat( sqlBuffer ,"  "); 
strcat( sqlBuffer , fname );
strcat( sqlBuffer , "=" );

if( value )  strcat( sqlBuffer , "\'t\'");
else  strcat( sqlBuffer , "\'f\'");

strcat( sqlBuffer , " ," );
}


void DB::SETBOOL( const char *fname , int  value )
{
// jedna jako tru 0 jako false -1 nic nemenit
if( value == 1 || value == 0 )
{
strcat( sqlBuffer ,"  "); 
strcat( sqlBuffer , fname );
strcat( sqlBuffer , "=" );

if( value )  strcat( sqlBuffer , "\'t\'");
else  strcat( sqlBuffer , "\'f\'");

strcat( sqlBuffer , " ," );
}

}

void DB::WHERE(const char *fname , const char * value )
{
int len;


  len = strlen( sqlBuffer );
  if(  sqlBuffer[len-1]  == ',' )  sqlBuffer[len-1]  = 0; // vymaz posledni carku 

  strcat( sqlBuffer ,"  WHERE " );
  strcat( sqlBuffer , fname );
  strcat( sqlBuffer , "='" );
  strcat( sqlBuffer , value );
  strcat( sqlBuffer , "' ;" );

}

void DB::WHEREOPP(  const  char *op ,  const  char *fname , const  char *p  , const  char * value )
{
int len;


  len = strlen( sqlBuffer );
  if(  sqlBuffer[len-1]  == ';' )
    {
       sqlBuffer[len-1]  = 0; // vymaz posledni strednik
       strcat( sqlBuffer , "  "  );
       strcat( sqlBuffer ,  op ); // operator AND OR LIKE
       strcat( sqlBuffer , " " );
       strcat( sqlBuffer ,  fname );
       strcat( sqlBuffer  , p );
       strcat( sqlBuffer , "'" );
       strcat( sqlBuffer , value );
       strcat( sqlBuffer , "' ;" ); // konec         
    }

}
void DB::WHERE( const  char *fname , int value )
{
char numStr[16];
sprintf( numStr , "%d" ,  value );
WHERE( fname , numStr ); 
}
// INSERT INTO funkce

void DB::INSERT( const char * table )
{
sqlBuffer = new char[4096];

sprintf( sqlBuffer ,  "INSERT INTO  %s  " , table );
}

void DB::INTO(const char *fname)
{
int len;
len = strlen( sqlBuffer );

 
// zacni zavorkou
if(  sqlBuffer[len-1]  == ' ' ) strcat ( sqlBuffer , " ( " ); // zavorka

strcat( sqlBuffer , fname );
strcat( sqlBuffer , " ," );

}

void DB::INTOVAL( const char *fname , const char * value )
{
// pokud josu nejaka data
if( strlen( value ) ) INTO( fname );
}

void DB::VAL(  const char * value)
{
if( strlen( value ) ) VALUE( value );
}


void DB::VALUES( const char * value  , bool esc )
{
int len , length , i , l;


len = strlen( sqlBuffer );


if(  sqlBuffer[len-1] == ';' ) // ukonceni 
{
sqlBuffer[len-1]  = 0;
sqlBuffer[len-2]  = ',';
} 
else
{

 if(  sqlBuffer[len-1]  == ',' )  
 {
   sqlBuffer[len-1]  = 0; // vymaz carku
   strcat ( sqlBuffer , " ) " ); // uzavri zavorku 
  }

  strcat ( sqlBuffer , " VALUES ( " );

}

strcat( sqlBuffer , " '" );
// esacepe
if( esc)
{
len = strlen( sqlBuffer );
length =  strlen(  value );

for( i  = 0 , l = len ; i < length ; i ++ , l++ )
 {
   if(  value[i] == '\\' )  { sqlBuffer[l] =  '\\' ;  l++;  sqlBuffer[l] =  '\\' ; }
   else if(  value[i] == '\'' ) { sqlBuffer[l] =  '\\' ;  l++; sqlBuffer[l] = '\'' ; }
        else  if( value[i] == '\r' ||  value[i] == '\n' )  sqlBuffer[l] = ' ';
              else  sqlBuffer[l] = value[i];
 }
sqlBuffer[l] = 0 ;
}
else strcat( sqlBuffer ,  value );


strcat( sqlBuffer , "' );" ); // vzdy ukoncit
 
}

void DB::VALUESC( const char * value )
{
VALUES( value , true );
}

void DB::VALUE( const char * value )
{
VALUES( value , false );
}

void DB::VALUE( int  value )
{
char numStr[16];
sprintf( numStr , "%d" ,  value );
VALUE( numStr );
}

void DB::VALUE( bool  value )
{
if( value ) VALUE( "t" );
else VALUE( "f" );
}


bool DB::EXEC()
{
bool ret;
// proved SQL prikaz
ret =  ExecSQL( sqlBuffer );

// uvolni pamet
delete sqlBuffer;
// vrat vysledek
return ret;
}


bool DB::SELECT(const char *table  , const char *fname , const char * value )
{
char sqlString[512];

sprintf( sqlString , "SELECT * FROM %s WHERE %s=\'%s\';" , table , fname , value);
return ExecSelect( sqlString );
}

bool DB::SELECT(const char *table  , const char *fname , int value )
{
char sqlString[512];

sprintf( sqlString , "SELECT * FROM %s WHERE %s=%d;" , table , fname , value);
return ExecSelect( sqlString );
}



bool DB::SELECTCONTACTMAP( char *map , int id )
{
char sqlString[512];

sprintf( sqlString , "SELECT  handle FROM CONTACT  JOIN  %s_contact_map ON %s_contact_map.contactid=contact.id WHERE %s_contact_map.%sid=%d;" ,  map  , map , map  , map , id );
return ExecSelect( sqlString );
}


