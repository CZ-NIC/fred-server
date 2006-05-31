#include <libpq-fe.h>

#define debug printf



class PQ{
public:
// constructor nastaveni spojeni conninfo
PQ(){  }; // neni vybran zadny select
~PQ() {  } 

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

// vraci velikost 
int  GetValueLength(int row  , int col);



bool DeleteFromTable(char *table , char *fname , int id );

// zapocteni creditu za operace domain create a domain renew
bool Credit( int registrarID , int domainID , int period , bool create );
// zpracovani action 
bool BeginAction(int clientID , int action ,char *clTRID );
char * EndAction(int response  );

// vraci handle nebo id tabulky
int GetNumericFromTable( char *table , char *vname ,  char *fname ,  char *value);
int GetNumericFromTable( char *table , char *vname ,  char *fname ,  int numeric);
char * GetValueFromTable( char *table , char *vname ,  char *fname ,  char *value);
char * GetValueFromTable( char *table , char *vname ,  char *fname ,  int numeric);
int GetSequenceID( char *sequence ); // id ze sequnce
// kontroluje jestli je contactid pro dane id v tabulce nsset_contact_map nebo domain_contact_map 
bool CheckContactMap(char * table , int id , int contactid );


// vyssi funkce na vraceni value
int GetLoginRegistrarID(int id) { return GetNumericFromTable( "LOGIN" , "registrarid" , "id" , id ); };
int GetRegistrarID( char *handle ) { return GetNumericFromTable( "REGISTRAR", "id" , "handle" , handle ); };
char * GetRegistrarHandle(int id ) { return GetValueFromTable( "REGISTRAR", "handle" , "id" , id ); };
char * GetStatusFromTable( char *table , int id ) {  return GetValueFromTable( table , "status" , "id" , id ); };

// vraci id registratora z domeny
int GetClientDomainRegistrant( int clID , int contactID );

// test na vazu mezi tabulkami pro kontakt a nsset
bool TestNSSetRelations(int id );
bool TestContactRelations(int id );


// funkce na ulozeni obsahu radku ID tabluky  do 
int MakeHistory(); // zapise do tabulky history
bool SaveHistory(char *table , char *fname ,  int id ); // ulozi radek tabulky

// funkce co vraci z enum_status string 
char * GetStatusString( int status );
// funkce vracejici id statusu
int  GetStatusNumber( char  *status );

// spusti select a vrati pocet radek
bool ExecSelect(char *sqlString);
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


// vraci pocet radku
int GetSelectRows(){ return nRows;};
int GetSelectCols(){ return nCols;};

int GetActionID() { return actionID; } // vraci odkaz do tabulky action ID


private:
PGconn     *connection;
PGresult   *result;
int nRows , nCols; // pocet radek pri selectu
int actionID; // id tabulky akce
int historyID; // id tabulky historie
};
