#include "admin.h"
#include "log.h"
#include "util.h"
#include "dbsql.h"
#include "register/register.h"
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <math.h>
#include <memory>
#include <iomanip>

using namespace boost::posix_time;
using namespace boost::gregorian;

std::string 
formatTime(ptime p,bool date)
{
  if (p.is_special()) return "";
  std::ostringstream stime;
  stime << std::setfill('0') << std::setw(2)
        << p.date().day() << "." 
        << std::setw(2)
        << (int)p.date().month() << "." 
        << std::setw(2)
        << p.date().year();
  if (date) 
   stime << " "
         << std::setw(2)
         << p.time_of_day().hours() << ":"
         << std::setw(2)
         << p.time_of_day().minutes() << ":"
         << std::setw(2)
         << p.time_of_day().seconds();
  return stime.str();
}

#define DUPSTR(s) CORBA::string_dup(s)
#define DUPSTRC(s) CORBA::string_dup(s.c_str())
#define DUPSTRFUN(f) DUPSTRC(f())
//#define DUPSTRDATE(f) DUPSTR(to_simple_string(f()).c_str())
#define DUPSTRDATE(f) DUPSTRC(formatTime(f(),true))
#define DUPSTRDATESHORT(f) DUPSTRC(formatTime(f(),false))
#define COLHEAD(x,i,title,tp) \
  (*x)[i].name = DUPSTR(title); \
  (*x)[i].type = ccReg::Table::tp 

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_PageTable_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_PageTable_i::ccReg_PageTable_i()
  : aPageSize(10), aPage(0)
{
}

CORBA::Short 
ccReg_PageTable_i::pageSize()
{
  return aPageSize;
}

void 
ccReg_PageTable_i::pageSize(CORBA::Short _v)
{
  aPageSize = _v;
  aPage = 0;
}

CORBA::Short 
ccReg_PageTable_i::page()
{
  return aPage;
}

void 
ccReg_PageTable_i::setPage(CORBA::Short _v) 
  throw (ccReg::PageTable::INVALID_PAGE)
{
  aPage = _v;
}

CORBA::Short 
ccReg_PageTable_i::start()
{
  return aPage*aPageSize;
}

CORBA::Short 
ccReg_PageTable_i::numPages()
{
  return (unsigned)ceil((double)numRows()/aPageSize);
}

ccReg::TableRow* 
ccReg_PageTable_i::getPageRow(CORBA::Short pageRow)
  throw (ccReg::Table::INVALID_ROW)
{
  return getRow(pageRow + start());
}

CORBA::Short 
ccReg_PageTable_i::numPageRows()
{
  unsigned s = start();
  unsigned n = numRows();
  if (s > n) return 0; /// something wrong
  unsigned l = n - s;
  return l < aPageSize ? l : aPageSize;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_Admin_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_Admin_i::ccReg_Admin_i(const std::string _database) : 
  database(_database), session(new ccReg_Session_i(_database))
{
}

ccReg_Admin_i::~ccReg_Admin_i() 
{}

#define SWITCH_CONVERT(x) case Register::x : ch->handleClass = ccReg::x; break
void
ccReg_Admin_i::checkHandle(const char* handle, ccReg::CheckHandleType_out ch)
{
  std::auto_ptr<Register::Manager> r(Register::Manager::create(NULL));
  Register::CheckHandle chd;
  r->checkHandle(handle,chd);
  ch = new ccReg::CheckHandleType;
  ch->newHandle = CORBA::string_dup(chd.newHandle.c_str());
  switch (chd.handleClass) {
    SWITCH_CONVERT(CH_ENUM_BAD_ZONE);
    SWITCH_CONVERT(CH_ENUM); 
    SWITCH_CONVERT(CH_DOMAIN_PART); 
    SWITCH_CONVERT(CH_DOMAIN_BAD_ZONE); 
    SWITCH_CONVERT(CH_DOMAIN_LONG); 
    SWITCH_CONVERT(CH_DOMAIN);
    SWITCH_CONVERT(CH_NSSET);
    SWITCH_CONVERT(CH_CONTACT);
    SWITCH_CONVERT(CH_INVALID);
  } 
}
 
char* 
ccReg_Admin_i::login(const char* username, const char* password)
{
  return CORBA::string_dup("XXX");
}

ccReg::Session_ptr 
ccReg_Admin_i::getSession(const char* sessionID)
{
  return session->_this();
}

void 
ccReg_Admin_i::fillRegistrar(
  ccReg::Registrar& creg, Register::Registrar::Registrar *reg
)
{
  creg.id = reg->getId();
  creg.name = DUPSTRFUN(reg->getName);
  creg.handle = DUPSTRFUN(reg->getHandle);
  creg.url = DUPSTRFUN(reg->getURL);
  creg.organization = DUPSTRFUN(reg->getOrganization);
  creg.street1 = DUPSTRFUN(reg->getStreet1);
  creg.street2 = DUPSTRFUN(reg->getStreet2);
  creg.street3 = DUPSTRFUN(reg->getStreet3);
  creg.city = DUPSTRFUN(reg->getCity);
  creg.postalcode = DUPSTRFUN(reg->getPostalCode);
  creg.stateorprovince = DUPSTRFUN(reg->getProvince);
  creg.country = DUPSTRFUN(reg->getCountry);
  creg.telephone = DUPSTRFUN(reg->getTelephone);
  creg.fax = DUPSTRFUN(reg->getFax);
  creg.email = DUPSTRFUN(reg->getEmail);
  creg.credit = reg->getCredit();
  if (reg->getACLSize()) {
     creg.md5Cert = DUPSTRFUN(reg->getACL(0)->getCertificateMD5);
     creg.password = DUPSTRFUN(reg->getACL(0)->getPassword);
  } else {
     creg.md5Cert = DUPSTR("");
     creg.password = DUPSTR("");
  }
} 

ccReg::RegistrarList* 
ccReg_Admin_i::getRegistrars()
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> regm(Register::Manager::create(&db));
  Register::Registrar::Manager *rm = regm->getRegistrarManager();
  Register::Registrar::RegistrarList *rl = rm->getList();
  rl->reload();
  LOG( NOTICE_LOG, "getRegistrars: num -> %d",  rl->size() );
  ccReg::RegistrarList* reglist = new ccReg::RegistrarList;
  reglist->length(rl->size());
  for (unsigned i=0; i<rl->size(); i++)
    fillRegistrar((*reglist)[i],rl->get(i));
  db.Disconnect();
  return reglist;
}

