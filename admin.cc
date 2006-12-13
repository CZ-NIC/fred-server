#include "admin.h"
#include "log.h"
//#include "util.h"
#include "dbsql.h"
#include "register/register.h"
#include "mailer_manager.h"
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

time_period 
setPeriod(const ccReg::DateTimeInterval& _v)
{
  date from;
  date to;
  try {
    from = date(_v.from.date.year,_v.from.date.month,_v.from.date.day);
  }
  catch (...) {}
  try {
    to = date(_v.to.date.year,_v.to.date.month,_v.to.date.day);
  }
  catch (...) {}
  return time_period(
    ptime(from,time_duration(_v.from.hour,_v.from.minute,_v.from.second)),
    ptime(to,time_duration(_v.to.hour,_v.to.minute,_v.to.second))
  );
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

ccReg::TID 
ccReg_PageTable_i::getPageRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  return getRowId(row + start());
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_Admin_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_Admin_i::ccReg_Admin_i(const std::string _database, NameService *_ns) : 
  database(_database), ns(_ns)
{
}

ccReg_Admin_i::~ccReg_Admin_i() 
{
/// clean sessions
}

#define SWITCH_CONVERT(x) case Register::x : ch->handleClass = ccReg::x; break
void
ccReg_Admin_i::checkHandle(const char* handle, ccReg::CheckHandleTypeSeq_out chso)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  ccReg::CheckHandleTypeSeq* chs = new ccReg::CheckHandleTypeSeq;
  chs->length(1);
  Register::CheckHandle chd;
  r->checkHandle(handle,chd);
  ccReg::CheckHandleType *ch = &(*chs)[0];
  ch->newHandle = CORBA::string_dup(chd.newHandle.c_str());
  switch (chd.handleClass) {
   case Register::CH_ENUM_BAD_ZONE:
     ch->handleClass = ccReg::CH_UNREGISTRABLE;
     ch->hType = ccReg::HT_ENUM_NUMBER;
     break;
   case Register::CH_ENUM:
     ch->handleClass = ccReg::CH_FREE;
     ch->hType = ccReg::HT_ENUM_NUMBER;
     break;
   case Register::CH_DOMAIN_PART:
     ch->handleClass = ccReg::CH_PART;
     ch->hType = ccReg::HT_DOMAIN;
     break;
   case Register::CH_DOMAIN_BAD_ZONE:
     ch->handleClass = ccReg::CH_UNREGISTRABLE;
     ch->hType = ccReg::HT_DOMAIN;
     break;
   case Register::CH_DOMAIN_LONG:
     ch->handleClass = ccReg::CH_LONG;
     ch->hType = ccReg::HT_DOMAIN;
     break;
   case Register::CH_DOMAIN:
     ch->handleClass = ccReg::CH_FREE;
     ch->hType = ccReg::HT_DOMAIN;
     break;
   case Register::CH_NSSET:
     ch->handleClass = ccReg::CH_FREE;
     ch->hType = ccReg::HT_NSSET;
     break;
   case Register::CH_CONTACT:
     ch->handleClass = ccReg::CH_FREE;
     ch->hType = ccReg::HT_CONTACT;
     break;
   case Register::CH_INVALID:
     ch->handleClass = ccReg::CH_FREE;
     ch->hType = ccReg::HT_OTHER;
     break;
  }
  chso = chs;
  db.Disconnect(); 
}
 
char* 
ccReg_Admin_i::login(const char* username, const char* password)
  throw (ccReg::Admin::AuthFailed)
{
  std::vector<std::string> userList;
  userList.push_back("superuser");
  userList.push_back("martin");
  userList.push_back("pavel");
  userList.push_back("jara");
  userList.push_back("zuzka");
  userList.push_back("david");
  std::vector<std::string>::const_iterator i = find(
    userList.begin(), userList.end(), username
  );
  if (i == userList.end()) throw ccReg::Admin::AuthFailed();
  SessionListType::const_iterator j = sessionList.find(username);
  if (j == sessionList.end())
    sessionList[username] = new ccReg_Session_i(database,ns);
  return CORBA::string_dup(username);
}

ccReg::Session_ptr 
ccReg_Admin_i::getSession(const char* sessionID)
  throw (ccReg::Admin::ObjectNotFound)
{
  SessionListType::const_iterator i = sessionList.find(sessionID);
  if (i == sessionList.end()) throw ccReg::Admin::ObjectNotFound();
  return (*i).second->_this();
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
  creg.access.length(reg->getACLSize());
  for (unsigned i=0; i<reg->getACLSize(); i++) {
    creg.access[i].md5Cert = DUPSTRFUN(reg->getACL(i)->getCertificateMD5);
    creg.access[i].password = DUPSTRFUN(reg->getACL(i)->getPassword);
  }
  creg.hidden = reg->getHandle() == "REG-CZNIC" ? true : false;
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

ccReg::Registrar* ccReg_Admin_i::getRegistrarById(ccReg::TID id)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  LOG( NOTICE_LOG, "getRegistarByHandle: id -> %lld", (unsigned long long)id );
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> regm(Register::Manager::create(&db));
  Register::Registrar::Manager *rm = regm->getRegistrarManager();
  Register::Registrar::RegistrarList *rl = rm->getList();
  rl->setIdFilter(id);
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
  reg->clearACLList();
  for (unsigned i=0; i<regData.access.length(); i++) {
    Register::Registrar::ACL *acl = reg->newACL();
    acl->setCertificateMD5((const char *)regData.access[i].md5Cert);
    acl->setPassword((const char *)regData.access[i].password);
  }
  try {
    reg->save();
    db.Disconnect();
  } catch (...) {
    db.Disconnect();
    throw ccReg::Admin::UpdateFailed();
  }
}

