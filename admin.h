#ifndef _MAILER_H_
#define _MAILER_H_

#include "ccReg.hh"
#include "register/register.h"
#include "dbsql.h"
#include "mailer_manager.h"
#include <memory>
#include <string>
#include <map>

class ccReg_PageTable_i : virtual public POA_ccReg::PageTable {
  unsigned int aPageSize;
  unsigned int aPage;
 public:
  ccReg_PageTable_i();
  CORBA::Short pageSize();
  void pageSize(CORBA::Short _v);
  CORBA::Short page();
  void setPage(CORBA::Short page) throw (ccReg::PageTable::INVALID_PAGE);
  CORBA::Short start();
  CORBA::Short numPages();
  ccReg::TableRow* getPageRow(CORBA::Short pageRow) 
    throw (ccReg::Table::INVALID_ROW);
  CORBA::Short numPageRows();
  ccReg::TID getPageRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
};

class ccReg_EPPActions_i : virtual public POA_ccReg::EPPActions, 
                           public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
  Register::Registrar::EPPActionList *eal;
  ccReg::TID registrarFilter;
  std::string registrarHandleFilter;
  std::string typeFilter;
  std::string handleFilter;
  CORBA::Short resultFilter;
  ccReg::DateTimeInterval timeFilter;
  std::string clTRIDFilter;
  std::string svTRIDFilter;
  std::string xmlFilter;
  ccReg::EPPActionsFilter::ResultType resultClassFilter;
 public:
  ccReg_EPPActions_i(Register::Registrar::EPPActionList *eal);
  ~ccReg_EPPActions_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  ccReg::TID getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  ccReg::TID registrar();
  void registrar(ccReg::TID _v);
  char* registrarHandle();
  void registrarHandle(const char* _v);
  char* type();
  void type(const char* _v);
  char* handle();
  void handle(const char* _v);
  char* xml();
  void xml(const char* _v);
  CORBA::Short result();
  void result(CORBA::Short _v);
  ccReg::DateTimeInterval time();
  void time(const ccReg::DateTimeInterval& _v);
  char* clTRID();
  void clTRID(const char* _v);
  char* svTRID();
  void svTRID(const char* _v);
  ccReg::EPPActionsFilter::ResultType resultClass();
  void resultClass(ccReg::EPPActionsFilter::ResultType _v);
  void reload();
  void clear();
  ccReg::Filter_ptr aFilter();
  CORBA::ULongLong resultSize();
};

class ccReg_Registrars_i : virtual public POA_ccReg::Registrars,
                           public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {  
  Register::Registrar::RegistrarList *rl;
  std::string fulltextFilter;
  std::string nameFilter;
  std::string handleFilter;
 public:
  ccReg_Registrars_i(Register::Registrar::RegistrarList *rl);
  ~ccReg_Registrars_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  ccReg::TID getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  char* fulltext();
  void fulltext(const char* _v);
  char* name();
  void name(const char* _v);
  char* handle();
  void handle(const char* _v);
  ccReg::Filter_ptr aFilter();
  void reload();
  void clear();
  CORBA::ULongLong resultSize();
};

class ccReg_RegObjectFilter_i : virtual public POA_ccReg::RegObjectFilter {
  ccReg::TID registrarFilter;
  std::string registrarHandleFilter;
  ccReg::TID createRegistrarFilter;
  std::string createRegistrarHandleFilter;
  ccReg::TID updateRegistrarFilter;
  std::string updateRegistrarHandleFilter;
  ccReg::DateInterval crDateFilter;
  ccReg::DateInterval upDateFilter;
  ccReg::DateInterval trDateFilter;
  ccReg::ObjectStatusSeq statusFilter;
  Register::ObjectList *ol;
 public:
  ccReg_RegObjectFilter_i(Register::ObjectList *_ol);
  ccReg::TID registrar();
  void registrar(ccReg::TID _v);
  char* registrarHandle();
  void registrarHandle(const char* _v);
  ccReg::TID createRegistrar();
  void createRegistrar(ccReg::TID _v);
  char* createRegistrarHandle();
  void createRegistrarHandle(const char* _v);
  ccReg::TID updateRegistrar();
  void updateRegistrar(ccReg::TID _v);
  char* updateRegistrarHandle();
  void updateRegistrarHandle(const char* _v);
  ccReg::DateInterval crDate();
  void crDate(const ccReg::DateInterval& _v);
  ccReg::DateInterval upDate();
  void upDate(const ccReg::DateInterval& _v);
  ccReg::DateInterval trDate();
  void trDate(const ccReg::DateInterval& _v);
  ccReg::ObjectStatusSeq *status();
  void status(const ccReg::ObjectStatusSeq& _v);
  void clear();
};

