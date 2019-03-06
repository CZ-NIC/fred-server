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
#include "src/bin/corba/admin/pagetable_publicrequests.hh"

ccReg_PublicRequests_i::ccReg_PublicRequests_i(LibFred::PublicRequest::List *_list) :
  request_list_(_list) {
}

ccReg_PublicRequests_i::~ccReg_PublicRequests_i() {
  TRACE("[CALL] ccReg_PublicRequests_i::~ccReg_PublicRequests_i()");
}

ccReg::Filters::Compound_ptr ccReg_PublicRequests_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_PublicRequests_i::add()");
  Database::Filters::PublicRequest *f = new Database::Filters::PublicRequestImpl();
  uf.addFilter(f);
  return it.addE(f);
}

void ccReg_PublicRequests_i::reload_worker() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_PublicRequests_i::reload_worker()");
  request_list_->setTimeout(query_timeout);
  request_list_->setLimit(limit_);
  request_list_->reload(uf);
}

Registry::Table::ColumnHeaders* ccReg_PublicRequests_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_PublicRequests_i::getColumnHeaders()");
  Registry::Table::ColumnHeaders *columns = new Registry::Table::ColumnHeaders();
  columns->length(4);
  COLHEAD(columns, 0, "Create Time", CT_OTHER);
  COLHEAD(columns, 1, "Resolve Time", CT_OTHER);
  COLHEAD(columns, 2, "Type", CT_OTHER);
  COLHEAD(columns, 3, "Status", CT_OTHER);
  return columns;
}

Registry::TableRow* ccReg_PublicRequests_i::getRow(CORBA::UShort _row)
{
  Logging::Context ctx(base_context_);

  const LibFred::PublicRequest::PublicRequest *request = request_list_->get(_row);
  if (!request)
    throw Registry::Table::INVALID_ROW();
  Registry::TableRow *row = new Registry::TableRow();
  row->length(4);
  (*row)[0] <<= C_STR(request->getCreateTime());
  (*row)[1] <<= C_STR(request->getResolveTime());
  (*row)[2] <<= C_STR(request->getType());
  (*row)[3] <<= C_STR(LibFred::PublicRequest::Status2Str(request->getStatus()));
  return row;
}

void ccReg_PublicRequests_i::sortByColumn(CORBA::Short _column, CORBA::Boolean _dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_PublicRequests_i::sortByColumn(%1%, %2%)") % _column % _dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(_column, _dir);

  switch (_column) {
    case 0:
      request_list_->sort(LibFred::PublicRequest::MT_CRDATE, _dir);
      break;
    case 1:
      request_list_->sort(LibFred::PublicRequest::MT_RDATE, _dir);
      break;
    case 2:
      request_list_->sort(LibFred::PublicRequest::MT_TYPE, _dir);
      break;
    case 3:
      request_list_->sort(LibFred::PublicRequest::MT_STATUS, _dir);
      break;
  }
}

ccReg::TID ccReg_PublicRequests_i::getRowId(CORBA::UShort _row)
{
  Logging::Context ctx(base_context_);

  const LibFred::PublicRequest::PublicRequest *request = request_list_->get(_row);
  if (!request)
    throw Registry::Table::INVALID_ROW();
  return request->getId();
}

char* ccReg_PublicRequests_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_PublicRequests_i::numRows() {
  Logging::Context ctx(base_context_);

  return request_list_->getCount();
}

CORBA::Short ccReg_PublicRequests_i::numColumns() {
  Logging::Context ctx(base_context_);

  return 4;
}

void ccReg_PublicRequests_i::clear() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_PublicRequests_i::clear()");
  ccReg_PageTable_i::clear();
  request_list_->clear();
}

CORBA::ULongLong ccReg_PublicRequests_i::resultSize() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("ccReg_PublicRequests_i::resultSize()");
  return request_list_->getRealCount(uf);
}

void ccReg_PublicRequests_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_PublicRequests_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::PublicRequest *tmp = dynamic_cast<Database::Filters::PublicRequest* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_PublicRequests_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void ccReg_PublicRequests_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_PublicRequests_i::saveFilter('%1%')") % _name);

  std::unique_ptr<LibFred::Filter::Manager>
      tmp_filter_manager(LibFred::Filter::Manager::create());
  tmp_filter_manager->save(LibFred::Filter::FT_PUBLICREQUEST, _name, uf);
}

LibFred::PublicRequest::PublicRequest* ccReg_PublicRequests_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    LibFred::PublicRequest::PublicRequest *request = dynamic_cast<LibFred::PublicRequest::PublicRequest* >(request_list_->findId(_id));
    if (request) {
      return request;
    }
    return 0;
  }
  catch (LibFred::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_PublicRequests_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return request_list_->isLimited(); 
}

