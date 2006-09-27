#ifndef __PQSQL_H__
#define __PQSQL_H__

#include <libpq-fe.h>

class PQ{
public:
// constructor a destruktor
PQ();
~PQ();

// pripoji databazi s nastavenim conninfo
bool OpenDatabase(const char *conninfo); 
// provede sqlString 
bool ExecSQL(const char *sqlString);
// odpojeni od databaze 
void Disconnect();

// nastaveni kodovani 
void  SetEncoding(  const char *encoding) ;
// vraci velikost  prvku
int  GetValueLength(int row  , int col);

// spusti select a vrati pocet radek
bool ExecSelect(const char *sqlString);
// vyprazdni result selectu a nastav pocet radku a sloupcu na -1
void  FreeSelect();
// vraci hodnotu
char * GetFieldValueName(char *fname , int row );
// vraci retezec hodnoty
char * GetFieldValue( int row , int col );


// vraci boolean hodnoty
bool GetFieldBooleanValueName(char *fname , int row );
// vraci integer hodnoty
bool GetFieldNumericValueName(char *fname , int row );

// jmeno pole
char *  GetFieldName( int col );
// jestli neni null
bool IsNotNull( int row , int col );

// escape string 
bool Escape(char *str ,  const char *String ,  int length  );

// vraci pocet radku
int GetSelectRows();
int GetSelectCols();


private:
PGconn     *connection;
PGresult   *result;
int nRows , nCols; // pocet radek pri selectu
};

#endif
