#include "pagetable_domains.h"

ccReg_Domains_i::ccReg_Domains_i(Fred::Domain::List *_dl, const Settings *_ptr) : dl(_dl) {
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

void ccReg_Domains_i::reload_worker() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_Domains_i::reload_worker()");
  dl->setTimeout(query_timeout);
  dl->reload(uf);
  dl->deleteDuplicatesId();
}

Registry::Table::ColumnHeaders* ccReg_Domains_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Domains_i::getColumnHeaders()");
  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(21);
  COLHEAD(ch, 0, "FQDN",            CT_OID);
  COLHEAD(ch, 1, "Registrant",      CT_OID);
  COLHEAD(ch, 2, "Registrant name", CT_OTHER);
  COLHEAD(ch, 3, "Registrant organization", CT_OTHER);
  COLHEAD(ch, 4, "Registrant phone", CT_OTHER);//
  COLHEAD(ch, 5, "Registrar",       CT_OID);
  COLHEAD(ch, 6, "In zone",         CT_OTHER);
  COLHEAD(ch, 7, "Create date",     CT_OTHER);
  COLHEAD(ch, 8, "Expiration date", CT_OTHER);
  COLHEAD(ch, 9, "Out Zone date",   CT_OTHER);
  COLHEAD(ch, 10, "Delete date",     CT_OTHER);
  COLHEAD(ch, 11, "Validation",     CT_OTHER);
  COLHEAD(ch, 12, "1. admin name",     CT_OTHER);
  COLHEAD(ch, 13, "1. admin organization",     CT_OTHER);
  COLHEAD(ch, 14, "1. admin phone",     CT_OTHER);
  COLHEAD(ch, 15, "2. admin name",     CT_OTHER);
  COLHEAD(ch, 16, "2. admin organization",     CT_OTHER);
  COLHEAD(ch, 17, "2. admin phone",     CT_OTHER);
  COLHEAD(ch, 18, "3. admin name",     CT_OTHER);
  COLHEAD(ch, 19, "3. admin organization",     CT_OTHER);
  COLHEAD(ch, 20, "3. admin phone",     CT_OTHER);
  return ch;
}

Registry::TableRow* ccReg_Domains_i::getRow(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);
    try
    {
      const Fred::Domain::Domain *d = dl->getDomain(row);
      if (!d)
        throw Registry::Table::INVALID_ROW();
      Registry::TableRow *tr = new Registry::TableRow;
      tr->length(21);

      MAKE_OID(oid_fqdn, d->getId(), C_STR(d->getFQDN()), FT_DOMAIN)
      MAKE_OID(oid_registrant, d->getRegistrantId(), C_STR(d->getRegistrantHandle()), FT_CONTACT)
      MAKE_OID(oid_registrar, d->getRegistrarId(), C_STR(d->getRegistrarHandle()), FT_REGISTRAR)

      (*tr)[0]  <<= oid_fqdn;                                       // fqdn
      (*tr)[1]  <<= oid_registrant;                                 // registrant handle
      (*tr)[2]  <<= C_STR(d->getRegistrantName());                  // registrant name
      (*tr)[3]  <<= C_STR(d->getRegistrantOrganization());          // registrant organization
      (*tr)[4]  <<= C_STR(d->getRegistrantPhone());                 // registrant phone
      (*tr)[5]  <<= oid_registrar;                                  // registrar handle
      (*tr)[6]  <<= C_STR(d->getZoneStatus() == 1 ? "IN" : "OUT");  // zone generation
      (*tr)[7]  <<= C_STR(d->getCreateDate());                      // crdate
      (*tr)[8]  <<= C_STR(d->getExpirationDate());                  // expiration date
      (*tr)[9]  <<= C_STR(d->getOutZoneDate());                     // out from zone file
      (*tr)[10]  <<= C_STR(d->getCancelDate());                     // delete from registry
      (*tr)[11] <<= C_STR(d->getValExDate());                       // validation
      (*tr)[12] <<= C_STR(d->getAdminNameByIdx(0));                 // 1. admin name
      (*tr)[13] <<= C_STR(d->getAdminOrganizationByIdx(0));         // 1. admin organization
      (*tr)[14] <<= C_STR(d->getAdminPhoneByIdx(0));                // 1. admin phone
      (*tr)[15] <<= C_STR(d->getAdminNameByIdx(1));                 // 2. admin name
      (*tr)[16] <<= C_STR(d->getAdminOrganizationByIdx(1));         // 2. admin organization
      (*tr)[17] <<= C_STR(d->getAdminPhoneByIdx(1));                // 2. admin phone
      (*tr)[18] <<= C_STR(d->getAdminNameByIdx(2));                 // 3. admin name
      (*tr)[19] <<= C_STR(d->getAdminOrganizationByIdx(2));         // 3. admin organization
      (*tr)[20] <<= C_STR(d->getAdminPhoneByIdx(2));                // 3. admin phone

      return tr;
    }//try
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Domains_i::getRow error");
        throw Registry::Table::INVALID_ROW();
    }//catch(...)
}

