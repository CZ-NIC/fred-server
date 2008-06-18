#include "pagetable_contacts.h"

ccReg_Contacts_i::ccReg_Contacts_i(Register::Contact::List *_cl) :
  cl(_cl) {
}

ccReg_Contacts_i::~ccReg_Contacts_i() {
  TRACE("[CALL] ccReg_Contacts_i::~ccReg_Contacts_i()");
}

ccReg::Filters::Compound_ptr ccReg_Contacts_i::add() {
  TRACE("[CALL] ccReg_Contacts_i::add()");
  it.clearF();
  DBase::Filters::Contact *f = new DBase::Filters::ContactHistoryImpl();
  uf.addFilter(f);
  return it.addE(f);
}

ccReg::Table::ColumnHeaders* ccReg_Contacts_i::getColumnHeaders() {
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(6);
  COLHEAD(ch, 0, "Handle", CT_CONTACT_HANDLE);
  COLHEAD(ch, 1, "Name", CT_OTHER);
  COLHEAD(ch, 2, "Organization", CT_OTHER);
  COLHEAD(ch, 3, "Create date", CT_OTHER);
  COLHEAD(ch, 4, "Delete date", CT_OTHER);
  COLHEAD(ch, 5, "Registrar", CT_REGISTRAR_HANDLE);
  return ch;
}

ccReg::TableRow* ccReg_Contacts_i::getRow(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Contact::Contact *c = cl->getContact(row);
  if (!c) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(6);
  (*tr)[0] = DUPSTRFUN(c->getHandle);
  (*tr)[1] = DUPSTRFUN(c->getName);
  (*tr)[2] = DUPSTRFUN(c->getOrganization);
  (*tr)[3] = DUPSTRDATE(c->getCreateDate);
  (*tr)[4] = DUPSTRDATED(c->getDeleteDate);
  (*tr)[5] = DUPSTRFUN(c->getRegistrarHandle);
  return tr;
}

void ccReg_Contacts_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  TRACE(boost::format("[CALL] ccReg_Contacts_i::sortByColumn(%1%, %2%)") % column % dir);
  switch (column) {
    case 0:
      cl->sort(Register::Contact::MT_HANDLE, dir);
      sorted_by_ = 0;
      break;
    case 1:
      cl->sort(Register::Contact::MT_NAME, dir);
      sorted_by_ = 1;
      break;
    case 2:
      cl->sort(Register::Contact::MT_ORGANIZATION, dir);
      sorted_by_ = 2;
      break;
    case 3:
      cl->sort(Register::Contact::MT_CRDATE, dir);
      sorted_by_ = 3;
      break;
    case 4:
      cl->sort(Register::Contact::MT_ERDATE, dir);
      sorted_by_ = 4;
      break;
    case 5:
      cl->sort(Register::Contact::MT_REGISTRAR_HANDLE, dir);
      sorted_by_ = 5;
      break;
  }
}

ccReg::TID ccReg_Contacts_i::getRowId(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Contact::Contact *c = cl->getContact(row);
  if (!c) throw ccReg::Table::INVALID_ROW();
  return c->getId();
}

char* ccReg_Contacts_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Contacts_i::numRows() {
  return cl->getCount();
}

CORBA::Short ccReg_Contacts_i::numColumns() {
  return 6;
}

void ccReg_Contacts_i::reload() {
//  cl->makeRealCount();
  cl->reload2(uf, dbm);
}

void ccReg_Contacts_i::clear() {
  cl->clearFilter();
  
  ccReg_PageTable_i::clear();
  cl->clear();
}

CORBA::ULongLong ccReg_Contacts_i::resultSize() {
  TRACE("[CALL] ccReg_Contacts_i::resultSize()");
  return cl->getRealCount(uf);
}

void ccReg_Contacts_i::loadFilter(ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_Contacts_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  DBase::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    it.addE(dynamic_cast<DBase::Filters::Contact*>(*uit));
  }
}

void ccReg_Contacts_i::saveFilter(const char* _name) {
  TRACE(boost::format("[CALL] ccReg_Contacts_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
  tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_CONTACT, _name, uf);
}

Register::Contact::Contact* ccReg_Contacts_i::findId(ccReg::TID _id) {
  try {
    Register::Contact::Contact *contact =
        dynamic_cast<Register::Contact::Contact*> (cl->findId(_id));
    if (contact) {
      return contact;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}

