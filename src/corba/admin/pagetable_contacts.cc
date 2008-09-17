#include "pagetable_contacts.h"

ccReg_Contacts_i::ccReg_Contacts_i(Register::Contact::List *_cl, const Settings *_ptr) : cl(_cl) {
  uf.settings(_ptr);
}

ccReg_Contacts_i::~ccReg_Contacts_i() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Contacts_i::~ccReg_Contacts_i()");
}

ccReg::Filters::Compound_ptr ccReg_Contacts_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Contacts_i::add()");
  it.clearF();
  Database::Filters::Contact *f = new Database::Filters::ContactHistoryImpl();
  uf.addFilter(f);
  return it.addE(f);
}

Registry::Table::ColumnHeaders* ccReg_Contacts_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(6);
  COLHEAD(ch, 0, "Handle", CT_OID);
  COLHEAD(ch, 1, "Name", CT_OTHER);
  COLHEAD(ch, 2, "Organization", CT_OTHER);
  COLHEAD(ch, 3, "Create date", CT_OTHER);
  COLHEAD(ch, 4, "Delete date", CT_OTHER);
  COLHEAD(ch, 5, "Registrar", CT_OID);
  return ch;
}

Registry::TableRow* ccReg_Contacts_i::getRow(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Register::Contact::Contact *c = cl->getContact(row);
  if (!c) throw ccReg::Table::INVALID_ROW();
  Registry::TableRow *tr = new Registry::TableRow;
  tr->length(6);

  MAKE_OID(oid_handle, c->getId(), DUPSTRFUN(c->getHandle), FT_CONTACT)
  MAKE_OID(oid_registrar, c->getRegistrarId(), DUPSTRFUN(c->getRegistrarHandle), FT_REGISTRAR)

  (*tr)[0] <<= oid_handle;
  (*tr)[1] <<= DUPSTRFUN(c->getName);
  (*tr)[2] <<= DUPSTRFUN(c->getOrganization);
  (*tr)[3] <<= DUPSTRDATE(c->getCreateDate);
  (*tr)[4] <<= DUPSTRDATE(c->getDeleteDate);
  (*tr)[5] <<= oid_registrar;
  return tr;
}

void ccReg_Contacts_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Contacts_i::sortByColumn(%1%, %2%)") % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);
  
  switch (column) {
    case 0:
      cl->sort(Register::Contact::MT_HANDLE, dir);
      break;
    case 1:
      cl->sort(Register::Contact::MT_NAME, dir);
      break;
    case 2:
      cl->sort(Register::Contact::MT_ORGANIZATION, dir);
      break;
    case 3:
      cl->sort(Register::Contact::MT_CRDATE, dir);
      break;
    case 4:
      cl->sort(Register::Contact::MT_ERDATE, dir);
      break;
    case 5:
      cl->sort(Register::Contact::MT_REGISTRAR_HANDLE, dir);
      break;
  }
}

ccReg::TID ccReg_Contacts_i::getRowId(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Register::Contact::Contact *c = cl->getContact(row);
  if (!c) throw ccReg::Table::INVALID_ROW();
  return c->getId();
}

char* ccReg_Contacts_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Contacts_i::numRows() {
  Logging::Context ctx(base_context_);

  return cl->getCount();
}

CORBA::Short ccReg_Contacts_i::numColumns() {
  Logging::Context ctx(base_context_);

  return 6;
}

void ccReg_Contacts_i::reload() {
  Logging::Context ctx(base_context_);

//  cl->makeRealCount();
  cl->reload(uf, dbm);
  cl->deleteDuplicatesId();
}

void ccReg_Contacts_i::clear() {
  Logging::Context ctx(base_context_);

  cl->clearFilter();
  
  ccReg_PageTable_i::clear();
  cl->clear();
}

CORBA::ULongLong ccReg_Contacts_i::resultSize() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Contacts_i::resultSize()");
  return cl->getRealCount(uf);
}

void ccReg_Contacts_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Contacts_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    it.addE(dynamic_cast<Database::Filters::Contact*>(*uit));
  }
}

void ccReg_Contacts_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Contacts_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
  tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_CONTACT, _name, uf);
}

Register::Contact::Contact* ccReg_Contacts_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

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

CORBA::Boolean ccReg_Contacts_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return cl->isLimited(); 
}