void 
ccReg_Admin_i::fillContact(
  ccReg::ContactDetail* cc, Register::Contact::Contact* c
)
{
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
  cc->ssnType = DUPSTRFUN(c->getSSNType); 
  cc->vat = DUPSTRFUN(c->getVAT); 
  cc->discloseName = c->getDiscloseName(); 
  cc->discloseOrganization = c->getDiscloseOrganization(); 
  cc->discloseAddress = c->getDiscloseAddr(); 
  cc->discloseEmail = c->getDiscloseEmail(); 
  cc->discloseTelephone = c->getDiscloseTelephone(); 
  cc->discloseFax = c->getDiscloseFax(); 
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
  fillContact(cc,cl->get(0));
  db.Disconnect();
  return cc;
}

ccReg::ContactDetail* 
ccReg_Admin_i::getContactById(ccReg::TID id)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Contact::Manager *cr = r->getContactManager();
  Register::Contact::List *cl = cr->getList();
  cl->setIdFilter(id);
  cl->reload();
  if (cl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  } 
  ccReg::ContactDetail* cc = new ccReg::ContactDetail;
  fillContact(cc,cl->get(0));
  db.Disconnect();
  return cc;
}

void 
ccReg_Admin_i::fillNSSet(ccReg::NSSetDetail* cn, Register::NSSet::NSSet* n)
{
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
  fillNSSet(cn,nl->get(0));
  db.Disconnect();
  return cn;
}

ccReg::NSSetDetail* 
ccReg_Admin_i::getNSSetById(ccReg::TID id)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::NSSet::Manager *nr = r->getNSSetManager();
  Register::NSSet::List *nl = nr->getList();
  nl->setIdFilter(id);
  nl->reload();
  if (nl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  }
  ccReg::NSSetDetail* cn = new ccReg::NSSetDetail;
  fillNSSet(cn,nl->get(0));
  db.Disconnect();
  return cn;
}

void
ccReg_Admin_i::fillEPPAction(
  ccReg::EPPAction* cea, 
  const Register::Registrar::EPPAction *rea
)
{
  cea->id = rea->getId();
  cea->xml = DUPSTRFUN(rea->getEPPMessage);
  cea->time = DUPSTRDATE(rea->getStartTime);
  cea->type = DUPSTRFUN(rea->getTypeName);
  cea->objectHandle = DUPSTRFUN(rea->getHandle);
  cea->registrarHandle = DUPSTRFUN(rea->getRegistrarHandle);
  cea->result = rea->getResult();
  cea->clTRID = DUPSTRFUN(rea->getClientTransactionId);
  cea->svTRID = DUPSTRFUN(rea->getServerTransactionId);
}

ccReg::EPPAction* 
ccReg_Admin_i::getEPPActionBySvTRID(const char* svTRID)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Registrar::Manager *rm = r->getRegistrarManager();
  Register::Registrar::EPPActionList *eal = rm->getEPPActionList();
  eal->setSvTRIDFilter(svTRID);
  eal->reload();
  if (eal->size() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  } 
  ccReg::EPPAction* ea = new ccReg::EPPAction;
  fillEPPAction(ea,eal->get(0));
  db.Disconnect();
  return ea;
}

ccReg::EPPAction* 
ccReg_Admin_i::getEPPActionById(ccReg::TID id)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Registrar::Manager *rm = r->getRegistrarManager();
  Register::Registrar::EPPActionList *eal = rm->getEPPActionList();
  eal->setIdFilter(id);
  eal->reload();
  if (eal->size() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  } 
  ccReg::EPPAction* ea = new ccReg::EPPAction;
  fillEPPAction(ea,eal->get(0));
  db.Disconnect();
  return ea;
}

void 
ccReg_Admin_i::fillDomain(ccReg::DomainDetail* cd, Register::Domain::Domain* d)
{
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
  fillDomain(cd,dl->get(0));
  db.Disconnect();
  return cd;
}

ccReg::DomainDetail*
ccReg_Admin_i::getDomainById(ccReg::TID id)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Domain::Manager *dm = r->getDomainManager();
  Register::Domain::List *dl = dm->getList();
  dl->setIdFilter(id);
  dl->reload();
  if (dl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  } 
  ccReg::DomainDetail* cd = new ccReg::DomainDetail;
  fillDomain(cd,dl->get(0));
  db.Disconnect();
  return cd;
}

