#include "pagetable_files.h"

ccReg_Files_i::ccReg_Files_i(Register::File::List *_list) :
  file_list_(_list) {
}

ccReg_Files_i::~ccReg_Files_i() {
  TRACE("[CALL] ccReg_Files_i::~ccReg_Files_i()");
}

ccReg::Filters::Compound_ptr ccReg_Files_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Files_i::add()");
  it.clearF();
  Database::Filters::File *f = new Database::Filters::FileImpl(true);
  uf.addFilter(f);
  return it.addE(f);
}

Registry::Table::ColumnHeaders* ccReg_Files_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(4);
  COLHEAD(ch, 0, "Name", CT_OTHER);
  COLHEAD(ch, 1, "Create Time",CT_OTHER);
  COLHEAD(ch, 2, "Type",CT_OTHER);
  COLHEAD(ch, 3, "Size",CT_OTHER);
  return ch;
}

Registry::TableRow* ccReg_Files_i::getRow(CORBA::Short _row)
    throw (ccReg::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Register::File::File *file = file_list_->get(_row);
  if (!file)
    throw ccReg::Table::INVALID_ROW();

  Registry::TableRow *tr = new Registry::TableRow;
  tr->length(4);
  (*tr)[0] <<= str_corbaout(file->getName());
  (*tr)[1] <<= str_corbaout(file->getCreateTime());
  (*tr)[2] <<= str_corbaout(file->getTypeDesc());
  (*tr)[3] <<= str_corbaout(file->getSize());
  return tr;
}

void ccReg_Files_i::sortByColumn(CORBA::Short _column, CORBA::Boolean _dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Files_i::sortByColumn(%1%, %2%)") % _column % _dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(_column, _dir);

  switch (_column) {
    case 0:
      file_list_->sort(Register::File::MT_NAME, _dir);
      break;
    case 1:
      file_list_->sort(Register::File::MT_CRDATE, _dir);
      break;
    case 2:
      file_list_->sort(Register::File::MT_TYPE, _dir);
      break;
    case 3:
      file_list_->sort(Register::File::MT_SIZE, _dir);
      break;
  }
}

ccReg::TID ccReg_Files_i::getRowId(CORBA::Short _row)
    throw (ccReg::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Register::File::File *file = file_list_->get(_row);
  if (!file)
    throw ccReg::Table::INVALID_ROW();
  return file->getId();
}

char* ccReg_Files_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Files_i::numRows() {
  Logging::Context ctx(base_context_);

  return file_list_->getCount();
}

CORBA::Short ccReg_Files_i::numColumns() {
  Logging::Context ctx(base_context_);

  return 4;
}

void ccReg_Files_i::reload() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Files_i::reload()");
  file_list_->reload(uf);
}

void ccReg_Files_i::clear() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Files_i::clear()");
  ccReg_PageTable_i::clear();
  file_list_->clear();
}

CORBA::ULongLong ccReg_Files_i::resultSize() {
  Logging::Context ctx(base_context_);

  TRACE("ccReg_Files_i::resultSize()");
  return file_list_->getRealCount(uf);
}

void ccReg_Files_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Files_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::File *tmp = dynamic_cast<Database::Filters::File* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_Files_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void ccReg_Files_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Files_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_FILE, _name, uf);
}

Register::File::File* ccReg_Files_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

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

CORBA::Boolean ccReg_Files_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return file_list_->isLimited(); 
}

