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
  TRACE("[CALL] void ccReg_Registrars_i::reload()");
  rl->reload2(uf,dbm);
}

ccReg::Filters::Compound_ptr 
ccReg_Registrars_i::add() {
  TRACE("[CALL] ccReg_Registrars_i::add()");
  it.clearF();
  DBase::Filters::Registrar *f = new DBase::Filters::RegistrarImpl(true);
  uf.addFilter(f);
  return it.addE(f); 
}

ccReg::Table::ColumnHeaders* 
ccReg_Registrars_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(5);
  COLHEAD(ch,0,"Name",CT_OTHER);
  COLHEAD(ch,1,"Handle",CT_REGISTRAR_HANDLE); 
  COLHEAD(ch,2,"URL",CT_OTHER);
  COLHEAD(ch,3,"Mail",CT_OTHER);
  COLHEAD(ch,4,"Credit",CT_OTHER);
  return ch;
}

ccReg::TableRow* 
ccReg_Registrars_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Registrar::Registrar *r = rl->get(row);
  if (!r) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(5);
  (*tr)[0] = DUPSTRFUN(r->getName); 
  (*tr)[1] = DUPSTRFUN(r->getHandle); 
  (*tr)[2] = DUPSTRFUN(r->getURL);
  (*tr)[3] = DUPSTRFUN(r->getEmail);
  (*tr)[4] = DUPSTRC(Util::stream_cast<std::string>(r->getCredit()));
  return tr;
}

void 
ccReg_Registrars_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  switch (column) {
    case 0:
      rl->sort(Register::Registrar::MT_NAME, dir);
      sorted_by_ = 0;
      break;
    case 1:
      rl->sort(Register::Registrar::MT_HANDLE, dir);
      sorted_by_ = 1;
      break;
    case 2:
      rl->sort(Register::Registrar::MT_URL, dir);
      sorted_by_ = 2;
      break;
    case 3:
      rl->sort(Register::Registrar::MT_MAIL, dir);
      sorted_by_ = 3;
      break;
    case 4:
      rl->sort(Register::Registrar::MT_CREDIT, dir);
      sorted_by_ = 4;
      break;
  }
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
  return 5;
}

void
ccReg_Registrars_i::clear()
{
  TRACE("[CALL] ccReg_Registrars_i::clear()");
  rl->clearFilter();
  
  ccReg_PageTable_i::clear();
  rl->clear();
}

CORBA::ULongLong 
ccReg_Registrars_i::resultSize()
{
  TRACE("ccReg_Registrars_i::resultSize()");
  return rl->getRealCount(uf);
}

void
ccReg_Registrars_i::loadFilter(ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_Registrars_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  DBase::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    DBase::Filters::Registrar *tmp = dynamic_cast<DBase::Filters::Registrar* >(*uit);
    it.addE(tmp);
    TRACE(boost::format("[IN] ccReg_Registrars_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
  }
}

void
ccReg_Registrars_i::saveFilter(const char* _name) {
  TRACE(boost::format("[CALL] ccReg_Registrars_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_REGISTRAR, _name, uf);
}

Register::Registrar::Registrar* ccReg_Registrars_i::findId(ccReg::TID _id) {
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
