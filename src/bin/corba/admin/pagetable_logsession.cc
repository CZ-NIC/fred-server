/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#include "src/bin/corba/admin/pagetable_logsession.hh"

const int ccReg_LogSession_i::NUM_COLUMNS = 4;

ccReg_LogSession_i::ccReg_LogSession_i(LibFred::Session::List *_list) : m_lel (_list)  {
}

ccReg_LogSession_i::~ccReg_LogSession_i() {
  TRACE("[CALL] ccReg_LogSession_i::~ccReg_LogSession_i()");
}

ccReg::Filters::Compound_ptr ccReg_LogSession_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_LogSession_i::add()");
  Database::Filters::Session *f = new Database::Filters::SessionImpl();
  uf.addFilter(f);
  return it.addE(f);
}

Registry::Table::ColumnHeaders* ccReg_LogSession_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(NUM_COLUMNS);
  COLHEAD(ch,0,"name",CT_OTHER);
  COLHEAD(ch,1,"login_date",CT_OTHER);
  COLHEAD(ch,2,"logout_date",CT_OTHER);
  COLHEAD(ch,3,"lang",CT_OTHER);
  return ch;
}

Registry::TableRow* ccReg_LogSession_i::getRow(CORBA::UShort row)
{
  Logging::Context ctx(base_context_);

  try {
    const LibFred::Session::Session *a = m_lel->get(row);
    Registry::TableRow *tr = new Registry::TableRow;
    tr->length(NUM_COLUMNS);

    (*tr)[0] <<= C_STR(a->getName());
    (*tr)[1] <<= C_STR(a->getLoginDate());
    (*tr)[2] <<= C_STR(a->getLogoutDate());
    (*tr)[3] <<= C_STR(a->getLang());
    return tr;
  }
  catch (...) {
    throw Registry::Table::INVALID_ROW();
  }
}

void ccReg_LogSession_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_LogSession_i::sortByColumn(%1%, %2%)")
      % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);

  switch (column) {
    case 0:
      m_lel->sort(LibFred::Session::MT_NAME, dir);
      break;
    case 1:
      m_lel->sort(LibFred::Session::MT_LOGIN_DATE, dir);
      break;
    case 2:
      m_lel->sort(LibFred::Session::MT_LOGOUT_DATE, dir);
      break;
    case 3:
      m_lel->sort(LibFred::Session::MT_LANG, dir);
      break;
  }
}

ccReg::TID ccReg_LogSession_i::getRowId(CORBA::UShort row)
{
  Logging::Context ctx(base_context_);

  const LibFred::Session::Session *a = m_lel->get(row);
  if (!a)
    throw Registry::Table::INVALID_ROW();
  return a->getId();
}

char* ccReg_LogSession_i::outputCSV() {
  return CORBA::string_dup("1,1,1,1");
}

CORBA::Short ccReg_LogSession_i::numRows() {
  Logging::Context ctx(base_context_);

  return m_lel->size();
}

CORBA::Short ccReg_LogSession_i::numColumns() {
  Logging::Context ctx(base_context_);

  return NUM_COLUMNS;
}

void ccReg_LogSession_i::reload_worker() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_LogSession_i::reload_worker()");
//  m_lel->reload(uf, dbm);
  m_lel->setTimeout(query_timeout);
  m_lel->setLimit(limit_);
  m_lel->reload(uf);
}

void ccReg_LogSession_i::clear() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_LogSession_i::clear()");
  ccReg_PageTable_i::clear();
  m_lel->clear();
}

CORBA::ULongLong ccReg_LogSession_i::resultSize() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("ccReg_LogSession_i::resultSize()");
  return m_lel->getRealCount(uf);
}

void ccReg_LogSession_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_LogSession_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::Session *tmp = dynamic_cast<Database::Filters::Session* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_LogSession_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void ccReg_LogSession_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_LogSession_i::saveFilter('%1%')") % _name);

  std::unique_ptr<LibFred::Filter::Manager>
      tmp_filter_manager(LibFred::Filter::Manager::create());
  tmp_filter_manager->save(LibFred::Filter::FT_SESSION, _name, uf);
}

LibFred::Session::Session* ccReg_LogSession_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    LibFred::Session::Session *s = dynamic_cast<LibFred::Session::Session* >(m_lel->findId(_id));
    if (s) {
      return s;
    }
    return 0;
  }
  catch (LibFred::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_LogSession_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return m_lel->isLimited();
}


