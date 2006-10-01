#include "admin.h"
#include "log.h"
#include "util.h"
#include "dbsql.h"
#include "register/register.h"
#include <boost/date_time/posix_time/time_formatters.hpp>

using namespace boost::posix_time;

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
ccReg_PageTable_i::page(CORBA::Short _v)
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
  return numRows()/aPageSize;
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
    reg->id =  0;
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

// primitivni vypis
ccReg::Lists*  ccReg_Admin_i::ObjectList( char* table , char *fname )
{
  DB DBsql;
  ccReg::Lists *list;
  char sqlString[128];
  int rows =0, i;

  list = new ccReg::Lists;

  if( DBsql.OpenDatabase( database.c_str() ) )
  {
    sprintf( sqlString , "SELECT %s FROM %s;" , fname , table );

    if( DBsql.ExecSelect( sqlString ) )
    {
      rows = DBsql.GetSelectRows();
      LOG( NOTICE_LOG, "List: %s  num -> %d",  table , rows );
      list->length( rows );
      for( i = 0 ; i < rows ; i ++ )
      {
        (*list)[i]=CORBA::string_dup( DBsql.GetFieldValue(  i , 0 )  ); 
      }
      DBsql.FreeSelect();
    }
    DBsql.Disconnect();
  }
  if( rows == 0 ) list->length( 0 ); // nulova delka
  return list;
}

ccReg::Lists* ccReg_Admin_i::ListRegistrar()
{
  return ObjectList( "REGISTRAR" , "handle" );
}
ccReg::Lists* ccReg_Admin_i::ListDomain()
{
  return ObjectList( "DOMAIN" , "fqdn" );
}
ccReg::Lists* ccReg_Admin_i::ListContact()
{
  return ObjectList( "CONTACT" , "handle" );
}
ccReg::Lists* ccReg_Admin_i::ListNSSet()
{
  return ObjectList( "NSSET" , "handle" );
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


ccReg_Session_i::ccReg_Session_i(const std::string& database)
{
  db.OpenDatabase(database.c_str());
  m.reset(Register::Manager::create(&db));
  reg = new ccReg_Registrars_i(m->getRegistrarManager()->getList());
  eppa = new ccReg_EPPActions_i(m->getRegistrarManager()->getEPPActionList());
  dm = new ccReg_Domains_i(m->getDomainManager()->getList());
}

ccReg_Session_i::~ccReg_Session_i()
{
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

ccReg::TableRow*
ccReg_Registrars_i::getColumnNames()
{
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(3);
  (*tr)[0] = CORBA::string_dup("Name"); 
  (*tr)[1] = CORBA::string_dup("Handle"); 
  (*tr)[2] = CORBA::string_dup("URL"); 
  return tr;
}

ccReg::TableRow* 
ccReg_Registrars_i::getRow(CORBA::Short row)
{
  const Register::Registrar::Registrar *r = rl->get(row);
  if (!r) return NULL;
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
  rl->setFulltextFilter(fulltextFilter);
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

ccReg::TableRow* 
ccReg_EPPActions_i::getColumnNames()
{
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(4);
  (*tr)[0] = CORBA::string_dup("SessionID"); 
  (*tr)[1] = CORBA::string_dup("Type"); 
  (*tr)[2] = CORBA::string_dup("Registrar"); 
  (*tr)[3] = CORBA::string_dup("Time"); 
  return tr;
}

ccReg::TableRow* 
ccReg_EPPActions_i::getRow(CORBA::Short row)
{
  const Register::Registrar::EPPAction *a = eal->get(row);
  if (!a) return NULL;
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
  eal->setRegistrarFiltr(_v);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ccReg_Domains_i
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ccReg_Domains_i::ccReg_Domains_i(
  Register::Domain::List *_dl
)
  : dl(_dl), registrantFilter(0), registrarFilter(0)
{
}

ccReg_Domains_i::~ccReg_Domains_i()
{
}

ccReg::TableRow* 
ccReg_Domains_i::getColumnNames()
{
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(5);
  (*tr)[0] = CORBA::string_dup("FQDN");
  (*tr)[1] = CORBA::string_dup("CrDate"); 
  (*tr)[2] = CORBA::string_dup("Registrar"); 
  (*tr)[3] = CORBA::string_dup("Registrant"); 
  (*tr)[4] = CORBA::string_dup("NSSet"); 
  return tr;
}

ccReg::TableRow* 
ccReg_Domains_i::getRow(CORBA::Short row)
{
  const Register::Domain::Domain *d = dl->get(row);
  if (!d) return NULL;
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(5);
  (*tr)[0] = CORBA::string_dup(d->getFQDN().c_str());
  (*tr)[1] = CORBA::string_dup(to_simple_string(d->getCreateDate()).c_str());
  (*tr)[2] = CORBA::string_dup(d->getRegistrarHandle().c_str()); 
  (*tr)[3] = CORBA::string_dup(d->getRegistrantHandle().c_str()); 
  (*tr)[4] = CORBA::string_dup(d->getNSSetHandle().c_str()); 
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
  return 5;
}

void 
ccReg_Domains_i::reload()
{
  dl->reload();
}

CORBA::Short 
ccReg_Domains_i::registrar()
{
  return registrarFilter;
}

void 
ccReg_Domains_i::registrar(CORBA::Short _v)
{
  registrarFilter = _v;
  dl->setRegistrarFilter(_v);
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
  //  dl->setRegistrarFilter(_v);
}
