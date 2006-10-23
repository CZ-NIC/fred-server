#include "admin.h"
#include "log.h"
#include "util.h"
#include "dbsql.h"
#include "register/register.h"
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <math.h>
#include <memory>

using namespace boost::posix_time;
using namespace boost::gregorian;

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

ccReg::RegistrarList* ccReg_Admin_i::getRegistrars()
{
  int rows = 0 , i ;
  DB DBsql;
  ccReg::RegistrarList *reglist;

  reglist = new ccReg::RegistrarList;

  if( DBsql.OpenDatabase( database.c_str() ) )
  {
    if( DBsql.ExecSelect( "SELECT * FROM REGISTRAR;"  ) )
    {
      rows = DBsql.GetSelectRows();
      LOG( NOTICE_LOG, "getRegistrars: num -> %d",  rows );
      reglist->length( rows );
      for( i = 0 ; i < rows ; i ++ )
      {    
        //  *(reglist[i]) = new ccReg::Registrar;   
        (*reglist)[i].id = atoi( DBsql.GetFieldValueName("ID" , i ) );
        (*reglist)[i].handle=CORBA::string_dup(
          DBsql.GetFieldValueName("handle" , i ) ); // handle
        (*reglist)[i].name=CORBA::string_dup(
          DBsql.GetFieldValueName("name" , i ) ); 
        (*reglist)[i].organization=CORBA::string_dup( 
          DBsql.GetFieldValueName("organization" , i ) ); 
        (*reglist)[i].street1=CORBA::string_dup( 
          DBsql.GetFieldValueName("street1" , i ) );
        (*reglist)[i].street2=CORBA::string_dup( 
          DBsql.GetFieldValueName("street2" , i ) );
        (*reglist)[i].street3=CORBA::string_dup( 
          DBsql.GetFieldValueName("street3" , i ) );
        (*reglist)[i].city=CORBA::string_dup( 
          DBsql.GetFieldValueName("city" , i ) );
        (*reglist)[i].stateorprovince=CORBA::string_dup( 
          DBsql.GetFieldValueName("stateorprovince" , i ) );
        (*reglist)[i].postalcode=CORBA::string_dup( 
          DBsql.GetFieldValueName("postalcode" , i ) );
        (*reglist)[i].country=CORBA::string_dup( 
          DBsql.GetFieldValueName("country" , i ) );
        (*reglist)[i].telephone=CORBA::string_dup( 
          DBsql.GetFieldValueName("telephone" , i ) );
        (*reglist)[i].fax=CORBA::string_dup( 
          DBsql.GetFieldValueName("fax" , i ) );
        (*reglist)[i].email=CORBA::string_dup( 
          DBsql.GetFieldValueName("email" , i ) );
        (*reglist)[i].url=CORBA::string_dup( 
          DBsql.GetFieldValueName("url" , i ) );
        (*reglist)[i].credit=get_price( 
          DBsql.GetFieldValueName("credit" , i ) );
      }
      DBsql.FreeSelect();
    }
    DBsql.Disconnect();
  }
  if( rows == 0 ) reglist->length( 0 ); // nulova delka
  return  reglist;
}

