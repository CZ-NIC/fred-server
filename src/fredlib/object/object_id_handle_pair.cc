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
 *  object id and handle pair
 */

#include <string>
#include "util/printable.h"

#include "object_id_handle_pair.h"
#include <boost/algorithm/string.hpp>

namespace Fred
{

    ObjectIdHandlePair::ObjectIdHandlePair(const unsigned long long _id, const std::string& _handle)
    : id(_id), handle(_handle)
    {}

    ObjectIdHandlePair::ObjectIdHandlePair()
    : id()
    {}

    bool ObjectIdHandlePair::operator==(const ObjectIdHandlePair& rhs) const
    {
        return (id == rhs.id) && (boost::algorithm::to_upper_copy(handle).compare(boost::algorithm::to_upper_copy(rhs.handle)) == 0);
    }

    bool ObjectIdHandlePair::operator!=(const ObjectIdHandlePair& rhs) const
    {
        return !(*this == rhs);
    }

    std::string ObjectIdHandlePair::to_string() const
    {
        return Util::format_data_structure("ObjectIdHandlePair",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id)))
        (std::make_pair("handle",handle))
        );
    };

}//namespace Fred

