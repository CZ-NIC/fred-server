#include <libpq-fe.h>


#define LANG_EN 0
#define LANG_CS 1


class PQ{
public:
// constructor a destruktor
PQ();
~PQ();

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

// nastaveni kodovani 
void  SetEncoding(  const char *encoding) {  PQsetClientEncoding( connection , encoding ); };
// vraci velikost 
int  GetValueLength(int row  , int col);



bool DeleteFromTable(char *table , char *fname , int id );

// zapocteni creditu za operace domain create a domain renew
bool Credit( int registrarID , int domainID , int period , bool create );
// zpracovani action 
bool BeginAction(int clientID , int action ,char *clTRID );
char * EndAction(int response  );

// vraci jazyk klienta
int GetClientLanguage();

// vraci handle nebo id tabulky
int GetNumericFromTable( const char *table , const char *vname ,  const char *fname ,  const char *value);
int GetNumericFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric);
char * GetValueFromTable( const char *table , const char *vname ,  const char *fname ,  const char *value);
char * GetValueFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric);
int GetSequenceID( char *sequence ); // id ze sequnce
// kontroluje jestli je contactid pro dane id v tabulce nsset_contact_map nebo domain_contact_map 
bool CheckContactMap(char * table , int id , int contactid );

// teck pri Check funkcih case insensitiv
int CheckObject( const char *table ,  const char *fname ,  const char *value);

// vyssi funkce na vraceni value
int GetLoginRegistrarID(int id) { return GetNumericFromTable( "LOGIN" , "registrarid" , "id" , id ); };
int GetRegistrarID( char *handle ) { return GetNumericFromTable( "REGISTRAR", "id" , "handle" , handle ); };
char * GetRegistrarHandle(int id ) { return GetValueFromTable( "REGISTRAR", "handle" , "id" , id ); };
char * GetStatusFromTable( char *table , int id ) {  return GetValueFromTable( table , "status" , "id" , id ); };


// vraci id registratora z domeny
int GetClientDomainRegistrant( int clID , int contactID );

// vraci chybovou zpravu z enum_error
char * GetErrorMessageEN(int err ) {  return GetValueFromTable( "enum_error", "status" , "id" , err ); };
char * GetErrorMessageCS(int err ) {  return GetValueFromTable( "enum_error", "status_cs" , "id" , err ); };
char * GetErrorMessage( int err ); // test na jazyk klienta

// test kodu zemo
bool TestCountryCode(const char *cc);
char * GetCountryNameEN( const char *cc ) { return GetValueFromTable("enum_country" , "country" , "id" , cc ); };
char * GetCountryNameCS( const char *cc ) { return  GetValueFromTable("enum_country" , "country_cs" , "id" , cc ); };

// test na vazu mezi tabulkami pro kontakt a nsset
bool TestNSSetRelations(int id );
bool TestContactRelations(int id );

bool AuthTable(  char *table , char *auth , int id );


// funkce na ulozeni obsahu radku ID tabluky  do 
int MakeHistory(); // zapise do tabulky history
bool SaveHistory(char *table , char *fname ,  int id ); // ulozi radek tabulky


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


// SQL UPDATE funkce
void UPDATE( const  char * table );
void SET( const  char *fname , const  char * value );
void SET( const  char *fname , int   value );
void SET( const  char *fname , bool  value );
void SETBOOL( const char *fname , int  value );
void WHERE( const  char *fname , const  char * value );
void WHERE( const  char *fname , int   value );
void WHEREID( int id ) { WHERE( "id" , id ); };
//  SQL INSERT funkce
void INSERT( const  char * table );
void INTO(const  char *fname);
void INTOVAL(const  char *fname , const char * value );
void VAL( const  char * value);
void VALUE( const  char * value );
void VALUE( int  value );
void VALUE( bool value );

bool EXEC();
bool SELECT(const char *table  , const char *fname , const char * value );

// vraci pocet radku
int GetSelectRows(){ return nRows;};
int GetSelectCols(){ return nCols;};

int GetActionID() { return actionID; } // vraci odkaz do tabulky action ID
int GetLoginID() { return loginID; } // vraci id klienta

private:
PGconn     *connection;
PGresult   *result;
char *memHandle;
char *svrTRID;
char *sqlBuffer;
int nRows , nCols; // pocet radek pri selectu
int actionID; // id tabulky akce
int historyID; // id tabulky historie
int loginID; // id klienta
};
