#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sstream>

#ifdef TIMELOG
#include <sys/time.h>
#include <time.h>
#endif

#include "timeclock.h" // for time profiler
#include "pqsql.h"

#include "log.h" // logger via syslog

// constructor 
PQ::PQ()
{ 
#ifdef TIMECLOCK
timeclock_start();
#endif
}


// destructor
PQ::~PQ()
{
#ifdef TIMECLOCK
timeclock_quit();
#endif
}

bool PQ::OpenDatabase(const char *conninfo)
{

  LOG( NOTICE_LOG , "PQ: connectdb  %s" , conninfo);
connection = PQconnectdb(conninfo);

// Check to see that the backend connection was successfully made 
if (PQstatus(connection) != CONNECTION_OK)
   {
     LOG( ALERT_LOG ,  "Connection to database failed: %s",  
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


// get number of selected tows and cols
int PQ::GetSelectRows(){ return nRows;};
int PQ::GetSelectCols(){ return nCols;};


// get string by name of filed on the row 
char * PQ::GetFieldValueName(char *fname , int row )
{
int col;

col = PQfnumber(result, fname);

if( col == -1 ) {  LOG( WARNING_LOG , "UNKNOW FieldName: %s" , fname );   return "";  }
else return  GetFieldValue( row , col );
}

int  PQ::GetNameField(char *fname )
{
return  PQfnumber(result, fname);
}

// get string name of field 
char *  PQ::GetFieldName( int col )
{
if( PQfname(result, col) == NULL  ) return "";
else return PQfname(result, col);
}

// test if is not  NULL value
bool  PQ::IsNotNull( int row , int col )
{ 
   if( PQgetisnull( result , row , col ) ) return false;
   else return true; // neni NULL 
}

// return string value at row and col
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


// return Boolean value true or false
bool PQ::GetFieldBooleanValueName(char *fname , int row )
{
char *val;
val = GetFieldValueName( fname , row );
if( val[0] == 't' ) return true;
else return false;
}

// convert to integer value
int PQ::GetFieldNumericValueName(char *fname , int row )
{
return atoi( GetFieldValueName( fname , row ) );
}



// return lenghth  
int  PQ::GetValueLength(int row  , int col)
{
return PQgetlength( result , row , col );
}


// run SQL select truu if is success
bool PQ::ExecSelect(const char *sqlString)
{

#ifdef TIMECLOCK
timeclock_begin();
#endif

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

#ifdef TIMECLOCK
timeclock_end();
#endif

return true;
}

// free memory after SELECT and clear result 
void   PQ::FreeSelect()
{
     LOG( SQL_LOG , "Free  select" ) ;
     PQclear(result);

}

void  PQ::Disconnect()
{
 LOG( NOTICE_LOG , "PQ: finish");
 PQfinish(connection); 
}

// escape string to SQL 
bool PQ::Escape(char *str ,  const char *String  , int length )
{
//int err;
size_t  len;



len =  PQescapeString( str,  String ,  length);

LOG( SQL_LOG , "escape len  %d [%s]" ,  (int ) len  , str   );

return true;
}

// EXEC SQL string  
bool PQ::ExecSQL(const char *sqlString)
{
PGresult   *res;
if( strlen( sqlString)  )
{
#ifdef TIMECLOCK
timeclock_begin();
#endif

LOG( SQL_LOG , "EXECSQL: [%s]" , sqlString );
res =  PQexec( connection , sqlString);

LOG( SQL_LOG , "result:  %s %s" ,  PQresStatus( PQresultStatus(res) ) ,PQcmdStatus( res ) );
#ifdef TIMECLOCK
timeclock_end();
#endif


if( PQresultStatus(res) == PGRES_COMMAND_OK )
  {
     LOG( SQL_LOG ,  "PQcmdTuples: %s" ,  PQcmdTuples( res) );
     PQclear(res);
     return true;
  }
else
{
   LOG( ERROR_LOG ,  "EXECSQL: SQL ERROR: %s" , PQresultErrorMessage(res) );
   PQclear(res);
   return false;
}

}
else
{
   LOG( SQL_LOG ,  "EXECSQL:  empty string return OK" );
 return true;
}


} 




