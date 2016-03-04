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
 *  contact info output structure
 */

#include <string>

#include <boost/lexical_cast.hpp>

#include "info_contact_output.h"
#include "util/util.h"

namespace Fred
{
    bool InfoContactOutput::operator==(const InfoContactOutput& rhs) const
    {
        return info_contact_data == rhs.info_contact_data;
    }

    bool InfoContactOutput::operator!=(const InfoContactOutput& rhs) const
    {
        return !this->operator ==(rhs);
    }

    std::string InfoContactOutput::to_string() const
    {
        return Util::format_data_structure("InfoContactOutput",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("info_contact_data",info_contact_data.to_string()))
        (std::make_pair("utc_timestamp",boost::lexical_cast<std::string>(utc_timestamp)))
        (std::make_pair("next_historyid",next_historyid.print_quoted()))
        (std::make_pair("history_valid_from",boost::lexical_cast<std::string>(history_valid_from)))
        (std::make_pair("history_valid_to",history_valid_to.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id.print_quoted()))
        );
    }

}//namespace Fred

