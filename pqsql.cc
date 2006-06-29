#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
// #include <libpq-fe.h>

#include "pqsql.h"
#include "util.h"

#include "status.h"

#include "log.h"

// constructor 
PQ::PQ()
{ 
// nastav memory buffry
svrTRID = NULL;
memHandle=NULL;  
actionID = 0 ;
loginID = 0;
}

// uvolni memory buffry
PQ::~PQ()
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

bool PQ::OpenDatabase(char *conninfo)
{

connection = PQconnectdb(conninfo);

// Check to see that the backend connection was successfully made 
if (PQstatus(connection) != CONNECTION_OK)
   {
     LOG( ERROR_LOG ,  "Connection to database failed: %s",  
                       PQerrorMessage(connection));
     PQfinish(connection);
     return false;   
   }
else 
 {
  LOG( NOTICE_LOG , "Database connection OK user %s host %s port %s DB %s" , 
         PQuser(connection), PQhost(connection),
          PQport(connection), PQdb(connection)); 

#ifdef ENCODING
    SetEncoding(  ENCODING );
    LOG( NOTICE_LOG , "Database set client encoding %s" , ENCODING );
#endif 
  return true;
 }

}

// vraci retezec hodnoty podle nazvu 
char * PQ::GetFieldValueName(char *fname , int row )
{
int col;

col = PQfnumber(result, fname);

if( col == -1 ) return ""; 
else return  GetFieldValue( row , col );
}

// jmeno pole
char *  PQ::GetFieldName( int col )
{
if( PQfname(result, col) == NULL  ) return "";
else return PQfname(result, col);
}

// jestli neni null
bool  PQ::IsNotNull( int row , int col )
{ 
   if( PQgetisnull( result , row , col ) ) return false;
   else return true; // neni NULL 
}

// vraci retezec hodnoty
char * PQ::GetFieldValue( int row , int col )
{
if( row < nRows && col < nCols )
  {
   
   if( PQgetisnull( result , row , col ) ){ LOG( SQL_LOG , "RETURN [%d,%d] NULL" , row, col ); return "" ; }
   else  
     { 
       LOG( SQL_LOG , "RETURN [%d,%d] , %s" , row , col , PQgetvalue(result, row, col ) );
       return PQgetvalue(result, row, col ); 
     }
  }
else { LOG( ERROR_LOG , "NOT FOUND return NULL" ); return  ""; } 
}



// vraci boolean hodnoty
bool PQ::GetFieldBooleanValueName(char *fname , int row )
{
char *val;
val = GetFieldValueName( fname , row );
if( val[0] == 't' ) return true;
else return false;
}
// vraci integer hodnoty
bool PQ::GetFieldNumericValueName(char *fname , int row )
{
return atoi( GetFieldValueName( fname , row ) );
}


// vraci velikost aktualniho prvku 
int  PQ::GetValueLength(int row  , int col)
{
return PQgetlength( result , row , col );
}


// spusti select a vrati pocet radek
bool PQ::ExecSelect(char *sqlString)
{

/*
// pokud je nejaky predchozi select allokoval pamet tak ji uvolni
if( nRows >= 0 || nCols >= 0 )  
 {
   LOG( SQL_LOG , "Clear select" ) ;
   PQclear(result); 
   nRows = -1;
   nCols = -1;
  } 
*/
LOG( SQL_LOG , "SELECT: [%s]" , sqlString );
        result = PQexec(connection, sqlString );

        if (PQresultStatus(result) != PGRES_TUPLES_OK)
        {
            LOG( ERROR_LOG ,  "SELECT [%s]failed: %s", sqlString , PQerrorMessage(connection));
            LOG( ERROR_LOG ,  "SQL ERROR: %s" , PQresultErrorMessage(result) );
            PQclear(result);
            return false;
        }


nRows = PQntuples( result );
nCols = PQnfields(result);

LOG( SQL_LOG , "result number of rows (tuples) %d and nfields %d" , nRows , nCols );

return true;
}

// uvolneni pameti po selectu
void   PQ::FreeSelect()
{
     LOG( SQL_LOG , "Free  select" ) ;
     PQclear(result);


/*
 if( nRows >= 0 || nCols >= 0  ) 
  { 
     LOG( SQL_LOG , "Free  select" ) ; 
     PQclear(result);
     nRows = -1; nCols = -1;  
  }
*/
}