ccReg::Registrar* ccReg_Admin_i::getRegistrarByHandle(const char* handle)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  LOG( NOTICE_LOG, "getRegistarByHandle: handle -> %s", handle );
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> regm(Register::Manager::create(&db));
  Register::Registrar::Manager *rm = regm->getRegistrarManager();
  Register::Registrar::RegistrarList *rl = rm->getList();
  rl->setHandleFilter(handle);
  rl->reload();
  if (rl->size() < 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  } 
  ccReg::Registrar* creg = new ccReg::Registrar;
  fillRegistrar(*creg,rl->get(0));
  db.Disconnect();
  return creg;
}

void 
ccReg_Admin_i::putRegistrar(const ccReg::Registrar& regData)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Registrar::Manager *rm = r->getRegistrarManager();
  Register::Registrar::RegistrarList *rl = rm->getList();
  Register::Registrar::Registrar *reg; // registrar to be created or updated
  if (!regData.id) reg = rl->create();
  else {
    rl->setIdFilter(regData.id);
    rl->reload();
    if (rl->size() != 1) {
      db.Disconnect();
      throw ccReg::Admin::UpdateFailed();
    }
    reg = rl->get(0);
  }
  reg->setHandle((const char *)regData.handle);
  reg->setURL((const char *)regData.url);
  reg->setName((const char *)regData.name);
  reg->setOrganization((const char *)regData.organization);
  reg->setStreet1((const char *)regData.street1);
  reg->setStreet2((const char *)regData.street2);
  reg->setStreet3((const char *)regData.street3);
  reg->setCity((const char *)regData.city);
  reg->setProvince((const char *)regData.stateorprovince);
  reg->setPostalCode((const char *)regData.postalcode);
  reg->setCountry((const char *)regData.country);
  reg->setTelephone((const char *)regData.telephone);
  reg->setFax((const char *)regData.fax);
  reg->setEmail((const char *)regData.email);
  Register::Registrar::ACL *acl;
  if (reg->getACLSize()) acl = reg->getACL(0);
  else acl = reg->newACL();
  acl->setCertificateMD5((const char *)regData.md5Cert);
  acl->setPassword((const char *)regData.password);
  try {
    reg->save();
    db.Disconnect();
  } catch (...) {
    db.Disconnect();
    throw ccReg::Admin::UpdateFailed();
  }
}

ccReg::ContactDetail* 
ccReg_Admin_i::getContactByHandle(const char* handle)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Contact::Manager *cr = r->getContactManager();
  Register::Contact::List *cl = cr->getList();
  cl->setHandleFilter(handle);
  cl->reload();
  if (cl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  } 
  ccReg::ContactDetail* cc = new ccReg::ContactDetail;
  Register::Contact::Contact *c = cl->get(0);
  cc->id = c->getId();
  cc->handle = DUPSTRFUN(c->getHandle);
  cc->roid = DUPSTRFUN(c->getROID);
  cc->registrarHandle = DUPSTRFUN(c->getRegistrarHandle);
  cc->transferDate = DUPSTRDATE(c->getTransferDate);
  cc->updateDate = DUPSTRDATE(c->getUpdateDate);
  cc->createDate = DUPSTRDATE(c->getCreateDate);
  cc->createRegistrarHandle = DUPSTRFUN(c->getCreateRegistrarHandle);
  cc->updateRegistrarHandle = DUPSTRFUN(c->getUpdateRegistrarHandle); 
  cc->authInfo = DUPSTRFUN(c->getAuthPw); 
  cc->name = DUPSTRFUN(c->getName); 
  cc->organization = DUPSTRFUN(c->getOrganization);
  cc->street1 = DUPSTRFUN(c->getStreet1);
  cc->street2 = DUPSTRFUN(c->getStreet2);
  cc->street3 = DUPSTRFUN(c->getStreet3);
  cc->province = DUPSTRFUN(c->getProvince);
  cc->postalcode = DUPSTRFUN(c->getPostalCode);
  cc->city = DUPSTRFUN(c->getCity);
  cc->country = DUPSTRFUN(c->getCountry);
  cc->telephone = DUPSTRFUN(c->getTelephone);
  cc->fax = DUPSTRFUN(c->getFax);
  cc->email = DUPSTRFUN(c->getEmail);
  cc->notifyEmail = DUPSTRFUN(c->getNotifyEmail);
  cc->ssn = DUPSTRFUN(c->getSSN);
  db.Disconnect();
  return cc;
}

