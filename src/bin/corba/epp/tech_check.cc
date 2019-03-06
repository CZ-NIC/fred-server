/*
 * Copyright (C) 2007-2019  CZ.NIC, z. s. p. o.
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
#include "src/bin/corba/epp/tech_check.hh"
#include "util/log/logger.hh"

#include "src/deprecated/util/log.hh"

TechCheckManager::TechCheckManager(NameService *ns) : ns_ptr(ns)
{
  // try {
  //   _resolveInit();
  // }
  // catch(...) {
  //   LOG<WARNING_LOG>( "tech_check_manager: can't connect to tech_check in initialization..."); 
  // }
}

void 
TechCheckManager::checkFromRegistrar(
  const std::string& registrar, const std::string& nsset, 
  int level, const FQDNList& fqdns, const char *cltrid
)
{
  try {
    ccReg::Lists cfqdns;
    cfqdns.length(fqdns.size());
    for (unsigned i=0; i<fqdns.size(); i++)
      cfqdns[i] = fqdns[i].c_str();
    CORBA::String_var reg = registrar.c_str();
    CORBA::String_var nss = nsset.c_str();
    _resolveInit();
    tc->checkNssetAsynch(reg,nss,level,true,true,ccReg::CHKR_EPP,cfqdns,cltrid);
  }
  catch (ccReg::TechCheck::NssetNotFound&) {
    throw NSSET_NOT_FOUND();
  }
  catch (ccReg::TechCheck::RegistrarNotFound&) {
    throw REGISTRAR_NOT_FOUND();
  }
  catch (...) {
    throw INTERNAL_ERROR();
  }
}

void 
TechCheckManager::_resolveInit()
{
  try {
    boost::mutex::scoped_lock scoped_lock(mutex);
    if (!CORBA::is_nil(tc))
      return;
    LOGGER.debug("resolving corba reference");
    tc = ccReg::TechCheck::_narrow(ns_ptr->resolve("TechCheck"));
  } 
  catch (...) { 
    LOGGER.debug("resolving of corba 'TechCheck' object failed");
    throw RESOLVE_FAILED(); 
  }
  LOGGER.debug("resolving of corba 'TechCheck' object ok");
}

