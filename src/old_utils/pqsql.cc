/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sstream>
#include <vector>

#ifdef TIMELOG
#include <sys/time.h>
#include <time.h>
#endif

#include "timeclock.h" // for time profiler
#include "pqsql.h"

#include "log.h" // logger via syslog
#include "log/logger.h"
// constructor 
PQ::PQ()
{
#ifdef TIMECLOCK
  timeclock_start();
#endif
  result = NULL;
}

// destructor
PQ::~PQ()
{
#ifdef TIMECLOCK
  timeclock_quit();
#endif
  FreeSelect();
}

static void noActionNoticeReceiver(
  void *arg, const PGresult *res)
{
}

bool PQ::OpenDatabase(
  const char *conninfo)
{

  LOG( NOTICE_LOG , "PQ: connectdb  %s" , conninfo);
  result = NULL;
  connection = PQconnectdb(conninfo);

  // Check to see that the backend connection was successfully made 
  if (PQstatus(connection) != CONNECTION_OK) {
    LOG( ALERT_LOG , "Connection to database failed: %s",
        PQerrorMessage(connection));
    PQfinish(connection);
    connection = NULL;
    return false;
  } else {
    LOG( NOTICE_LOG , "Database connection OK user %s host %s port %s DB %s" ,
        PQuser(connection), PQhost(connection),
        PQport(connection), PQdb(connection));

#ifdef ENCODING
    SetEncoding( ENCODING );
    LOG( NOTICE_LOG , "Database set client encoding %s" , ENCODING );
#endif 

    // according to http://www.postgresql.org/docs/8.1/interactive/libpq-notice-processing.html
    // there is no need to print NOTICE on client side, so disable ti
    PQsetNoticeReceiver(connection, &noActionNoticeReceiver, NULL);
    return true;
  }

}

bool PQ::OpenDatabase(const std::string& conninfo) {
  return OpenDatabase(conninfo.c_str());
}

// get number of selected rows and cols
int PQ::GetSelectRows()
{
  return nRows;
}
;
int PQ::GetSelectCols()
{
  return nCols;
}
;

// get string by name of filed on the row 
const char * PQ::GetFieldValueName(
  const char *fname, int row)
{
  int col;

  col = PQfnumber(result, fname);

  if (col == -1) {
    LOG( WARNING_LOG , "UNKNOW FieldName: %s" , fname );
    return "";
  } else
    return GetFieldValue(row, col);
}

int PQ::GetNameField(
  const char *fname)
{
  return PQfnumber(result, fname);
}

// get string name of field 
const char * PQ::GetFieldName(
  int col)
{
  if (PQfname(result, col) == NULL)
    return "";
  else
    return PQfname(result, col);
}

// test if is not  NULL value
bool PQ::IsNotNull(
  int row, int col)
{
  if (PQgetisnull(result, row, col) )
    return false;
  else
    return true; // isn't NULL 
}

// return string value at row and col
const char * PQ::GetFieldValue(
  int row, int col)
{
  if (row < nRows && col < nCols) {

    if (PQgetisnull(result, row, col) ) {
      LOG( SQL_LOG , "RETURN [%d,%d] NULL" , row, col );
      return "";
    } else {
      LOG( SQL_LOG , "RETURN [%d,%d] , %s" , row , col , PQgetvalue(result, row, col ) );
      return PQgetvalue(result, row, col);
    }
  } else {
    LOG( ERROR_LOG , "NOT FOUND return NULL" );
    return "";
  }
}

// return Boolean value true or false
bool PQ::GetFieldBooleanValueName(
  const char *fname, int row)
{
  const char *val;
  val = GetFieldValueName(fname, row);
  if (val[0] == 't')
    return true;
  else
    return false;
}

// convert to integer value
int PQ::GetFieldNumericValueName(
  const char *fname, int row)
{
  return atoi(GetFieldValueName(fname, row) );
}

// return lenght  
int PQ::GetValueLength(
  int row, int col)
{
  return PQgetlength(result, row, col);
}

// run SQL select true if is success
bool PQ::ExecSelect(
  const char *sqlString)
{

#ifdef TIMECLOCK
  timeclock_begin();
#endif

  LOG( SQL_LOG , "SELECT: [%s]" , sqlString );

  FreeSelect();
  result = PQexec(connection, sqlString);

  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    LOG( ERROR_LOG , "SELECT [%s]failed: %s", sqlString , PQerrorMessage(connection));
    LOG( ERROR_LOG , "SQL ERROR: %s" , PQresultErrorMessage(result) );
    FreeSelect();
    return false;
  }

  nRows = PQntuples(result);
  nCols = PQnfields(result);

  LOG( SQL_LOG , "result number of rows (tuples) %d and nfields %d" , nRows , nCols );

#ifdef TIMECLOCK
  timeclock_end();
#endif

  return true;
}

// free memory after SELECT and clear result 
void PQ::FreeSelect()
{
  LOG( SQL_LOG , "Free  select" );
  if(result != NULL) {
      PQclear(result);
      result = NULL;
  }
}

void PQ::Disconnect()
{
  LOG( NOTICE_LOG , "PQ: finish");
  if(connection != NULL) {
      PQfinish(connection);
      connection = NULL;
  }
}

std::string PQ::Escape2(
  const std::string& str)
{
  std::vector<char> buffer(3*str.length()+10,0);
  PQescapeStringConn(connection, &buffer[0], str.c_str(), str.length(), NULL);
  std::string ret (&buffer[0]);
  return ret;
}

// escape string to SQL 
bool PQ::Escape(
  char *str, const char *String, int length)
{
  //int err;
  size_t len;

  len = PQescapeString(str, String, length);

  LOG( SQL_LOG , "escape len  %d [%s]" , (int ) len , str );

  return true;
}

// EXEC SQL string  
bool PQ::ExecSQL(
  const char *sqlString)
{
  PGresult *res;
  if (strlen(sqlString) ) {
#ifdef TIMECLOCK
    timeclock_begin();
#endif

    LOG( SQL_LOG , "EXECSQL: [%s]" , sqlString );
    res = PQexec(connection, sqlString);

    LOG( SQL_LOG , "result:  %s %s" , PQresStatus( PQresultStatus(res) ) ,PQcmdStatus( res ) );
#ifdef TIMECLOCK
    timeclock_end();
#endif

    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
      LOG( SQL_LOG , "PQcmdTuples: %s" , PQcmdTuples( res) );
      PQclear(res);
      res = NULL;
      return true;
    } else {
      LOG( ERROR_LOG , "EXECSQL: SQL ERROR: %s" , PQresultErrorMessage(res) );
      PQclear(res);
      res = NULL;
      return false;
    }

  } else {
    LOG( SQL_LOG , "EXECSQL:  empty string return OK" );
    return true;
  }

}