ccReg::NSSetDetail* 
ccReg_Admin_i::getNSSetByHandle(const char* handle)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::NSSet::Manager *nr = r->getNSSetManager();
  Register::NSSet::List *nl = nr->getList();
  nl->setHandleFilter(handle);
  nl->reload();
  if (nl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::NSSetDetail* cn = new ccReg::NSSetDetail;
  Register::NSSet::NSSet *n = nl->get(0);
  cn->id = n->getId();
  cn->handle = DUPSTRFUN(n->getHandle);
  cn->roid = DUPSTRFUN(n->getROID);
  cn->registrarHandle = DUPSTRFUN(n->getRegistrarHandle);
  cn->transferDate = DUPSTRDATE(n->getTransferDate);
  cn->updateDate = DUPSTRDATE(n->getUpdateDate);
  cn->createDate = DUPSTRDATE(n->getCreateDate);
  cn->createRegistrarHandle = DUPSTRFUN(n->getCreateRegistrarHandle);
  cn->updateRegistrarHandle = DUPSTRFUN(n->getUpdateRegistrarHandle); 
  cn->authInfo = DUPSTRFUN(n->getAuthPw); 
  cn->admins.length(n->getAdminCount());
  try {
    for (unsigned i=0; i<n->getAdminCount(); i++)
      cn->admins[i] = DUPSTRC(n->getAdminByIdx(i));
  }
  catch (Register::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }
  cn->hosts.length(n->getHostCount());
  for (unsigned i=0; i<n->getHostCount(); i++) {
    cn->hosts[i].fqdn = DUPSTRFUN(n->getHostByIdx(i)->getName);
    cn->hosts[i].inet.length(n->getHostByIdx(i)->getAddrCount());
    for (unsigned j=0; j<n->getHostByIdx(i)->getAddrCount(); j++)
      cn->hosts[i].inet[j] = DUPSTRC(n->getHostByIdx(i)->getAddrByIdx(j));
  }
  db.Disconnect();
  return cn;
}

ccReg::DomainDetail*
ccReg_Admin_i::getDomainByFQDN(const char* fqdn)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Domain::Manager *dm = r->getDomainManager();
  Register::Domain::List *dl = dm->getList();
  dl->setFQDNFilter(fqdn);
  dl->reload();
  if (dl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  } 
  ccReg::DomainDetail* cd = new ccReg::DomainDetail;
  Register::Domain::Domain *d = dl->get(0);
  cd->id = d->getId();
  cd->fqdn = DUPSTRFUN(d->getFQDN);
  cd->roid = DUPSTRFUN(d->getROID);
  cd->registrarHandle = DUPSTRFUN(d->getRegistrarHandle);
  cd->transferDate = DUPSTRDATE(d->getTransferDate);
  cd->updateDate = DUPSTRDATE(d->getUpdateDate);
  cd->createDate = DUPSTRDATE(d->getCreateDate);
  cd->createRegistrarHandle = DUPSTRFUN(d->getCreateRegistrarHandle); 
  cd->updateRegistrarHandle = DUPSTRFUN(d->getUpdateRegistrarHandle); 
  cd->authInfo = DUPSTRFUN(d->getAuthPw); 
  cd->registrantHandle = DUPSTRFUN(d->getRegistrantHandle);
  cd->expirationDate = DUPSTRDATE(d->getExpirationDate);
  cd->valExDate = DUPSTRDATE(d->getValExDate);
  cd->nssetHandle = DUPSTRFUN(d->getNSSetHandle);
  cd->admins.length(d->getAdminCount());
  try {
    for (unsigned i=0; i<d->getAdminCount(); i++)
      cd->admins[i] = CORBA::string_dup(d->getAdminHandleByIdx(i).c_str());
  }
  catch (Register::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }
  db.Disconnect();
  return cd;
}

CORBA::Long 
ccReg_Admin_i::getEnumDomainCount()
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getEnumDomainCount();
  db.Disconnect();
  return ret;
}

