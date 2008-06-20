#include "pagetable_mails.h"

ccReg_Mails_i::ccReg_Mails_i(Register::Mail::List *_list, NameService *ns) :
  mail_list_(_list), mm(ns) {
}

ccReg_Mails_i::~ccReg_Mails_i() {
  TRACE("[CALL] ccReg_Mails_i::~ccReg_Mails_i()");
}

ccReg::Filters::Compound_ptr ccReg_Mails_i::add() {
  TRACE("[CALL] ccReg_Mails_i::add()");
  it.clearF();
  DBase::Filters::Mail *f = new DBase::Filters::MailImpl();
  uf.addFilter(f);
  return it.addE(f);
}

ccReg::Table::ColumnHeaders* ccReg_Mails_i::getColumnHeaders() {
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(3);
  COLHEAD(ch, 0, "Create Time", CT_OTHER);
  COLHEAD(ch, 1, "Type", CT_OTHER);
  COLHEAD(ch, 2, "Status", CT_OTHER);
  return ch;
}

ccReg::TableRow* ccReg_Mails_i::getRow(CORBA::Short _row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Mail::Mail *mail = mail_list_->get(_row);
  if (!mail)
    throw ccReg::Table::INVALID_ROW();

  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(3);
  (*tr)[0] = DUPSTRDATE(mail->getCreateTime);
  (*tr)[1] = DUPSTRC(mail->getTypeDesc());
  (*tr)[2] = DUPSTRC(Util::stream_cast<std::string>(mail->getStatus()));
  return tr;
}

void ccReg_Mails_i::sortByColumn(CORBA::Short _column, CORBA::Boolean _dir) {
  switch (_column) {
    case 0:
      mail_list_->sort(Register::Mail::MT_CRDATE, _dir);
      sorted_by_ = 0;
      break;
    case 1:
      mail_list_->sort(Register::Mail::MT_TYPE, _dir);
      sorted_by_ = 1;
      break;
    case 2:
      mail_list_->sort(Register::Mail::MT_STATUS, _dir);
      sorted_by_ = 2;
      break;
  }
}

ccReg::TID ccReg_Mails_i::getRowId(CORBA::Short _row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Mail::Mail *mail = mail_list_->get(_row);
  if (!mail)
    throw ccReg::Table::INVALID_ROW();
  return mail->getId();
}

char* ccReg_Mails_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Mails_i::numRows() {
  return mail_list_->getCount();
}

CORBA::Short ccReg_Mails_i::numColumns() {
  return 3;
}

void ccReg_Mails_i::reload() {
  TRACE("[CALL] ccReg_Mails_i::reload()");
  mail_list_->reload(uf);
}

void ccReg_Mails_i::clear() {
  TRACE("[CALL] ccReg_Mails_i::clear()");
  ccReg_PageTable_i::clear();
  mail_list_->clear();
}

CORBA::ULongLong ccReg_Mails_i::resultSize() {
  TRACE("ccReg_Mails_i::resultSize()");
  return mail_list_->getRealCount(uf);
}

void ccReg_Mails_i::loadFilter(ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_Mails_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  DBase::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    DBase::Filters::Mail *tmp = dynamic_cast<DBase::Filters::Mail* >(*uit);
    it.addE(tmp);
    TRACE(boost::format("[IN] ccReg_Mails_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
  }
}

void ccReg_Mails_i::saveFilter(const char* _name) {
  TRACE(boost::format("[CALL] ccReg_Mails_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_MAIL, _name, uf);
}

Register::Mail::Mail* ccReg_Mails_i::findId(ccReg::TID _id) {
  try {
    Register::Mail::Mail *mail = dynamic_cast<Register::Mail::Mail* >(mail_list_->findId(_id));
    if (mail) {
      return mail;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}
