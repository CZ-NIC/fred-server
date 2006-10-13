#include "ccReg.hh"
#include <string>
#include "register/register.h"
#include "dbsql.h"
#include <memory>

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
  ccReg::DateInterval timeFilter;
  std::string clTRIDFilter;
  std::string svTRIDFilter;
 public:
  ccReg_EPPActions_i(Register::Registrar::EPPActionList *eal);
  ~ccReg_EPPActions_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
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
  CORBA::Short result();
  void result(CORBA::Short _v);
  ccReg::DateInterval time();
  void time(const ccReg::DateInterval& _v);
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
  ccReg::DateInterval crDateFilter;
  Register::ObjectList *ol;
 public:
  ccReg_RegObjectFilter_i(Register::ObjectList *_ol);
  CORBA::Short registrar();
  void registrar(CORBA::Short _v);
  char* registrarHandle();
  void registrarHandle(const char* _v);
  ccReg::DateInterval crDate();
  void crDate(const ccReg::DateInterval& _v);
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
 public:
  ccReg_Domains_i(Register::Domain::List *dl);
  ~ccReg_Domains_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
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
  ccReg::Filter_ptr aFilter();  
  void clear();
};

class ccReg_Contacts_i : virtual public POA_ccReg::Contacts, 
                         virtual public ccReg_RegObjectFilter_i,
                         public ccReg_PageTable_i,
                         public PortableServer::RefCountServantBase {
  Register::Contact::List *cl;
  std::string handleFilter;
 public:
  ccReg_Contacts_i(Register::Contact::List *cl);
  ~ccReg_Contacts_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  char* handle();
  void handle(const char* _v);
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
 public:
  ccReg_NSSets_i(Register::NSSet::List *dl);
  ~ccReg_NSSets_i();
  ccReg::Table::ColumnHeaders* getColumnHeaders();
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  char* handle();
  void handle(const char* _v);
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

// implementace interface Admin
class ccReg_Admin_i: public POA_ccReg::Admin, 
                     public PortableServer::RefCountServantBase {
  std::string database;
  ccReg_Session_i *session;
 public:
  ccReg_Admin_i(const std::string database);
  virtual ~ccReg_Admin_i();

  //session
  virtual char* login(const char* username, const char* password);
  virtual ccReg::Session_ptr getSession(const char* sessionID);

  // vypis registratoru
  ccReg::RegistrarList* getRegistrars();
  ccReg::Registrar* getRegistrarByHandle(const char* handle) 
    throw (ccReg::Admin::ObjectNotFound);

  void putRegistrar(const ccReg::Registrar& regData);
  ccReg::RegObject* getContactByHandle(const char* handle);
  ccReg::RegObject* getNSSetByHandle(const char* handle);
  ccReg::RegObject* getDomainByFQDN(const char* fqdn);

  /// testovaci fce na typ objektu
  void checkHandle(const char* handle, ccReg::CheckHandleType_out ch);
  
};
