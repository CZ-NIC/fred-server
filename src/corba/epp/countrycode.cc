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



#include "countrycode.h"
#include <algorithm>
#include "fredlib/db_settings.h"
#include "log/logger.h"

CountryCode::CountryCode()
    : cc_(0)
{}

CountryCode::CountryCode(int num)
    : cc_(num)
{}

bool CountryCode::AddCode(const char *code)
{
    std::string code_str(code);
    if (code_str.length() == MAX_CC)
    {
        cc_.push_back(code_str);
        return true;
    }

  return false;
}

CountryCode::~CountryCode()
{}

bool CountryCode::TestCountryCode(const char *cc)
{
    const std::string code_str (cc);
    if(code_str.length() == 0) return true;// if no country code
    if(code_str.length() != MAX_CC) return false;
    return ( std::find(cc_.begin(),cc_.end(), code_str) != cc_.end());
}

void CountryCode::load()
{
    std::string query("SELECT id FROM enum_country order by id");
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res  = conn.exec(query);
    std::size_t res_size = res.size();
    if(cc_.capacity() < res_size)
        cc_.reserve(res_size);

    for (unsigned i=0; i < res_size; i++)
    {
        std::string code_str = res[i][0];
        cc_.push_back(code_str);
    }//for i
}