CORBA::Long
ccReg_Admin_i::getEnumNumberCount()
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Domain::Manager *dm = r->getDomainManager();
  CORBA::Long ret = dm->getEnumNumberCount();
  db.Disconnect();
  return ret;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_Session_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_Session_i::ccReg_Session_i(const std::string& database)
{
  db.OpenDatabase(database.c_str());
  m.reset(Register::Manager::create(&db));
  reg = new ccReg_Registrars_i(m->getRegistrarManager()->getList());
  eppa = new ccReg_EPPActions_i(m->getRegistrarManager()->getEPPActionList());
  dm = new ccReg_Domains_i(m->getDomainManager()->getList());
  cm = new ccReg_Contacts_i(m->getContactManager()->getList());
  nm = new ccReg_NSSets_i(m->getNSSetManager()->getList());
  reg->reload();
  eppa->reload();
  dm->reload();
  cm->reload();
  nm->reload();
}

ccReg_Session_i::~ccReg_Session_i()
{
  db.Disconnect();
}


ccReg::Registrars_ptr 
ccReg_Session_i::getRegistrars()
{
  return reg->_this();
}
ccReg::EPPActions_ptr
ccReg_Session_i::getEPPActions()
{
  return eppa->_this();
}

ccReg::Domains_ptr 
ccReg_Session_i::getDomains()
{
  return dm->_this();
}

ccReg::Contacts_ptr 
ccReg_Session_i::getContacts()
{
  return cm->_this();
}

ccReg::NSSets_ptr 
ccReg_Session_i::getNSSets()
{
  return nm->_this();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_Registrars_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_Registrars_i::ccReg_Registrars_i(Register::Registrar::RegistrarList *_rl)
  : rl(_rl)
{
}

ccReg_Registrars_i::~ccReg_Registrars_i()
{
}

ccReg::Table::ColumnHeaders* 
ccReg_Registrars_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(3);
  COLHEAD(ch,0,"Name",CT_OTHER);
  COLHEAD(ch,1,"Handle",CT_REGISTRAR_HANDLE); 
  COLHEAD(ch,2,"URL",CT_OTHER);
  return ch;
}

ccReg::TableRow* 
ccReg_Registrars_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Registrar::Registrar *r = rl->get(row);
  if (!r) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(3);
  (*tr)[0] = DUPSTRFUN(r->getName); 
  (*tr)[1] = DUPSTRFUN(r->getHandle); 
  (*tr)[2] = DUPSTRFUN(r->getURL); 
  return tr;
}

void 
ccReg_Registrars_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

char* 
ccReg_Registrars_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_Registrars_i::numRows()
{
  return rl->size();
}

CORBA::Short 
ccReg_Registrars_i::numColumns()
{
  return 3;
}

void 
ccReg_Registrars_i::reload()
{
  rl->reload();
}

char* 
ccReg_Registrars_i::fulltext()
{
  return CORBA::string_dup(fulltextFilter.c_str());
}

void 
ccReg_Registrars_i::fulltext(const char* _v)
{
  fulltextFilter = _v;
  rl->setNameFilter(fulltextFilter);
}

char* 
ccReg_Registrars_i::name()
{
  return CORBA::string_dup(nameFilter.c_str());
}

void 
ccReg_Registrars_i::name(const char* _v)
{
  nameFilter = _v;
 rl->setNameFilter(_v);
}

char* 
ccReg_Registrars_i::handle()
{
  return CORBA::string_dup(handleFilter.c_str());
}

void 
ccReg_Registrars_i::handle(const char* _v)
{
  handleFilter = _v;
  rl->setHandleFilter(_v);
}

ccReg::Filter_ptr
ccReg_Registrars_i::aFilter()
{
  return _this();
}

void
ccReg_Registrars_i::clear()
{
  fulltextFilter = "";
  handleFilter = "";
  nameFilter = "";
  rl->clearFilter();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_EPPActions_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_EPPActions_i::ccReg_EPPActions_i(
  Register::Registrar::EPPActionList *_eal
)
  : eal(_eal), registrarFilter(0)
{
}

ccReg_EPPActions_i::~ccReg_EPPActions_i()
{
}

ccReg::Table::ColumnHeaders* 
ccReg_EPPActions_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(4);
  COLHEAD(ch,0,"Type",CT_OTHER);
  COLHEAD(ch,1,"Registrar",CT_REGISTRAR_HANDLE);
  COLHEAD(ch,2,"Time",CT_OTHER);
  COLHEAD(ch,3,"Result",CT_OTHER);
  return ch;
}

ccReg::TableRow* 
ccReg_EPPActions_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)

{
  const Register::Registrar::EPPAction *a = eal->get(row);
  if (!a) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(4);
  (*tr)[0] = DUPSTRFUN(a->getTypeName);
  (*tr)[1] = DUPSTRFUN(a->getRegistrarHandle); 
  (*tr)[2] = DUPSTRDATE(a->getStartTime);
  (*tr)[3] = DUPSTRFUN(a->getResultStatus);
  return tr;
}

void 
ccReg_EPPActions_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

char*
ccReg_EPPActions_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_EPPActions_i::numRows()
{
  return eal->size();
}

CORBA::Short 
ccReg_EPPActions_i::numColumns()
{
  return 4;
}

void 
ccReg_EPPActions_i::reload()
{
  eal->reload();
}

