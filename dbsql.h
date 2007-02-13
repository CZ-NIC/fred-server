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
long GetPrice(   int  operation  ,  int zone , int period  );
bool SaveInvoiceCredit(int regID , int objectID , int  operation  , int zone  ,  int period ,  const char *ExDate , long price ,  long price2  ,   int invoiceID , int invoiceID2 );
// odecitani castky price od credity  ze zalohove faktury 
bool InvoiceCountCredit( long  price , int invoiceID  );

// operace registrace domeny  CREATE
bool BillingCreateDomain( int regID , int  zone , int  objectID  );
// operace prodlouzeni domeny RENEW
bool BillingRenewDomain(  int regID , int  zone , int  objectID , int period  ,  const char *ExDate );

// zpracovani creditu
bool UpdateInvoiceCredit( int regID ,   int operation  , int zone  , int period  ,  const char *ExDate , int objectID );

long GetRegistrarCredit(int regID , int zoneID );

// test otisdku certifikatu a hesla v tabulce registrar ACL
bool  TestRegistrarACL( int  regID , const char *  pass , const char * cert );


// ukladada vytvorene XML z mod_eppd
int SaveXMLout( const char *svTRID , const char *xml  );

// zpracovani action a ulozeni XML 
// vraci registarID id registratora
bool BeginAction(int clientID , int action ,const char *clTRID  , const char *xml );
char * EndAction(int response  );
// vraci jazyk klienta
// int GetClientLanguage() {return clientLang;}
// zjisti komunikujici jazyk z tabulky login
// int ReturnClientLanguage();

// vraci handle nebo id tabulky
int GetNumericFromTable( const char *table , const char *vname ,  const char *fname ,  const char *value);
int GetNumericFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric);
char * GetValueFromTable( const char *table , const char *vname ,  const char *fname ,  const char *value);
char * GetValueFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric);
int GetSequenceID( char *sequence ); // id ze sequnce
// kontroluje jestli je contactid pro dane id v tabulce nsset_contact_map nebo domain_contact_map 
bool CheckContactMap(const char * table , int id , int contactid );
// pridave contact do pole kontaktu
bool AddContactMap( const char * table , int id , int contactid );
 
// vraci datum a cas dle rfc3339
char * GetFieldDateTimeValueName(  const char *fname , int row );
char * GetFieldDateValueName(  const char *fname , int row );

// pro select domeny pri info
// bool SELECTDOMAIN(   const char *fqdn , int zone , bool enum_zone );
// vraci ID kontaktu z handlu zadaneho ipres mala pismena nebo nula pokud je chyba
int  GetContactID( const char *handle );
int  GetNSSetID( const char *handle );
// vraci id domeny ( special pro enum )
int  GetDomainID( const char *fqdn , bool enum_zone );

// objekt byl nekde nemam prirazen jako tech ci admin registrant domeny ci nsset u domeny
bool ObjectModify( int id );

// update object dle id  a registrator
bool ObjectUpdate( int id , int regID , const char *authInfo );
// test ClID=regID objektu
bool TestObjectClientID( int id  , int regID );

// vraci ID objektu podle jeho nazvu
int GetObjectID( int type , const char *name );

char * GetDomainExDate( int id ); // vraci ExDate domeny jako datum (lokalni datum)

char * GetObjectCrDateTime( int id ); // vraci datum a cas vytvoreni objektu
char * GetObjectName( int id );

// ukladani smazaneho objektu ErDate
bool SaveObjectDelete( int id );
// ukladani crhistoryID do tabulky object_regiustry
bool SaveObjectCreate( int id );

bool TestContactHandleHistory( const char * handle , int days );
bool TestNSSetHandleHistory( const char * handle , int days );
bool TestDomainFQDNHistory( const char * fqdn , int days );

// test na objekty v historii
bool TestObjectHistory(   const char * name , int days );

// vraci id uctu
int GetBankAccount( const char *accountStr ,  const char *codeStr   );
// vraci cislo zony pro kterou je ucet pouzit
int GetBankAccountZone( int accountID );

// test zustatku na uctu pro import bankovniho vypisu
int TestBankAccount( const char *accountStr , int num , long oldBalance );


// update zustatku na uctu
bool UpdateBankAccount( int accountID , char *date , int num ,  long newBalance  );

// ulozeni hlavicky vypisu return ID hlavicky
int SaveBankHead( int accountID ,int num ,  char *date  ,  char *oldDate , long oldBalance , long newBalance , long credit , long debet );

// ulozeni polozky vypisu
bool SaveBankItem( int statemetID , char *account  , char *bank , char *evidNum, char *date , char *memo , int code , 
                       char *konstSymb ,  char *varSymb , char *specsymb  , long price );

int TestEBankaList(const char*ident ); // vraci id zaznamu podle ident

int SaveEBankaList( int account_id , const char *ident , long  price , const char *datetimeStr ,  const char *accountStr , const char *codeStr ,
                        const char *varsymb , const char *konstsymb ,  const char *nameStr ,  const char *memoStr );


// nastav vypis ebanky jako zpracovany na zalohovou FA
bool UpdateEBankaListInvoice( int id , int invoiceID );


int GetSystemVAT();  // vraci hodnotu DPH pro sluzby registrace
double GetSystemKOEF(); // vraci hodnotu prepocitavaciho koeficientu

