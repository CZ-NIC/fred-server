#include "pagetable_domains.h"

ccReg_Domains_i::ccReg_Domains_i(Register::Domain::List *_dl) :
  dl(_dl) {
}

ccReg_Domains_i::~ccReg_Domains_i() {
  TRACE("[CALL] ccReg_Domains_i::~ccReg_Domains_i()");
}

ccReg::Filters::Compound_ptr ccReg_Domains_i::add() {
  TRACE("[CALL] ccReg_Domains_i::add()");
  it.clearF();
  Database::Filters::Domain *f = new Database::Filters::DomainHistoryImpl();
  uf.addFilter(f);
  return it.addE(f);
}

void ccReg_Domains_i::reload() {
  TRACE("[CALL] ccReg_Domains_i::reload()");
//  dl->makeRealCount();
  dl->reload(uf, dbm);
}

ccReg::Table::ColumnHeaders* ccReg_Domains_i::getColumnHeaders() {
  TRACE("[CALL] ccReg_Domains_i::getColumnHeaders()");
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(11);
  COLHEAD(ch, 0, "FQDN", CT_DOMAIN_HANDLE);
  COLHEAD(ch, 1, "Create date", CT_OTHER);
  COLHEAD(ch, 2, "Delete date", CT_OTHER);
  COLHEAD(ch, 3, "Registrant", CT_CONTACT_HANDLE);
  COLHEAD(ch, 4, "Registrant name", CT_OTHER);
  COLHEAD(ch, 5, "Registrar", CT_REGISTRAR_HANDLE);
  COLHEAD(ch, 6, "In zone", CT_OTHER);
  COLHEAD(ch, 7, "Expiration date", CT_OTHER);
  COLHEAD(ch, 8, "Out Zone date", CT_OTHER);
  COLHEAD(ch, 9, "Cancel date", CT_OTHER);
  COLHEAD(ch, 10, "Validation", CT_OTHER);
  return ch;
}

ccReg::TableRow* ccReg_Domains_i::getRow(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Domain::Domain *d = dl->getDomain(row);
  if (!d)
    throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(11);
  (*tr)[0] = DUPSTRFUN(d->getFQDN); // fqdn
  (*tr)[1] = DUPSTRDATE(d->getCreateDate); // crdate
  (*tr)[2] = DUPSTRDATED(d->getDeleteDate); // zruseni ??
  (*tr)[3] = DUPSTRFUN(d->getRegistrantHandle); // registrant handle
  (*tr)[4] = DUPSTRFUN(d->getRegistrantName); // registrant name
  (*tr)[5] = DUPSTRFUN(d->getRegistrarHandle); // registrar handle 
  (*tr)[6] = DUPSTR(d->getZoneStatus() == 1 ? "IN" : "OUT"); // zone generation 
  (*tr)[7] = DUPSTRDATED(d->getExpirationDate); // expiration date 
  (*tr)[8] = DUPSTRDATED(d->getOutZoneDate); // vyrazeni z dns
  (*tr)[9] = DUPSTRDATED(d->getCancelDate); // vyrazeni z dns
  (*tr)[10] = DUPSTRDATED(d->getValExDate); // validace
  return tr;
}

void ccReg_Domains_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  TRACE(boost::format("[CALL] ccReg_Domains_i::sortByColumn(%1%, %2%)") % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);

  switch (column) {
  case 0:
    dl->sort(Register::Domain::MT_FQDN, dir);
    break;      
  case 1:
    dl->sort(Register::Domain::MT_CRDATE, dir);
    break;
  case 2:
    dl->sort(Register::Domain::MT_ERDATE, dir);
    break;
  case 3:
    dl->sort(Register::Domain::MT_REGISTRANT_HANDLE, dir);
    break;
  case 4:
    dl->sort(Register::Domain::MT_REGISTRANT_NAME, dir);
    break;
  case 5:
    dl->sort(Register::Domain::MT_REGISTRAR_HANDLE, dir);
    break;
  case 6:
    dl->sort(Register::Domain::MT_ZONE_STATUS, dir);
    break;
  case 7:
    dl->sort(Register::Domain::MT_EXDATE, dir);
    break;
  case 8:
    dl->sort(Register::Domain::MT_OUTZONEDATE, dir);
    break;
  case 9:
    dl->sort(Register::Domain::MT_CANCELDATE, dir);
    break;
  }
}

ccReg::TID ccReg_Domains_i::getRowId(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Domain::Domain *d = dl->getDomain(row);
  if (!d)
    throw ccReg::Table::INVALID_ROW();
  return d->getId();
}

char* ccReg_Domains_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Domains_i::numRows() {
  return dl->getCount();
}

CORBA::Short ccReg_Domains_i::numColumns() {
  return 11;
}

void ccReg_Domains_i::clear() {
  TRACE("[CALL] ccReg_Domains_i::clear()");
  dl->clearFilter();
  
  ccReg_PageTable_i::clear();
  dl->clear();
}

CORBA::ULongLong ccReg_Domains_i::resultSize() {
  TRACE("[CALL] ccReg_Domains_i::resultSize()");
  return dl->getRealCount(uf);
}

void ccReg_Domains_i::loadFilter(ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_Domains_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::Domain *tmp = dynamic_cast<Database::Filters::Domain* >(*uit);
    it.addE(tmp);
    TRACE(boost::format("[IN] ccReg_Domains_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
  }
}

void ccReg_Domains_i::saveFilter(const char* _name) {
  TRACE(boost::format("[CALL] ccReg_Domains_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_DOMAIN, _name, uf);
}

Register::Domain::Domain* ccReg_Domains_i::findId(ccReg::TID _id) {
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
  return dl->isLimited(); 
}