void
ccReg_Admin_i::fillAuthInfoRequest(
  ccReg::AuthInfoRequest::Detail *carid,
  Register::AuthInfoRequest::Detail *rarid
)
{
  carid->id = rarid->getId();
  carid->handle = DUPSTRFUN(rarid->getObjectHandle);
  switch (rarid->getRequestStatus()) {
    case Register::AuthInfoRequest::RS_NEW :
      carid->status = ccReg::AuthInfoRequest::RS_NEW; break;
    case Register::AuthInfoRequest::RS_ANSWERED :
      carid->status = ccReg::AuthInfoRequest::RS_ANSWERED; break;
    case Register::AuthInfoRequest::RS_INVALID :
      carid->status = ccReg::AuthInfoRequest::RS_INVALID; break;
  }
  switch (rarid->getRequestType()) {
    case Register::AuthInfoRequest::RT_EPP :
      carid->type = ccReg::AuthInfoRequest::RT_EPP; break;
    case Register::AuthInfoRequest::RT_AUTO_PIF :
      carid->type = ccReg::AuthInfoRequest::RT_AUTO_PIF; break;
    case Register::AuthInfoRequest::RT_EMAIL_PIF :
      carid->type = ccReg::AuthInfoRequest::RT_EMAIL_PIF; break;
    case Register::AuthInfoRequest::RT_POST_PIF :
      carid->type = ccReg::AuthInfoRequest::RT_POST_PIF; break;
  }
  carid->crTime = DUPSTRDATE(rarid->getCreationTime);
  carid->closeTime = DUPSTRDATE(rarid->getClosingTime);
  carid->reason = DUPSTRFUN(rarid->getReason);
  carid->svTRID = DUPSTRFUN(rarid->getSvTRID);
  carid->email = DUPSTRFUN(rarid->getEmailToAnswer);
  carid->answerEmailId = rarid->getAnswerEmailId();
  switch (rarid->getObjectType()) {
    case Register::AuthInfoRequest::OT_DOMAIN :
      carid->oType = ccReg::AuthInfoRequest::OT_DOMAIN; break;
    case Register::AuthInfoRequest::OT_CONTACT :
      carid->oType = ccReg::AuthInfoRequest::OT_CONTACT; break;
    case Register::AuthInfoRequest::OT_NSSET :
      carid->oType = ccReg::AuthInfoRequest::OT_NSSET; break;
  }
  carid->objectId = rarid->getObjectId();
  carid->registrar = DUPSTRFUN(rarid->getRegistrarName);
}

ccReg::AuthInfoRequest::Detail* 
ccReg_Admin_i::getAuthInfoRequestById(ccReg::TID id)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB db;
  db.OpenDatabase(database.c_str());
  MailerManager mm(ns);
  std::auto_ptr<Register::AuthInfoRequest::Manager> r(
    Register::AuthInfoRequest::Manager::create(&db,&mm)
  );
  Register::AuthInfoRequest::List *airl = r->getList();
  airl->setIdFilter(id);
  airl->reload();
  if (airl->getCount() != 1) {
    db.Disconnect();
    throw ccReg::Admin::ObjectNotFound();
  } 
  ccReg::AuthInfoRequest::Detail* aird = new ccReg::AuthInfoRequest::Detail;
  fillAuthInfoRequest(aird,airl->get(0));
  db.Disconnect();
  return aird;
}

ccReg::Mailing::Detail* 
ccReg_Admin_i::getEmailById(ccReg::TID id)
  throw (ccReg::Admin::ObjectNotFound)
{
  MailerManager mm(ns);
  MailerManager::Filter mf;
  mf.id = id;
  try { mm.reload(mf); }
  catch (...) { throw ccReg::Admin::ObjectNotFound(); }
  if (mm.getMailList().size() != 1) throw ccReg::Admin::ObjectNotFound();
  MailerManager::Detail& mld = mm.getMailList()[0];
  ccReg::Mailing::Detail* md = new ccReg::Mailing::Detail;
  md->id = mld.id;
  md->status = mld.status;
  md->createTime = DUPSTRC(mld.createTime);
  md->modTime = DUPSTRC(mld.modTime);
  md->content = DUPSTRC(mld.content);
  md->handles.length(mld.handles.size());
  for (unsigned i=0; i<mld.handles.size(); i++)
    md->handles[i] = DUPSTRC(mld.handles[i]);
  md->attachments.length(mld.attachments.size());
  for (unsigned i=0; i<mld.attachments.size(); i++)
    md->attachments[i] = DUPSTRC(mld.attachments[i]);
  return md;
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

ccReg::EPPActionTypeSeq* 
ccReg_Admin_i::getEPPActionTypeList()
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Registrar::Manager *rm = r->getRegistrarManager();
  ccReg::EPPActionTypeSeq *et = new ccReg::EPPActionTypeSeq;
  et->length(rm->getEPPActionTypeCount());
  for (unsigned i=0; i<rm->getEPPActionTypeCount(); i++)
    (*et)[i] = DUPSTRC(rm->getEPPActionTypeByIdx(i));
  return et;
}

