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
#define DUPSTRFUN(f) DUPSTR(f().c_str())
//#define DUPSTRDATE(f) DUPSTR(to_simple_string(f()).c_str())
#define DUPSTRDATE(f) DUPSTR(formatTime(f(),true).c_str())

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
  cn->admins.length(1);
  try {
    for (unsigned i=0; i<n->getAdminCount(); i++)
      cn->admins[i] = DUPSTR(n->getAdminByIdx(i).c_str());
  }
  catch (Register::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }
  cn->hosts.length(n->getHostCount());
  for (unsigned i=0; i<n->getHostCount(); i++) {
    cn->hosts[i].fqdn = DUPSTR(n->getHostByIdx(i)->getName().c_str());
    cn->hosts[i].inet.length(n->getHostByIdx(i)->getAddrCount());
    for (unsigned j=0; j<n->getHostByIdx(i)->getAddrCount(); j++)
      cn->hosts[i].inet[j] = DUPSTR(n->getHostByIdx(i)->getAddrByIdx(j).c_str());
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
  (*ch)[0].name = CORBA::string_dup("Name"); 
  (*ch)[0].type = ccReg::Table::CT_OTHER; 
  (*ch)[1].name = CORBA::string_dup("Handle"); 
  (*ch)[1].type = ccReg::Table::CT_REGISTRAR_HANDLE; 
  (*ch)[2].name = CORBA::string_dup("URL"); 
  (*ch)[2].type = ccReg::Table::CT_OTHER;
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
  (*tr)[0] = CORBA::string_dup(r->getName().c_str()); 
  (*tr)[1] = CORBA::string_dup(r->getHandle().c_str()); 
  (*tr)[2] = CORBA::string_dup(r->getURL().c_str()); 
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
  (*ch)[0].name = CORBA::string_dup("SessionID"); 
  (*ch)[0].type = ccReg::Table::CT_OTHER; 
  (*ch)[1].name = CORBA::string_dup("Type"); 
  (*ch)[1].type = ccReg::Table::CT_OTHER; 
  (*ch)[2].name = CORBA::string_dup("Registrar"); 
  (*ch)[2].type = ccReg::Table::CT_REGISTRAR_HANDLE; 
  (*ch)[3].name = CORBA::string_dup("Time"); 
  (*ch)[3].type = ccReg::Table::CT_OTHER;
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
  std::ostringstream buffer;
  buffer << a->getSessionId();
  (*tr)[0] = CORBA::string_dup(buffer.str().c_str());
  buffer.str("");
  buffer << a->getTypeName();
  (*tr)[1] = CORBA::string_dup(buffer.str().c_str());
  (*tr)[2] = CORBA::string_dup(a->getRegistrarHandle().c_str()); 
  buffer.str("");
  buffer << to_simple_string(a->getStartTime());
  (*tr)[3] = CORBA::string_dup(buffer.str().c_str());
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
  (*ch)[0].name = CORBA::string_dup("Jméno"); 
  (*ch)[0].type = ccReg::Table::CT_DOMAIN_HANDLE; 
  (*ch)[1].name = CORBA::string_dup("Registrace"); 
  (*ch)[1].type = ccReg::Table::CT_OTHER; 
  (*ch)[2].name = CORBA::string_dup("Ukončení"); 
  (*ch)[2].type = ccReg::Table::CT_OTHER; 
  (*ch)[3].name = CORBA::string_dup("Držitel"); 
  (*ch)[3].type = ccReg::Table::CT_CONTACT_HANDLE; 
  (*ch)[4].name = CORBA::string_dup("Jméno držitele"); 
  (*ch)[4].type = ccReg::Table::CT_OTHER; 
  (*ch)[5].name = CORBA::string_dup("Registrátor"); 
  (*ch)[5].type = ccReg::Table::CT_REGISTRAR_HANDLE; 
  (*ch)[6].name = CORBA::string_dup("Zóna"); 
  (*ch)[6].type = ccReg::Table::CT_OTHER;
  (*ch)[7].name = CORBA::string_dup("Expirace"); 
  (*ch)[7].type = ccReg::Table::CT_OTHER;
  (*ch)[8].name = CORBA::string_dup("Zrušeni"); 
  (*ch)[8].type = ccReg::Table::CT_OTHER;
  (*ch)[9].name = CORBA::string_dup("Ze zóny"); 
  (*ch)[9].type = ccReg::Table::CT_OTHER; 
  (*ch)[10].name = CORBA::string_dup("Validace"); 
  (*ch)[10].type = ccReg::Table::CT_OTHER;

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
  // fqdn
  (*tr)[0] = CORBA::string_dup(d->getFQDN().c_str());
  // crdate
  (*tr)[1] = CORBA::string_dup(
    formatTime(d->getCreateDate(),true).c_str()
//  to_simple_string(d->getCreateDate()).c_str()
  );
  // delete date
  (*tr)[2] = CORBA::string_dup(
   ""
//    to_simple_string(ptime(not_a_date_time)).c_str()
  );
  // registrant handle
  (*tr)[3] = CORBA::string_dup(d->getRegistrantHandle().c_str());
  // registrant name 
  (*tr)[4] = CORBA::string_dup(d->getRegistrantName().c_str());
  // registrar handle 
  (*tr)[5] = CORBA::string_dup(d->getRegistrarHandle().c_str());
  // zone generation 
  (*tr)[6] = CORBA::string_dup("");
  // expiration date 
  ptime et = d->getExpirationDate();
  std::ostringstream extime;
  unsigned m = et.date().month();
  extime << std::setfill('0') << std::setw(2)
         << et.date().day() << "." 
         << std::setw(2)
	 << m << "." 
         << std::setw(2)
         << et.date().year();
  (*tr)[7] = CORBA::string_dup(
      formatTime(d->getExpirationDate(),false).c_str()
//    to_simple_string(d->getExpirationDate()).c_str()
  );
  // zruseni ??
  (*tr)[8] = CORBA::string_dup(
    ""
    //to_simple_string(ptime(not_a_date_time)).c_str()
  );
  // vyrazeni z dns
  (*tr)[9] = CORBA::string_dup(
   ""
//    to_simple_string(ptime(not_a_date_time)).c_str()
  );
  // validace
  (*tr)[10] = CORBA::string_dup(
    formatTime(d->getValExDate(),false).c_str()
//  to_simple_string(d->getValExDate()).c_str() 
  );
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
  (*ch)[0].name = CORBA::string_dup("Handle"); 
  (*ch)[0].type = ccReg::Table::CT_CONTACT_HANDLE; 
  (*ch)[1].name = CORBA::string_dup("Name"); 
  (*ch)[1].type = ccReg::Table::CT_OTHER; 
  (*ch)[2].name = CORBA::string_dup("CrDate"); 
  (*ch)[2].type = ccReg::Table::CT_OTHER; 
  (*ch)[3].name = CORBA::string_dup("Registrar"); 
  (*ch)[3].type = ccReg::Table::CT_REGISTRAR_HANDLE;
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
  (*tr)[0] = CORBA::string_dup(c->getHandle().c_str());
  (*tr)[1] = CORBA::string_dup(c->getName().c_str());
  (*tr)[2] = CORBA::string_dup(to_simple_string(c->getCreateDate()).c_str());
  (*tr)[3] = CORBA::string_dup(c->getRegistrarHandle().c_str()); 
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
  return CORBA::string_dup(handleFilter.c_str());
}

void 
ccReg_Contacts_i::handle(const char* _v)
{
  handleFilter = _v;
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
  (*ch)[0].name = CORBA::string_dup("Handle"); 
  (*ch)[0].type = ccReg::Table::CT_NSSET_HANDLE; 
  (*ch)[1].name = CORBA::string_dup("CrDate"); 
  (*ch)[1].type = ccReg::Table::CT_OTHER; 
  (*ch)[2].name = CORBA::string_dup("Registrar"); 
  (*ch)[2].type = ccReg::Table::CT_REGISTRAR_HANDLE;
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
  (*tr)[0] = CORBA::string_dup(n->getHandle().c_str());
  (*tr)[1] = CORBA::string_dup(to_simple_string(n->getCreateDate()).c_str());
  (*tr)[2] = CORBA::string_dup(n->getRegistrarHandle().c_str()); 
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
  return CORBA::string_dup(handleFilter.c_str());
}

void 
ccReg_NSSets_i::handle(const char* _v)
{
  handleFilter = _v;
  nl->setHandleFilter(_v);
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
  nl->clearFilter();
}