CORBA::Short 
ccReg_EPPActions_i::registrar()
{
  return registrarFilter;
}

void 
ccReg_EPPActions_i::registrar(CORBA::Short _v)
{
  registrarFilter = _v;
  eal->setRegistrarFilter(_v);
}

char* 
ccReg_EPPActions_i::registrarHandle()
{
  return CORBA::string_dup(registrarHandleFilter.c_str());
}

void 
ccReg_EPPActions_i::registrarHandle(const char* _v)
{
  registrarHandleFilter = _v;
  eal->setRegistrarHandleFilter(registrarHandleFilter);
}

char* 
ccReg_EPPActions_i::type()
{
  return CORBA::string_dup(typeFilter.c_str());
}

void 
ccReg_EPPActions_i::type(const char* _v)
{
  typeFilter = _v;
  eal->setTextTypeFilter(_v);
}

char* 
ccReg_EPPActions_i::handle()
{
  return CORBA::string_dup(handleFilter.c_str());
}

void 
ccReg_EPPActions_i::handle(const char* _v)
{
  handleFilter = _v;
  eal->setHandleFilter(_v); 
}

CORBA::Short 
ccReg_EPPActions_i::result()
{
  return resultFilter;
}

void 
ccReg_EPPActions_i::result(CORBA::Short _v)
{
  resultFilter = _v;
  eal->setReturnCodeFilter(_v);
}

ccReg::DateInterval 
ccReg_EPPActions_i::time()
{
  return timeFilter;
}

void 
ccReg_EPPActions_i::time(const ccReg::DateInterval& _v)
{
  timeFilter = _v;
  date from;
  date to;
  try {
    from = date(_v.from.year,_v.from.month,_v.from.day);
  }
  catch (...) {}
  try {
    to = date(_v.to.year,_v.to.month,_v.to.day);
  }
  catch (...) {}
  eal->setTimePeriodFilter(
    time_period(
      ptime(from,time_duration(0,0,0)),
      ptime(to,time_duration(0,0,0))
    )
  );
}

char* 
ccReg_EPPActions_i::clTRID()
{
  return CORBA::string_dup(clTRIDFilter.c_str());
}

void 
ccReg_EPPActions_i::clTRID(const char* _v)
{
  clTRIDFilter = _v;
  eal->setClTRIDFilter(_v);
}

char* 
ccReg_EPPActions_i::svTRID()
{
  return CORBA::string_dup(svTRIDFilter.c_str());
}

void 
ccReg_EPPActions_i::svTRID(const char* _v)
{
  svTRIDFilter = _v;
  eal->setSvTRIDFilter(_v);
}

ccReg::Filter_ptr
ccReg_EPPActions_i::aFilter()
{
  return _this();
}

