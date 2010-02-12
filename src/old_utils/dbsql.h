#ifndef __DBSQL_H__
#define __DBSQL_H__

#include "register/db_settings.h"

#include "util.h"
#include "pqsql.h"
#include "register/types.h"
#include <map>
#include <cstdlib>
#include <boost/shared_ptr.hpp>


#define LANG_EN 0
#define LANG_CS 1
#define CMD_OK 1000 // OK command to the commit transaction
#define CMD_FAILED(x) (x<2000) // all successfull codes
#define MAX_SQLBUFFER 4096*25 // maximal lenght od the sqlBuffer
#define MAX_SVTID 32 // length of the server  ticket  svTRID

class DB;
class ParsedAction 
{
  std::map<unsigned, std::string> elements;
public:
  void add(unsigned id, const std::string& value);
  bool executeSQL(Register::TID actionid, DB* db);
};


class DB : public PQ
{
public:
  // constructor and destructor
  DB();
  ~DB();

  /* HACK! HACK! HACK! */
  DB(Database::Connection &_conn) : PQ(_conn.__getConn__())
  {
      svrTRID = NULL;
      memHandle=NULL;
      actionID = 0;
      enum_action=0;
      loginID = 0;
  }

  // transaction function
  bool BeginTransaction()
  {
    return ExecSQL("START TRANSACTION  ISOLATION LEVEL READ COMMITTED");
  }
  ;
  bool EndTransaction()
  {
    return ExecSQL("END TRANSACTION");
  }
  ;
  bool RollbackTransaction()
  {
    return ExecSQL("ROLLBACK TRANSACTION");
  }
  ;
  bool CommitTransaction()
  {
    return ExecSQL("COMMIT TRANSACTION");
  }
  ;
  bool QuitTransaction(
    int code)
  {
    if (CMD_FAILED(code))
      return CommitTransaction();
    else
      return RollbackTransaction();
  }
  ;

  ///------------------------
  //   BILLING

  // get price for operation defined in enum_operation_table from table price_list
  long GetPrice(
    int operation, int zone, int period);
  bool SaveInvoiceCredit(
    int regID, int objectID, int operation, int zone, int period,
    const char *ExDate, long price, long price2, int invoiceID, int invoiceID2);
  //  get credit from invoice  
  bool InvoiceCountCredit(
    long price, int invoiceID);

  // operation  CREATE
  bool BillingCreateDomain(
    int regID, int zone, int objectID);
  // operation RENEW
  bool BillingRenewDomain(
    int regID, int zone, int objectID, int period, const char *ExDate);

  // count credit from invoce 
  bool UpdateInvoiceCredit(
    int regID, int operation, int zone, int period, const char *ExDate,
    int objectID);

  long GetRegistrarCredit(
    int regID, int zoneID);

  ///------------------------
  /// INVOICING 

  // count balence and used credit on invoice
  long GetInvoiceBalance(
    int aID, long credit);
  long GetInvoiceSumaPrice(
    int iID, int aID); //

  // make factoring 
  int MakeFactoring(
    int regID, int zone, const char *timestampStr, const char *taxDateStr);

  // make Invoice
  int MakeNewInvoice(
    const char *taxDateStr, const char *fromdateStr, const char *todateStr,
    int zone, int regID, long price, unsigned int count);

  // make new Invoice 
  int MakeNewInvoiceAdvance(
    const char *taxDateStr, int zone, int regID, long price);

  // make prefix for invoice
  int GetPrefixType(
    const char *dateStr, int typ, int zone);
  long GetInvoicePrefix(
    const char *dateStr, int typ, int zone);

  ///-------------------
  // BANKING 

  // return id of the account 
  int GetBankAccount(
    const char *accountStr, const char *codeStr);
  // return number of the account for zone
  int GetBankAccountZone(
    int accountID);

  // test oldBalance at account 
  int TestBankAccount(
    const char *accountStr, int num, long oldBalance, char* bank);

  // update Balance  at the account
  bool UpdateBankAccount(
    int accountID, char *date, int num, long newBalance);

