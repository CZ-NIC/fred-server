#include "pagetable_mails.h"

ccReg_Mails_i::ccReg_Mails_i(Fred::Mail::List *_list, NameService *ns) :
  mail_list_(_list), mm(ns) {
}

ccReg_Mails_i::~ccReg_Mails_i() {
  TRACE("[CALL] ccReg_Mails_i::~ccReg_Mails_i()");
}

ccReg::Filters::Compound_ptr ccReg_Mails_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Mails_i::add()");
  Database::Filters::Mail *f = new Database::Filters::MailImpl();
  uf.addFilter(f);
  return it.addE(f);
}

Registry::Table::ColumnHeaders* ccReg_Mails_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(3);
  COLHEAD(ch, 0, "Create Time", CT_OTHER);
  COLHEAD(ch, 1, "Type", CT_OTHER);
  COLHEAD(ch, 2, "Status", CT_OTHER);
  return ch;
}

Registry::TableRow* ccReg_Mails_i::getRow(CORBA::UShort _row)
    throw (Registry::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Fred::Mail::Mail *mail = mail_list_->get(_row);
  if (!mail)
    throw Registry::Table::INVALID_ROW();

  Registry::TableRow *tr = new Registry::TableRow;
  tr->length(3);

  (*tr)[0] <<= C_STR(mail->getCreateTime());
  (*tr)[1] <<= C_STR(mail->getTypeDesc());
  (*tr)[2] <<= C_STR(mail->getStatus());
  return tr;
}

void ccReg_Mails_i::sortByColumn(CORBA::Short _column, CORBA::Boolean _dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Mails_i::sortByColumn(%1%, %2%)") % _column % _dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(_column, _dir);
  
  switch (_column) {
    case 0:
      mail_list_->sort(Fred::Mail::MT_CRDATE, _dir);
      break;
    case 1:
      mail_list_->sort(Fred::Mail::MT_TYPE, _dir);
      break;
    case 2:
      mail_list_->sort(Fred::Mail::MT_STATUS, _dir);
      break;
  }
}

ccReg::TID ccReg_Mails_i::getRowId(CORBA::UShort _row)
    throw (Registry::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Fred::Mail::Mail *mail = mail_list_->get(_row);
  if (!mail)
    throw Registry::Table::INVALID_ROW();
  return mail->getId();
}

char* ccReg_Mails_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Mails_i::numRows() {
  Logging::Context ctx(base_context_);

  return mail_list_->getCount();
}

CORBA::Short ccReg_Mails_i::numColumns() {
  Logging::Context ctx(base_context_);

  return 3;
}

void ccReg_Mails_i::reload_worker() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_Mails_i::reload_worker()");
  mail_list_->setTimeout(query_timeout);
  mail_list_->reload(uf);
}

void ccReg_Mails_i::clear() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Mails_i::clear()");
  ccReg_PageTable_i::clear();
  mail_list_->clear();
}

CORBA::ULongLong ccReg_Mails_i::resultSize() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("ccReg_Mails_i::resultSize()");
  return mail_list_->getRealCount(uf);
}

void ccReg_Mails_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Mails_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::Mail *tmp = dynamic_cast<Database::Filters::Mail* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_Mails_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void ccReg_Mails_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Mails_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Fred::Filter::Manager>
      tmp_filter_manager(Fred::Filter::Manager::create());
  tmp_filter_manager->save(Fred::Filter::FT_MAIL, _name, uf);
}

Fred::Mail::Mail* ccReg_Mails_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    Fred::Mail::Mail *mail = dynamic_cast<Fred::Mail::Mail* >(mail_list_->findId(_id));
    if (mail) {
      return mail;
    }
    return 0;
  }
  catch (Fred::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_Mails_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return mail_list_->isLimited(); 
}

