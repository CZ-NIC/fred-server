/*
 * Copyright (C) 2008-2020  CZ.NIC, z. s. p. o.
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
#include <math.h>
#include <memory>
#include <iomanip>
#include "corba/Admin.hh"

#include "src/bin/corba/admin/pagetable_impl.hh"
#include "util/log/logger.hh"

ccReg_PageTable_i::ccReg_PageTable_i()
  : aPageSize(10), aPage(0), filterType(), sorted_by_(-1), sorted_dir_(false), query_timeout(DEFAULT_QUERY_TIMEOUT),
    limit_(1000)
{
  base_context_ = Logging::Context::get();
}

ccReg_PageTable_i::~ccReg_PageTable_i()
{
  //TRACE("[CALL] ccReg_PageTable_i::~ccReg_PageTable_i()");
}

CORBA::Short 
ccReg_PageTable_i::pageSize()
{
  return aPageSize;
}

void 
ccReg_PageTable_i::pageSize(CORBA::Short _v)
{
  aPageSize = _v;
  aPage = 0;
}

CORBA::Short 
ccReg_PageTable_i::page()
{
  return aPage;
}

void 
ccReg_PageTable_i::setPage(CORBA::Short _v) 

{
  aPage = _v;
}

void 
ccReg_PageTable_i::setOffset(CORBA::Long _offset [[gnu::unused]])
{}

void 
ccReg_PageTable_i::setLimit(CORBA::Long _limit)
{
    limit_ = _limit;
}

void
ccReg_PageTable_i::setTimeout(CORBA::Long _timeout)
{
    query_timeout = _timeout;
}

CORBA::Short 
ccReg_PageTable_i::start()
{
  return aPage*aPageSize;
}

CORBA::Short 
ccReg_PageTable_i::numPages()
{
  return (unsigned)ceil((double)numRows()/aPageSize);
}

Registry::TableRow* 
ccReg_PageTable_i::getPageRow(CORBA::Short pageRow)
{
  return getRow(pageRow + start());
}

CORBA::Short 
ccReg_PageTable_i::numPageRows()
{
  unsigned s = start();
  unsigned n = numRows();
  if (s > n) return 0; /// something wrong
  unsigned l = n - s;
  return l < aPageSize ? l : aPageSize;
}

ccReg::TID 
ccReg_PageTable_i::getPageRowId(CORBA::Short row) 
{
  return getRowId(row + start());
}

void
ccReg_PageTable_i::reloadF()
{
}

ccReg::Filters::Compound_ptr
ccReg_PageTable_i::add()
{
  return ccReg::Filters::Compound::_nil();
}

ccReg::Filters::Iterator_ptr 
ccReg_PageTable_i::getIterator()
{
  TRACE("[CALL] ccPageTable_i::getIterator()");
  return it._this();
}

void 
ccReg_PageTable_i::setDB()
{
}

void
ccReg_PageTable_i::clear() {
  TRACE("[CALL] ccReg_PageTable_i::clear()");
  it.clearF();
  uf.clear();
}

void
ccReg_PageTable_i::loadFilter(ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_PageTable_i::loadFilter(%1%)") % _id);
  uf.clear();
  it.clearF();

  std::unique_ptr<LibFred::Filter::Manager>
      tmp_filter_manager(LibFred::Filter::Manager::create());
  tmp_filter_manager->load(_id, uf);
}

void
ccReg_PageTable_i::saveFilter(const char*  _name [[gnu::unused]]) {
}

void ccReg_PageTable_i::sortByColumn(CORBA::Short _column, CORBA::Boolean _dir) {
  sorted_by_ = (_column >= numColumns() ? numColumns() - 1 : _column);
  sorted_dir_ = _dir;
}

void ccReg_PageTable_i::getSortedBy(CORBA::Short &_column, CORBA::Boolean &_dir) {
  _column = sorted_by_;
  _dir = sorted_dir_;
}

CORBA::Boolean ccReg_PageTable_i::numRowsOverLimit() {
  return false; 
}

void ccReg_PageTable_i::reload() {
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    try {
        reload_worker();
    } catch(Database::Exception &ex) {
          std::string message = ex.what();
          if(message.find(Database::Connection::getTimeoutString()) != std::string::npos) {
              throw ccReg::Filters::SqlQueryTimeout();
          }
    }
}
