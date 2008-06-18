#include "pagetable_invoices.h"

ccReg_Invoices_i::ccReg_Invoices_i(Register::Invoicing::List *_invoice_list) :
  invoice_list_(_invoice_list) {
}

ccReg_Invoices_i::~ccReg_Invoices_i() {
  TRACE("[CALL] ccReg_Invoices_i::~ccReg_Invoices_i()");
}

ccReg::Filters::Compound_ptr ccReg_Invoices_i::add() {
  TRACE("[CALL] ccReg_Invoices_i::add()");
  it.clearF();
  DBase::Filters::Invoice *filter = new DBase::Filters::InvoiceImpl();
  uf.addFilter(filter);
  return it.addE(filter);
}


ccReg::Table::ColumnHeaders* ccReg_Invoices_i::getColumnHeaders() {
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(9);
  COLHEAD(ch,0,"Create Date",CT_OTHER);
  COLHEAD(ch,1,"Number",CT_OTHER);
  COLHEAD(ch,2,"Registrar",CT_REGISTRAR_HANDLE);
  COLHEAD(ch,3,"Total",CT_OTHER);
  COLHEAD(ch,4,"Credit",CT_OTHER);
  COLHEAD(ch,5,"Type",CT_OTHER);
  COLHEAD(ch,6,"Zone",CT_OTHER);
  COLHEAD(ch,7,"PDF",CT_FILE_ID);
  COLHEAD(ch,8,"XML",CT_FILE_ID);
  return ch;
}

ccReg::TableRow* ccReg_Invoices_i::getRow(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Invoicing::Invoice *inv = invoice_list_->get(row);
  if (!inv)
    throw ccReg::Table::INVALID_ROW();
  
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(9);
  Register::Invoicing::Type invoice_type = inv->getType();
  std::string credit = (invoice_type == Register::Invoicing::IT_DEPOSIT ? formatMoney(inv->getCredit()) : "");
  
  (*tr)[0] = DUPSTRDATE(inv->getCrTime);
  (*tr)[1] = DUPSTRC(Util::stream_cast<std::string>(inv->getNumber()));
  (*tr)[2] = DUPSTRFUN(inv->getClient()->getHandle);
  (*tr)[3] = DUPSTRC(formatMoney(inv->getPrice()));
  (*tr)[4] = DUPSTRC(credit);
  (*tr)[5] = DUPSTR(invoice_type == Register::Invoicing::IT_DEPOSIT ? "DEPOSIT" : "ACCOUNT");
  (*tr)[6] = DUPSTRC(inv->getZoneName());
  (*tr)[7] = DUPSTRC(Util::stream_cast<std::string>(inv->getFilePDF()));
  (*tr)[8] = DUPSTRC(Util::stream_cast<std::string>(inv->getFileXML()));
  return tr;
}

void ccReg_Invoices_i::sortByColumn(CORBA::Short _column, CORBA::Boolean _dir) {
  TRACE(boost::format("[CALL] ccReg_Invoices_i::sortByColumn(%1%, %2%)") % _column % _dir);
  switch (_column) {
    case 0:
      invoice_list_->sort(Register::Invoicing::MT_CRDATE, _dir);
      sorted_by_ = 0;
      break;
    case 1:
      invoice_list_->sort(Register::Invoicing::MT_NUMBER, _dir);
      sorted_by_ = 1;
      break;
    case 2:
      invoice_list_->sort(Register::Invoicing::MT_REGISTRAR, _dir);
      sorted_by_ = 2;
      break;
    case 3:
      invoice_list_->sort(Register::Invoicing::MT_TOTAL, _dir);
      sorted_by_ = 3;
      break;
    case 4:
      invoice_list_->sort(Register::Invoicing::MT_CREDIT, _dir);
      sorted_by_ = 4;
      break;
    case 5:
      invoice_list_->sort(Register::Invoicing::MT_TYPE, _dir);
      sorted_by_ = 5;
      break;
    case 6:
      invoice_list_->sort(Register::Invoicing::MT_ZONE, _dir);
      sorted_by_ = 6;
      break;
  }
}

ccReg::TID ccReg_Invoices_i::getRowId(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Invoicing::Invoice *inv = invoice_list_->get(row);
  if (!inv)
    throw ccReg::Table::INVALID_ROW();
  return inv->getId();
}

char* ccReg_Invoices_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Invoices_i::numRows() {
  return invoice_list_->getCount();
}

CORBA::Short ccReg_Invoices_i::numColumns() {
  return 9;
}

void ccReg_Invoices_i::reload() {
  invoice_list_->setPartialLoad(true);
  invoice_list_->reload(uf, dbm);
}

void ccReg_Invoices_i::clear() {
  TRACE("[CALL] ccReg_Invoices_i::clear()");
  ccReg_PageTable_i::clear();
  invoice_list_->clear();
}

CORBA::ULongLong ccReg_Invoices_i::resultSize() {
  TRACE("ccReg_Invoices_i::resultSize()");
  return invoice_list_->getRealCount(uf);
}

void ccReg_Invoices_i::loadFilter(ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_Invoices_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  DBase::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    DBase::Filters::Invoice *tmp = dynamic_cast<DBase::Filters::Invoice* >(*uit);
    it.addE(tmp);
    TRACE(boost::format("[IN] ccReg_Invoices_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
  }
}

void ccReg_Invoices_i::saveFilter(const char* _name) {
  TRACE(boost::format("[CALL] ccReg_PublicRequests_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_INVOICE, _name, uf);
}

Register::Invoicing::Invoice* ccReg_Invoices_i::findId(ccReg::TID _id) {
  try {
    Register::Invoicing::Invoice *invoice = dynamic_cast<Register::Invoicing::Invoice* >(invoice_list_->findId(_id));
    if (invoice) {
      return invoice;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}
