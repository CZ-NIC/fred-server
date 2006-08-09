#include "pqsql.h"


#define LANG_EN 0
#define LANG_CS 1
#define CMD_OK 1000 // OK prikaz pro konec transakce

class DB : public PQ{
public:
// constructor a destruktor
DB();
~DB();

// transaction function
bool BeginTransaction()
{ return ExecSQL("START TRANSACTION  ISOLATION LEVEL SERIALIZABLE"); };
bool EndTransaction()
{return ExecSQL("END TRANSACTION"); };
bool RollbackTransaction()
{return ExecSQL("ROLLBACK TRANSACTION");};
bool CommitTransaction()
{return ExecSQL("COMMIT TRANSACTION");};
bool QuitTransaction(int code) {
  if( code == CMD_OK ) return CommitTransaction();
   else RollbackTransaction(); 
};


// vymaz z tabulky
bool DeleteFromTable(char *table , char *fname , int id );
// vymaz data z map tabulky
bool DeleteFromTableMap(char *map ,int  id , int contactid );

// parametry zone
int GetExPreriodMin(int zone){ return GetNumericFromTable( "zone", "ex_period_min" , "id" , zone ); };
int GetExPreriodMax(int zone){ return GetNumericFromTable( "zone", "ex_period_max" , "id" , zone ); };
int GetValPreriod(int zone){ return GetNumericFromTable( "zone", "val_period" , "id" , zone ); };

// zapocteni creditu za operace domain create a domain renew
bool UpdateCredit( int registrarID , int objectID ,  int zone ,  int period , int operation  );
// zpracovani action a ulozeni XML 
bool BeginAction(int clientID , int action ,const char *clTRID  , const char *xml );
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

// test pri Check funkcih case insensitiv
int CheckObject( const char *table ,  const char *fname ,  const char *value);

// vraci ID hostu
int CheckHost(  const char *fqdn , int nssetID );
// vraci pocet nssetu
int GetNSSetNum( int nssetID );



// vyssi funkce na vraceni value
int GetLoginRegistrarID(int id) { 
if( id == 0 ) return 0;
else return GetNumericFromTable( "LOGIN" , "registrarid" , "id" , id ); 
};

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



// SQL UPDATE funkce
void UPDATE( const  char * table );
void SET( const  char *fname , const  char * value );
void SET( const  char *fname , int   value );
void SET( const  char *fname , bool  value );
void SETBOOL( const char *fname , int  value );
void WHERE( const  char *fname , const  char * value );
void WHERE( const  char *fname , int   value );
void WHEREOPP(  const  char *op ,  const  char *fname , const  char *p  , const  char * value );
void WHEREID( int id ) { WHERE( "id" , id ); };
//  SQL INSERT funkce
void INSERT( const  char * table );
void INTO(const  char *fname);
void INTOVAL(const  char *fname , const char * value );
void VAL( const  char * value);
void VALUESC( const char * value );
void VALUES( const char * value  , bool esc );
void VALUE( const  char * value );
void VALUE( int  value );
void VALUE( bool value );

bool EXEC();
bool SELECT(const char *table  , const char *fname , const char * value );
bool SELECT(const char *table  , const char *fname , int value );

bool SELECTCONTACTMAP( char *map , int id ); // admin a tech kontakty



int GetActionID() { return actionID; } // vraci odkaz do tabulky action ID
int GetLoginID() { return loginID; } // vraci id klienta

private:
char *memHandle;
char *svrTRID;
char *sqlBuffer;
int actionID; // id tabulky akce
int historyID; // id tabulky historie
int loginID; // id klienta
};