ccReg::Registrar* ccReg_Admin_i::getRegistrarByHandle(const char* handle)
  throw (ccReg::Admin::ObjectNotFound)
{
  DB DBsql;
  ccReg::Registrar *reg;
  bool find=false;
  reg = new ccReg::Registrar ;
  if( DBsql.OpenDatabase( database.c_str() ) )
  { 
    LOG( NOTICE_LOG, "getRegistrByHandle: handle -> %s", handle );
    if( DBsql.SELECTONE( "REGISTRAR" , "handle" , handle  ) )
    {
      if( DBsql.GetSelectRows() != 1 ) throw ccReg::Admin::ObjectNotFound();
      else
        {
          reg->id =  atoi( DBsql.GetFieldValueName("ID" , 0 ) );
          reg->handle=CORBA::string_dup( 
            DBsql.GetFieldValueName("handle" , 0 ) ); // handle
          reg->name=CORBA::string_dup( DBsql.GetFieldValueName("name" , 0 ) );
          reg->organization=CORBA::string_dup( 
            DBsql.GetFieldValueName("organization" , 0 ) );
          reg->street1=CORBA::string_dup( 
            DBsql.GetFieldValueName("street1" , 0 ) );
          reg->street2=CORBA::string_dup( 
            DBsql.GetFieldValueName("street2" , 0 ) );
          reg->street3=CORBA::string_dup( 
            DBsql.GetFieldValueName("street3" , 0 ) );
          reg->city=CORBA::string_dup( 
            DBsql.GetFieldValueName("city" , 0 ) );
          reg->stateorprovince=CORBA::string_dup( 
            DBsql.GetFieldValueName("stateorprovince" , 0 ) );
          reg->postalcode=CORBA::string_dup( 
            DBsql.GetFieldValueName("postalcode" , 0 ) );
          reg->country=CORBA::string_dup( 
            DBsql.GetFieldValueName("country" , 0 ) );
          reg->telephone=CORBA::string_dup( 
            DBsql.GetFieldValueName("telephone" , 0 ) );
          reg->fax=CORBA::string_dup( 
            DBsql.GetFieldValueName("fax" , 0 ) );
          reg->email=CORBA::string_dup( 
            DBsql.GetFieldValueName("email" , 0 ) );
          reg->url=CORBA::string_dup( DBsql.GetFieldValueName("url" , 0 ) );
          reg->credit=get_price( DBsql.GetFieldValueName("credit" , 0 ) );
          find = true;
        }   
      DBsql.FreeSelect();
    }
    DBsql.Disconnect();
  }
 
  if( find == false )
  {
    reg->id = 0;
    reg->handle=CORBA::string_dup( ""  ); // handle
    reg->name=CORBA::string_dup( ""  );
    reg->organization=CORBA::string_dup( "" );
    reg->street1=CORBA::string_dup( "" );
    reg->street2=CORBA::string_dup( "" );
    reg->street3=CORBA::string_dup( "" );
    reg->city=CORBA::string_dup( "" );
    reg->stateorprovince=CORBA::string_dup( "" );
    reg->postalcode=CORBA::string_dup( "" );
    reg->country=CORBA::string_dup( "" );
    reg->telephone=CORBA::string_dup( "" );
    reg->fax=CORBA::string_dup( "" );
    reg->email=CORBA::string_dup( "" );
    reg->url=CORBA::string_dup( "" );
    reg->credit=0;
  }
  return reg;
}

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
ccReg_Admin_i::putRegistrar(const ccReg::Registrar& regData)
{
  DB db;
  db.OpenDatabase(database.c_str());
  std::auto_ptr<Register::Manager> r(Register::Manager::create(&db));
  Register::Registrar::Manager *rm = r->getRegistrarManager();
  Register::Registrar::RegistrarList *rl = rm->getList();
  Register::Registrar::Registrar *reg;
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
  cc->handle = CORBA::string_dup(c->getHandle().c_str());
  cc->registrarHandle = CORBA::string_dup(c->getRegistrarHandle().c_str());
  cc->createDate = CORBA::string_dup(to_simple_string(c->getCreateDate()).c_str());
  db.Disconnect();
  return cc;
}

