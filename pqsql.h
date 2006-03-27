#include <libpq-fe.h>

#define debug printf



class PQ{
public:
// constructor nastaveni spojeni conninfo
PQ(){};
// pripoji databazi s nastavenim conninfo
bool OpenDatabase(char *conninfo); 
// provede sqlString 
bool ExecSQL(char *sqlString);
// odpojeni od databaze 
void Disconnect() { PQfinish(connection); } 
// transaction function
bool BeginTransaction()
{ return ExecSQL("START TRANSACTION  ISOLATION LEVEL SERIALIZABLE"); };
bool EndTransaction()
{return ExecSQL("END TRANSACTION"); };
bool RollbackTransaction()
{return ExecSQL("ROLLBACK TRANSACTION");};
bool CommitTransaction()
{return ExecSQL("COMMIT TRANSACTION");};

// spusti select a vrati pocet radek
bool ExecSelect(char *sqlString);
// vyprazdni result selectu a nastavi zpet pocet radku a sloupcu
void  FreeSelect(){  PQclear(result);  };
// vraci hodnotu
char * GetFieldValueName(char *fname , int row );
// vraci retezec hodnoty
char * GetFieldValue( int row , int col );

// vraci pocet radku
int GetSelectRows(){ return nRows;};
int GetSelectCols(){ return nCols;};

private:
PGconn     *connection;
PGresult   *result;
int nRows , nCols; // pocet radek pri selectu
};
