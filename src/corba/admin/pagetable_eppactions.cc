#include "pagetable_eppactions.h"

ccReg_EPPActions_i::ccReg_EPPActions_i(Register::Registrar::EPPActionList *_eal) :
  eal(_eal) {
}

ccReg_EPPActions_i::~ccReg_EPPActions_i() {
  TRACE("[CALL] ccReg_EPPActions_i::~ccReg_EPPActions_i()");
}

ccReg::Filters::Compound_ptr ccReg_EPPActions_i::add() {
  TRACE("[CALL] ccReg_EPPActions_i::add()");
  it.clearF();
  DBase::Filters::EppAction *f = new DBase::Filters::EppActionImpl();
  uf.addFilter(f);
  return it.addE(f);
}

ccReg::Table::ColumnHeaders* ccReg_EPPActions_i::getColumnHeaders() {
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(7);
  COLHEAD(ch,0,"SvTRID",CT_OTHER);
  COLHEAD(ch,1,"ClTRID",CT_OTHER);
  COLHEAD(ch,2,"Type",CT_OTHER);
  COLHEAD(ch,3,"Handle",CT_OTHER);
  COLHEAD(ch,4,"Registrar",CT_REGISTRAR_HANDLE);
  COLHEAD(ch,5,"Time",CT_OTHER);
  COLHEAD(ch,6,"Result",CT_OTHER);
  return ch;
}

ccReg::TableRow* ccReg_EPPActions_i::getRow(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Registrar::EPPAction *a = eal->get(row);
  if (!a)
    throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(7);
  (*tr)[0] = DUPSTRFUN(a->getServerTransactionId);
  (*tr)[1] = DUPSTRFUN(a->getClientTransactionId);
  (*tr)[2] = DUPSTRFUN(a->getTypeName);
  (*tr)[3] = DUPSTRFUN(a->getHandle);
  (*tr)[4] = DUPSTRFUN(a->getRegistrarHandle);
  (*tr)[5] = DUPSTRDATE(a->getStartTime);
  (*tr)[6] = DUPSTRFUN(a->getResultStatus);
  return tr;
}

void ccReg_EPPActions_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  TRACE(boost::format("[CALL] ccReg_EPPActions_i::sortByColumn(%1%, %2%)")
      % column % dir);
  switch (column) {
    case 0:
      eal->sort(Register::Registrar::MT_SVTRID, dir);
      sorted_by_ = 0;
      break;
    case 1:
      eal->sort(Register::Registrar::MT_CLTRID, dir);
      sorted_by_ = 1;
      break;
    case 2:
      eal->sort(Register::Registrar::MT_TYPE, dir);
      sorted_by_ = 2;
      break;
    case 3:
      eal->sort(Register::Registrar::MT_OBJECT_HANDLE, dir);
      sorted_by_ = 3;
      break;
    case 4:
      eal->sort(Register::Registrar::MT_REGISTRAR_HANDLE, dir);
      sorted_by_ = 4;
      break;
    case 5:
      eal->sort(Register::Registrar::MT_TIME, dir);
      sorted_by_ = 5;
      break;
    case 6:
      eal->sort(Register::Registrar::MT_RESULT, dir);
      sorted_by_ = 6;
      break;
  }
}

ccReg::TID ccReg_EPPActions_i::getRowId(CORBA::Short row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::Registrar::EPPAction *a = eal->get(row);
  if (!a)
    throw ccReg::Table::INVALID_ROW();
  return a->getId();
}

char* ccReg_EPPActions_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_EPPActions_i::numRows() {
  return eal->size();
}

CORBA::Short ccReg_EPPActions_i::numColumns() {
  return 7;
}

void ccReg_EPPActions_i::reload() {
  TRACE("[CALL] ccReg_EPPActions_i::reload()");
  eal->setPartialLoad(true);
  eal->reload2(uf, dbm);
}

void ccReg_EPPActions_i::clear() {
  uf.clear();
}

CORBA::ULongLong ccReg_EPPActions_i::resultSize() {
  return 12345;
}

void ccReg_EPPActions_i::loadFilter(ccReg::TID _id) {
}

void ccReg_EPPActions_i::saveFilter(const char* _name) {
}
