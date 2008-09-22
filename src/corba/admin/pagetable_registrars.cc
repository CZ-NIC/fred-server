#include "pagetable_registrars.h"

ccReg_Registrars_i::ccReg_Registrars_i(Register::Registrar::RegistrarList *_rl)
  : rl(_rl)
{
}

ccReg_Registrars_i::~ccReg_Registrars_i() {
  TRACE("[CALL] ccReg_Registrars_i::~ccReg_Registrars_i()");
}

void 
ccReg_Registrars_i::reload() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] void ccReg_Registrars_i::reload()");
  rl->reload(uf,dbm);
}

ccReg::Filters::Compound_ptr 
ccReg_Registrars_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Registrars_i::add()");
  it.clearF();
  Database::Filters::Registrar *f = new Database::Filters::RegistrarImpl(true);
  uf.addFilter(f);
  return it.addE(f); 
}

Registry::Table::ColumnHeaders* 
ccReg_Registrars_i::getColumnHeaders()
{
  Logging::Context ctx(base_context_);

  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(5);
  COLHEAD(ch,0,"Name",CT_OTHER);
  COLHEAD(ch,1,"Handle",CT_OID); 
  COLHEAD(ch,2,"URL",CT_OTHER);
  COLHEAD(ch,3,"Mail",CT_OTHER);
  COLHEAD(ch,4,"Credit",CT_OTHER);
  return ch;
}

Registry::TableRow* 
ccReg_Registrars_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  Logging::Context ctx(base_context_);

  const Register::Registrar::Registrar *r = rl->get(row);
  if (!r) throw ccReg::Table::INVALID_ROW();
  Registry::TableRow *tr = new Registry::TableRow;
  tr->length(5);

  MAKE_OID(oid_handle, r->getId(), DUPSTRFUN(r->getHandle), FT_REGISTRAR)

  (*tr)[0] <<= DUPSTRFUN(r->getName); 
  (*tr)[1] <<= oid_handle;
  (*tr)[2] <<= DUPSTRFUN(r->getURL);
  (*tr)[3] <<= DUPSTRFUN(r->getEmail);
  (*tr)[4] <<= DUPSTRC(Conversion<long unsigned>::to_string(r->getCredit()));
  return tr;
}

void 
ccReg_Registrars_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Registrars_i::sortByColumn(%1%, %2%)") % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);

  switch (column) {
    case 0:
      rl->sort(Register::Registrar::MT_NAME, dir);
      break;
    case 1:
      rl->sort(Register::Registrar::MT_HANDLE, dir);
      break;
    case 2:
      rl->sort(Register::Registrar::MT_URL, dir);
      break;
    case 3:
      rl->sort(Register::Registrar::MT_MAIL, dir);
      break;
    case 4:
      rl->sort(Register::Registrar::MT_CREDIT, dir);
      break;
  }
}

ccReg::TID 
ccReg_Registrars_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  Logging::Context ctx(base_context_);

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
  Logging::Context ctx(base_context_);

  return rl->size();
}

CORBA::Short 
ccReg_Registrars_i::numColumns()
{
  Logging::Context ctx(base_context_);

  return 5;
}

void
ccReg_Registrars_i::clear()
{
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Registrars_i::clear()");
  rl->clearFilter();
  
  ccReg_PageTable_i::clear();
  rl->clear();
}

CORBA::ULongLong 
ccReg_Registrars_i::resultSize()
{
  Logging::Context ctx(base_context_);

  TRACE("ccReg_Registrars_i::resultSize()");
  return rl->getRealCount(uf);
}

void
ccReg_Registrars_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Registrars_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::Registrar *tmp = dynamic_cast<Database::Filters::Registrar* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_Registrars_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void
ccReg_Registrars_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Registrars_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_REGISTRAR, _name, uf);
}

Register::Registrar::Registrar* ccReg_Registrars_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    Register::Registrar::Registrar *registrar = dynamic_cast<Register::Registrar::Registrar* >(rl->findId(_id));
    if (registrar) {
      return registrar;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_Registrars_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return rl->isLimited(); 
}