class ccReg_Domains_i : virtual public POA_ccReg::Domains,
                        virtual public ccReg_RegObjectFilter_i,
                        public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
  Register::Domain::List *dl;
  ccReg::TID registrantFilter;
  std::string registrantHandleFilter;
  ccReg::TID nssetFilter;
  std::string nssetHandleFilter;
  ccReg::TID adminFilter;
  std::string adminHandleFilter;
  std::string fqdnFilter;
  ccReg::DateInterval exDateFilter;
  ccReg::DateInterval valExDateFilter;
  std::string techAdminHandleFilter;
  std::string nssetIPFilter;
 public:
  ccReg_Domains_i(Register::Domain::List *dl);
  ~ccReg_Domains_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  ccReg::TID getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  void reload();
  ccReg::TID registrant();
  void registrant(ccReg::TID _v);
  char* registrantHandle();
  void registrantHandle(const char* _v);
  ccReg::TID nsset();
  void nsset(ccReg::TID _v);
  char* nssetHandle();
  void nssetHandle(const char* _v);
  ccReg::TID admin();
  void admin(ccReg::TID _v);
  char* adminHandle();
  void adminHandle(const char* _v);
  char* fqdn();
  void fqdn(const char* _v);
  ccReg::DateInterval exDate();
  void exDate(const ccReg::DateInterval& _v);
  ccReg::DateInterval valExDate();
  void valExDate(const ccReg::DateInterval& _v);
  char *techAdminHandle();
  void techAdminHandle(const char * _v);
  char *nssetIP();
  void nssetIP(const char *_v);
  ccReg::Filter_ptr aFilter();  
  void clear();
  CORBA::ULongLong resultSize();
};

class ccReg_Contacts_i : virtual public POA_ccReg::Contacts, 
                         virtual public ccReg_RegObjectFilter_i,
                         public ccReg_PageTable_i,
                         public PortableServer::RefCountServantBase {
  Register::Contact::List *cl;
  std::string handleFilter;
  std::string nameFilter;
  std::string orgFilter;
  std::string emailFilter;
  std::string identFilter;
  std::string vatFilter;
 public:
  ccReg_Contacts_i(Register::Contact::List *cl);
  ~ccReg_Contacts_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  ccReg::TID getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  char* handle();
  void handle(const char* _v);
  char* name();
  void name(const char* _v);
  char* org();
  void org(const char* _v);
  char* ident();
  void ident(const char* _v);
  char* email();
  void email(const char* _v);
  char* vat();
  void vat(const char* _v);
  void reload();
  ccReg::Filter_ptr aFilter();  
  void clear();
  CORBA::ULongLong resultSize();
};

class ccReg_NSSets_i : virtual public POA_ccReg::NSSets, 
                       virtual public ccReg_RegObjectFilter_i,
                       public ccReg_PageTable_i,
                       public PortableServer::RefCountServantBase {
  Register::NSSet::List *nl;
  std::string handleFilter;
  std::string adminHandleFilter;
  std::string hostnameFilter;
  std::string ipFilter;
 public:
  ccReg_NSSets_i(Register::NSSet::List *dl);
  ~ccReg_NSSets_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  ccReg::TID getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  char* handle();
  void handle(const char* _v);
  char* adminHandle();
  void adminHandle(const char* _v);
  char* hostname();
  void hostname(const char* _v);
  char* ip();
  void ip(const char* _v);
  void reload();
  ccReg::Filter_ptr aFilter();  
  void clear();
  CORBA::ULongLong resultSize();
};


