#include "ccReg.hh"
#include "register/register.h"
#include "dbsql.h"
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
  CORBA::Long getPageRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
};

class ccReg_EPPActions_i : virtual public POA_ccReg::EPPActions, 
                           public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
  Register::Registrar::EPPActionList *eal;
  CORBA::Short registrarFilter;
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
  CORBA::Long getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  CORBA::Short registrar();
  void registrar(CORBA::Short _v);
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
  CORBA::Long getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
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
};

class ccReg_RegObjectFilter_i : virtual public POA_ccReg::RegObjectFilter {
  CORBA::Short registrarFilter;
  std::string registrarHandleFilter;
  CORBA::Short createRegistrarFilter;
  std::string createRegistrarHandleFilter;
  CORBA::Short updateRegistrarFilter;
  std::string updateRegistrarHandleFilter;
  ccReg::DateInterval crDateFilter;
  ccReg::DateInterval upDateFilter;
  ccReg::DateInterval trDateFilter;
  ccReg::ObjectStatusSeq statusFilter;
  Register::ObjectList *ol;
 public:
  ccReg_RegObjectFilter_i(Register::ObjectList *_ol);
  CORBA::Short registrar();
  void registrar(CORBA::Short _v);
  char* registrarHandle();
  void registrarHandle(const char* _v);
  CORBA::Short createRegistrar();
  void createRegistrar(CORBA::Short _v);
  char* createRegistrarHandle();
  void createRegistrarHandle(const char* _v);
  CORBA::Short updateRegistrar();
  void updateRegistrar(CORBA::Short _v);
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
  CORBA::Short registrantFilter;
  std::string registrantHandleFilter;
  CORBA::Short nssetFilter;
  std::string nssetHandleFilter;
  CORBA::Short adminFilter;
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
  CORBA::Long getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  void reload();
  CORBA::Short registrant();
  void registrant(CORBA::Short _v);
  char* registrantHandle();
  void registrantHandle(const char* _v);
  CORBA::Short nsset();
  void nsset(CORBA::Short _v);
  char* nssetHandle();
  void nssetHandle(const char* _v);
  CORBA::Short admin();
  void admin(CORBA::Short _v);
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
  CORBA::Long getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
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
  CORBA::Long getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
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
};

class ccReg_Session_i : public POA_ccReg::Session,
                        public PortableServer::RefCountServantBase {
  ccReg_Registrars_i* reg;
  ccReg_EPPActions_i* eppa;
  ccReg_Domains_i* dm;
  ccReg_Contacts_i* cm;
  ccReg_NSSets_i* nm;
  DB db;
  std::auto_ptr<Register::Manager> m;
 public:
  ccReg_Session_i(const std::string& database);
  ~ccReg_Session_i();
  ccReg::Registrars_ptr getRegistrars();
  ccReg::EPPActions_ptr getEPPActions();
  ccReg::Domains_ptr getDomains();
  ccReg::Contacts_ptr getContacts();
  ccReg::NSSets_ptr getNSSets();
};

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
 public:
  ccReg_Admin_i(const std::string database);
  virtual ~ccReg_Admin_i();

  // session
  virtual char* login(const char* username, const char* password)
    throw (ccReg::Admin::AuthFailed);
  virtual ccReg::Session_ptr getSession(const char* sessionID)
    throw (ccReg::Admin::ObjectNotFound);

  // registrar management
  ccReg::RegistrarList* getRegistrars();
  ccReg::Registrar* getRegistrarById(CORBA::Long id) 
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::Registrar* getRegistrarByHandle(const char* handle) 
    throw (ccReg::Admin::ObjectNotFound);
  void putRegistrar(const ccReg::Registrar& regData);
  
  // object detail
  void fillContact(ccReg::ContactDetail* cv, Register::Contact::Contact* c);
  ccReg::ContactDetail* getContactByHandle(const char* handle)
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::ContactDetail* getContactById(CORBA::Long id)
    throw (ccReg::Admin::ObjectNotFound);
  void fillNSSet(ccReg::NSSetDetail* cn, Register::NSSet::NSSet* n);
  ccReg::NSSetDetail* getNSSetByHandle(const char* handle)
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::NSSetDetail* getNSSetById(CORBA::Long id)
    throw (ccReg::Admin::ObjectNotFound);
  void fillDomain(ccReg::DomainDetail* cd, Register::Domain::Domain* d);
  ccReg::DomainDetail* getDomainByFQDN(const char* fqdn)
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::DomainDetail* getDomainById(CORBA::Long id)
    throw (ccReg::Admin::ObjectNotFound);
  ccReg::EPPAction* getEPPActionById(CORBA::Long id)
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
  void checkHandle(const char* handle, ccReg::CheckHandleType_out ch);
  
  
};
