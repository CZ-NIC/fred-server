#ifndef _MAILER_H_
#define _MAILER_H_

#include "ccReg.hh"
#include "register/register.h"
#include "register/invoice.h"
#include "dbsql.h"
#include "mailer_manager.h"
#include "conf.h"
#include <memory>
#include <string>
#include <map>

#define DECL_ATTRIBUTE(name,type,settype,gettype) \
 private: \
  type name##Filter; \
 public: \
  void name(settype _v); \
  gettype name()

#define DECL_ATTRIBUTE_TYPE(name,type) \
  DECL_ATTRIBUTE(name,type,type,type)
  
#define DECL_ATTRIBUTE_STR(name) \
  DECL_ATTRIBUTE(name,std::string,const char *,char *)

#define DECL_ATTRIBUTE_ID(name) \
  DECL_ATTRIBUTE(name,ccReg::TID,ccReg::TID,ccReg::TID)
  
#define DECL_ATTRIBUTE_DATE(name) \
  DECL_ATTRIBUTE(name,ccReg::DateInterval,\
                 const ccReg::DateInterval&,ccReg::DateInterval)

#define DECL_ATTRIBUTE_DATETIME(name) \
  DECL_ATTRIBUTE(name,ccReg::DateTimeInterval,\
                 const ccReg::DateTimeInterval&,ccReg::DateTimeInterval)

#define DECL_PAGETABLE_I \
  ccReg::Table::ColumnHeaders* getColumnHeaders(); \
  ccReg::TableRow* getRow(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);\
  ccReg::TID getRowId(CORBA::Short row) throw (ccReg::Table::INVALID_ROW);\
  void sortByColumn(CORBA::Short column, CORBA::Boolean dir);\
  char* outputCSV();\
  CORBA::Short numRows();\
  CORBA::Short numColumns();\
  void reload();\
  void clear();\
  ccReg::Filter_ptr aFilter();\
  CORBA::ULongLong resultSize()\

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
 public:
  ccReg_EPPActions_i(Register::Registrar::EPPActionList *eal);
  ~ccReg_EPPActions_i();
  DECL_PAGETABLE_I;
  DECL_ATTRIBUTE_ID(registrar);
  DECL_ATTRIBUTE_STR(registrarHandle);
  DECL_ATTRIBUTE_STR(type);
  DECL_ATTRIBUTE_STR(handle);
  DECL_ATTRIBUTE_STR(xml);
  DECL_ATTRIBUTE_TYPE(result,CORBA::Short);
  DECL_ATTRIBUTE_DATETIME(time);
  DECL_ATTRIBUTE_STR(clTRID);
  DECL_ATTRIBUTE_STR(svTRID);
  DECL_ATTRIBUTE_TYPE(resultClass,ccReg::EPPActionsFilter::ResultType);
};

class ccReg_Registrars_i : virtual public POA_ccReg::Registrars,
                           public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {  
  Register::Registrar::RegistrarList *rl;
 public:
  ccReg_Registrars_i(Register::Registrar::RegistrarList *rl);
  ~ccReg_Registrars_i();
  DECL_PAGETABLE_I;
  DECL_ATTRIBUTE_STR(fulltext);
  DECL_ATTRIBUTE_STR(name);
  DECL_ATTRIBUTE_STR(handle);
};

class ccReg_RegObjectFilter_i : virtual public POA_ccReg::RegObjectFilter {
  Register::ObjectList *ol;
 public:
  ccReg_RegObjectFilter_i(Register::ObjectList *_ol);
  DECL_ATTRIBUTE_ID(registrar);
  DECL_ATTRIBUTE_STR(registrarHandle);
  DECL_ATTRIBUTE_ID(createRegistrar);
  DECL_ATTRIBUTE_STR(createRegistrarHandle);
  DECL_ATTRIBUTE_ID(updateRegistrar);
  DECL_ATTRIBUTE_STR(updateRegistrarHandle);
  DECL_ATTRIBUTE_DATE(crDate);
  DECL_ATTRIBUTE_DATE(upDate);
  DECL_ATTRIBUTE_DATE(trDate);
  DECL_ATTRIBUTE(status, ccReg::ObjectStatusSeq,
                 const ccReg::ObjectStatusSeq&, ccReg::ObjectStatusSeq *);
  void clear();
};