void
ccReg_EPPActions_i::clear()
{
  registrarHandleFilter = "";
  typeFilter = "";
  handleFilter = "";
  resultFilter = 0;
  timeFilter.from.year = 0;;
  timeFilter.from.month = 0;
  timeFilter.from.day = 0;
  timeFilter.to.year = 0;;
  timeFilter.to.month = 0;
  timeFilter.to.day = 0;
  clTRIDFilter = "";
  svTRIDFilter = "";
  eal->clearFilter();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_RegObjectFilter_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_RegObjectFilter_i::ccReg_RegObjectFilter_i(Register::ObjectList *_ol)
  : registrarFilter(0), ol(_ol)
{
}

CORBA::Short 
ccReg_RegObjectFilter_i::registrar()
{
  return registrarFilter;
}

void 
ccReg_RegObjectFilter_i::registrar(CORBA::Short _v)
{
  registrarFilter = _v;
  ol->setRegistrarFilter(_v);
}

char* 
ccReg_RegObjectFilter_i::registrarHandle()
{
  return CORBA::string_dup(registrarHandleFilter.c_str());
}

void 
ccReg_RegObjectFilter_i::registrarHandle(const char* _v)
{
  registrarHandleFilter = _v;
  ol->setRegistrarHandleFilter(registrarHandleFilter);
}

// ---

CORBA::Short 
ccReg_RegObjectFilter_i::createRegistrar()
{
  return createRegistrarFilter;
}

void 
ccReg_RegObjectFilter_i::createRegistrar(CORBA::Short _v)
{
  createRegistrarFilter = _v;
  ol->setCreateRegistrarFilter(_v);
}

char* 
ccReg_RegObjectFilter_i::createRegistrarHandle()
{
  return CORBA::string_dup(createRegistrarHandleFilter.c_str());
}

void 
ccReg_RegObjectFilter_i::createRegistrarHandle(const char* _v)
{
  createRegistrarHandleFilter = _v;
  ol->setCreateRegistrarHandleFilter(createRegistrarHandleFilter);
}

// --

CORBA::Short 
ccReg_RegObjectFilter_i::updateRegistrar()
{
  return updateRegistrarFilter;
}

void 
ccReg_RegObjectFilter_i::updateRegistrar(CORBA::Short _v)
{
  updateRegistrarFilter = _v;
  ol->setUpdateRegistrarFilter(_v);
}

char* 
ccReg_RegObjectFilter_i::updateRegistrarHandle()
{
  return CORBA::string_dup(updateRegistrarHandleFilter.c_str());
}

void 
ccReg_RegObjectFilter_i::updateRegistrarHandle(const char* _v)
{
  updateRegistrarHandleFilter = _v;
  ol->setUpdateRegistrarHandleFilter(updateRegistrarHandleFilter);
}

// ---

ccReg::DateInterval 
ccReg_RegObjectFilter_i::crDate()
{
  return crDateFilter;
}

void 
ccReg_RegObjectFilter_i::crDate(const ccReg::DateInterval& _v)
{
  crDateFilter = _v;
  date from;
  date to;
  try {
    from = date(_v.from.year,_v.from.month,_v.from.day);
  }
  catch (...) {}
  try {
    to = date(_v.to.year,_v.to.month,_v.to.day);
  }
  catch (...) {}
  ol->setCrDateIntervalFilter(
    time_period(
      ptime(from,time_duration(0,0,0)),
      ptime(to,time_duration(0,0,0))
    )
  );
}

// --

ccReg::DateInterval 
ccReg_RegObjectFilter_i::trDate()
{
  return trDateFilter;
}

void 
ccReg_RegObjectFilter_i::trDate(const ccReg::DateInterval& _v)
{
  trDateFilter = _v;
  date from;
  date to;
  try {
    from = date(_v.from.year,_v.from.month,_v.from.day);
  }
  catch (...) {}
  try {
    to = date(_v.to.year,_v.to.month,_v.to.day);
  }
  catch (...) {}
  ol->setTrDateIntervalFilter(
    time_period(
      ptime(from,time_duration(0,0,0)),
      ptime(to,time_duration(0,0,0))
    )
  );
}

// --

ccReg::DateInterval 
ccReg_RegObjectFilter_i::upDate()
{
  return upDateFilter;
}

void 
ccReg_RegObjectFilter_i::upDate(const ccReg::DateInterval& _v)
{
  upDateFilter = _v;
  date from;
  date to;
  try {
    from = date(_v.from.year,_v.from.month,_v.from.day);
  }
  catch (...) {}
  try {
    to = date(_v.to.year,_v.to.month,_v.to.day);
  }
  catch (...) {}
  ol->setUpdateIntervalFilter(
    time_period(
      ptime(from,time_duration(0,0,0)),
      ptime(to,time_duration(0,0,0))
    )
  );
}

void 
ccReg_RegObjectFilter_i::clear()
{
  registrarFilter = 0;
  registrarHandleFilter = "";
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_Domains_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_Domains_i::ccReg_Domains_i(
  Register::Domain::List *_dl
) : ccReg_RegObjectFilter_i(_dl),dl(_dl), registrantFilter(0)
{
}

ccReg_Domains_i::~ccReg_Domains_i()
{
}

ccReg::Table::ColumnHeaders* 
ccReg_Domains_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(11);
  COLHEAD(ch,0,"Jméno",CT_DOMAIN_HANDLE);
  COLHEAD(ch,1,"Registrace",CT_OTHER);
  COLHEAD(ch,2,"Ukončení",CT_OTHER);
  COLHEAD(ch,3,"Držitel",CT_CONTACT_HANDLE);
  COLHEAD(ch,4,"Jméno držitele",CT_OTHER);
  COLHEAD(ch,5,"Registrátor",CT_REGISTRAR_HANDLE);
  COLHEAD(ch,6,"Zóna",CT_OTHER);
  COLHEAD(ch,7,"Expirace",CT_OTHER);
  COLHEAD(ch,8,"Zrušeni",CT_OTHER);
  COLHEAD(ch,9,"Ze zóny",CT_OTHER);
  COLHEAD(ch,10,"Validace",CT_OTHER);
  return ch;
}

ccReg::TableRow* 
ccReg_Domains_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Domain::Domain *d = dl->get(row);
  if (!d) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(11);
  (*tr)[0] = DUPSTRFUN(d->getFQDN); // fqdn
  (*tr)[1] = DUPSTRDATE(d->getCreateDate); // crdate
  (*tr)[2] = DUPSTR(""); // delete date
  (*tr)[3] = DUPSTRFUN(d->getRegistrantHandle); // registrant handle
  (*tr)[4] = DUPSTRFUN(d->getRegistrantName); // registrant name
  (*tr)[5] = DUPSTRFUN(d->getRegistrarHandle); // registrar handle 
  (*tr)[6] = DUPSTR(""); // zone generation 
  (*tr)[7] = DUPSTRDATESHORT(d->getExpirationDate); // expiration date 
  (*tr)[8] = DUPSTR(""); // zruseni ??
  (*tr)[9] = DUPSTR(""); // vyrazeni z dns
  (*tr)[10] = DUPSTRDATESHORT(d->getValExDate); // validace
  return tr;
}

void 
ccReg_Domains_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

char*
ccReg_Domains_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_Domains_i::numRows()
{
  return dl->getCount();
}

CORBA::Short 
ccReg_Domains_i::numColumns()
{
  return 11;
}

