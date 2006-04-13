#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
// #include <libpq-fe.h>

#include "pqsql.h"


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
    if( ExecSelect("SELECT NEXTVAL('action_id_seq');" ) )
     {
      actionID = atoi( GetFieldValue( 0 , 0 ) );
      debug("action id = %d\n" , actionID  );
      FreeSelect();
     }

  // zapis do action tabulky
   sprintf( sqlString , "INSERT INTO ACTION ( id , clientid , action ,  clienttrid   )  VALUES ( %d , %ld , %d , \'%s\' );" ,
                  actionID , clientID  , action  , clTRID);

return ExecSQL( sqlString );

}

bool PQ:: EndAction(int response , char *svrTRID )
{
char sqlString[512];
// cislo transakce co vraci server
sprintf( svrTRID , "ccReg-%010d" , actionID );

sprintf( sqlString , "UPDATE Action SET response=%d , enddate='now()' , servertrid=\'%s\' WHERE id=%d;" ,  response  , svrTRID , actionID );

// update tabulky
actionID=0;
return ExecSQL(  sqlString );
}

