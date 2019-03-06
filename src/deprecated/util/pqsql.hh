/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PQSQL_HH_71724B7F68524A8DA6C9985AECD7E53B
#define PQSQL_HH_71724B7F68524A8DA6C9985AECD7E53B

#include <libpq-fe.h>
#include <string>

class PQ
{
public:
  // constructor a destruktor
  PQ();
  ~PQ();

  PQ(PGconn *_conn) : connection(_conn), result(NULL)
  {
  }

  //  connect to database whith  conninfo string
  bool OpenDatabase(
    const char *conninfo);
  bool OpenDatabase(
    const std::string& conninfo);
  // exec sqlString 
  bool ExecSQL(
    const char *sqlString);
  // disconnect from database
  void Disconnect();

  // client encoding 
  void SetEncoding(
    const char *encoding);
  // get lenght 
  int GetValueLength(
    int row, int col);

  // rum SQL select 
  bool ExecSelect(
    const char *sqlString);
  // free  result  of selectu 
  void FreeSelect();
  // return value of field name fname and row
  const char * GetFieldValueName(
    const char *fname, int row);
  // return value of filed with row and column
  const char * GetFieldValue(
    int row, int col);

  // get BOOL value 
  bool GetFieldBooleanValueName(
    const char *fname, int row);
  // get integer value
  int GetFieldNumericValueName(
    const char *fname, int row);

  // name of field
  const char * GetFieldName(
    int col);
  int GetNameField(
    const char *fname);

  // test null value true if not NULL false if is NULL
  bool IsNotNull(
    int row, int col);

  // escape string to escape sequence  using  libpq
  bool Escape(
    char *str, const char *String, int length);

  std::string Escape2(
    const std::string& str);

  // return number of selected rows and cols
  int GetSelectRows();
  int GetSelectCols();

  PGconn* __getPGconn() {
    return connection;
  }

private:
  PGconn *connection;
  PGresult *result;
  int nRows, nCols; // number  of  rows and cols
};

#endif
