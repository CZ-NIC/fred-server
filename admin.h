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
  void page(CORBA::Short _v);
  CORBA::Short start();
  CORBA::Short numPages();
};

class ccReg_EPPActions_i : virtual public POA_ccReg::EPPActions, 
                           public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
  Register::Registrar::EPPActionList *eal;
  CORBA::Short registrarFilter;
 public:
  ccReg_EPPActions_i(Register::Registrar::EPPActionList *eal);
  ~ccReg_EPPActions_i();
  ccReg::TableRow* getColumnNames();
  ccReg::TableRow* getRow(CORBA::Short row);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  void reload();
  CORBA::Short registrar();
  void registrar(CORBA::Short _v);
};

class ccReg_Registrars_i : virtual public POA_ccReg::Registrars,
                           public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {  
  Register::Registrar::RegistrarList *rl;
  std::string fulltextFilter;
 public:
  ccReg_Registrars_i(Register::Registrar::RegistrarList *rl);
  ~ccReg_Registrars_i();
  ccReg::TableRow* getColumnNames();
  ccReg::TableRow* getRow(CORBA::Short row);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  void reload();
  char* fulltext();
  void fulltext(const char* _v);
};

class ccReg_Domains_i : virtual public POA_ccReg::Domains, 
                           public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
  Register::Domain::List *dl;
  CORBA::Short registrarFilter;
  CORBA::Short registrantFilter;
 public:
  ccReg_Domains_i(Register::Domain::List *dl);
  ~ccReg_Domains_i();
  ccReg::TableRow* getColumnNames();
  ccReg::TableRow* getRow(CORBA::Short row);
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);
  char* outputCSV();
  CORBA::Short numRows();
  CORBA::Short numColumns();
  void reload();
  CORBA::Short registrar();
  void registrar(CORBA::Short _v);
  CORBA::Short registrant();
  void registrant(CORBA::Short _v);
};

class ccReg_Session_i : public POA_ccReg::Session,
                        public PortableServer::RefCountServantBase {
  ccReg_Registrars_i* reg;
  ccReg_EPPActions_i* eppa;
  ccReg_Domains_i* dm;
  DB db;
  std::auto_ptr<Register::Manager> m;
 public:
  ccReg_Session_i(const std::string& database);
  ~ccReg_Session_i();
  ccReg::Registrars_ptr getRegistrars();
  ccReg::EPPActions_ptr getEPPActions();
  ccReg::Domains_ptr getDomains();
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

  // primitivni vypis
  ccReg::Lists*  ObjectList( char* table , char *fname );
  ccReg::Lists* ListRegistrar();
  ccReg::Lists* ListDomain();
  ccReg::Lists* ListContact();
  ccReg::Lists* ListNSSet();

  /// testovaci fce na typ objektu
  void checkHandle(const char* handle, ccReg::CheckHandleType_out ch);
  
};
