/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef UTIL_H_B03B5CE2A82E4F1EA6B482414595DB5D
#define UTIL_H_B03B5CE2A82E4F1EA6B482414595DB5D

#include "util/db/nullable.h"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

boost::optional<std::string>
trim(const boost::optional<std::string>& src);

boost::optional<Nullable<std::string> >
trim(const boost::optional<Nullable<std::string> >& src);

std::vector<boost::optional<Nullable<std::string> > >
trim(const std::vector<boost::optional<Nullable<std::string> > >& src);

} // namespace Epp::Contact
} // namespace Epp

#endif
