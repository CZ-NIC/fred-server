/*
 * Copyright (C) 2009-2020  CZ.NIC, z. s. p. o.
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
#ifndef LOG_IMPL_WRAP_HH_0D1DFB40B54E427BBDEC954326FCE2CC
#define LOG_IMPL_WRAP_HH_0D1DFB40B54E427BBDEC954326FCE2CC

#include "corba/LoggerFilter.hh"

#include "src/deprecated/libfred/requests/request_manager.hh"
#include "src/bin/corba/admin/usertype_conv.hh"
#include "src/bin/corba/admin/pagetable_logger.hh"
#include "src/bin/corba/admin/common.hh"



using namespace LibFred::Logger;


class ccReg_Log_i : public POA_ccReg::LoggerFilter,
  public PortableServer::RefCountServantBase
{

public:

  ccReg_Log_i(const std::string database); 

  // ccReg_Log_i(const std::string database) throw (Impl_Log_If::DB_CONNECT_FAILED): Impl_Log_If(database) {};
  virtual ~ccReg_Log_i();

  Registry::PageTable_ptr createPageTable(const char *session_id);
  void deletePageTable(const char *session_id);

  ccReg::LoggerFilter::Detail* getDetail(ccReg::TID _id);
  ccReg::LoggerFilter::Detail* createRequestDetail(LibFred::Logger::Request *req);

private:
  typedef std::map<std::string, ccReg_Logger_i*> pagetables_list;

  pagetables_list pagetables;
  std::unique_ptr<LibFred::Logger::Manager> back;

  boost::mutex pagetables_mutex;

public:
  typedef LibFred::Logger::Manager::DB_CONNECT_FAILED DB_CONNECT_FAILED ;
};

#endif

