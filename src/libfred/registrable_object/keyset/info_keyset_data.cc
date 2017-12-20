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
 *  common keyset info data
 */

#include <algorithm>
#include <string>

#include <boost/lexical_cast.hpp>

#include "src/util/util.hh"
#include "src/util/printable.hh"
#include "src/libfred/registrable_object/keyset/info_keyset_data.hh"
#include "src/libfred/registrable_object/keyset/info_keyset_diff.hh"

namespace LibFred
{

    InfoKeysetData::InfoKeysetData()
    : crhistoryid(0)
    , historyid(0)
    , id(0)
    {}

    bool InfoKeysetData::operator==(const InfoKeysetData& rhs) const
    {
        return diff_keyset_data(*this, rhs).is_empty();
    }

    bool InfoKeysetData::operator!=(const InfoKeysetData& rhs) const
    {
        return !this->operator ==(rhs);
    }

    std::string InfoKeysetData::to_string() const
    {
        return Util::format_data_structure("InfoKeysetData",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("crhistoryid",boost::lexical_cast<std::string>(crhistoryid)))
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid)))
        (std::make_pair("id",boost::lexical_cast<std::string>(id)))
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
        (std::make_pair("dns_keys", Util::format_container(dns_keys)))
        (std::make_pair("tech_contacts",Util::format_container(tech_contacts)))
        );
    }

} // namespace LibFred