bool PQ::ExecSQL(char *sqlString)
{
PGresult   *res;

LOG( SQL_LOG , "ExecSQL: [%s]" , sqlString );
res =  PQexec( connection , sqlString);

LOG( SQL_LOG , "result:  %s %s" ,  PQresStatus( PQresultStatus(res) ) ,PQcmdStatus( res ) );

if( PQresultStatus(res) == PGRES_COMMAND_OK )
  {

     LOG( SQL_LOG ,  "PQcmdTuples: %s" ,  PQcmdTuples( res) );
     PQclear(res);
    return true;
  }
else
{
   LOG( ERROR_LOG ,  "ExecSQL error");
   LOG( ERROR_LOG ,  "SQL ERROR: %s" , PQresultErrorMessage(result) );
   PQclear(res);
   return false;
}

} 
// zpracovani creditu
bool PQ::Credit( int registrarID , int domainID , int period , bool create )
{
char sqlString[256];
char b;

// true false pro create
if(  create ) b = 't' ;
else b = 'f' ;

sprintf( sqlString , "INSERT INTO CREDIT (  registrarID ,  domainID , date , domaincreate , period ) VALUES ( %d , %d , 'now' , \'%c\' ,  %d );" ,  registrarID  ,  domainID , b ,  period );

if( ExecSQL( sqlString ) ) return true;      
else return false;
}


// action
// zpracovani action
bool  PQ::BeginAction(int clientID , int action ,char *clTRID )
{
char sqlString[512];

// actionID pro logovani
actionID = GetSequenceID("action");
loginID = clientID; // id klienta

if( actionID ) 
  {
  // zapis do action tabulky
   sprintf( sqlString , "INSERT INTO ACTION ( id , clientid , action ,  clienttrid   )  VALUES ( %d , %d , %d , \'%s\' );" ,
                  actionID , clientID  , action  , clTRID);

   return ExecSQL( sqlString );
  }
else return false;
}

char * PQ:: EndAction(int response )
{
char sqlString[512];
// char svrTRID[32];
int id;

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


// vraci chybovou zpravu podle jazyka
char * PQ::GetErrorMessage(int err )
{

if( GetClientLanguage() == LANG_CS ) return GetErrorMessageCS( err );
else return GetErrorMessageEN( err );

}

// vraci jazyk klienta
int PQ::GetClientLanguage()
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

/*
char *  PQ::GetStatusString( int status )
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


int  PQ::GetStatusNumber( char  *status )
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

bool  PQ::CheckContactMap(char * table , int id , int contactid )
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
int PQ::CheckObject( const char *table ,   const char *fname ,  const char *value )
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



bool PQ::TestNSSetRelations(int id )
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

bool PQ::TestContactRelations(int id )
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
bool  PQ::AuthTable(  char *table , char *auth , int id )
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
 
// vraci id registratora z domeny
int PQ::GetClientDomainRegistrant( int clID , int contactID )
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

char *  PQ::GetValueFromTable( const char *table , const char *vname , const char *fname , const char *value)
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

char * PQ::GetValueFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric)
{
char value[16];

sprintf( value , "%d" ,  numeric );

return GetValueFromTable( table , vname , fname , value );
}

int PQ::GetNumericFromTable( const char *table , const char *vname , const char *fname , const char *value)
{
return atoi( GetValueFromTable( table , vname , fname , value )  );
}

int  PQ::GetNumericFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric)
{
char value[16];

sprintf( value , "%d" ,  numeric );

return  GetNumericFromTable( table , vname , fname , value  );
}

// vymaz data z tabulky
bool  PQ::DeleteFromTable(char *table , char *fname , int id )
{
char sqlString[128];

LOG( SQL_LOG , "DeleteFromTable %s fname %s id -> %d\n" , table , fname  , id );

sprintf(  sqlString , "DELETE FROM %s  WHERE %s=%d;" , table , fname , id );
return ExecSQL( sqlString );
}
 

int PQ::GetSequenceID( char *sequence )
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


int PQ::MakeHistory() // zapise do tabulky history
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
bool PQ::SaveHistory(char *table , char *fname , int id )
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
