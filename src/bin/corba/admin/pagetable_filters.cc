/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/bin/corba/admin/pagetable_filters.hh"

ccReg_Filters_i::ccReg_Filters_i(LibFred::Filter::List& _filter_list) :
	m_filter_list(_filter_list) {
}

ccReg_Filters_i::~ccReg_Filters_i() {
  TRACE("[CALL] ccReg_Filters_i::~ccReg_Filters_i()");
}

void 
ccReg_Filters_i::reload_worker() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_Filters_i::reload_worker()");
  m_filter_list.setLimit(limit_);
  m_filter_list.reload(uf);
}

ccReg::Filters::Compound_ptr ccReg_Filters_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Filters_i::add()");
  Database::Filters::FilterFilter *f = new Database::Filters::FilterFilterImpl();
  uf.addFilter(f);
  return it.addE(f); 
}

Registry::Table::ColumnHeaders* 
ccReg_Filters_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

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
ccReg_Filters_i::getRow(CORBA::UShort row) 
{
  Logging::Context ctx(base_context_);

  try {
    const LibFred::Filter::Filter *item = m_filter_list.get(row);
    if (!item) 
    	  throw Registry::Table::INVALID_ROW();
    
    Registry::TableRow *tr = new Registry::TableRow;
    tr->length(4);
    (*tr)[0] <<= C_STR(item->getUserId());
    (*tr)[1] <<= C_STR(item->getGroupId());
    (*tr)[2] <<= C_STR((int)item->getType());
    (*tr)[3] <<= C_STR(item->getName());
    return tr;
  }
  catch (...) {
    throw Registry::Table::INVALID_ROW();
  }
}

ccReg::TID 
ccReg_Filters_i::getRowId(CORBA::UShort row)
{
  Logging::Context ctx(base_context_);
 
  try {
    const LibFred::Filter::Filter *item = m_filter_list.get(row);
    if (!item) 
  	  throw Registry::Table::INVALID_ROW();
    
    return item->getId();
  }
  catch (...) {
    throw Registry::Table::INVALID_ROW();
  }
}

void 
ccReg_Filters_i::sortByColumn(CORBA::Short column [[gnu::unused]], CORBA::Boolean dir [[gnu::unused]]) {
  Logging::Context ctx(base_context_);

}

char* 
ccReg_Filters_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_Filters_i::numRows() {
  Logging::Context ctx(base_context_);

  return m_filter_list.size();
}

CORBA::Short 
ccReg_Filters_i::numColumns() {
  Logging::Context ctx(base_context_);

  return 4;
}

CORBA::ULongLong 
ccReg_Filters_i::resultSize() {
  Logging::Context ctx(base_context_);

  return 1234;
}

void
ccReg_Filters_i::clear() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Filters_i::clear()");
  ccReg_PageTable_i::clear();
  uf.clear();
}

void
ccReg_Filters_i::loadFilter(ccReg::TID _id [[gnu::unused]]) {
  Logging::Context ctx(base_context_);

}

void
ccReg_Filters_i::saveFilter(const char* _name [[gnu::unused]]) {
  Logging::Context ctx(base_context_);

}

CORBA::Boolean ccReg_Filters_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return m_filter_list.isLimited(); 
}

