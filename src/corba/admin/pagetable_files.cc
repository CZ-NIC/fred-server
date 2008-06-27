#include "pagetable_files.h"

ccReg_Files_i::ccReg_Files_i(Register::File::List *_list) :
  file_list_(_list) {
}

ccReg_Files_i::~ccReg_Files_i() {
  TRACE("[CALL] ccReg_Files_i::~ccReg_Files_i()");
}

ccReg::Filters::Compound_ptr ccReg_Files_i::add() {
  TRACE("[CALL] ccReg_Files_i::add()");
  it.clearF();
  DBase::Filters::File *f = new DBase::Filters::FileImpl(true);
  uf.addFilter(f);
  return it.addE(f);
}

ccReg::Table::ColumnHeaders* ccReg_Files_i::getColumnHeaders() {
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(4);
  COLHEAD(ch, 0, "Name", CT_OTHER);
  COLHEAD(ch, 1, "Create Time",CT_OTHER);
  COLHEAD(ch, 2, "Type",CT_OTHER);
  COLHEAD(ch, 3, "Size",CT_OTHER);
  return ch;
}

ccReg::TableRow* ccReg_Files_i::getRow(CORBA::Short _row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::File::File *file = file_list_->get(_row);
  if (!file)
    throw ccReg::Table::INVALID_ROW();

  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(4);
  (*tr)[0] = DUPSTRC(file->getName());
  (*tr)[1] = DUPSTRDATE(file->getCreateTime);
  (*tr)[2] = DUPSTRC(file->getTypeDesc());
  (*tr)[3] = DUPSTRC(Util::stream_cast<std::string>(file->getSize()));
  return tr;
}

void ccReg_Files_i::sortByColumn(CORBA::Short _column, CORBA::Boolean _dir) {
  switch (_column) {
    case 0:
      file_list_->sort(Register::File::MT_NAME, _dir);
      sorted_by_ = 0;
      break;
    case 1:
      file_list_->sort(Register::File::MT_CRDATE, _dir);
      sorted_by_ = 1;
      break;
    case 2:
      file_list_->sort(Register::File::MT_TYPE, _dir);
      sorted_by_ = 2;
      break;
    case 3:
      file_list_->sort(Register::File::MT_SIZE, _dir);
      sorted_by_ = 3;
      break;
  }
}

ccReg::TID ccReg_Files_i::getRowId(CORBA::Short _row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::File::File *file = file_list_->get(_row);
  if (!file)
    throw ccReg::Table::INVALID_ROW();
  return file->getId();
}

char* ccReg_Files_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Files_i::numRows() {
  return file_list_->getCount();
}

CORBA::Short ccReg_Files_i::numColumns() {
  return 4;
}

void ccReg_Files_i::reload() {
  TRACE("[CALL] ccReg_Files_i::reload()");
  file_list_->reload(uf);
}

void ccReg_Files_i::clear() {
  TRACE("[CALL] ccReg_Files_i::clear()");
  ccReg_PageTable_i::clear();
  file_list_->clear();
}

CORBA::ULongLong ccReg_Files_i::resultSize() {
  TRACE("ccReg_Files_i::resultSize()");
  return file_list_->getRealCount(uf);
}

void ccReg_Files_i::loadFilter(ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_Files_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  DBase::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    DBase::Filters::File *tmp = dynamic_cast<DBase::Filters::File* >(*uit);
    it.addE(tmp);
    TRACE(boost::format("[IN] ccReg_Files_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
  }
}

void ccReg_Files_i::saveFilter(const char* _name) {
  TRACE(boost::format("[CALL] ccReg_Files_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_FILE, _name, uf);
}

Register::File::File* ccReg_Files_i::findId(ccReg::TID _id) {
  try {
    Register::File::File *file = dynamic_cast<Register::File::File* >(file_list_->findId(_id));
    if (file) {
      return file;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}
