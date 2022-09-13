/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#include "test/poc/parallel-tests/fixtures/domain.hh"

#include "libfred/registrable_object/domain/info_domain.hh"

#include <stdexcept>


namespace {

decltype(auto) make_domain(LibFred::OperationContext& ctx, LibFred::CreateDomain& create)
{
    return LibFred::InfoDomainById{create.exec(ctx).create_object_result.object_id}.exec(ctx).info_domain_data;
}

std::string get_version(int index)
{
    if (index < 0)
    {
        throw std::runtime_error{"negative index is not allowed"};
    }
    static constexpr char number_of_letters = 'Z' - 'A';
    if (index <= number_of_letters)
    {
        return std::string(1, 'A' + index);
    }
    return std::to_string(index - number_of_letters);
}

void set_domain(LibFred::CreateDomain& op, bool is_enum_domain, int index)
{
    const auto version = get_version(index);
    if (is_enum_domain)
    {
        op.set_enum_validation_expiration(boost::gregorian::date{boost::gregorian::day_clock::local_day() + boost::gregorian::months{6}});
        op.set_enum_publish_flag((index % 2) == 1);
    }
    op.set_authinfo("2468" + version)
      .set_expiration_date(boost::gregorian::date{boost::gregorian::day_clock::local_day() + boost::gregorian::years{1}});
}

}//namespace {anonymous}

namespace Test {

Domain::Domain(LibFred::OperationContext& ctx, LibFred::CreateDomain create)
    : data{make_domain(ctx, create)}
{ }

}//namespace Test

using namespace Test::Setter;

LibFred::CreateDomain Test::Setter::domain(LibFred::CreateDomain create, int index)
{
    set_domain(create, false, index);
    return create;
}

LibFred::CreateDomain Test::Setter::enum_domain(LibFred::CreateDomain create, int index)
{
    set_domain(create, true, index);
    return create;
}
