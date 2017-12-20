#ifndef __PQSQL_H__
#define __PQSQL_H__

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
