#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
// #include <libpq-fe.h>

#include "pqsql.h"
#include "util.h"

#include "status.h"

bool PQ::OpenDatabase(char *conninfo)
{

connection = PQconnectdb(conninfo);

// Check to see that the backend connection was successfully made 
if (PQstatus(connection) != CONNECTION_OK)
   {
     fprintf(stderr, "Connection to database failed: %s\n",  
                       PQerrorMessage(connection));
     PQfinish(connection);
     return false;   
   }
else 
 {
  debug("Connection OK user %s host %s port %s DB %s\n" , 
         PQuser(connection), PQhost(connection),
          PQport(connection), PQdb(connection)); 
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
   
   if( PQgetisnull( result , row , col ) ){ debug("RETURN [%d,%d] NULL\n" , row, col ); return "" ; }
   else  
     { 
       debug("RETURN [%d,%d] , %s\n" , row , col , PQgetvalue(result, row, col ) );
       return PQgetvalue(result, row, col ); 
     }
  }
else { debug("NOT FOUND return NULL\n" ); return  ""; } 
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
   debug("Clear select\n" ) ;
   PQclear(result); 
   nRows = -1;
   nCols = -1;
  } 
*/
        result = PQexec(connection, sqlString );

        if (PQresultStatus(result) != PGRES_TUPLES_OK)
        {
            debug( "SELECT [%s]\nfailed: %s", sqlString , PQerrorMessage(connection));
            PQclear(result);
            return -1;
        }


nRows = PQntuples( result );
nCols = PQnfields(result);

debug("Select [%s]\nnumber of rows (tuples) %d and nfields %d\n" , sqlString , nRows , nCols );


}

// uvolneni pameti po selectu
void   PQ::FreeSelect()
{
     debug("Free  select\n" ) ;
     PQclear(result);


/*
 if( nRows >= 0 || nCols >= 0  ) 
  { 
     debug("Free  select\n" ) ; 
     PQclear(result);
     nRows = -1; nCols = -1;  
  }
*/
}

bool PQ::ExecSQL(char *sqlString)
{
PGresult   *res;

res =  PQexec( connection , sqlString);

debug("result [%s]\n %s %s\n", sqlString ,   PQresStatus( PQresultStatus(res) ) ,PQcmdStatus( res ) );

if( PQresultStatus(res) == PGRES_COMMAND_OK )
  {

     debug( "PQcmdTuples: %s\n" ,  PQcmdTuples( res) );
     PQclear(res);
    return true;
  }
else
{
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

if( actionID ) 
  {
  // zapis do action tabulky
   sprintf( sqlString , "INSERT INTO ACTION ( id , clientid , action ,  clienttrid   )  VALUES ( %d , %ld , %d , \'%s\' );" ,
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
  debug("alloc svrTRID\n");
  svrTRID= new char[32] ; 
 }

// cislo transakce co vraci server
sprintf( svrTRID , "ccReg-%010d" , actionID );

sprintf( sqlString , "UPDATE Action SET response=%d , enddate='now()' , servertrid=\'%s\' WHERE id=%d;" ,  response  , svrTRID , actionID );

// update tabulky
id  =  actionID;
actionID = 0 ; 

debug( "EndAction svrTRID: %s\n" ,  svrTRID );

if( ExecSQL(  sqlString ) ) return   svrTRID;
/* {

   sprintf( sqlString , "SELECT  servertrid FROM Action  WHERE id=%d;"  , id );
   if( ExecSelect( sqlString ) )
    {
      svr =  GetFieldValue( 0 , 0 );
      debug("GetsvrTRID %s\n" , svr );   
     //  FreeSelect(); 
    }
  return svr;
 }
*/

else return "svrTRID ERROR";
}



char *  PQ::GetStatusString( int status )
{
/*
char sqlString[128];
char *str;

str = new char[128];

sprintf( sqlString , "SELECT  status  FROM enum_status  WHERE id=%d;" , status );

if( ExecSelect( sqlString ) )
 {
      strcpy( str ,  GetFieldValue( 0 , 0 ) ) ;
      debug("GetStatustring \'%s\'  ->  %d\n" ,  str , status );
      FreeSelect(); 
      return str;
 }
else return "{ }" ; // prazdny status string pole
*/

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
      debug("GetStatusNumeric \'%s\'  ->  (%s) %d\n" ,   status , handle  , id  );
      FreeSelect();
 }

return id;
}

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

sprintf( sqlString , "SELECT authinfopw from %s WHERE id=%d\n" , table  , id );

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

sprintf( sqlString , "SELECT  clID FROM DOMAIN WHERE Registrant=%d AND clID=%d\n" , contactID , clID );
if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() > 0   )
     {
       regID = atoi(  GetFieldValue( 0 , 0 )  );
       debug("Get ClientDomainRegistrant  contactID \'%d\' -> regID %d\n" , contactID , regID );
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
          debug("re-alloc memHandle\n");
          memHandle = new char[size+1];   
        }
       else { debug("alloc memHandle\n");  memHandle = new char[size+1]; } 
 
      strcpy( memHandle , GetFieldValue( 0 , 0 ) );
      debug("GetValueFromTable \'%s\' field %s  value  %s ->  %s\n" , table ,  fname , value  , memHandle );
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
     debug("Sequence \'%s\' -> ID %d\n" , sequence , id );
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
   debug("MAkeHistory\n");
   historyID = GetSequenceID( "HISTORY" );
   if( historyID )
    {
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
    sprintf(  sqlString , "SELECT * FROM %s WHERE %s=%d\n" , table , fname ,  id );

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
