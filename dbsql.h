#ifndef __DBSQL_H__
#define __DBSQL_H__

#include "pqsql.h"


#define LANG_EN 0
#define LANG_CS 1
#define CMD_OK 1000 // OK prikaz pro konec transakce

#define MAX_SQLBUFFER 4096*4 // delka sqlBuffer
#define MAX_SVTID 32 // delka svtrid

class DB : public PQ{
public:
// constructor a destruktor
DB();
~DB();

// transaction function
bool BeginTransaction()
{ return ExecSQL("START TRANSACTION  ISOLATION LEVEL READ COMMITTED"); };
bool EndTransaction()
{return ExecSQL("END TRANSACTION"); };
bool RollbackTransaction()
{return ExecSQL("ROLLBACK TRANSACTION");};
bool CommitTransaction()
{return ExecSQL("COMMIT TRANSACTION");};
bool QuitTransaction(int code) {
  if( code == CMD_OK ) return CommitTransaction();
   else return RollbackTransaction(); 
};


// vymaz z tabulky
bool DeleteFromTable(char *table , char *fname , int id );
// vymaz data z map tabulky
bool DeleteFromTableMap(char *map ,int  id , int contactid );


// vraci castku za operaci
long GetPrice(   int action  ,  int zone , int period  );
// zpracovani creditu
long UpdateInvoiceCredit( int regID ,   int action  , int zone  , int period  );

// ukladada vytvorene XML z mod_eppd
int SaveXMLout( const char *svTRID , const char *xml  );

// zpracovani action a ulozeni XML 
// vraci registarID id registratora
int BeginAction(int clientID , int action ,const char *clTRID  , const char *xml );
char * EndAction(int response  );
// vraci jazyk klienta
int GetClientLanguage() {return clientLang;}
// zjisti komunikujici jazyk z tabulky login
int ReturnClientLanguage();

// vraci handle nebo id tabulky
int GetNumericFromTable( const char *table , const char *vname ,  const char *fname ,  const char *value);
int GetNumericFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric);
char * GetValueFromTable( const char *table , const char *vname ,  const char *fname ,  const char *value);
char * GetValueFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric);
int GetSequenceID( char *sequence ); // id ze sequnce
// kontroluje jestli je contactid pro dane id v tabulce nsset_contact_map nebo domain_contact_map 
bool CheckContactMap(char * table , int id , int contactid );

// vraci id objektu pokud ho nalezne
int CheckNSSet(const char *handle );
// pro nsset
int CheckContact(const char *handle);
// test pri Check funkci  na handle nssetu ci kontaktu
int CheckHandle( const char *table ,  const char *handle );
// vraci id domeny
int CheckDomain( const char *fqdn   , int zone , bool enum_zone );
// pro select domeny pri info
bool SELECTDOMAIN(   const char *fqdn , int zone , bool enum_zone );
// vraci ID kontaktu z handlu zadaneho ipres mala pismena nebo nula pokud je chyba
int  GetContactID( const char *handle );

bool TestContactHandleHistory( const char * handle , int period );
bool TestNSSetHandleHistory( const char * handle , int period );
bool TestDomainFQDNHistory( const char * fqdn , int period );

// test na objekty v historii
bool TestObjectHistory( const char *table , const char *fname ,  const char * name , int period );

// test zustatku na uctu pro import bankovniho vypisu
int TestBankAccount( char *accountStr , int num , long oldBalance );

// update zustatku na uctu
bool UpdateBankAccount( int accountID , char *date , int num ,  long newBalance  );

// ulozeni hlavicky vypisu return ID hlavicky
int SaveBankHead( int accountID ,int num ,  char *date  ,  char *oldDate , long oldBalance , long newBalance , long credit , long debet );

// ulozeni polozky vypisu
bool SaveBankItem( int statemetID , char *account  , char *bank , char *evidNum, char *date , char *memo , int code , 
                       char *konstSymb ,  char *varSymb , char *specsymb  , long price );


// uloz credit
bool SaveCredit(  int regID  , int zone  ,  long credit  ,int invoiceID );

// nastav bankovni vypis jako zpracovany
bool UpdateBankStatementItem( int id , int invoiceID);

// vytvoreni zalohove faktury pro registratora na castku price s vysi DPH vatNum odvedenou dani vat  a castkou bez DPH credit
int  MakeAInvoice( const char *prefixStr  , int regID , long price , int vatNum , long vat ,  long credit );