class ccReg_Domains_i : virtual public POA_ccReg::Domains,
                        virtual public ccReg_RegObjectFilter_i,
                        public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
  std::auto_ptr<Register::Domain::List> dl;
 public:
  ccReg_Domains_i(Register::Domain::List *dl);
  ~ccReg_Domains_i();
  DECL_PAGETABLE_I;
  DECL_ATTRIBUTE_ID(registrant);
  DECL_ATTRIBUTE_STR(registrantHandle);
  DECL_ATTRIBUTE_ID(nsset);
  DECL_ATTRIBUTE_STR(nssetHandle);
  DECL_ATTRIBUTE_ID(admin);
  DECL_ATTRIBUTE_STR(adminHandle);
  DECL_ATTRIBUTE_STR(fqdn);
  DECL_ATTRIBUTE_DATE(exDate);
  DECL_ATTRIBUTE_DATE(valExDate);
  DECL_ATTRIBUTE_STR(techAdmin);
  DECL_ATTRIBUTE_STR(techAdminHandle);
  DECL_ATTRIBUTE_STR(nssetIP);
};

class ccReg_Contacts_i : virtual public POA_ccReg::Contacts, 
                         virtual public ccReg_RegObjectFilter_i,
                         public ccReg_PageTable_i,
                         public PortableServer::RefCountServantBase {
  std::auto_ptr<Register::Contact::List> cl;
 public:
  ccReg_Contacts_i(Register::Contact::List *cl);
  ~ccReg_Contacts_i();
  DECL_PAGETABLE_I;
  DECL_ATTRIBUTE_STR(handle);
  DECL_ATTRIBUTE_STR(name);
  DECL_ATTRIBUTE_STR(org);
  DECL_ATTRIBUTE_STR(ident);
  DECL_ATTRIBUTE_STR(email);
  DECL_ATTRIBUTE_STR(vat);
};

class ccReg_NSSets_i : virtual public POA_ccReg::NSSets, 
                       virtual public ccReg_RegObjectFilter_i,
                       public ccReg_PageTable_i,
                       public PortableServer::RefCountServantBase 
{
  std::auto_ptr<Register::NSSet::List> nl;
 public:
  ccReg_NSSets_i(Register::NSSet::List *nl);
  ~ccReg_NSSets_i();
  DECL_PAGETABLE_I;
  DECL_ATTRIBUTE_STR(handle);
  DECL_ATTRIBUTE_STR(adminHandle);
  DECL_ATTRIBUTE_STR(hostname);
  DECL_ATTRIBUTE_STR(ip);
};

class ccReg_AIRequests_i : virtual public POA_ccReg::AuthInfoRequests, 
                           public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
  Register::AuthInfoRequest::List *airl;
 public:
  ccReg_AIRequests_i(Register::AuthInfoRequest::List *_airl);
  ~ccReg_AIRequests_i();
  DECL_PAGETABLE_I;
  DECL_ATTRIBUTE_ID(id);
  DECL_ATTRIBUTE_STR(handle);
  DECL_ATTRIBUTE_TYPE(status,ccReg::AuthInfoRequest::RequestStatus);
  DECL_ATTRIBUTE_TYPE(type,ccReg::AuthInfoRequest::RequestType);
  DECL_ATTRIBUTE_DATETIME(crTime);
  DECL_ATTRIBUTE_DATETIME(closeTime);
  DECL_ATTRIBUTE_STR(reason);
  DECL_ATTRIBUTE_STR(svTRID);
  DECL_ATTRIBUTE_STR(email);
};

class ccReg_Mails_i : virtual public POA_ccReg::Mails, 
                      public ccReg_PageTable_i,
                      public PortableServer::RefCountServantBase 
{
  MailerManager mm;
 public:
  ccReg_Mails_i(NameService *ns);
  ~ccReg_Mails_i();
  DECL_PAGETABLE_I;
  DECL_ATTRIBUTE_ID(id);
  DECL_ATTRIBUTE_TYPE(status,CORBA::Long);
  DECL_ATTRIBUTE_TYPE(type,CORBA::UShort);
  DECL_ATTRIBUTE_STR(handle);
  DECL_ATTRIBUTE_STR(fulltext);
  DECL_ATTRIBUTE_STR(attachment);
  DECL_ATTRIBUTE_DATETIME(createTime);
};
  
