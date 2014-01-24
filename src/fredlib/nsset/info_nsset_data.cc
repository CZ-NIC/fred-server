/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file
 *  common nsset info data
 */

#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/nsset/info_nsset_diff.h"

namespace Fred
{

    InfoNssetData::InfoNssetData()
    : crhistoryid(0)
    , historyid(0)
    , id(0)
    {}

    bool InfoNssetData::operator==(const InfoNssetData& rhs) const
    {
        return diff_nsset_data(*this, rhs).is_empty();
    }

    bool InfoNssetData::operator!=(const InfoNssetData& rhs) const
    {
        return !this->operator ==(rhs);
    }

    std::string InfoNssetData::to_string() const
    {
        return Util::format_data_structure("InfoNssetData",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("crhistoryid",boost::lexical_cast<std::string>(crhistoryid)))
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid)))
        (std::make_pair("delete_time",delete_time.print_quoted()))
        (std::make_pair("handle",handle))
        (std::make_pair("roid",roid))
        (std::make_pair("sponsoring_registrar_handle",sponsoring_registrar_handle))
        (std::make_pair("create_registrar_handle",create_registrar_handle))
        (std::make_pair("update_registrar_handle",update_registrar_handle.print_quoted()))
        (std::make_pair("creation_time",boost::lexical_cast<std::string>(creation_time)))
        (std::make_pair("update_time",update_time.print_quoted()))
        (std::make_pair("transfer_time",transfer_time.print_quoted()))
        (std::make_pair("authinfopw",authinfopw))
        (std::make_pair("tech_check_level",tech_check_level.print_quoted()))
        (std::make_pair("dns_hosts", Util::format_vector(dns_hosts)))
        (std::make_pair("tech_contacts",Util::format_vector(tech_contacts)))
        );
    }

}//namespace Fred

