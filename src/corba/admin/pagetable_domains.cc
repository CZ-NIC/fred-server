#include "pagetable_domains.h"

ccReg_Domains_i::ccReg_Domains_i(Register::Domain::List *_dl, const Settings *_ptr) : dl(_dl) {
  uf.settings(_ptr);
}

ccReg_Domains_i::~ccReg_Domains_i() {
  TRACE("[CALL] ccReg_Domains_i::~ccReg_Domains_i()");
}

ccReg::Filters::Compound_ptr ccReg_Domains_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Domains_i::add()");
  Database::Filters::Domain *f = new Database::Filters::DomainHistoryImpl();
  uf.addFilter(f);
  return it.addE(f);
}

void ccReg_Domains_i::reload() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Domains_i::reload()");
  dl->reload(uf, dbm);
  dl->deleteDuplicatesId();
}

Registry::Table::ColumnHeaders* ccReg_Domains_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Domains_i::getColumnHeaders()");
  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(11);
  COLHEAD(ch, 0, "FQDN",            CT_OID);
  COLHEAD(ch, 1, "Registrant",      CT_OID);
  COLHEAD(ch, 2, "Registrant name", CT_OTHER);
  COLHEAD(ch, 3, "Registrant organization", CT_OTHER);
  COLHEAD(ch, 4, "Registrar",       CT_OID);
  COLHEAD(ch, 5, "In zone",         CT_OTHER);
  COLHEAD(ch, 6, "Create date",     CT_OTHER);
  COLHEAD(ch, 7, "Expiration date", CT_OTHER);
  COLHEAD(ch, 8, "Out Zone date",   CT_OTHER);
  COLHEAD(ch, 9, "Delete date",     CT_OTHER);
  COLHEAD(ch, 10, "Validation",     CT_OTHER);
  return ch;
}

Registry::TableRow* ccReg_Domains_i::getRow(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Register::Domain::Domain *d = dl->getDomain(row);
  if (!d)
    throw ccReg::Table::INVALID_ROW();
  Registry::TableRow *tr = new Registry::TableRow;
  tr->length(11);
  
  MAKE_OID(oid_fqdn, d->getId(), C_STR(d->getFQDN()), FT_DOMAIN)
  MAKE_OID(oid_registrant, d->getRegistrantId(), C_STR(d->getRegistrantHandle()), FT_CONTACT)
  MAKE_OID(oid_registrar, d->getRegistrarId(), C_STR(d->getRegistrarHandle()), FT_REGISTRAR)

  (*tr)[0]  <<= oid_fqdn;                                       // fqdn
  (*tr)[1]  <<= oid_registrant;                                 // registrant handle
  (*tr)[2]  <<= C_STR(d->getRegistrantName());                  // registrant name
  (*tr)[3]  <<= C_STR(d->getRegistrantOrganization());          // registrant organization
  (*tr)[4]  <<= oid_registrar;                                  // registrar handle 
  (*tr)[5]  <<= C_STR(d->getZoneStatus() == 1 ? "IN" : "OUT");  // zone generation 
  (*tr)[6]  <<= C_STR(d->getCreateDate());                      // crdate
  (*tr)[7]  <<= C_STR(d->getExpirationDate());                  // expiration date
  (*tr)[8]  <<= C_STR(d->getOutZoneDate());                     // out from zone file
  (*tr)[9]  <<= C_STR(d->getCancelDate());                      // delete from register
  (*tr)[10] <<= C_STR(d->getValExDate());                       // validation
  return tr;
}

void ccReg_Domains_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Domains_i::sortByColumn(%1%, %2%)") % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);

  switch (column) {
  case 0:
    dl->sort(Register::Domain::MT_FQDN, dir);
    break;      
  case 1:
    dl->sort(Register::Domain::MT_REGISTRANT_HANDLE, dir);
    break;
  case 2:
    dl->sort(Register::Domain::MT_REGISTRANT_NAME, dir);
    break;
  case 3:
    dl->sort(Register::Domain::MT_REGISTRANT_ORG, dir);
    break;
  case 4:
    dl->sort(Register::Domain::MT_REGISTRAR_HANDLE, dir);
    break;
  case 5:
    dl->sort(Register::Domain::MT_ZONE_STATUS, dir);
    break;
  case 6:
    dl->sort(Register::Domain::MT_CRDATE, dir);
    break;
  case 7:
    dl->sort(Register::Domain::MT_EXDATE, dir);
    break;
  case 8:
    dl->sort(Register::Domain::MT_OUTZONEDATE, dir);
    break;
  case 9:
    dl->sort(Register::Domain::MT_CANCELDATE, dir);
    // dl->sort(Register::Domain::MT_CANCELDATE, dir);
    // dl->sort(Register::Domain::MT_ERDATE, dir);
    break;
  }
}

ccReg::TID ccReg_Domains_i::getRowId(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Register::Domain::Domain *d = dl->getDomain(row);
  if (!d)
    throw ccReg::Table::INVALID_ROW();
  return d->getId();
}

char* ccReg_Domains_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Domains_i::numRows() {
  Logging::Context ctx(base_context_);

  return dl->getCount();
}

CORBA::Short ccReg_Domains_i::numColumns() {
  Logging::Context ctx(base_context_);

  return 11;
}

void ccReg_Domains_i::clear() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Domains_i::clear()");
  dl->clearFilter();
  
  ccReg_PageTable_i::clear();
  dl->clear();
}

CORBA::ULongLong ccReg_Domains_i::resultSize() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Domains_i::resultSize()");
  return dl->getRealCount(uf);
}

void ccReg_Domains_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Domains_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::Domain *tmp = dynamic_cast<Database::Filters::Domain* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_Domains_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void ccReg_Domains_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Domains_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_DOMAIN, _name, uf);
}

Register::Domain::Domain* ccReg_Domains_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    Register::Domain::Domain *domain = dynamic_cast<Register::Domain::Domain* >(dl->findId(_id));
    if (domain) {
      return domain;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_Domains_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return dl->isLimited(); 
}

