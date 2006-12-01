#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sstream>


#include "pqsql.h"

#include "log.h"

// constructor 
PQ::PQ()
{ 
}

// destructor
PQ::~PQ()
{
}

bool PQ::OpenDatabase(const char *conninfo)
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


// vraci pocet radku a sloupcu
int PQ::GetSelectRows(){ return nRows;};
int PQ::GetSelectCols(){ return nCols;};


// vraci retezec hodnoty podle nazvu 
char * PQ::GetFieldValueName(char *fname , int row )
{
int col;

col = PQfnumber(result, fname);

if( col == -1 ) {  LOG( WARNING_LOG , "unknow FieldName: %s" , fname );   return "";  }
else return  GetFieldValue( row , col );
}

int  PQ::GetNameField(char *fname )
{
return  PQfnumber(result, fname);
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
int PQ::GetFieldNumericValueName(char *fname , int row )
{
return atoi( GetFieldValueName( fname , row ) );
}



// vraci velikost aktualniho prvku 
int  PQ::GetValueLength(int row  , int col)
{
return PQgetlength( result , row , col );
}


// spusti select a vrati pocet radek
bool PQ::ExecSelect(const char *sqlString)
{

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

}

void  PQ::Disconnect()
{
 LOG( SQL_LOG , "disconect");
 PQfinish(connection); 
}


bool PQ::Escape(char *str ,  const char *String  , int length )
{
//int err;
size_t  len;



// escape retezce
// len =  PQescapeStringConn(connection, str , String,  length, &err);

/*
if( err != NULL )
{
   LOG( ERROR_LOG ,  "ExecSQL escape error: %s" , String );
  delete str;
  return false;
}
else
*/

len =  PQescapeString( str,  String ,  length);

LOG( SQL_LOG , "escape len  %d [%s]" ,  (int ) len  , str   );

return true;
}

bool PQ::ExecSQL(const char *sqlString)
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
   LOG( ERROR_LOG ,  "SQL ERROR: %s" , PQresultErrorMessage(res) );
   PQclear(res);
   return false;
}



} 




