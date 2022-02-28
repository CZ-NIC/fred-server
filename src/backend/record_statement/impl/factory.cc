/*
 * Copyright (C) 2017-2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/record_statement/impl/factory.hh"

#include "src/util/tz/europe/prague.hh"
#include "src/util/tz/utc.hh"

#include <stdexcept>


namespace Fred {
namespace Backend {
namespace RecordStatement {
namespace Impl {

const Factory& get_default_factory()
{
    static const auto factory = []()
    {
        Factory factory{};
        register_producer<Tz::Europe::Prague>(factory);
        register_producer<Tz::UTC>(factory);
        return factory;
    }();
    return factory;
}

} // namespace Fred::Backend::RecordStatement::Impl
} // namespace Fred::Backend::RecordStatement
} // namespace Fred::Backend
} // namespace Fred