  // save bankstatement  head list for account 
  int SaveBankHead(
    int accountID, int num, char *date, char *oldDate, long oldBalance,
    long newBalance, long credit, long debet);

  // save bank item for statement
  int SaveBankItem(
    int statemetID, char *account, char *bank, char *evidNum, char *date,
    char *memo, int code, char *konstSymb, char *varSymb, char *specsymb,
    long price);

  // e-banka function for table bank_ebanka_list on-line bankstatement  via https
  int TestEBankaList(
    const char*ident); // return ident for E-banka list

  int SaveEBankaList(
    int account_id, const char *ident, long price, const char *datetimeStr,
    const char *accountStr, const char *codeStr, const char *varsymb,
    const char *konstsymb, const char *nameStr, const char *memoStr);

  // gererated invoice from incoming price to the e-banka 
  bool UpdateEBankaListInvoice(
    int id, int invoiceID);

  //
  int GetSystemVAT(); // return VAT for invoicing depends at the actual time
  double GetSystemKOEF(); // transformed koeficient to count VAT for price local function

  // this bank statement is processed
  bool UpdateBankStatementItem(
    int id, int invoiceID);

  //----------------------------
  // EPP function for table action and action_xml

  // save EPP masagege about transfered object to the table messages 
  bool SaveEPPTransferMessage(
    int oldregID, int regID, int objectID, int type);

  // save generated  XML from  mod_eppd
  int SaveXMLout(
    const char *svTRID, const char *xml);

  // start of the EPP operation with clientID and save xml from epp-client 
  bool BeginAction(
    int clientID, int action, const char *clTRID, const char *xml,
    ParsedAction* paction = NULL
  );
  // end of the EPP operation
  const char * EndAction(
    int response); // return svrTRID

  char * GetsvTRID()
  {
    return svrTRID;
  }
  ; // return actual generated server ticket  svrTRID


  //  test certificate fingerprint in the table registrarACL for registarID
  bool TestRegistrarACL(
    int regID, const char * pass, const char * cert);

  int GetEPPAction()
  {
    return enum_action;
  } // return  EPP operation
  int GetActionID()
  {
    return actionID;
  } // retrun action.id 
  int GetLoginID()
  {
    return loginID;
  } // return clientID

  //----------------------------
  // DATABASE  functions
  // delete data from table 
  bool DeleteFromTable(
    const char *table, const char *fname, int id);
  // delete data from *_contact_map table
  bool DeleteFromTableMap(
    const char *map, int id, int contactid);

  // return  handle or id of the table 
  int GetNumericFromTable(
    const char *table, const char *vname, const char *fname, const char *value);
  int GetNumericFromTable(
    const char *table, const char *vname, const char *fname, int numeric);
  const char * GetValueFromTable(
    const char *table, const char *vname, const char *fname, const char *value);
  const char * GetValueFromTable(
    const char *table, const char *vname, const char *fname, int numeric);
  int GetSequenceID(
    const char *sequence); // get  id from  sequence 
  // test   contactid at the table  nsset_contact_map or  domain_contact_map or keyset_contact_map
  bool CheckContactMap(
    const char * table, int id, int contactid, int role);
  
  //  add conaxctID to the table nsset_contact_map or  domain_contact_map or keyset_contact_map
  bool AddContactMap(
    const char * table, int id, int contactid);

  //  return  date or timestamp from database field converted to local time by function in util.cc
  char * GetFieldDateTimeValueName(
    const char *fname, int row);
  char * GetFieldDateValueName(
    const char *fname, int row);

  // return  ID of contact from  handle converted to upper case 
  int GetContactID(
    const char *handle);
  int GetNSSetID(
    const char *handle);
  // return  id of domain  ( special for enum )
  int GetDomainID(
    const char *fqdn, bool enum_zone);
  // return id of keyset
  int GetKeySetID(const char *handle);

  // return id of dsrecord
  int GetDSRecordId(int keysetId, int keytag, int alg, int digesttype, const char *digest, int maxsiglife);
  int GetDSRecordId(int keytag, int alg, int digesttype, const char *digest, int maxsiglife);
  
