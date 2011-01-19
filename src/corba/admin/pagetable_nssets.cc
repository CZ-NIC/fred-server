#include "pagetable_nssets.h"

ccReg_NSSets_i::ccReg_NSSets_i(Fred::NSSet::List *_nl, const Settings *_ptr) : nl(_nl) {
  uf.settings(_ptr);
}

ccReg_NSSets_i::~ccReg_NSSets_i() {
  TRACE("[CALL] ccReg_NSSets_i::~ccReg_NSSets_i()");
}

ccReg::Filters::Compound_ptr ccReg_NSSets_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_NSSets_i::add()");
  Database::Filters::NSSet *f = new Database::Filters::NSSetHistoryImpl();
  uf.addFilter(f);
  return it.addE(f); 
}


Registry::Table::ColumnHeaders* ccReg_NSSets_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_NSSets_i::getColumnHeaders()");
  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(4);
  COLHEAD(ch,0,"Handle",CT_OID);
  COLHEAD(ch,1,"Create date",CT_OTHER);
  COLHEAD(ch,2,"Delete date",CT_OTHER);
  COLHEAD(ch,3,"Registrar",CT_OID);
  return ch;
}

Registry::TableRow* 
ccReg_NSSets_i::getRow(CORBA::UShort row)
  throw (Registry::Table::INVALID_ROW)
{
  Logging::Context ctx(base_context_);

  const Fred::NSSet::NSSet *n = nl->getNSSet(row);
  if (!n) throw Registry::Table::INVALID_ROW();
  Registry::TableRow *tr = new Registry::TableRow;
  tr->length(4);

  MAKE_OID(oid_handle, n->getId(), C_STR(n->getHandle()), FT_NSSET)
  MAKE_OID(oid_registrar, n->getRegistrarId(), C_STR(n->getRegistrarHandle()), FT_REGISTRAR)

  (*tr)[0] <<= oid_handle;
  (*tr)[1] <<= C_STR(n->getCreateDate());
  (*tr)[2] <<= C_STR(n->getDeleteDate());
  (*tr)[3] <<= oid_registrar; 
  return tr;
}

void 
ccReg_NSSets_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_NSSets_i::sortByColumn(%1%, %2%)") % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);
  
  switch (column) {
    case 0:
      nl->sort(Fred::NSSet::MT_HANDLE, dir);
      break;
    case 1:
      nl->sort(Fred::NSSet::MT_CRDATE, dir);
      break;
    case 2:
      nl->sort(Fred::NSSet::MT_ERDATE, dir);
      break;
    case 3:
      nl->sort(Fred::NSSet::MT_REGISTRAR_HANDLE, dir);
      break;
  }
}

ccReg::TID 
ccReg_NSSets_i::getRowId(CORBA::UShort row) 
  throw (Registry::Table::INVALID_ROW)
{
  Logging::Context ctx(base_context_);

  const Fred::NSSet::NSSet *n = nl->getNSSet(row);
  if (!n) throw Registry::Table::INVALID_ROW();
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
  Logging::Context ctx(base_context_);

  return nl->getCount();
}

CORBA::Short 
ccReg_NSSets_i::numColumns()
{
  Logging::Context ctx(base_context_);

  return 4;
}

void 
ccReg_NSSets_i::reload_worker()
{
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_NSSets_i::reload_worker()");
//  nl->makeRealCount();
  nl->setTimeout(query_timeout);
  nl->reload(uf);
  nl->deleteDuplicatesId();
}

void
ccReg_NSSets_i::clear()
{
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_NSSets_i::clear()");
  nl->clearFilter();
  
  ccReg_PageTable_i::clear();
  nl->clear();
}

CORBA::ULongLong 
ccReg_NSSets_i::resultSize()
{
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_NSSets_i::resultSize()");
  return nl->getRealCount(uf);
}

void
ccReg_NSSets_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_NSSets_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::NSSet *tmp = dynamic_cast<Database::Filters::NSSet* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_NSSets_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void
ccReg_NSSets_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_NSSets_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Fred::Filter::Manager>
      tmp_filter_manager(Fred::Filter::Manager::create());
  tmp_filter_manager->save(Fred::Filter::FT_NSSET, _name, uf);
}

Fred::NSSet::NSSet* ccReg_NSSets_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    Fred::NSSet::NSSet *nsset =
        dynamic_cast<Fred::NSSet::NSSet*> (nl->findId(_id));
    if (nsset) {
      return nsset;
    }
    return 0;
  }
  catch (Fred::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_NSSets_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return nl->isLimited(); 
}