ccReg::CountryDescSeq* 
ccReg_Admin_i::getCountryDescList()
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  ccReg::CountryDescSeq *cd = new ccReg::CountryDescSeq;
  cd->length(r->getCountryDescSize());
  for (unsigned i=0; i<r->getCountryDescSize(); i++) {
    (*cd)[i].cc = DUPSTRC(r->getCountryDescByIdx(i).cc);
    (*cd)[i].name = DUPSTRC(r->getCountryDescByIdx(i).name);
  }
  return cd;
}

char* 
ccReg_Admin_i::getDefaultCountry()
{
  return CORBA::string_dup("CZ");
}

ccReg::ObjectStatusDescSeq* 
ccReg_Admin_i::getDomainStatusDescList()
{
  return new ccReg::ObjectStatusDescSeq; //TODO
}

ccReg::ObjectStatusDescSeq* 
ccReg_Admin_i::getContactStatusDescList()
{
  return new ccReg::ObjectStatusDescSeq; //TODO
}

ccReg::ObjectStatusDescSeq* 
ccReg_Admin_i::getNSSetStatusDescList()
{
  return new ccReg::ObjectStatusDescSeq; // TODO
}

ccReg::TID 
ccReg_Admin_i::createAuthInfoRequest(
  ccReg::TID objectId, 
  ccReg::AuthInfoRequest::RequestType type, 
  ccReg::TID eppActionId, 
  const char* requestReason,
  const char* emailToAnswer
) throw (
  ccReg::Admin::BAD_EMAIL, ccReg::Admin::OBJECT_NOT_FOUND, 
  ccReg::Admin::ACTION_NOT_FOUND, ccReg::Admin::SQL_ERROR,
  ccReg::Admin::INVALID_INPUT
)
{
  DB db;
  db.OpenDatabase(database.c_str());
  MailerManager mm(ns);
  std::auto_ptr<Register::AuthInfoRequest::Manager> r(
    Register::AuthInfoRequest::Manager::create(&db,&mm)
  );
  Register::AuthInfoRequest::RequestType rtype;
  switch (type) {
    case ccReg::AuthInfoRequest::RT_IGNORE:
      db.Disconnect();
      throw ccReg::Admin::INVALID_INPUT();
    case ccReg::AuthInfoRequest::RT_EPP:
      rtype = Register::AuthInfoRequest::RT_EPP; break;
    case ccReg::AuthInfoRequest::RT_AUTO_PIF:
      rtype = Register::AuthInfoRequest::RT_AUTO_PIF; break;
    case ccReg::AuthInfoRequest::RT_EMAIL_PIF:
      rtype = Register::AuthInfoRequest::RT_EMAIL_PIF; break;
    case ccReg::AuthInfoRequest::RT_POST_PIF:
      rtype = Register::AuthInfoRequest::RT_POST_PIF; break;
  };
  unsigned long ret = 0;
  try {
    ret = r->createRequest(
      objectId,rtype,eppActionId,requestReason,emailToAnswer
    );
  } 
  catch (Register::AuthInfoRequest::Manager::BAD_EMAIL) { 
    db.Disconnect();
    throw ccReg::Admin::BAD_EMAIL();
  } 
  catch (Register::AuthInfoRequest::Manager::OBJECT_NOT_FOUND) {
    db.Disconnect();    
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  } 
  catch (Register::AuthInfoRequest::Manager::ACTION_NOT_FOUND) { 
    db.Disconnect();
    throw ccReg::Admin::ACTION_NOT_FOUND();
  } 
  catch (Register::SQL_ERROR) {
    db.Disconnect();  
    throw ccReg::Admin::SQL_ERROR();
  } 
  db.Disconnect();
  return ret;
}

