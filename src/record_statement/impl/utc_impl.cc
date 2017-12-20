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

#include "src/record_statement/record_statement.hh"
#include "src/record_statement/impl/templates_impl.hh"
#include "src/util/tz/utc.hh"

namespace LibFred {
namespace RecordStatement {
namespace Impl {

template class InstanceOfNecessaryImpl<Tz::UTC>;

} // namespace LibFred::RecordStatement::Impl
} // namespace LibFred::RecordStatement
} // namespace LibFred
