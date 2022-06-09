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

#include "test/poc/parallel-tests/fixtures/keyset.hh"

#include "libfred/registrable_object/keyset/info_keyset.hh"

#include <stdexcept>


namespace {

decltype(auto) make_keyset(LibFred::OperationContext& ctx, LibFred::CreateKeyset& create)
{
    return LibFred::InfoKeysetById{create.exec(ctx).create_object_result.object_id}.exec(ctx).info_keyset_data;
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

void set_keyset(LibFred::CreateKeyset& op, int index)
{
    const auto version = get_version(index);
    static const auto dns_keys = {
        LibFred::DnsKey{257, 3, 10, "AwEAAaKGbJ79nMJ+Zi8142zjKJV5YsNdp8wDIxw7YSJVdpYeH/Zpx564b3z1NhXaSlJoswwEcb"
                                    "26WlQOJhkmtX9kcpvS9L4rwHkcdpXuZ3WTDBe3K+ER66DeVbQpy7RgtEnD6rcEIyx4WETED70y"
                                    "w1Tu3+zBaK2FQRt7UG0NHfdra0T8gFMEy58C9bYykmdndmfe62TQbTrXpg5UHTbQtriSBmxcF5"
                                    "2FezSIq4aPyNB4CXxEpVLwc/08aJJHQ4AY6skQMqfdI11gLCocJW4GuwrIzDYZWaO0F3YVCzae"
                                    "xeAc2b0V5RcJkd6pH0nvpK/6teXonqAy4e/KzdxsXsML748rq4c="},
        LibFred::DnsKey{257, 3, 5, "AwEAAc+9FAaymtiFtuPrWQeakL8bfA03zNCnvHRfZYevMMy7aoCN8wfAMY9XGz3H4lM67VyTf+"
                                   "BjnqmpKDLCnDkDmk1s/W+u2HymyjBGRa7RcAvNP5UIn8D5b4nxYO8dy69wR4qe/NavKMX6MnyI"
                                   "t0ybjvw3d1I3AJRxOw1TsneHymbtDWZ1Hrz65qRHBvg/sibmYtq3xxF/qyo43/a0uCiU6h0zIy"
                                   "0OTRoYUYgVZfR+VJcHjxTdC/uy9mI/oaM+zmCvxQ=="},
        LibFred::DnsKey{257, 3, 13, "8y6c5Wsr+LwQ90vx+s/PlyS/gwN7xt3+1dwVOgFZ3rlwH/uDutonptmJUBLyxLVyuW6lh0UB5K"
                                    "slgywqrCM56A=="}
    };
    op.set_authinfo("5432" + version)
      .set_dns_keys(dns_keys);
}
}//namespace {anonymous}

namespace Test {

Keyset::Keyset(LibFred::OperationContext& ctx, LibFred::CreateKeyset create)
    : data{make_keyset(ctx, create)}
{ }

}//namespace Test

using namespace Test::Setter;

LibFred::CreateKeyset Test::Setter::keyset(LibFred::CreateKeyset create, int index)
{
    set_keyset(create, index);
    return create;
}
