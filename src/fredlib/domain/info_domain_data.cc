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
 *  @file info_domain_data.cc
 *  common domain info data
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
#include "enum_validation_extension.h"
#include "info_domain_data.h"
#include "info_domain_diff.h"

namespace Fred
{

    InfoDomainData::InfoDomainData()
    : historyid(0)
    , crhistoryid(0)
    , id(0)
    {}

    bool InfoDomainData::operator==(const InfoDomainData& rhs) const
    {
        return diff_domain_data(*this, rhs).is_empty();
    }

    bool InfoDomainData::operator!=(const InfoDomainData& rhs) const
    {
        return !this->operator ==(rhs);
    }

    std::string InfoDomainData::to_string() const
    {
        return Util::format_data_structure("InfoDomainData",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid))
        (std::make_pair("fqdn",fqdn))
        (std::make_pair("registrant_handle",registrant_handle))
        (std::make_pair("nsset_handle",nsset_handle.print_quoted()))
        (std::make_pair("keyset_handle",keyset_handle.print_quoted()))
        (std::make_pair("sponsoring_registrar_handle",sponsoring_registrar_handle))
        (std::make_pair("create_registrar_handle",create_registrar_handle))
        (std::make_pair("update_registrar_handle",update_registrar_handle.print_quoted()))
        (std::make_pair("creation_time",boost::lexical_cast<std::string>(creation_time)))
        (std::make_pair("update_time",update_time.print_quoted()))
        (std::make_pair("transfer_time",transfer_time.print_quoted()))
        (std::make_pair("expiration_date",boost::lexical_cast<std::string>(expiration_date)))
        (std::make_pair("authinfopw",authinfopw))
        (std::make_pair("admin_contacts",Util::format_vector(admin_contacts)))
        (std::make_pair("enum_domain_validation",enum_domain_validation.print_quoted()))
        (std::make_pair("outzone_time",boost::lexical_cast<std::string>(outzone_time)))
        (std::make_pair("cancel_time",boost::lexical_cast<std::string>(cancel_time)))
        (std::make_pair("delete_time",delete_time.print_quoted()))
        (std::make_pair("crhistoryid",boost::lexical_cast<std::string>(crhistoryid)))
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid)))
        (std::make_pair("id",boost::lexical_cast<std::string>(id)))
        );


    }


}//namespace Fred