  // return id of dnskey
  int GetDNSKeyId(int keysetId, int flags, int protocol, int alg, const char *key);
  int GetDNSKeyId(int flags, int protocol, int alg, const char *key);

  //  save update for   object  id  by registrar regID and optionly save authInfo ( password)
  bool ObjectUpdate(
    int id, int regID, const char *authInfo);
  // test client ClID=regID for object
  bool TestObjectClientID(
    int id, int regID);

  // return  ID of actual object by name 
  int GetObjectID(
    int type, const char *name);
  // reverse function
  const char * GetObjectName(
    int id);

  const char * GetDomainExDate(
    int id); // return  domain.ExDate like local date 
  // special for ENUM
  const char * GetDomainValExDate(
    int id); // return  domain.ValDate like local date


  const char * GetObjectCrDateTime(
    int id); // return local timestamp of the created object

  // save object_registry.ErDate for deleted object
  bool SaveObjectDelete(
    int id);
  // save   object_registry.crhistoryID  for created object
  bool SaveObjectCreate(
    int id);

  // test validity of the enumdomain.exdate
  // use SQL for compare depend on the old value if is in protected interval
  bool TestValExDate(
    const char *valexDate, int period, int interval, int id);
  // for domain.exdate
  bool CountExDate(
    int domainID, int period, int max_period);
  bool RenewExDate(
    int domainID, int period);

  // test linked DNS ID host
  int GetHostID(
    const char *fqdn, int nssetID);
  // return number of the DNS assigned to nsset
  int GetNSSetHosts(
    int nssetID);
  // return number of the tech-c  assigned to nsset
  int GetNSSetContacts(
    int nssetID);

  int GetKeySetDSRecords(int keysetID);
  int GetKeySetContacts(int keysetid);
  int GetKeySetDNSKeys(int keysetID);

  int GetRegistrarID(
    const char *handle)
  {
    return GetNumericFromTable("REGISTRAR", "id", "handle", handle);
  }
  ;
  int GetRegistrarIDbyVarSymbol(
    const char *vs)
  {
    return GetNumericFromTable("REGISTRAR", "id", "varsymb", atoi(vs) );
  }
  ;

  // return true if the registar is system can make all operations test registrar.system is it true
  bool GetRegistrarSystem(
    int regID);

  const char * GetRegistrarHandle(
    int id)
  {
    return GetValueFromTable("REGISTRAR", "handle", "id", id);
  }
  ;
  const char * GetStatusFromTable(
    char *table, int id)
  {
    return GetValueFromTable(table, "status", "id", id);
  }
  ;

  // return registrarID
  int GetClientDomainRegistrant(
    int clID, int contactID);

  // test contacts, nsset and keyset relation for linked status
  bool TestNSSetRelations(
    int id);
  bool TestContactRelations(
    int id);
  bool TestKeySetRelations(int id);

  bool AuthTable(
    const char *table, const char *auth, int id);

  // is it right of registrar  to access to the zone 
  bool TestRegistrarZone(
    int regID, int zone);

  // create active object in the table object
  int CreateObject(
    const char *type, int regID, const char *name, const char *authInfoPw);

  ///---------------
  // history functions 
  int MakeHistory(
    int objectID);// create insert into table history
  bool SaveHistory(
    const char *table, const char *fname, int id); // save  row in table to the history table 

  // test if exist deleted  object in history 
  bool TestContactHandleHistory(
    const char * handle, int days);
  bool TestNSSetHandleHistory(
    const char * handle, int days);
  bool TestDomainFQDNHistory(
    const char * fqdn, int days);
  bool TestKeySetHandleHistory(
          const char *handle, int days);

  // general fuction
  bool TestObjectHistory(
    const char * name, int days);

  // save to the history and delete objects
  bool SaveNSSetHistory(
    int id);
  bool DeleteNSSetObject(
    int id);

  bool SaveDomainHistory(
    int id);
  bool DeleteDomainObject(
    int id);

  bool SaveContactHistory(
    int id);
  bool DeleteContactObject(
    int id);

  bool SaveKeySetHistory(int id);
  bool DeleteKeySetObject(int id);

