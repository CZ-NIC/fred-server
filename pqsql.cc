#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
// #include <libpq-fe.h>

#include "pqsql.h"
#include "util.h"

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



// spusti select a vrati pocet radek
bool PQ::ExecSelect(char *sqlString)
{
nRows = -1;
nCols = -1;

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
char svrTRID[32];
char *svr;
int id;

// cislo transakce co vraci server
sprintf( svrTRID , "ccReg-%010d" , actionID );

sprintf( sqlString , "UPDATE Action SET response=%d , enddate='now()' , servertrid=\'%s\' WHERE id=%d;" ,  response  , svrTRID , actionID );

// update tabulky
id  =  actionID;
actionID = 0 ; 

if( ExecSQL(  sqlString ) ) 
 {
   sprintf( sqlString , "SELECT  servertrid FROM Action  WHERE id=%d;"  , id );
   if( ExecSelect( sqlString ) )
    {
      svr =  GetFieldValue( 0 , 0 );
      debug("GetsvrTRID %s\n" , svr );   
      FreeSelect();
    }
  return svr;
 }

else return "svrTRID ERROR";
}



char *  PQ::GetStatusString( int status )
{
char sqlString[128];
char *handle;

sprintf( sqlString , "SELECT  status  FROM enum_status  WHERE id=%d;" , status );

if( ExecSelect( sqlString ) )
 {
      handle = GetFieldValue( 0 , 0 );
      debug("GetStatustring \'%s\'  ->  %d\n" ,  handle , status );
      FreeSelect();
 }

if( handle == NULL ) return "";
else return handle;
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
  

char *  PQ::GetValueFromTable( char *table , char *vname , char *fname , char *value)
{
char sqlString[128];
char *handle;

sprintf( sqlString , "SELECT  %s FROM %s WHERE %s=\'%s\';" , vname ,  table  ,  fname , value );

if( ExecSelect( sqlString ) )
 {
      handle = GetFieldValue( 0 , 0 );
      debug("GetValueFromTable \'%s\' field %s  value  %s ->  %s\n" , table ,  fname , value  , handle );
      FreeSelect();
  }

if( handle == NULL ) return "";
else return handle;
}

char * PQ::GetValueFromTable( char *table , char *vname ,  char *fname ,  int numeric)
{
char value[16];

sprintf( value , "%d" ,  numeric );

return GetValueFromTable( table , vname , fname , value );
}

int PQ::GetNumericFromTable( char *table , char *vname , char *fname , char *value)
{
return atoi( GetValueFromTable( table , vname , fname , value )  );
}

int  PQ::GetNumericFromTable( char *table , char *vname ,  char *fname ,  int numeric)
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
