#include "pagetable_filters.h"

ccReg_Filters_i::ccReg_Filters_i(Register::Filter::List& _filter_list) : 
	m_filter_list(_filter_list) {
}

ccReg_Filters_i::~ccReg_Filters_i() {
  TRACE("[CALL] ccReg_Filters_i::~ccReg_Filters_i()");
}

void 
ccReg_Filters_i::reload() {
  TRACE("[CALL] ccReg_Filters_i::reload()");
  m_filter_list.reload(uf);
}

ccReg::Filters::Compound_ptr ccReg_Filters_i::add() {
  TRACE("[CALL] ccReg_Filters_i::add()");
  it.clearF();
  Database::Filters::FilterFilter *f = new Database::Filters::FilterFilterImpl();
  uf.addFilter(f);
  return it.addE(f); 
}

Registry::Table::ColumnHeaders* 
ccReg_Filters_i::getColumnHeaders() {
  TRACE("[CALL] ccReg_Filters_i::getColumnHeaders()");
  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(4);
  COLHEAD(ch,0,"UserID", CT_OTHER);
  COLHEAD(ch,1,"GroupID", CT_OTHER);
  COLHEAD(ch,2,"Type", CT_OTHER);
  COLHEAD(ch,3,"Name", CT_OTHER);
  return ch;
}

Registry::TableRow* 
ccReg_Filters_i::getRow(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW) {

  try {
    const Register::Filter::Filter *item = m_filter_list.get(row);
    if (!item) 
    	  throw ccReg::Table::INVALID_ROW();
    
    Registry::TableRow *tr = new Registry::TableRow;
    tr->length(4);
    (*tr)[0] <<= DUPSTRC(item->getUserId().to_string());
    (*tr)[1] <<= DUPSTRC(item->getGroupId().to_string());
    (*tr)[2] <<= DUPSTRC(Conversion<int>::to_string(item->getType()));
    (*tr)[3] <<= DUPSTRFUN(item->getName);
    return tr;
  }
  catch (...) {
    throw ccReg::Table::INVALID_ROW();
  }
}

ccReg::TID 
ccReg_Filters_i::getRowId(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW) {
  
  try {
    const Register::Filter::Filter *item = m_filter_list.get(row);
    if (!item) 
  	  throw ccReg::Table::INVALID_ROW();
    
    return item->getId();
  }
  catch (...) {
    throw ccReg::Table::INVALID_ROW();
  }
}

void 
ccReg_Filters_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
}

char* 
ccReg_Filters_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_Filters_i::numRows() {
  return m_filter_list.size();
}

CORBA::Short 
ccReg_Filters_i::numColumns() {
  return 4;
}

CORBA::ULongLong 
ccReg_Filters_i::resultSize() {
  return 1234;
}

void
ccReg_Filters_i::clear() {
  TRACE("[CALL] ccReg_Filters_i::clear()");
  ccReg_PageTable_i::clear();
  uf.clear();
}

void
ccReg_Filters_i::loadFilter(ccReg::TID _id) {
}

void
ccReg_Filters_i::saveFilter(const char* _name) {
}

CORBA::Boolean ccReg_Filters_i::numRowsOverLimit() {
  return m_filter_list.isLimited(); 
}
