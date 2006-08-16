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

 // umozni  info funkce pro PIF
if( action == EPP_ContactInfo ||  action == EPP_NSsetInfo ||   action ==  EPP_DomainInfo ) 
{
if( clientID == 0 ) { actionID = 0 ; loginID =  0 ;  return true;  }
}

// actionID pro logovani
actionID = GetSequenceID("action");
loginID = clientID; // id klienta

if( actionID ) 
  {
  // zapis do action tabulky
  INSERT( "ACTION" );
  INTO( "id" );
  if( clientID > 0 ) INTO( "clientID"  );
  INTO( "action" );
  INTO( "clienttrid" );
  
  VALUE(  actionID  );
  if( clientID > 0 ) VALUE( clientID );
  VALUE( action );
  VALUE( clTRID );
  
   if( EXEC() ) 
     {
        if( strlen( xml ) )
           {
                INSERT("Action_XML");
                VALUE( actionID );
                VALUE( xml );
                return  EXEC();
           }
        else return true;
     }
   else return false;
  }
else return false;


}


char * DB:: EndAction(int response )
{
int id;

if( actionID == 0 ) return "no action";
else
{
if( svrTRID == NULL ) 
 {
  LOG( SQL_LOG , "alloc svrTRID");
  svrTRID= new char[MAX_SVTID] ; 
 }

// cislo transakce co vraci server
sprintf( svrTRID , "ccReg-%010d" , actionID );

UPDATE("ACTION");
SET( "response" , response );
SET( "enddate", "now" );
SET( "servertrid" , svrTRID );
WHEREID( actionID );


// update tabulky
id  =  actionID;
actionID = 0 ; 
LOG( SQL_LOG ,  "EndAction svrTRID: %s" ,  svrTRID );

if( EXEC() ) return   svrTRID;
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

if( hostID == 0 )  LOG( SQL_LOG , "Host fqdn=\'%s\' not found" , fqdn  );
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
char sqlString[512];
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
char sqlString[MAX_SQLBUFFER] , buf[1024] ;
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
sqlBuffer = new char[MAX_SQLBUFFER];
memset(  sqlBuffer , 0 , MAX_SQLBUFFER );
SQLCat( "UPDATE " );
SQLCat( table );
SQLCat( " SET " );
}

void DB::SQLDelEnd()
{
int len;
len = strlen( sqlBuffer );
// vymaz konec
if( len > 0 )  sqlBuffer[len-1]  = 0;
}

bool DB::SQLTestEnd( char c )
{
int len;
len = strlen( sqlBuffer );

// test
if(  sqlBuffer[len-1]  == c ) return true;
else return false;
}

void DB::SQLCat(const char *str )
{
int len , length ;
len = strlen( str );
length = strlen( sqlBuffer );

//  test na delku retezce
if(  len  + length < MAX_SQLBUFFER ) strcat( sqlBuffer , str );
}

// escape
void DB::SQLCatEscape( const char * value )
{
int length;
char *str;

length = strlen( value );

if( length )
{
    // zvetsi retezec
    // LOG( SQL_LOG , "alloc escape string length  %d" , length*2 );
    str = new char[length*2];
    // DB escape funkce
    Escape( str , value  , length ) ;
    SQLCat( str );
    // LOG( SQL_LOG , "free  escape string" );
    delete[] str;
}

}
void DB::SET( const char *fname , const char * value )
{
int length;
char *str;

if( strlen( value ) )
 {

  SQLCat(" ");
  SQLCat(fname );
  SQLCat( "=" );

   if( value[0] == 0x8 ) // specialne vymazani pri UPDATE
   {
     SQLCat( "NULL" );           
   }
   else 
   {
    SQLCat( "'" );
    SQLCatEscape(  value ); 
    SQLCat(  "'" );
   }

  SQLCat(  " ," ); // carka na konec
 }

}

void DB::SET( const char *fname , int value )
{
char numStr[16];

SQLCat( "  ");
SQLCat(  fname );
SQLCat(  "=" );
sprintf( numStr , "%d" ,  value  );
SQLCat(  numStr );
SQLCat( " ," );
}

void DB::SET( const char *fname , bool  value )
{

SQLCat( "  "); 
SQLCat(  fname );
SQLCat(  "=" );

if( value )  SQLCat( "\'t\'");
else  SQLCat( "\'f\'");

SQLCat(  " ," );
}

// nastavi but t nebo f 
void DB::SETBOOL( const char *fname , char c  )
{
// jedna jako tru 0 jako false -1 nic nemenit
if( c == 't' || c == 'f'  ||  c == 'T' || c == 'F' )
{
SQLCat("  "); 
SQLCat( fname );
SQLCat( "=" );

if(  c == 't' ||  c == 'T' )  SQLCat( "\'t\'");
else  SQLCat( "\'f\'");

SQLCat( " ," );
}

}

void DB::WHERE(const char *fname , const char * value )
{
  if( SQLTestEnd( ',' ) ||  SQLTestEnd( ';' ) ) SQLDelEnd();  // vymaz posledni znak

  SQLCat("  WHERE " );
  SQLCat( fname );
  SQLCat( "='" );
  SQLCatEscape(  value );
  SQLCat( "' ;" );

}

void DB::WHEREOPP(  const  char *op ,  const  char *fname , const  char *p  , const  char * value )
{


  if(  SQLTestEnd( ';' )  )
    {
       SQLDelEnd(); // umaz posledni strednik
       SQLCat( "  "  );
       SQLCat(  op ); // operator AND OR LIKE
       SQLCat( " " );
       SQLCat( fname );
       SQLCat(  p );
       SQLCat(  "'" );
       SQLCatEscape(  value );
       SQLCat(  "' ;" ); // konec         
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
sqlBuffer = new char[MAX_SQLBUFFER];
memset(  sqlBuffer , 0 , MAX_SQLBUFFER );
SQLCat( "INSERT INTO " );
SQLCat( table );
SQLCat( "  " );
}

void DB::INTO(const char *fname)
{
 
// zacni zavorkou
if( SQLTestEnd(' ') ) SQLCat(  " ( " ); // zavorka

SQLCat( fname );
SQLCat( " ," );

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
int len ;

len = strlen( sqlBuffer );


if(  SQLTestEnd(  ';' ) ) // ukonceni 
{
SQLDelEnd();
SQLDelEnd();
SQLCat( ",");
} 
else
{

 if(   SQLTestEnd( ',' )  )  
 {
   SQLDelEnd();
   SQLCat( " ) " ); // uzavri zavorku 
  }

  SQLCat( " VALUES ( " );

}

SQLCat( " '" );

// esacepe
if( esc) SQLCatEscape(  value );
else  SQLCat( value );


strcat( sqlBuffer , "' );" ); // vzdy ukoncit
 
}


void DB::VALUE( const char * value )
{
// pouzij ESCAPE 
VALUES( value , true ); 
}

void DB::VALUE( int  value )
{
char numStr[16];
sprintf( numStr , "%d" ,  value );
VALUES( numStr , false ); // bez ESC
}

void DB::VALUE( bool  value )
{
// bez ESC
if( value ) VALUES( "t" , false );
else VALUES( "f" , false );
}


bool DB::EXEC()
{
bool ret;
// proved SQL prikaz
ret =  ExecSQL( sqlBuffer );

// uvolni pamet
delete[] sqlBuffer;
// vrat vysledek
return ret;
}

bool DB::SELECT()
{
bool ret;
// provede select SQL 
ret = ExecSelect( sqlBuffer );

// uvolni pamet
delete[] sqlBuffer;
// vrat vysledek
return ret;
}

// SQL SELECT funkce
void DB::SELECTFROM( const char *fname  , const char * table  )
{
sqlBuffer = new char[MAX_SQLBUFFER];
memset(  sqlBuffer , 0 , MAX_SQLBUFFER );
SQLCat( "SELECT " );
if( strlen( fname )  ) SQLCat( fname );
else SQLCat( " * " ) ;
SQLCat( " FROM " );
SQLCat( table );
SQLCat( " ;" ); // ukoncit
}


bool DB::SELECTONE(  const char * table  , const char *fname ,  const char *value )
{
SELECTFROM( fname , table );
WHERE( fname , value );
return SELECT();
}

bool DB::SELECTONE(  const char * table  , const char *fname ,  int value )
{
SELECTFROM( fname , table );
WHERE( fname , value );
return SELECT();
}


bool DB::SELECTCONTACTMAP( char *map , int id )
{
char sqlString[512];

sprintf( sqlString , "SELECT  handle FROM CONTACT  JOIN  %s_contact_map ON %s_contact_map.contactid=contact.id WHERE %s_contact_map.%sid=%d;" ,  map  , map , map  , map , id );
return ExecSelect( sqlString );
}