void 
ccReg_Admin_i::processAuthInfoRequest(ccReg::TID id, CORBA::Boolean invalid) 
  throw (ccReg::Admin::SQL_ERROR, ccReg::Admin::OBJECT_NOT_FOUND)
{
  DB db;
  db.OpenDatabase(database.c_str());
  MailerManager mm(ns);
  std::auto_ptr<Register::AuthInfoRequest::Manager> r(
    Register::AuthInfoRequest::Manager::create(&db,&mm)
  );
  try {
    r->processRequest(id,invalid);
  } 
  catch (Register::SQL_ERROR) { 
    db.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  } 
  catch (Register::AuthInfoRequest::Manager::OBJECT_NOT_FOUND) { 
    db.Disconnect();
    throw ccReg::Admin::OBJECT_NOT_FOUND();
  } 
  db.Disconnect();  
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_Session_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_Session_i::ccReg_Session_i(const std::string& database, NameService *ns)
 : mm(ns)
{
  db.OpenDatabase(database.c_str());
  m.reset(Register::Manager::create(&db));
  am.reset(Register::AuthInfoRequest::Manager::create(&db,&mm));
  reg = new ccReg_Registrars_i(m->getRegistrarManager()->getList());
  eppa = new ccReg_EPPActions_i(m->getRegistrarManager()->getEPPActionList());
  dm = new ccReg_Domains_i(m->getDomainManager()->getList());
  cm = new ccReg_Contacts_i(m->getContactManager()->getList());
  nm = new ccReg_NSSets_i(m->getNSSetManager()->getList());
  airm = new ccReg_AIRequests_i(am->getList());
  mml = new ccReg_Mails_i(ns);
}

ccReg_Session_i::~ccReg_Session_i()
{
  // TODO: Delete all tables;
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

ccReg::AuthInfoRequests_ptr 
ccReg_Session_i::getAuthInfoRequests()
{
  return airm->_this();
}

ccReg::Mails_ptr 
ccReg_Session_i::getMails()
{
  return mml->_this();
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

ccReg::TID 
ccReg_Registrars_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Registrar::Registrar *r = rl->get(row);
  if (!r) throw ccReg::Table::INVALID_ROW();
  return r->getId();  
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

CORBA::ULongLong 
ccReg_Registrars_i::resultSize()
{
  return 12345;
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
  ch->length(7);
  COLHEAD(ch,0,"SvTRID",CT_OTHER);
  COLHEAD(ch,1,"ClTRID",CT_OTHER);
  COLHEAD(ch,2,"Type",CT_OTHER);
  COLHEAD(ch,3,"Handle",CT_OTHER);
  COLHEAD(ch,4,"Registrar",CT_REGISTRAR_HANDLE);
  COLHEAD(ch,5,"Time",CT_OTHER);
  COLHEAD(ch,6,"Result",CT_OTHER);
  return ch;
}

ccReg::TableRow* 
ccReg_EPPActions_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Registrar::EPPAction *a = eal->get(row);
  if (!a) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(7);
  (*tr)[0] = DUPSTRFUN(a->getServerTransactionId);
  (*tr)[1] = DUPSTRFUN(a->getClientTransactionId);
  (*tr)[2] = DUPSTRFUN(a->getTypeName);
  (*tr)[3] = DUPSTRFUN(a->getHandle);
  (*tr)[4] = DUPSTRFUN(a->getRegistrarHandle); 
  (*tr)[5] = DUPSTRDATE(a->getStartTime);
  (*tr)[6] = DUPSTRFUN(a->getResultStatus);
  return tr;
}

void 
ccReg_EPPActions_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

ccReg::TID 
ccReg_EPPActions_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Registrar::EPPAction *a = eal->get(row);
  if (!a) throw ccReg::Table::INVALID_ROW();
  return a->getId();  
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
  return 7;
}

void 
ccReg_EPPActions_i::reload()
{
  eal->reload();
}

ccReg::TID 
ccReg_EPPActions_i::registrar()
{
  return registrarFilter;
}

void 
ccReg_EPPActions_i::registrar(ccReg::TID _v)
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

char* 
ccReg_EPPActions_i::xml()
{
  return CORBA::string_dup(xmlFilter.c_str());
}

void 
ccReg_EPPActions_i::xml(const char* _v)
{
  xmlFilter = _v;
  eal->setXMLFilter(_v); 
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

ccReg::DateTimeInterval 
ccReg_EPPActions_i::time()
{
  return timeFilter;
}

void 
ccReg_EPPActions_i::time(const ccReg::DateTimeInterval& _v)
{
  timeFilter = _v;
  eal->setTimePeriodFilter(setPeriod(_v));
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

ccReg::EPPActionsFilter::ResultType 
ccReg_EPPActions_i::resultClass()
{
  return resultClassFilter;
}

void 
ccReg_EPPActions_i::resultClass(ccReg::EPPActionsFilter::ResultType _v)
{
  resultClassFilter = _v;
  eal->setResultFilter( 
    _v == ccReg::EPPActionsFilter::RT_OK ? Register::Registrar::EARF_OK :
    _v == ccReg::EPPActionsFilter::RT_FAIL ? Register::Registrar::EARF_FAIL :
                                             Register::Registrar::EARF_ALL
  );
    
  
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
  timeFilter.from.date.year = 0;;
  timeFilter.from.date.month = 0;
  timeFilter.from.date.day = 0;
  timeFilter.from.hour = 0;
  timeFilter.from.minute = 0;
  timeFilter.from.second = 0;
  timeFilter.to.date.year = 0;;
  timeFilter.to.date.month = 0;
  timeFilter.to.date.day = 0;
  timeFilter.to.hour = 0;
  timeFilter.to.minute = 0;
  timeFilter.to.second = 0;
  clTRIDFilter = "";
  svTRIDFilter = "";
  resultClassFilter = ccReg::EPPActionsFilter::RT_ALL;
  eal->clearFilter();
}

CORBA::ULongLong 
ccReg_EPPActions_i::resultSize()
{
  return 12345;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_RegObjectFilter_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_RegObjectFilter_i::ccReg_RegObjectFilter_i(Register::ObjectList *_ol)
  : registrarFilter(0), ol(_ol)
{
}

ccReg::TID 
ccReg_RegObjectFilter_i::registrar()
{
  return registrarFilter;
}

void 
ccReg_RegObjectFilter_i::registrar(ccReg::TID _v)
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

ccReg::TID 
ccReg_RegObjectFilter_i::createRegistrar()
{
  return createRegistrarFilter;
}

void 
ccReg_RegObjectFilter_i::createRegistrar(ccReg::TID _v)
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

ccReg::TID 
ccReg_RegObjectFilter_i::updateRegistrar()
{
  return updateRegistrarFilter;
}

void 
ccReg_RegObjectFilter_i::updateRegistrar(ccReg::TID _v)
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

ccReg::ObjectStatusSeq *
ccReg_RegObjectFilter_i::status()
{
  return new ccReg::ObjectStatusSeq(statusFilter);
}


void 
ccReg_RegObjectFilter_i::status(const ccReg::ObjectStatusSeq& _v)
{
  statusFilter = _v;
}


void 
ccReg_RegObjectFilter_i::clear()
{
  registrarFilter = 0;
  registrarHandleFilter = "";
  createRegistrarFilter = 0;
  createRegistrarHandleFilter = "";
  updateRegistrarFilter = 0;
  updateRegistrarHandleFilter = "";

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
  COLHEAD(ch,0,"FQDN",CT_DOMAIN_HANDLE);
  COLHEAD(ch,1,"CrDate",CT_OTHER);
  COLHEAD(ch,2,"Delete",CT_OTHER);
  COLHEAD(ch,3,"Registrant",CT_CONTACT_HANDLE);
  COLHEAD(ch,4,"Registrant name",CT_OTHER);
  COLHEAD(ch,5,"Registrator",CT_REGISTRAR_HANDLE);
  COLHEAD(ch,6,"In zone",CT_OTHER);
  COLHEAD(ch,7,"Expiration",CT_OTHER);
  COLHEAD(ch,8,"Cancel",CT_OTHER);
  COLHEAD(ch,9,"Out Zone",CT_OTHER);
  COLHEAD(ch,10,"Validation",CT_OTHER);
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

ccReg::TID 
ccReg_Domains_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Domain::Domain *d = dl->get(row);
  if (!d) throw ccReg::Table::INVALID_ROW();
  return d->getId();  
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

ccReg::TID 
ccReg_Domains_i::registrant()
{
  return registrantFilter;
}

void 
ccReg_Domains_i::registrant(ccReg::TID _v)
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

ccReg::TID 
ccReg_Domains_i::nsset()
{
  return nssetFilter;
}

void 
ccReg_Domains_i::nsset(ccReg::TID _v)
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

ccReg::TID 
ccReg_Domains_i::admin()
{
  return adminFilter;
}

void 
ccReg_Domains_i::admin(ccReg::TID _v)
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

CORBA::ULongLong 
ccReg_Domains_i::resultSize()
{
  return 12345;
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
  ch->length(5);
  COLHEAD(ch,0,"Handle",CT_CONTACT_HANDLE);
  COLHEAD(ch,1,"Name",CT_OTHER);
  COLHEAD(ch,2,"Organization",CT_OTHER);
  COLHEAD(ch,3,"CrDate",CT_OTHER);
  COLHEAD(ch,4,"Registrar",CT_REGISTRAR_HANDLE);
  return ch;
}

ccReg::TableRow* 
ccReg_Contacts_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Contact::Contact *c = cl->get(row);
  if (!c) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(5);
  (*tr)[0] = DUPSTRFUN(c->getHandle);
  (*tr)[1] = DUPSTRFUN(c->getName);
  (*tr)[2] = DUPSTRFUN(c->getOrganization);
  (*tr)[3] = DUPSTRDATE(c->getCreateDate);
  (*tr)[4] = DUPSTRFUN(c->getRegistrarHandle); 
  return tr;
}

void 
ccReg_Contacts_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

ccReg::TID 
ccReg_Contacts_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Contact::Contact *c = cl->get(row);
  if (!c) throw ccReg::Table::INVALID_ROW();
  return c->getId();  
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
  return 5;
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

CORBA::ULongLong 
ccReg_Contacts_i::resultSize()
{
  return 12345;
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

ccReg::TID 
ccReg_NSSets_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::NSSet::NSSet *n = nl->get(row);
  if (!n) throw ccReg::Table::INVALID_ROW();
  return n->getId();
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

CORBA::ULongLong 
ccReg_NSSets_i::resultSize()
{
  return 12345;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_AIRequests_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_AIRequests_i::ccReg_AIRequests_i(
  Register::AuthInfoRequest::List *_airl
)
  : airl(_airl)
{
}

ccReg_AIRequests_i::~ccReg_AIRequests_i()
{
}

ccReg::Table::ColumnHeaders* 
ccReg_AIRequests_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(5);
  COLHEAD(ch,0,"RequestId",CT_OTHER);
  COLHEAD(ch,1,"CrDate",CT_OTHER);
  COLHEAD(ch,2,"Handle",CT_OTHER);
  COLHEAD(ch,3,"Type",CT_OTHER);
  COLHEAD(ch,4,"Status",CT_OTHER);
  return ch;
}

ccReg::TableRow* 
ccReg_AIRequests_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::AuthInfoRequest::Detail *aird = airl->get(row);
  if (!aird) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(5);
  std::stringstream id;
  id << aird->getId();
  (*tr)[0] = DUPSTRFUN(id.str);
  (*tr)[1] = DUPSTRDATE(aird->getCreationTime);
  (*tr)[2] = DUPSTRFUN(aird->getObjectHandle);
  std::string type;
  switch (aird->getRequestType()) {
    case Register::AuthInfoRequest::RT_EPP : type = "EPP"; break;
    case Register::AuthInfoRequest::RT_AUTO_PIF : type = "AUTO_PIF"; break;
    case Register::AuthInfoRequest::RT_EMAIL_PIF : type = "MAIL_PIF"; break;
    case Register::AuthInfoRequest::RT_POST_PIF : type = "POST_PIF"; break;
  }
  (*tr)[3] = DUPSTRC(type);
  std::string status;
  switch (aird->getRequestStatus()) {
    case Register::AuthInfoRequest::RS_NEW : status = "NEW"; break;
    case Register::AuthInfoRequest::RS_ANSWERED : status = "CLOSED"; break;
    case Register::AuthInfoRequest::RS_INVALID : status = "INVALID"; break;
  }
  (*tr)[4] = DUPSTRC(status);
  return tr;
}

void 
ccReg_AIRequests_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

ccReg::TID 
ccReg_AIRequests_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::AuthInfoRequest::Detail *aird = airl->get(row);
  if (!aird) throw ccReg::Table::INVALID_ROW();
  return aird->getId();
}

char*
ccReg_AIRequests_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_AIRequests_i::numRows()
{
  return airl->getCount();
}

CORBA::Short 
ccReg_AIRequests_i::numColumns()
{
  return 5;
}

void 
ccReg_AIRequests_i::reload()
{
  airl->reload();
}

#define FILTER_IMPL(FUNC,PTYPEGET,PTYPESET,MEMBER,MEMBERG, SETF) \
PTYPEGET FUNC() { return MEMBERG; } \
void FUNC(PTYPESET _v) { MEMBER = _v; SETF; }

#define FILTER_IMPL_L(FUNC,MEMBER,SETF) \
  FILTER_IMPL(FUNC,ccReg::TID,ccReg::TID,MEMBER,MEMBER, SETF)

#define FILTER_IMPL_S(FUNC,MEMBER,SETF) \
  FILTER_IMPL(FUNC,char *,const char *,MEMBER,DUPSTRC(MEMBER), SETF)

FILTER_IMPL_L(ccReg_AIRequests_i::id,idFilter,airl->setIdFilter(_v));

FILTER_IMPL_S(ccReg_AIRequests_i::handle,handleFilter,
              airl->setHandleFilter(_v));

FILTER_IMPL_S(ccReg_AIRequests_i::email,emailFilter,
              airl->setEmailFilter(_v));

FILTER_IMPL_S(ccReg_AIRequests_i::reason,reasonFilter,
              airl->setReasonFilter(_v));

FILTER_IMPL_S(ccReg_AIRequests_i::svTRID,svTRIDFilter,
              airl->setSvTRIDFilter(_v));

FILTER_IMPL(ccReg_AIRequests_i::type,
            ccReg::AuthInfoRequest::RequestType,
            ccReg::AuthInfoRequest::RequestType,
            requestTypeFilter,requestTypeFilter,
            airl->setRequestTypeIgnoreFilter(
              _v == ccReg::AuthInfoRequest::RT_IGNORE
            );
            airl->setRequestTypeFilter(
             _v == ccReg::AuthInfoRequest::RT_EPP ? 
               Register::AuthInfoRequest::RT_EPP : 
             _v == ccReg::AuthInfoRequest::RT_AUTO_PIF ? 
               Register::AuthInfoRequest::RT_AUTO_PIF : 
             _v == ccReg::AuthInfoRequest::RT_EMAIL_PIF ? 
               Register::AuthInfoRequest::RT_EMAIL_PIF : 
               Register::AuthInfoRequest::RT_POST_PIF 
            ));

FILTER_IMPL(ccReg_AIRequests_i::status,
            ccReg::AuthInfoRequest::RequestStatus,
            ccReg::AuthInfoRequest::RequestStatus,
            requestStatusFilter,requestStatusFilter,
            airl->setRequestStatusIgnoreFilter(
              _v == ccReg::AuthInfoRequest::RS_IGNORE
            );
            airl->setRequestStatusFilter(
             _v == ccReg::AuthInfoRequest::RS_NEW ? 
               Register::AuthInfoRequest::RS_NEW : 
             _v == ccReg::AuthInfoRequest::RS_ANSWERED ? 
               Register::AuthInfoRequest::RS_ANSWERED : 
               Register::AuthInfoRequest::RS_INVALID 
            ));
            
FILTER_IMPL(ccReg_AIRequests_i::crTime,
            ccReg::DateTimeInterval,
            const ccReg::DateTimeInterval&,
            crTimeFilter,crTimeFilter,
            airl->setCreationTimeFilter(setPeriod(_v)));

FILTER_IMPL(ccReg_AIRequests_i::closeTime,
            ccReg::DateTimeInterval,
            const ccReg::DateTimeInterval&,
            closeTimeFilter,closeTimeFilter,
            airl->setCloseTimeFilter(setPeriod(_v)));

ccReg::Filter_ptr
ccReg_AIRequests_i::aFilter()
{
  return _this();
}

void
ccReg_AIRequests_i::clear()
{
  idFilter = 0;
  handleFilter = "";
  emailFilter = "";
  svTRIDFilter = "";
  reasonFilter = "";
  // TODO CLEAR OTHER 
  airl->clearFilter();
}

CORBA::ULongLong 
ccReg_AIRequests_i::resultSize()
{
  return 12345;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_Mails_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_Mails_i::ccReg_Mails_i(NameService *ns) : 
  idFilter(0), statusFilter(-1), mm(ns)
{
}

ccReg_Mails_i::~ccReg_Mails_i()
{
}

ccReg::Table::ColumnHeaders* 
ccReg_Mails_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(3);
  COLHEAD(ch,0,"Id",CT_OTHER);
  COLHEAD(ch,1,"CrDate",CT_OTHER);
  COLHEAD(ch,2,"Type",CT_OTHER);
  return ch;
}

ccReg::TableRow* 
ccReg_Mails_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  if ((unsigned)row >= mm.getMailList().size()) throw ccReg::Table::INVALID_ROW();
  MailerManager::Detail& md = mm.getMailList()[row];
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(3);
  std::stringstream id;
  id << md.id;
  (*tr)[0] = DUPSTRC(id.str());
  (*tr)[1] = DUPSTRC(md.createTime);
  (*tr)[2] = DUPSTRC(std::string(""));
  return tr;
}

void 
ccReg_Mails_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

ccReg::TID 
ccReg_Mails_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  if ((unsigned)row >= mm.getMailList().size()) throw ccReg::Table::INVALID_ROW();
  MailerManager::Detail& md = mm.getMailList()[row];
  return md.id;
}

char*
ccReg_Mails_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_Mails_i::numRows()
{
  return mm.getMailList().size();
}

CORBA::Short 
ccReg_Mails_i::numColumns()
{
  return 3;
}

void 
ccReg_Mails_i::reload()
{
  MailerManager::Filter mf;
  mf.id = idFilter;
  mf.status = statusFilter;
  mf.handle = handleFilter;
  mf.attachment = attachmentFilter;
  try {
    mm.reload(mf);
  } catch (...) {}
}

#define FILTER_IMPL(FUNC,PTYPEGET,PTYPESET,MEMBER,MEMBERG, SETF) \
PTYPEGET FUNC() { return MEMBERG; } \
void FUNC(PTYPESET _v) { MEMBER = _v; SETF; }

#define FILTER_IMPL_L(FUNC,MEMBER,SETF) \
  FILTER_IMPL(FUNC,ccReg::TID,ccReg::TID,MEMBER,MEMBER, SETF)

#define FILTER_IMPL_S(FUNC,MEMBER,SETF) \
  FILTER_IMPL(FUNC,char *,const char *,MEMBER,DUPSTRC(MEMBER), SETF)

FILTER_IMPL_L(ccReg_Mails_i::id,idFilter,clear());

FILTER_IMPL(ccReg_Mails_i::status,CORBA::Long,CORBA::Long,statusFilter,statusFilter,clear());

FILTER_IMPL(ccReg_Mails_i::type,CORBA::UShort,CORBA::UShort,typeFilter,typeFilter,clear());

FILTER_IMPL_S(ccReg_Mails_i::handle,handleFilter,
              clear());

FILTER_IMPL_S(ccReg_Mails_i::fulltext,fulltextFilter,
              clear());

FILTER_IMPL_S(ccReg_Mails_i::attachment,attachmentFilter,
              clear());

FILTER_IMPL(ccReg_Mails_i::createTime,
            ccReg::DateTimeInterval,
            const ccReg::DateTimeInterval&,
            createTimeFilter,createTimeFilter,
            clear() // TODO: function
             );

ccReg::Filter_ptr
ccReg_Mails_i::aFilter()
{
  return _this();
}

void
ccReg_Mails_i::clear()
{
  idFilter = 0;
//  handleFilter = "";
//  emailFilter = "";
//  svTRIDFilter = "";
//  reasonFilter = "";
  // TODO CLEAR OTHER 
//  airl->clearFilter();
}

CORBA::ULongLong 
ccReg_Mails_i::resultSize()
{
  return 12345;
}