class ccReg_AIRequests_i : virtual public POA_ccReg::AuthInfoRequests, 
                           public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
  Register::AuthInfoRequest::List *airl;
  ccReg::TID idFilter;
  std::string handleFilter;
  ccReg::DateTimeInterval crTimeFilter;
  ccReg::DateTimeInterval closeTimeFilter;
  std::string svTRIDFilter;
  std::string emailFilter;
  std::string reasonFilter;
  ccReg::AuthInfoRequest::RequestType requestTypeFilter;
  ccReg::AuthInfoRequest::RequestStatus requestStatusFilter;
 public:
  ccReg_AIRequests_i(Register::AuthInfoRequest::List *_airl);
  ~ccReg_AIRequests_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  ccReg::TID getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  ccReg::TID id();
  void id(ccReg::TID _v);
  char* handle();
  void handle(const char* _v);
  ccReg::AuthInfoRequest::RequestStatus status();
  void status(ccReg::AuthInfoRequest::RequestStatus _v);
  ccReg::AuthInfoRequest::RequestType type();
  void type(ccReg::AuthInfoRequest::RequestType _v);
  ccReg::DateTimeInterval crTime();
  void crTime(const ccReg::DateTimeInterval& _v);
  ccReg::DateTimeInterval closeTime();
  void closeTime(const ccReg::DateTimeInterval& _v);
  char* reason();
  void reason(const char* _v);
  char* svTRID();
  void svTRID(const char* _v);
  char* email();
  void email(const char* _v);
  void reload();
  void clear();
  ccReg::Filter_ptr aFilter();
  CORBA::ULongLong resultSize();
};

class ccReg_Mails_i : virtual public POA_ccReg::Mails, 
                      public ccReg_PageTable_i,
                      public PortableServer::RefCountServantBase {
  ccReg::TID idFilter;
  CORBA::Long statusFilter;
  CORBA::UShort typeFilter;
  std::string fulltextFilter;
  std::string handleFilter;
  std::string attachmentFilter;
  ccReg::DateTimeInterval createTimeFilter;
  MailerManager mm;
 public:
  ccReg_Mails_i(NameService *ns);
  ~ccReg_Mails_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  ccReg::TID getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  ccReg::TID id();
  void id(ccReg::TID _v);
  CORBA::Long status();
  void status(CORBA::Long _v);
  CORBA::UShort type();
  void type(CORBA::UShort _v);
  char* handle();
  void handle(const char* _v);
  char* fulltext();
  void fulltext(const char* _v);
  char* attachment();
  void attachment(const char* _v);
  ccReg::DateTimeInterval createTime();
  void createTime(const ccReg::DateTimeInterval& _v);
  void reload();
  void clear();
  ccReg::Filter_ptr aFilter();
  CORBA::ULongLong resultSize();
};

class ccReg_Session_i : public POA_ccReg::Session,
                        public PortableServer::RefCountServantBase {
  ccReg_Registrars_i* reg;
  ccReg_EPPActions_i* eppa;
  ccReg_Domains_i* dm;
  ccReg_Contacts_i* cm;
  ccReg_NSSets_i* nm;
  ccReg_AIRequests_i* airm;
  ccReg_Mails_i* mml;
  DB db;
  std::auto_ptr<Register::Manager> m;
  std::auto_ptr<Register::AuthInfoRequest::Manager> am;
  MailerManager mm;
 public:
  ccReg_Session_i(const std::string& database, NameService *ns);
  ~ccReg_Session_i();
  ccReg::Registrars_ptr getRegistrars();
  ccReg::EPPActions_ptr getEPPActions();
  ccReg::Domains_ptr getDomains();
  ccReg::Contacts_ptr getContacts();
  ccReg::NSSets_ptr getNSSets();
  ccReg::AuthInfoRequests_ptr getAuthInfoRequests();
  ccReg::Mails_ptr getMails();
};

