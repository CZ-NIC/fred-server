/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tech_check.h"

TechCheckManager::TechCheckManager(NameService *ns)
  throw (RESOLVE_FAILED)
{
  try {
    tc = ccReg::TechCheck::_narrow(ns->resolve("TechCheck"));
  } catch (...) { throw RESOLVE_FAILED(); }
}

void 
TechCheckManager::checkFromRegistrar(
  const std::string& registrar, const std::string& nsset, 
  int level, const FQDNList& fqdns, const char *cltrid
) throw (INTERNAL_ERROR, REGISTRAR_NOT_FOUND, NSSET_NOT_FOUND)
{
  try {
    ccReg::Lists cfqdns;
    cfqdns.length(fqdns.size());
    for (unsigned i=0; i<fqdns.size(); i++)
      cfqdns[i] = fqdns[i].c_str();
    CORBA::String_var reg = registrar.c_str();
    CORBA::String_var nss = nsset.c_str();
    tc->checkNssetAsynch(reg,nss,level,false,true,ccReg::CHKR_EPP,cfqdns,cltrid);
  }
  catch (ccReg::TechCheck::NssetNotFound) {
    throw NSSET_NOT_FOUND();
  }
  catch (ccReg::TechCheck::RegistrarNotFound) {
    throw REGISTRAR_NOT_FOUND();
  }
  catch (...) {
    throw INTERNAL_ERROR();
  }
}
