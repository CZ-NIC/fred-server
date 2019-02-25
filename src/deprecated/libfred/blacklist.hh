/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
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
#ifndef BLACKLIST_HH_50696502AC5F4D9082557C36C1ACB7F7
#define BLACKLIST_HH_50696502AC5F4D9082557C36C1ACB7F7

#include <string>
#include "src/deprecated/libfred/exceptions.hh"
#include "src/deprecated/util/dbsql.hh"

namespace LibFred
{
  namespace Domain
  {
    /// domain registration blacklist
    class Blacklist {
     public:
      virtual ~Blacklist() {}
      /// check domain against actual blacklist
      virtual bool checkDomain(const std::string& fqdn) const 
        = 0;
      /// factory function
      static Blacklist *create(DBSharedPtr db);
    };
  };
};

#endif
