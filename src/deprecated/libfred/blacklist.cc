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

#include <sstream>
#include "src/deprecated/libfred/blacklist.hh"
#include "src/deprecated/util/dbsql.hh"

namespace LibFred
{
  namespace Domain
  {
    class BlacklistImpl : virtual public Blacklist {
     DBSharedPtr db;
     public:
      BlacklistImpl(DBSharedPtr _db) : db(_db)
      {}
      bool checkDomain(const std::string& fqdn) const 
       
      {
        bool ret = false;
        std::ostringstream sql;
        sql << "SELECT id FROM domain_blacklist b "
            << "WHERE '" << fqdn << "' ~ b.regexp "
            << "AND NOW()>b.valid_from "
            << "AND (b.valid_to ISNULL OR NOW()<b.valid_to) ";
        DBSharedPtr  db_freeselect_guard = DBFreeSelectPtr(db.get());
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        if (db->GetSelectRows() > 0) ret = true;
        return ret;
      }
    };
    Blacklist* Blacklist::create(DBSharedPtr db)
    {
      return new BlacklistImpl(db);
    }
  };
};
