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

// zpracovani action 
bool BeginAction(int clientID , int action ,char *clTRID );
char * EndAction(int response  );

// vraci handle nebo id tabulky
char * GetHandleFrom( char *table , int id );
int GetIDFrom( char *table , char *handle);
int GetSequenceID( char *sequence ); // id ze sequnce



int  GetLoginRegistrarID(int clientID);
 
// spusti select a vrati pocet radek
bool ExecSelect(char *sqlString);
// vyprazdni result selectu a nastavi zpet pocet radku a sloupcu
bool  FreeSelect(){  PQclear(result);  };
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
int actionID;
};