void 
ccReg_Domains_i::reload()
{
  dl->reload();
}

CORBA::Short 
ccReg_Domains_i::registrant()
{
  return registrantFilter;
}

void 
ccReg_Domains_i::registrant(CORBA::Short _v)
{
  registrantFilter = _v;
  dl->setRegistrarFilter(_v);
}

char* 
ccReg_Domains_i::registrantHandle()
{
  return CORBA::string_dup(registrantHandleFilter.c_str());
}

void 
ccReg_Domains_i::registrantHandle(const char* _v)
{
  registrantHandleFilter = _v;
  dl->setRegistrantHandleFilter(registrantHandleFilter);
}

CORBA::Short 
ccReg_Domains_i::nsset()
{
  return nssetFilter;
}

void 
ccReg_Domains_i::nsset(CORBA::Short _v)
{
  nssetFilter = _v;
  dl->setNSSetFilter(_v);
}

char* 
ccReg_Domains_i::nssetHandle()
{
  return CORBA::string_dup(nssetHandleFilter.c_str());
}

void 
ccReg_Domains_i::nssetHandle(const char* _v)
{
  nssetHandleFilter = _v;
  dl->setNSSetHandleFilter(_v);
}

CORBA::Short 
ccReg_Domains_i::admin()
{
  return adminFilter;
}

void 
ccReg_Domains_i::admin(CORBA::Short _v)
{
  adminFilter = _v;
  dl->setAdminFilter(_v);
}

char* 
ccReg_Domains_i::adminHandle()
{
  return CORBA::string_dup(adminHandleFilter.c_str());
}

void 
ccReg_Domains_i::adminHandle(const char* _v)
{
  adminHandleFilter = _v;
  dl->setAdminHandleFilter(_v);
}

char* 
ccReg_Domains_i::fqdn()
{
  return CORBA::string_dup(fqdnFilter.c_str());
}

void 
ccReg_Domains_i::fqdn(const char* _v)
{
  fqdnFilter = _v;
  dl->setFQDNFilter(_v);
}

ccReg::DateInterval 
ccReg_Domains_i::exDate()
{
  return exDateFilter;
}

void 
ccReg_Domains_i::exDate(const ccReg::DateInterval& _v)
{
  exDateFilter = _v;
  date from;
  date to;
  try {
    from = date(_v.from.year,_v.from.month,_v.from.day);
  }
  catch (...) {}
  try {
    to = date(_v.to.year,_v.to.month,_v.to.day);
  }
  catch (...) {}
  dl->setExpirationDateFilter(
    time_period(
      ptime(from,time_duration(0,0,0)),
      ptime(to,time_duration(0,0,0))
    )
  );
}

ccReg::DateInterval 
ccReg_Domains_i::valExDate()
{
  return valExDateFilter;
}

void 
ccReg_Domains_i::valExDate(const ccReg::DateInterval& _v)
{
  valExDateFilter = _v;
  date from;
  date to;
  try {
    from = date(_v.from.year,_v.from.month,_v.from.day);
  }
  catch (...) {}
  try {
    to = date(_v.to.year,_v.to.month,_v.to.day);
  }
  catch (...) {}
  dl->setValExDateFilter(
    time_period(
      ptime(from,time_duration(0,0,0)),
      ptime(to,time_duration(0,0,0))
    )
  );
}

char *
ccReg_Domains_i::techAdminHandle()
{
  return DUPSTRC(techAdminHandleFilter);
}

void 
ccReg_Domains_i::techAdminHandle(const char * _v)
{
  techAdminHandleFilter = _v;
  dl->setTechAdminHandleFilter(techAdminHandleFilter);
}

char *
ccReg_Domains_i::nssetIP()
{
  return DUPSTRC(nssetIPFilter);
}

void 
ccReg_Domains_i::nssetIP(const char *_v)
{
  nssetIPFilter = _v;
  dl->setHostIPFilter(nssetIPFilter);
}

ccReg::Filter_ptr
ccReg_Domains_i::aFilter()
{
  return _this();
}

void
ccReg_Domains_i::clear()
{
  ccReg_RegObjectFilter_i::clear();
  dl->clearFilter();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_Contact_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_Contacts_i::ccReg_Contacts_i(
  Register::Contact::List *_cl
)
  : ccReg_RegObjectFilter_i(_cl), cl(_cl)
{
}

ccReg_Contacts_i::~ccReg_Contacts_i()
{
}

ccReg::Table::ColumnHeaders* 
ccReg_Contacts_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(4);
  COLHEAD(ch,0,"Handle",CT_CONTACT_HANDLE);
  COLHEAD(ch,1,"Name",CT_OTHER);
  COLHEAD(ch,2,"CrDate",CT_OTHER);
  COLHEAD(ch,3,"Registrar",CT_REGISTRAR_HANDLE);
  return ch;
}