class NameService;
// interface Admin implementation
class ccReg_Admin_i: public POA_ccReg::Admin, 
                     public PortableServer::RefCountServantBase {
  std::string database;
  typedef std::map<std::string,ccReg_Session_i *> SessionListType;
  SessionListType sessionList;
  void fillRegistrar(
    ccReg::Registrar& creg, 
    Register::Registrar::Registrar *reg
  ); 
  NameService *ns;
 public:
  ccReg_Admin_i(const std::string database, NameService *ns);
  virtual ~ccReg_Admin_i();

  // session
  virtual char* login(const char* username, const char* password)
    throw (ccReg::Admin::AuthFailed);
  virtual ccReg::Session_ptr getSession(const char* sessionID)
    throw (ccReg::Admin::ObjectNotFound);

  // registrar management
  ccReg::RegistrarList* getRegistrars();
  ccReg::Registrar* getRegistrarById(ccReg::TID id) 
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::Registrar* getRegistrarByHandle(const char* handle) 
    throw (ccReg::Admin::ObjectNotFound);
  void putRegistrar(const ccReg::Registrar& regData);
  
  // object detail
  void fillContact(ccReg::ContactDetail* cv, Register::Contact::Contact* c);
  ccReg::ContactDetail* getContactByHandle(const char* handle)
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::ContactDetail* getContactById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound);
  void fillNSSet(ccReg::NSSetDetail* cn, Register::NSSet::NSSet* n);
  ccReg::NSSetDetail* getNSSetByHandle(const char* handle)
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::NSSetDetail* getNSSetById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound);
  void fillDomain(ccReg::DomainDetail* cd, Register::Domain::Domain* d);
  ccReg::DomainDetail* getDomainByFQDN(const char* fqdn)
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::DomainDetail* getDomainById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound);
  void fillEPPAction(
    ccReg::EPPAction* cea, 
    const Register::Registrar::EPPAction *rea
  );
  ccReg::EPPAction* getEPPActionById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::EPPAction* getEPPActionBySvTRID(const char* svTRID)
    throw (ccReg::Admin::ObjectNotFound);
  void fillAuthInfoRequest(
    ccReg::AuthInfoRequest::Detail *carid,
    Register::AuthInfoRequest::Detail *rarid
  );
  ccReg::AuthInfoRequest::Detail* getAuthInfoRequestById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::Mailing::Detail* getEmailById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound);


  // statistics
  CORBA::Long getEnumDomainCount();
  CORBA::Long getEnumNumberCount();
  
  // counters
  ccReg::EPPActionTypeSeq* getEPPActionTypeList();
  ccReg::CountryDescSeq* getCountryDescList();
  char* getDefaultCountry();
  ccReg::ObjectStatusDescSeq* getDomainStatusDescList();
  ccReg::ObjectStatusDescSeq* getContactStatusDescList();
  ccReg::ObjectStatusDescSeq* getNSSetStatusDescList();
  /// testovaci fce na typ objektu
  void checkHandle(const char* handle, ccReg::CheckHandleTypeSeq_out ch);
  ccReg::TID createAuthInfoRequest(
    ccReg::TID objectId, 
    ccReg::AuthInfoRequest::RequestType type, 
    ccReg::TID eppActionId, 
    const char* requestReason,
    const char* emailToAnswer
  ) throw (
    ccReg::Admin::BAD_EMAIL, ccReg::Admin::OBJECT_NOT_FOUND, 
    ccReg::Admin::ACTION_NOT_FOUND, ccReg::Admin::SQL_ERROR, 
    ccReg::Admin::INVALID_INPUT
  );
  void processAuthInfoRequest(ccReg::TID id, CORBA::Boolean invalid) 
    throw (ccReg::Admin::SQL_ERROR, ccReg::Admin::OBJECT_NOT_FOUND);
};

#endif
