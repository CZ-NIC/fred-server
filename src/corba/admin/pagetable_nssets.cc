#include "pagetable_nssets.h"

ccReg_NSSets_i::ccReg_NSSets_i(Register::NSSet::List *_nl) : nl(_nl) {
}

ccReg_NSSets_i::~ccReg_NSSets_i() {
  TRACE("[CALL] ccReg_NSSets_i::~ccReg_NSSets_i()");
}

ccReg::Filters::Compound_ptr ccReg_NSSets_i::add() {
  TRACE("[CALL] ccReg_NSSets_i::add()");
  it.clearF();
  DBase::Filters::NSSet *f = new DBase::Filters::NSSetHistoryImpl();
  uf.addFilter(f);
  return it.addE(f); 
}


ccReg::Table::ColumnHeaders* ccReg_NSSets_i::getColumnHeaders() {
  TRACE("[CALL] ccReg_NSSets_i::getColumnHeaders()");
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(4);
  COLHEAD(ch,0,"Handle",CT_NSSET_HANDLE);
  COLHEAD(ch,1,"Create date",CT_OTHER);
  COLHEAD(ch,2,"Delete date",CT_OTHER);
  COLHEAD(ch,3,"Registrar",CT_REGISTRAR_HANDLE);
  return ch;
}

ccReg::TableRow* 
ccReg_NSSets_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::NSSet::NSSet *n = nl->getNSSet(row);
  if (!n) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(4);
  (*tr)[0] = DUPSTRFUN(n->getHandle);
  (*tr)[1] = DUPSTRDATE(n->getCreateDate);
  (*tr)[2] = DUPSTRDATED(n->getDeleteDate);
  (*tr)[3] = DUPSTRFUN(n->getRegistrarHandle); 
  return tr;
}

void 
ccReg_NSSets_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  TRACE(boost::format("[CALL] ccReg_NSSets_i::sortByColumn(%1%, %2%)") % column % dir);
  switch (column) {
    case 0:
      nl->sort(Register::NSSet::MT_HANDLE, dir);
      sorted_by_ = 0;
      break;
    case 1:
      nl->sort(Register::NSSet::MT_CRDATE, dir);
      sorted_by_ = 1;
      break;
    case 2:
      nl->sort(Register::NSSet::MT_ERDATE, dir);
      sorted_by_ = 2;
      break;
    case 3:
      nl->sort(Register::NSSet::MT_REGISTRAR_HANDLE, dir);
      sorted_by_ = 3;
      break;
  }
}

ccReg::TID 
ccReg_NSSets_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::NSSet::NSSet *n = nl->getNSSet(row);
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
  return 4;
}

void 
ccReg_NSSets_i::reload()
{
  TRACE("[CALL] ccReg_NSSets_i::reload()");
//  nl->makeRealCount();  
  nl->reload2(uf, dbm);
}

void
ccReg_NSSets_i::clear()
{
  TRACE("[CALL] ccReg_NSSets_i::clear()");
  nl->clearFilter();
  
  ccReg_PageTable_i::clear();
  nl->clear();
}

CORBA::ULongLong 
ccReg_NSSets_i::resultSize()
{
  TRACE("[CALL] ccReg_NSSets_i::resultSize()");
  return nl->getRealCount(uf);
}

void
ccReg_NSSets_i::loadFilter(ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_NSSets_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  DBase::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    it.addE(dynamic_cast<DBase::Filters::NSSet* >(*uit));
  }
}

void
ccReg_NSSets_i::saveFilter(const char* _name) {
  TRACE(boost::format("[CALL] ccReg_NSSets_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_NSSET, _name, uf);
}

Register::NSSet::NSSet* ccReg_NSSets_i::findId(ccReg::TID _id) {
  try {
    Register::NSSet::NSSet *nsset =
        dynamic_cast<Register::NSSet::NSSet*> (nl->findId(_id));
    if (nsset) {
      return nsset;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}