// generovani cisla faktur a update countru prefixu 
bool GetInvoicePrefix( char *prefixStr , int typ , int zone );

// test doby expirace validace
bool TestValExDate(const char *dateStr ,  int period  , int interval , int id );
// vypocet doby exprace
bool GetExpDate(char *dateStr , int domainID , int period  , int max_period );

// vraci ID hostu
int CheckHost(  const char *fqdn , int nssetID );
// vraci pocet nssetu
int GetNSSetHosts( int nssetID );
// zjistuje pocet  tech pro dany nsset
int GetNSSetContacts( int nssetID );


// vyssi funkce na vraceni value
int GetLoginRegistrarID(int id) { return  registrarID; } 

int GetRegistrarID( char *handle ) { return GetNumericFromTable( "REGISTRAR", "id" , "handle" , handle ); };
char * GetRegistrarHandle(int id ) { return GetValueFromTable( "REGISTRAR", "handle" , "id" , id ); };
char * GetRegistrarCredit(int id ) { return GetValueFromTable( "REGISTRAR", "credit" , "id" , id ); };
char * GetStatusFromTable( char *table , int id ) {  return GetValueFromTable( table , "status" , "id" , id ); };


// vraci id registratora z domeny
int GetClientDomainRegistrant( int clID , int contactID );

// test na vazu mezi tabulkami pro kontakt a nsset
bool TestNSSetRelations(int id );
bool TestContactRelations(int id );

bool AuthTable(  char *table , char *auth , int id );

// testuje pravo registratora na zapis do zony
bool TestRegistrarZone(int regID , int zone );

// vytvoreni objektu v tabulce object pri create fce
int CreateObject( const char *type , int regID , const char *name , const char *authInfoPw );


// funkce na ulozeni obsahu radku ID tabluky  do 
int MakeHistory(); // zapise do tabulky history
bool SaveHistory(char *table , char *fname ,  int id ); // ulozi radek tabulky



// SQL UPDATE funkce
void UPDATE( const  char * table );

// set 
void SSET( const char *fname , const char * value ); // bez escape
void SET( const char *fname , const char * value ); // s escape
// void NSET( const char *fname , const char * value , bool null );
void SETS( const char *fname , const char * value , bool esc  /* , bool null  */ ); 

void SET( const  char *fname , int   value );
void SET( const  char *fname , bool  value );
void SETBOOL( const char *fname , char c );
void SETPRICE( const char *fname , long price );

void WHERE( const  char *fname , const  char * value );
void WHERE( const  char *fname , int   value );
void WHEREOPP(  const  char *op ,  const  char *fname , const  char *p  , const  char * value );
void WHEREID( int id ) { WHERE( "id" , id ); };
//  SQL INSERT funkce
void INSERTHISTORY( const char * table );
void INSERT( const  char * table );
void INTO(const  char *fname);
void INTOVAL(const  char *fname , const char * value );
void VAL( const  char * value);
void VALUESC( const char * value );
void VALUES( const char * value  , bool esc , bool amp); // pouzivat esc sequence a uvozovat do ampersandu
void VALUE( const  char * value );
void VVALUE( const char * value ); // bez escape
void VALUE( int  value );
void VALUE( bool value );
void VALPRICE( long price ); // cena v halirich

// zadani aktualni cas
void VALUENOW();
// zadani aktualnic as puls interval period v mesicich
void VALUEPERIOD( int period );
// zadani null values
void VALUENULL();

// SQL SELECT funkce
void SELECTFROM( const char *fname  , const char * table  );
// pro funkce select field from table where field=value
bool SELECTONE(  const char * table  , const char *fname ,  const char *value );
bool SELECTONE(  const char * table  , const char *fname ,  int value );

// SQL string funkce

// umazani konce retezce
void SQLDelEnd();
// test konce retezce sqlBuffer na znak c 
bool SQLTestEnd( char c );
// pouziti strcat do sqlBuffer s testem na delku retezce 
void SQLCat(const char *str );
// escape
void SQLCatEscape( const char * value ); 


// konecne funkce
bool EXEC();
bool SELECT();

bool SELECTCONTACTMAP( char *map , int id ); //  pro admin a tech kontakty



int GetActionID() { return actionID; } // vraci odkaz do tabulky action ID
int GetLoginID() { return loginID; } // vraci id klienta

private:
char *memHandle;
char *svrTRID;
char *sqlBuffer;
int actionID; // id tabulky akce
int historyID; // id tabulky historie
int loginID; // id klienta
int clientLang;
int registrarID;
};

#endif
