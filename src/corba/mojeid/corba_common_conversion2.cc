/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#include "src/corba/mojeid/corba_common_conversion2.h"
#include <boost/algorithm/string/trim.hpp>
#include <stdexcept>

namespace Corba {
namespace Conversion {

from_into< char*, std::string >::dst_value_ref
from_into< char*, std::string >::operator()(src_value src, dst_value_ref dst)const
{
    if (src == NULL) {
        throw std::runtime_error("unable convert NULL pointer into std::string value");
    }
    std::string tmp = src;
    boost::algorithm::trim(tmp);
    return dst = tmp;
}

}//Corba::Conversion
}//Corba