  /// SQL language 

  // SQL UPDATE funkce
  void UPDATE(
    const char * table);

  // set 
  void SSET(
    const char *fname, const char * value); // without escape sequence
  void SET(
    const char *fname, const char * value); // with escape
  void SETS(
    const char *fname, const char * value, bool esc /* , bool null  */);

  void SET(
    const char *fname, long value);
  void SET(
    const char *fname, int value);
  void SET(
    const char *fname, bool value);
  void SETNULL(
    const char *fname);
  void SETBOOL(
    const char *fname, char c);
  void SETPRICE(
    const char *fname, long price);

  void WHERE(
    const char *fname, const char * value);
  void WHERE(
    const char *fname, int value);
  void WHEREOPP(
    const char *op, const char *fname, const char *p, const char * value);
  void OPERATOR(
    const char *op);
  void WHEREID(
    int id)
  {
    WHERE("id", id);
  }
  ;
  //  SQL INSERT fuction
  void INSERTHISTORY(
    const char * table);
  void INSERT(
    const char * table);
  void INTO(
    const char *fname);
  void INTOVAL(
    const char *fname, const char * value);
  void VAL(
    const char * value);
  void VALUESC(
    const char * value);
  void VALUES(
    const char * value, bool esc, bool amp, int uplo); // use esc sequence a use '
  void VALUE(
    const char * value);
  void VVALUE(
    const char * value); // without escape
  void VALUE(
    long value);
  void VALUE(
    int value);
  void VALUE(
    unsigned long long value);
  void VALUE(
    bool value);
  void VALPRICE(
    long price); // price in long

  void VALUELOWER(
    const char * value);
  void VALUEUPPER(
    const char * value);

  // current_timestamop
  void VALUENOW();
  // make interval'%month' 
  void VALUEPERIOD(
    int period);
  // make NULL value
  void VALUENULL();

  // SQL SELECT function
  void SELECTFROM(
    const char *fname, const char * table);
  // function "select field from table where field=value"
  bool SELECTONE(
    const char * table, const char *fname, const char *value);
  bool SELECTONE(
    const char * table, const char *fname, int value);

  // SQL string function
  void SQLCatLower(
    const char *str);
  void SQLCatUpper(
    const char *str);
  // to lower or upper
  void SQLCatLW(
    const char *str, bool lw);

  // del end of the sqlString
  void SQLDelEnd();
  // test the end  sqlBuffer to char c
  bool SQLTestEnd(
    char c);
  // use strcat to sqlBuffer with test on the length
  void SQLCat(
    const char *str);
  // escape function 
  void SQLCatEscape(
    const char * value);

  // run function 
  bool EXEC(); // use ExecSQL
  bool SELECT(); // use ExecSelect

  /// loading contact map for nsset admin a domain admin and tmp_contact
  bool SELECTCONTACTMAP(
    char *map, int id, unsigned role);
  bool SELECTOBJECTID(
    const char *table, const char *fname, int id);

private:
  char *memHandle;
  char *svrTRID;
  char *sqlBuffer;
  char dtStr[MAX_DATE+1]; //  pfor return date
  int actionID; // id from action table
  int historyID; // id from history table
  int loginID; // id of the client action.clientID
  short enum_action; // ID of the EPP operation from enum_action
};//class DB


//to be able to do something, when DB* goes out of scope
typedef boost::shared_ptr<DB> DBSharedPtr;
template < typename DELETER >
class DBPtrT
{
protected:
    DBSharedPtr m_ptr;
public:
    DBPtrT(DB* db)
        : m_ptr(db,DELETER())
    {}

    DBPtrT()
        : m_ptr(0,DELETER())
    {}

    operator DBSharedPtr() const
    {
        return m_ptr;
    }

};
///deleter functor for DB calling FreeSelect only
struct DBFreeSelect
{
    void operator()(DB* db)
    {
        try
        {
            if(db) db->FreeSelect();
        }
        catch(...){}
    }
};
///DBSharedPtr factory
typedef DBPtrT<DBFreeSelect> DBFreeSelectPtr;

#endif
