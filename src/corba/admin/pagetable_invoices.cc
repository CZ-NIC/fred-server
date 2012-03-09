#include "pagetable_invoices.h"

ccReg_Invoices_i::ccReg_Invoices_i(Fred::Invoicing::List *_invoice_list) :
  invoice_list_(_invoice_list) {
}

ccReg_Invoices_i::~ccReg_Invoices_i() {
  TRACE("[CALL] ccReg_Invoices_i::~ccReg_Invoices_i()");
}

ccReg::Filters::Compound_ptr ccReg_Invoices_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Invoices_i::add()");
  Database::Filters::Invoice *filter = new Database::Filters::InvoiceImpl();
  uf.addFilter(filter);
  return it.addE(filter);
}


Registry::Table::ColumnHeaders* ccReg_Invoices_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(9);
  COLHEAD(ch,0,"Create Date",CT_OTHER);
  COLHEAD(ch,1,"Number",CT_OTHER);
  COLHEAD(ch,2,"Registrar",CT_OID);
  COLHEAD(ch,3,"Total",CT_OTHER);
  COLHEAD(ch,4,"Credit",CT_OTHER);
  COLHEAD(ch,5,"Type",CT_OTHER);
  COLHEAD(ch,6,"Zone",CT_OTHER);
  COLHEAD(ch,7,"PDF",CT_OID_ICON);
  COLHEAD(ch,8,"XML",CT_OID_ICON);
  return ch;
}

Registry::TableRow* ccReg_Invoices_i::getRow(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Fred::Invoicing::Invoice *inv = invoice_list_->get(row);
  if (!inv)
    throw Registry::Table::INVALID_ROW();
  
  Registry::TableRow *tr = new Registry::TableRow;
  tr->length(9);
  Fred::Invoicing::Type invoice_type = inv->getType();
  std::string credit = (invoice_type == Fred::Invoicing::IT_DEPOSIT ? formatMoney(inv->getCredit()) : "");
  std::string itype  = (invoice_type == Fred::Invoicing::IT_DEPOSIT ? "DEPOSIT" : "ACCOUNT");

  MAKE_OID(oid_registrar, inv->getClient()->getId(), C_STR(inv->getClient()->getHandle()), FT_REGISTRAR)
  MAKE_OID(oid_pdf, inv->getFileId(), "", FT_FILE)
  MAKE_OID(oid_xml, inv->getFileXmlId(), "", FT_FILE)


  (*tr)[0] <<= stringify(formatTime(inv->getCrDate(), true,false).c_str()).c_str();
  (*tr)[1] <<= C_STR(inv->getPrefix());
  (*tr)[2] <<= oid_registrar;

  if(invoice_type == Fred::Invoicing::IT_DEPOSIT) {
      (*tr)[3] <<= formatMoney( inv->getTotal() + inv->getTotalVAT() ).c_str();
  } else {
      (*tr)[3] <<= formatMoney(inv->getPrice()).c_str();
  }

  (*tr)[4] <<= C_STR(credit);
  (*tr)[5] <<= C_STR(itype);
  (*tr)[6] <<= C_STR(inv->getZoneFqdn());
  (*tr)[7] <<= oid_pdf;
  (*tr)[8] <<= oid_xml;
  return tr;
}

void ccReg_Invoices_i::sortByColumn(CORBA::Short _column, CORBA::Boolean _dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Invoices_i::sortByColumn(%1%, %2%)") % _column % _dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(_column, _dir);

  switch (_column) {
    case 0:
      invoice_list_->sort(Fred::Invoicing::MT_CRTIME, _dir);
      break;
    case 1:
      invoice_list_->sort(Fred::Invoicing::MT_NUMBER, _dir);
      break;
    case 2:
      invoice_list_->sort(Fred::Invoicing::MT_REGISTRAR, _dir);
      break;
    case 3:
      invoice_list_->sort(Fred::Invoicing::MT_PRICE, _dir);
      break;
    case 4:
      invoice_list_->sort(Fred::Invoicing::MT_CREDIT, _dir);
      break;
    case 5:
      invoice_list_->sort(Fred::Invoicing::MT_TYPE, _dir);
      break;
    case 6:
      invoice_list_->sort(Fred::Invoicing::MT_ZONE, _dir);
      break;
  }
}

ccReg::TID ccReg_Invoices_i::getRowId(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Fred::Invoicing::Invoice *inv = invoice_list_->get(row);
  if (!inv)
    throw Registry::Table::INVALID_ROW();
  return inv->getId();
}

char* ccReg_Invoices_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Invoices_i::numRows() {
  Logging::Context ctx(base_context_);

  return invoice_list_->getSize();
}

CORBA::Short ccReg_Invoices_i::numColumns() {
  Logging::Context ctx(base_context_);

  return 9;
}

void ccReg_Invoices_i::reload_worker() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  invoice_list_->setPartialLoad(true);
  invoice_list_->setTimeout(query_timeout);
  invoice_list_->reload(uf);
}

void ccReg_Invoices_i::clear() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Invoices_i::clear()");
  ccReg_PageTable_i::clear();
  invoice_list_->clear();
}

CORBA::ULongLong ccReg_Invoices_i::resultSize() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("ccReg_Invoices_i::resultSize()");
  return invoice_list_->getRealCount(uf);
}

void ccReg_Invoices_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Invoices_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::Invoice *tmp = dynamic_cast<Database::Filters::Invoice* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_Invoices_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void ccReg_Invoices_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_PublicRequests_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Fred::Filter::Manager>
      tmp_filter_manager(Fred::Filter::Manager::create());
  tmp_filter_manager->save(Fred::Filter::FT_INVOICE, _name, uf);
}

Fred::Invoicing::Invoice* ccReg_Invoices_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    Fred::Invoicing::Invoice *invoice = dynamic_cast<Fred::Invoicing::Invoice* >(invoice_list_->findId(_id));
    if (invoice) {
      return invoice;
    }
    return 0;
  }
  catch (Fred::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_Invoices_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return invoice_list_->isLimited(); 
}