ccReg::TableRow* 
ccReg_Contacts_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Contact::Contact *c = cl->get(row);
  if (!c) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(4);
  (*tr)[0] = DUPSTRFUN(c->getHandle);
  (*tr)[1] = DUPSTRFUN(c->getName);
  (*tr)[2] = DUPSTRDATE(c->getCreateDate);
  (*tr)[3] = DUPSTRFUN(c->getRegistrarHandle); 
  return tr;
}

void 
ccReg_Contacts_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

char*
ccReg_Contacts_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_Contacts_i::numRows()
{
  return cl->getCount();
}

CORBA::Short 
ccReg_Contacts_i::numColumns()
{
  return 4;
}

void 
ccReg_Contacts_i::reload()
{
  cl->reload();
}

char* 
ccReg_Contacts_i::handle()
{
  return DUPSTRC(handleFilter);
}

void 
ccReg_Contacts_i::handle(const char* _v)
{
  handleFilter = _v;
  cl->setHandleFilter(_v);
}

char* 
ccReg_Contacts_i::name()
{
  return DUPSTRC(nameFilter);
}

void 
ccReg_Contacts_i::name(const char* _v)
{
  nameFilter = _v;
  cl->setNameFilter(_v);
}

char* 
ccReg_Contacts_i::org()
{
  return DUPSTRC(orgFilter);
}

void 
ccReg_Contacts_i::org(const char* _v)
{
  orgFilter = _v;
  cl->setOrganizationFilter(_v);
}

char* 
ccReg_Contacts_i::email()
{
  return DUPSTRC(emailFilter);
}

void 
ccReg_Contacts_i::email(const char* _v)
{
  emailFilter = _v;
  cl->setEmailFilter(_v);
}

char* 
ccReg_Contacts_i::ident()
{
  return DUPSTRC(identFilter);
}

void 
ccReg_Contacts_i::ident(const char* _v)
{
  identFilter = _v;
  cl->setIdentFilter(_v);
}

char* 
ccReg_Contacts_i::vat()
{
  return DUPSTRC(vatFilter);
}

void 
ccReg_Contacts_i::vat(const char* _v)
{
  vatFilter = _v;
  cl->setHandleFilter(_v);
}

ccReg::Filter_ptr
ccReg_Contacts_i::aFilter()
{
  return _this();
}

void
ccReg_Contacts_i::clear()
{
  ccReg_RegObjectFilter_i::clear();
  handleFilter = "";
  nameFilter = "";
  orgFilter = "";
  emailFilter = "";
  identFilter = "";
  vatFilter = "";
  cl->clearFilter();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_NSSets_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_NSSets_i::ccReg_NSSets_i(
  Register::NSSet::List *_nl
)
  : ccReg_RegObjectFilter_i(_nl), nl(_nl)
{
}

ccReg_NSSets_i::~ccReg_NSSets_i()
{
}

ccReg::Table::ColumnHeaders* 
ccReg_NSSets_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(3);
  COLHEAD(ch,0,"Handle",CT_NSSET_HANDLE);
  COLHEAD(ch,1,"CrDate",CT_OTHER);
  COLHEAD(ch,2,"Registrar",CT_REGISTRAR_HANDLE);
  return ch;
}

ccReg::TableRow* 
ccReg_NSSets_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::NSSet::NSSet *n = nl->get(row);
  if (!n) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(3);
  (*tr)[0] = DUPSTRFUN(n->getHandle);
  (*tr)[1] = DUPSTRDATE(n->getCreateDate);
  (*tr)[2] = DUPSTRFUN(n->getRegistrarHandle); 
  return tr;
}

void 
ccReg_NSSets_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

char*
ccReg_NSSets_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_NSSets_i::numRows()
{
  return nl->getCount();
}

CORBA::Short 
ccReg_NSSets_i::numColumns()
{
  return 3;
}

void 
ccReg_NSSets_i::reload()
{
  nl->reload();
}

char* 
ccReg_NSSets_i::handle()
{
  return DUPSTRC(handleFilter);
}

void 
ccReg_NSSets_i::handle(const char* _v)
{
  handleFilter = _v;
  nl->setHandleFilter(_v);
}

char* 
ccReg_NSSets_i::adminHandle()
{
  return DUPSTRC(adminHandleFilter);
}

void 
ccReg_NSSets_i::adminHandle(const char* _v)
{
  adminHandleFilter = _v;
  nl->setAdminFilter(_v);
}

char* 
ccReg_NSSets_i::ip()
{
  return DUPSTRC(ipFilter);
}

void 
ccReg_NSSets_i::ip(const char* _v)
{
  ipFilter = _v;
  nl->setHostIPFilter(_v);
}

char* 
ccReg_NSSets_i::hostname()
{
  return DUPSTRC(hostnameFilter);
}

void 
ccReg_NSSets_i::hostname(const char* _v)
{
  hostnameFilter = _v;
  nl->setHostNameFilter(_v);
}

ccReg::Filter_ptr
ccReg_NSSets_i::aFilter()
{
  return _this();
}

void
ccReg_NSSets_i::clear()
{
  ccReg_RegObjectFilter_i::clear();
  handleFilter = "";
  adminHandleFilter = "";
  ipFilter = "";
  hostnameFilter = "";
  nl->clearFilter();
}
