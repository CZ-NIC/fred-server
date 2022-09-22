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

#include "test/poc/parallel-tests/fixtures/nsset.hh"

#include "libfred/registrable_object/nsset/info_nsset.hh"

#include <stdexcept>


namespace {

decltype(auto) make_nsset(LibFred::OperationContext& ctx, LibFred::CreateNsset& create)
{
    return LibFred::InfoNssetById{create.exec(ctx).create_object_result.object_id}.exec(ctx).info_nsset_data;
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

void set_nsset(LibFred::CreateNsset& op, int index)
{
    const auto version = get_version(index);
    static const auto dns_hosts = {
            LibFred::DnsHost{
                    "athena.chr.cz", { boost::asio::ip::address::from_string("213.226.210.27"),
                                       boost::asio::ip::address::from_string("2a00:5100:2017:110::1001")}},
            LibFred::DnsHost{
                    "hades.chr.cz", { boost::asio::ip::address::from_string("213.226.210.29"),
                                      boost::asio::ip::address::from_string("2a00:5100:2017:110::1029")}},
            LibFred::DnsHost{
                    "ns0.1984.is", {}},
            LibFred::DnsHost{
                    "ns1.1984hosting.com", {}},
            LibFred::DnsHost{
                    "ns1.1984.is", {}},
            LibFred::DnsHost{
                    "ns2.1984hosting.com", {}},
            LibFred::DnsHost{
                    "ns2.1984.is", {}},
            LibFred::DnsHost{
                    "ns.omegaplus.cz", {}}};
    op.set_authinfo("6543" + version)
      .set_tech_check_level(index % 7)
      .set_dns_hosts(dns_hosts);
}

}//namespace {anonymous}

namespace Test {

Nsset::Nsset(LibFred::OperationContext& ctx, LibFred::CreateNsset create)
    : data{make_nsset(ctx, create)}
{ }

}//namespace Test

using namespace Test::Setter;

LibFred::CreateNsset Test::Setter::nsset(LibFred::CreateNsset create, int index)
{
    set_nsset(create, index);
    return create;
}