void ccReg_Domains_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Domains_i::sortByColumn(%1%, %2%)") % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);

  switch (column) {
  case 0:
    dl->sort(Fred::Domain::MT_FQDN, dir);
    break;      
  case 1:
    dl->sort(Fred::Domain::MT_REGISTRANT_HANDLE, dir);
    break;
  case 2:
    dl->sort(Fred::Domain::MT_REGISTRANT_NAME, dir);
    break;
  case 3:
    dl->sort(Fred::Domain::MT_REGISTRANT_ORG, dir);
    break;
  case 4:
    dl->sort(Fred::Domain::MT_REGISTRANT_PHONE, dir);
    break;
  case 5:
    dl->sort(Fred::Domain::MT_REGISTRAR_HANDLE, dir);
    break;
  case 6:
    dl->sort(Fred::Domain::MT_ZONE_STATUS, dir);
    break;
  case 7:
    dl->sort(Fred::Domain::MT_CRDATE, dir);
    break;
  case 8:
    dl->sort(Fred::Domain::MT_EXDATE, dir);
    break;
  case 9:
    dl->sort(Fred::Domain::MT_OUTZONEDATE, dir);
    break;
  case 10:
    dl->sort(Fred::Domain::MT_CANCELDATE, dir);
    // dl->sort(Fred::Domain::MT_CANCELDATE, dir);
    // dl->sort(Fred::Domain::MT_ERDATE, dir);
    break;
  case 12:
    dl->sort(Fred::Domain::MT_1ADMIN_NAME, dir);
    break;
  case 13:
    dl->sort(Fred::Domain::MT_1ADMIN_ORG, dir);
    break;
  case 14:
    dl->sort(Fred::Domain::MT_1ADMIN_PHONE, dir);
    break;
  case 15:
    dl->sort(Fred::Domain::MT_2ADMIN_NAME, dir);
    break;
  case 16:
    dl->sort(Fred::Domain::MT_2ADMIN_ORG, dir);
    break;
  case 17:
    dl->sort(Fred::Domain::MT_2ADMIN_PHONE, dir);
    break;
  case 18:
    dl->sort(Fred::Domain::MT_2ADMIN_NAME, dir);
    break;
  case 19:
    dl->sort(Fred::Domain::MT_2ADMIN_ORG, dir);
    break;
  case 20:
    dl->sort(Fred::Domain::MT_2ADMIN_PHONE, dir);
    break;
  }
}

ccReg::TID ccReg_Domains_i::getRowId(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW)
{
  Logging::Context ctx(base_context_);
    try
    {
      const Fred::Domain::Domain *d = dl->getDomain(row);
      if (!d)
        throw Registry::Table::INVALID_ROW();
      return d->getId();
    }//try
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Domains_i::getRowId error");
        throw Registry::Table::INVALID_ROW();
    }//catch(...)
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

  return 21;
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
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_Domains_i::resultSize()");
  return dl->getRealCount(uf);
}

void ccReg_Domains_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

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
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Domains_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Fred::Filter::Manager>
      tmp_filter_manager(Fred::Filter::Manager::create());
  tmp_filter_manager->save(Fred::Filter::FT_DOMAIN, _name, uf);
}

Fred::Domain::Domain* ccReg_Domains_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    Fred::Domain::Domain *domain = dynamic_cast<Fred::Domain::Domain* >(dl->findId(_id));
    if (domain) {
      return domain;
    }
    return 0;
  }
  catch (Fred::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_Domains_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return dl->isLimited(); 
}