ccReg::RegObject* 
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
  ccReg::RegObject* o = new ccReg::RegObject;
  Register::NSSet::NSSet *n = nl->get(0);
  o->id = n->getId();
  o->handle = CORBA::string_dup(n->getHandle().c_str());
  o->registrarHandle = CORBA::string_dup(n->getRegistrarHandle().c_str());
  o->crDate = CORBA::string_dup(to_simple_string(n->getCreateDate()).c_str());
  db.Disconnect();
  return o;
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
  cd->fqdn = CORBA::string_dup(d->getFQDN().c_str());
  cd->roid = CORBA::string_dup(d->getROID().c_str());
  cd->registrarHandle = CORBA::string_dup(
    d->getRegistrarHandle().c_str()
  );
  cd->transferDate = CORBA::string_dup(
    to_simple_string(d->getTransferDate()).c_str()
  );
  cd->updateDate = CORBA::string_dup(
    to_simple_string(d->getUpdateDate()).c_str()
  );
  cd->createDate = CORBA::string_dup(
    to_simple_string(d->getCreateDate()).c_str()
  );
  cd->createRegistrarHandle = CORBA::string_dup(
    d->getCreateRegistrarHandle().c_str()
  ); 
  cd->updateRegistrarHandle = CORBA::string_dup(
    d->getUpdateRegistrarHandle().c_str()
  ); 
  cd->authInfo = CORBA::string_dup(
    d->getAuthPw().c_str()
  ); 
  cd->registrantHandle = CORBA::string_dup(
    d->getRegistrantHandle().c_str()
  );
  cd->expirationDate = CORBA::string_dup(
    to_simple_string(d->getExpirationDate()).c_str()
  );
  cd->valExDate = CORBA::string_dup(
    to_simple_string(d->getValExDate()).c_str()
  );
  cd->admins.length(d->getAdminCount());
  for (unsigned i=0; i<d->getAdminCount(); i++)
    cd->admins[i] = CORBA::string_dup("CID:JOUDA");
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
  (*ch)[1].name = CORBA::string_dup("Datum registrace"); 
  (*ch)[1].type = ccReg::Table::CT_OTHER; 
  (*ch)[2].name = CORBA::string_dup("Datum ukončení registrace"); 
  (*ch)[2].type = ccReg::Table::CT_OTHER; 
  (*ch)[3].name = CORBA::string_dup("Držitel"); 
  (*ch)[3].type = ccReg::Table::CT_CONTACT_HANDLE; 
  (*ch)[4].name = CORBA::string_dup("Jméno držitele"); 
  (*ch)[4].type = ccReg::Table::CT_OTHER; 
  (*ch)[5].name = CORBA::string_dup("Registrátor"); 
  (*ch)[5].type = ccReg::Table::CT_REGISTRAR_HANDLE; 
  (*ch)[6].name = CORBA::string_dup("Generováni"); 
  (*ch)[6].type = ccReg::Table::CT_OTHER;
  (*ch)[7].name = CORBA::string_dup("Expirace"); 
  (*ch)[7].type = ccReg::Table::CT_OTHER;
  (*ch)[8].name = CORBA::string_dup("Zrušeni"); 
  (*ch)[8].type = ccReg::Table::CT_OTHER;
  (*ch)[9].name = CORBA::string_dup("Vyřazení z DNS"); 
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
  (*tr)[1] = CORBA::string_dup(to_simple_string(d->getCreateDate()).c_str());
  // delete date
  (*tr)[2] = CORBA::string_dup(
    to_simple_string(ptime(not_a_date_time)).c_str()
  );
  // registrant handle
  (*tr)[3] = CORBA::string_dup(d->getRegistrantHandle().c_str());
  // registrant name 
  (*tr)[4] = CORBA::string_dup(d->getRegistrantName().c_str());
  // registrar handle 
  (*tr)[5] = CORBA::string_dup(d->getRegistrarHandle().c_str());
  // zone generation 
  (*tr)[6] = CORBA::string_dup("N/A");
  // expiration date 
  (*tr)[7] = CORBA::string_dup(
    to_simple_string(d->getExpirationDate()).c_str()
  );
  // zruseni ??
  (*tr)[8] = CORBA::string_dup(
    to_simple_string(ptime(not_a_date_time)).c_str()
  );
  // vyrazeni z dns
  (*tr)[9] = CORBA::string_dup(
    to_simple_string(ptime(not_a_date_time)).c_str()
  );
  // validace
  (*tr)[10] = CORBA::string_dup(to_simple_string(d->getValExDate()).c_str());
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
