#ifndef DBSQL_HH_1EE7FA4B5D8E4144AEFA46DDDE716764
#define DBSQL_HH_1EE7FA4B5D8E4144AEFA46DDDE716764

#include "src/deprecated/libfred/db_settings.hh"

#include "src/deprecated/util/util.hh"
#include "src/deprecated/util/pqsql.hh"
#include "libfred/types.hh"

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>

#define LANG_EN 0
#define LANG_CS 1
#define CMD_OK 1000 // OK command to the commit transaction
inline bool is_command_successfully_done(int result) { return result < 2000; } // all successful codes
#define MAX_SQLBUFFER 4096*25 // maximal lenght od the sqlBuffer
#define MAX_SVTID 64 // length of the server  ticket  svTRID

class DB : public PQ
{
public:
  DB(Database::Connection &_conn);

  ~DB();

  //----------------------------
  // EPP function for table action and action_xml


  // start of the EPP operation with clientID and save xml from epp-client 
  bool BeginAction(   
    unsigned long long clientID, int action, const char *clTRID, const char *xml,
    unsigned long long requestID
  );
  // end of the EPP operation
  const char * EndAction(
    int response); // return svrTRID

  char * GetsvTRID()
  {
    return svrTRID;
  }
  // return actual generated server ticket  svrTRID

  int GetEPPAction()
  {
    return enum_action;
  } // return  EPP operation

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
    int objectID, unsigned long long requestID);// create insert into table history
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
    int id, unsigned long long request_id);
  bool DeleteNSSetObject(
    int id);

  bool SaveDomainHistory(
    int id, unsigned long long request_id);
  bool DeleteDomainObject(
    int id);

  bool SaveContactHistory(
    int id, unsigned long long request_id);
  bool DeleteContactObject(
    int id);

  bool SaveKeySetHistory(
    int id, unsigned long long request_id);
  bool DeleteKeySetObject(
    int id);

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
  DB();
  char *memHandle;
  char *svrTRID;
  char *sqlBuffer;
  char dtStr[MAX_DATE+1]; //  pfor return date
  int historyID; // id from history table
  unsigned long long loginID; // id of the client action.clientID
  short enum_action; // ID of the EPP operation from enum_action
};//class DB


//to be able to do something, when DB* goes out of scope
typedef std::shared_ptr<DB> DBSharedPtr;
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
