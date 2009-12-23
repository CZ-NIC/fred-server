#include "pagetable_eppactions.h"

ccReg_EPPActions_i::ccReg_EPPActions_i(Register::Registrar::EPPActionList *_eal) :
  eal(_eal) {
}

ccReg_EPPActions_i::~ccReg_EPPActions_i() {
  TRACE("[CALL] ccReg_EPPActions_i::~ccReg_EPPActions_i()");
}

ccReg::Filters::Compound_ptr ccReg_EPPActions_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_EPPActions_i::add()");
  Database::Filters::EppAction *f = new Database::Filters::EppActionImpl();
  uf.addFilter(f);
  return it.addE(f);
}

Registry::Table::ColumnHeaders* ccReg_EPPActions_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(7);
  COLHEAD(ch,0,"SvTRID",CT_OTHER);
  COLHEAD(ch,1,"ClTRID",CT_OTHER);
  COLHEAD(ch,2,"Type",CT_OTHER);
  COLHEAD(ch,3,"Handle",CT_OTHER);
  COLHEAD(ch,4,"Registrar",CT_OID);
  COLHEAD(ch,5,"Time",CT_OTHER);
  COLHEAD(ch,6,"Result",CT_OTHER);
  return ch;
}

Registry::TableRow* ccReg_EPPActions_i::getRow(CORBA::UShort row)
    throw (ccReg::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  try {
    const Register::Registrar::EPPAction *a = eal->get(row);
    Registry::TableRow *tr = new Registry::TableRow;
    tr->length(7);

    MAKE_OID(oid_registrar, a->getRegistrarId(), C_STR(a->getRegistrarHandle()), FT_REGISTRAR)

    (*tr)[0] <<= C_STR(a->getServerTransactionId());
    (*tr)[1] <<= C_STR(a->getClientTransactionId());
    (*tr)[2] <<= C_STR(a->getTypeName());
    (*tr)[3] <<= C_STR(a->getHandle());
    (*tr)[4] <<= oid_registrar;
    (*tr)[5] <<= C_STR(a->getStartTime());
    (*tr)[6] <<= C_STR(a->getResultStatus());
    return tr;
  }
  catch (...) {
    throw ccReg::Table::INVALID_ROW();
  }
}

void ccReg_EPPActions_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_EPPActions_i::sortByColumn(%1%, %2%)")
      % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);
  
  switch (column) {
    case 0:
      eal->sort(Register::Registrar::MT_SVTRID, dir);
      break;
    case 1:
      eal->sort(Register::Registrar::MT_CLTRID, dir);
      break;
    case 2:
      eal->sort(Register::Registrar::MT_TYPE, dir);
      break;
    case 3:
      eal->sort(Register::Registrar::MT_OBJECT_HANDLE, dir);
      break;
    case 4:
      eal->sort(Register::Registrar::MT_REGISTRAR_HANDLE, dir);
      break;
    case 5:
      eal->sort(Register::Registrar::MT_TIME, dir);
      break;
    case 6:
      eal->sort(Register::Registrar::MT_RESULT, dir);
      break;
  }
}

ccReg::TID ccReg_EPPActions_i::getRowId(CORBA::UShort row)
    throw (ccReg::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Register::Registrar::EPPAction *a = eal->get(row);
  if (!a)
    throw ccReg::Table::INVALID_ROW();
  return a->getId();
}

char* ccReg_EPPActions_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_EPPActions_i::numRows() {
  Logging::Context ctx(base_context_);

  return eal->size();
}

CORBA::Short ccReg_EPPActions_i::numColumns() {
  Logging::Context ctx(base_context_);

  return 7;
}

void ccReg_EPPActions_i::reload() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_EPPActions_i::reload()");
  eal->setPartialLoad(true);
  eal->reload(uf);
}

void ccReg_EPPActions_i::clear() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_EPPActions_i::clear()");
  ccReg_PageTable_i::clear();
  eal->clear();
}

CORBA::ULongLong ccReg_EPPActions_i::resultSize() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("ccReg_EPPActions_i::resultSize()");
  return eal->getRealCount(uf);
}

void ccReg_EPPActions_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_EPPActions_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::EppAction *tmp = dynamic_cast<Database::Filters::EppAction* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_EPPActions_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void ccReg_EPPActions_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_EPPActions_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create());
  tmp_filter_manager->save(Register::Filter::FT_ACTION, _name, uf);
}

Register::Registrar::EPPAction* ccReg_EPPActions_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    Register::Registrar::EPPAction *epp_action = dynamic_cast<Register::Registrar::EPPAction* >(eal->findId(_id));
    if (epp_action) {
      return epp_action;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_EPPActions_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return eal->isLimited(); 
}

