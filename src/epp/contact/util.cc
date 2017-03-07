/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#include "src/epp/contact/util.h"

#include "util/db/nullable.h"

#include <boost/algorithm/string/trim.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

boost::optional<std::string> trim(const boost::optional<std::string>& src)
{
    return src ? boost::optional<std::string>(boost::trim_copy(*src))
               : boost::optional<std::string>();
}


boost::optional<Nullable<std::string> > trim(const boost::optional<Nullable<std::string> >& src)
{
    return src && !src->isnull() ? boost::optional<Nullable<std::string> >(boost::trim_copy(src->get_value()))
                                 : src;
}


std::vector<boost::optional<Nullable<std::string> > > trim(
        const std::vector<boost::optional<Nullable<std::string> > >& src)
{
    std::vector<boost::optional<Nullable<std::string> > > result;
    result.reserve(src.size());
    for (std::vector<boost::optional<Nullable<std::string> > >::const_iterator data_ptr = src.begin();
         data_ptr != src.end();
         ++data_ptr)
    {
        result.push_back(trim(*data_ptr));
    }
    return result;
}


} // namespace Epp::Contact
} // namespace Epp