class ccReg_Invoices_i : virtual public POA_ccReg::Invoices, 
                         public ccReg_PageTable_i,
                         public PortableServer::RefCountServantBase 
{
  std::auto_ptr<Register::Invoicing::InvoiceList> invl;
 public:
  ccReg_Invoices_i(Register::Invoicing::InvoiceList* _invl);
  ~ccReg_Invoices_i();
  DECL_PAGETABLE_I;
  DECL_ATTRIBUTE_ID(id);
  DECL_ATTRIBUTE_STR(number);
  DECL_ATTRIBUTE_DATE(crDate);
  DECL_ATTRIBUTE_ID(registrarId);
  DECL_ATTRIBUTE_STR(registrarHandle);
  DECL_ATTRIBUTE_ID(zone);
  DECL_ATTRIBUTE_TYPE(type,ccReg::Invoicing::InvoiceType);
  DECL_ATTRIBUTE_STR(varSymbol);
  DECL_ATTRIBUTE_DATE(taxDate);
  DECL_ATTRIBUTE_STR(objectName);
  DECL_ATTRIBUTE_ID(objectId);
  DECL_ATTRIBUTE_STR(advanceNumber);
};

class ccReg_Session_i : public POA_ccReg::Session,
                        public PortableServer::RefCountServantBase 
{
  ccReg_Registrars_i* reg;
  ccReg_EPPActions_i* eppa;
  ccReg_Domains_i* dm;
  ccReg_Contacts_i* cm;
  ccReg_NSSets_i* nm;
  ccReg_AIRequests_i* airm;
  ccReg_Mails_i* mml;
  ccReg_Invoices_i* invl;
  DB db;
  std::auto_ptr<Register::Manager> m;
  std::auto_ptr<Register::AuthInfoRequest::Manager> am;
  std::auto_ptr<Register::Document::Manager> docman;
  std::auto_ptr<Register::Invoicing::Manager> invm;
  MailerManager mm;
 public:
  ccReg_Session_i(const std::string& database, NameService *ns, Conf& cfg);
  ~ccReg_Session_i();
  ccReg::Registrars_ptr getRegistrars();
  ccReg::EPPActions_ptr getEPPActions();
  ccReg::Domains_ptr getDomains();
  ccReg::Contacts_ptr getContacts();
  ccReg::NSSets_ptr getNSSets();
  ccReg::AuthInfoRequests_ptr getAuthInfoRequests();
  ccReg::Mails_ptr getMails();
  ccReg::Invoices_ptr getInvoices();
};

class NameService;
// interface Admin implementation
class ccReg_Admin_i: public POA_ccReg::Admin, 
                     public PortableServer::RefCountServantBase 
{
  std::string database;
  typedef std::map<std::string,ccReg_Session_i *> SessionListType;
  SessionListType sessionList;
  void fillRegistrar(
    ccReg::Registrar& creg, 
    Register::Registrar::Registrar *reg
  ); 
  NameService *ns;
  Conf& cfg;
 public:
  ccReg_Admin_i(const std::string database, NameService *ns, Conf& _cfg);
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
  ccReg::DomainDetails* getDomainsByInverseKey(
    const char* key, ccReg::DomainInvKeyType type, CORBA::Long limit
  );
  ccReg::NSSetDetails* getNSSetsByInverseKey(
    const char* key, ccReg::NSSetInvKeyType type, CORBA::Long limit
  );
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
  ccReg::Admin::Buffer* getAuthInfoRequestPDF(ccReg::TID id, const char *lang);
  void fillInvoice(
    ccReg::Invoicing::Invoice *ci, Register::Invoicing::Invoice *i
  );  
  ccReg::Invoicing::Invoice* getInvoiceById(ccReg::TID id)
    throw (ccReg::Admin::ObjectNotFound);   
};

#endif
