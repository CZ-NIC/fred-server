/*
 * Copyright (C) 2007-2020  CZ.NIC, z. s. p. o.
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
#ifndef TECH_CHECK_HH_8C054AB60655442B89DB1D8CCEA8E431
#define TECH_CHECK_HH_8C054AB60655442B89DB1D8CCEA8E431

#include <vector>
#include <stdexcept>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "corba/TechCheck.hh"
#include "src/bin/corba/nameservice.hh"

class TechCheckManager {
 public:
  typedef std::vector<std::string> FQDNList;
  class RESOLVE_FAILED : public std::runtime_error
  {
  public:
      RESOLVE_FAILED()
      : std::runtime_error("TechCheckManager RESOLVE_FAILED")
      {}
  };

  class INTERNAL_ERROR : public std::runtime_error
  {
  public:
      INTERNAL_ERROR()
      : std::runtime_error("TechCheckManager INTERNAL_ERROR")
      {}
  };
  class REGISTRAR_NOT_FOUND : public std::runtime_error
  {
  public:
      REGISTRAR_NOT_FOUND()
      : std::runtime_error("TechCheckManager REGISTRAR_NOT_FOUND")
      {}
  };
  class NSSET_NOT_FOUND : public std::runtime_error
  {
  public:
      NSSET_NOT_FOUND()
      : std::runtime_error("TechCheckManager NSSET_NOT_FOUND")
      {}
  };
  TechCheckManager(NameService *ns);
  void checkFromRegistrar(
    const std::string& registrar, const std::string& nsset, 
    int level, const FQDNList& fqdns, const char *cltrid
  );
  
 private:
  NameService          *ns_ptr;
  ccReg::TechCheck_var tc;
  boost::mutex         mutex;

  void _resolveInit();
};

#endif