// nastav bankovni vypis jako zpracovany
bool UpdateBankStatementItem( int id , int invoiceID);

// spocte zustatek a vycerpany credit za zalohovych FA
long GetInvoiceBalance( int aID , long credit );
long GetInvoiceSumaPrice(int  iID , int aID );

// zpusti fakturaci do zadaneho timestamp datum zdanitelneho plneni je taxdate 
int MakeFactoring(  int regID , int zone , const char *timestampStr ,  const char *taxDateStr  );


// vytvoreni ostre faktury
int MakeNewInvoice(  const char *taxDateStr , const char *fromdateStr , const char *todateStr , int zone ,  int regID ,  long price , unsigned int count );

// vytvoreni nove  zalohove faktury pro registratora na castku price  s odvedenim dph VAT=true nebo bez
int  MakeNewInvoiceAdvance( const char *taxDateStr , int zone ,  int regID ,  long price , bool VAT );

// generovani cisla faktur a update countru prefixu 
int GetPrefixType( const char *dateStr , int typ , int zone );
int GetInvoicePrefix( const char *dateStr , int typ , int zone );


// test doby expirace validace
bool TestValExDate(const char *dateStr ,  int period  , int interval , int id );
// test doby expirace
bool CountExDate(  int domainID , int period  , int max_period );
bool RenewExDate(  int domainID , int period  );


// vraci ID hostu
int GetHostID(  const char *fqdn , int nssetID );
// vraci pocet nssetu
int GetNSSetHosts( int nssetID );
// zjistuje pocet  tech pro dany nsset
int GetNSSetContacts( int nssetID );



int GetRegistrarID( char *handle ) { return GetNumericFromTable( "REGISTRAR", "id" , "handle" , handle ); };
int GetRegistrarIDbyVarSymbol( char *vs )  { return GetNumericFromTable( "REGISTRAR", "id" , "varsymb" , vs ); };

// vraci true pokud je to systemovy registrator
bool GetRegistrarSystem( int regID );


char * GetRegistrarHandle(int id ) { return GetValueFromTable( "REGISTRAR", "handle" , "id" , id ); };
char * GetStatusFromTable( char *table , int id ) {  return GetValueFromTable( table , "status" , "id" , id ); };


// vraci id registratora z domeny
int GetClientDomainRegistrant( int clID , int contactID );

// test na vazu mezi tabulkami pro kontakt a nsset
bool TestNSSetRelations(int id );
bool TestContactRelations(int id );

bool AuthTable(const  char *table , char *auth , int id );

// testuje pravo registratora na zapis do zony
bool TestRegistrarZone(int regID , int zone );

// vytvoreni objektu v tabulce object pri create fce
int CreateObject( const char *type , int regID , const char *name , const char *authInfoPw );


// funkce na ulozeni obsahu radku ID tabluky  do 
int MakeHistory(int objectID); // zapise do tabulky history vraci historyID
bool SaveHistory(char *table , char *fname ,  int id ); // ulozi radek tabulky



// uloz do historie 
bool SaveNSSetHistory( int id );
bool DeleteNSSetObject( int id );

bool SaveDomainHistory( int id );
bool DeleteDomainObject( int id ); 

bool SaveContactHistory( int id ); 
bool DeleteContactObject( int id ); // smaz kontakt


// SQL UPDATE funkce
void UPDATE( const  char * table );

// set 
void SSET( const char *fname , const char * value ); // bez escape
void SET( const char *fname , const char * value ); // s escape
// void NSET( const char *fname , const char * value , bool null );
void SETS( const char *fname , const char * value , bool esc  /* , bool null  */ ); 

void SET( const  char *fname , int   value );
void SET( const  char *fname , bool  value );
void SETNULL( const char *fname  );
void SETBOOL( const char *fname , char c );
void SETPRICE( const char *fname , long price );

void WHERE( const  char *fname , const  char * value );
void WHERE( const  char *fname , int   value );
void WHEREOPP(  const  char *op ,  const  char *fname , const  char *p  , const  char * value );
void OPERATOR(  const  char *op );
void WHEREID( int id ) { WHERE( "id" , id ); };
//  SQL INSERT funkce
void INSERTHISTORY( const char * table );
void INSERT( const  char * table );
void INTO(const  char *fname);
void INTOVAL(const  char *fname , const char * value );
void VAL( const  char * value);
void VALUESC( const char * value );
void VALUES( const char * value  , bool esc , bool amp ,  int uplo ); // pouzivat esc sequence a uvozovat do ampersandu
void VALUE( const  char * value );
void VVALUE( const char * value ); // bez escape
void VALUE( int  value );
void VALUE( bool value );
void VALPRICE( long price ); // cena v halirich

void VALUELOWER( const char * value  );
void VALUEUPPER( const char * value  );

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


void SQLCatLower( const char *str );
void SQLCatUpper(const char *str );
// prevadi na mala ci velka pismena
void SQLCatLW( const char *str , bool lw );

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
bool SELECTOBJECTID(  const char *table , const char *fname ,  int id  );

 


int GetActionID() { return actionID; } // vraci odkaz do tabulky action ID
int GetLoginID() { return loginID; } // vraci id klienta

private:
char *memHandle;
char *svrTRID;
char *sqlBuffer;
char dtStr[32]; //  pro datum
int actionID; // id tabulky akce
int historyID; // id tabulky historie
int loginID; // id klienta
};

#endif
