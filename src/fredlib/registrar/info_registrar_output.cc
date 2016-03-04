/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  registrar info output structure
 */

#include <algorithm>
#include <string>
#include <sstream>

#include <boost/date_time/posix_time/ptime.hpp>

#include "info_registrar_output.h"
#include "src/fredlib/opcontext.h"
#include "util/util.h"

namespace Fred
{
    bool InfoRegistrarOutput::operator==(const InfoRegistrarOutput& rhs) const
    {
        return info_registrar_data == rhs.info_registrar_data;
    }

    bool InfoRegistrarOutput::operator!=(const InfoRegistrarOutput& rhs) const
    {
        return !this->operator ==(rhs);
    }

    std::string InfoRegistrarOutput::to_string() const
    {
        return Util::format_data_structure("InfoRegistrarOutput",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("info_registrar_data",info_registrar_data.to_string()))
        (std::make_pair("utc_timestamp",boost::lexical_cast<std::string>(utc_timestamp)))
        );
    }

}//namespace Fred

