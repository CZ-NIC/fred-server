/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef FIXTURES_HH_174ECEF52C4A4397958E5FC02ED3817F
#define FIXTURES_HH_174ECEF52C4A4397958E5FC02ED3817F

#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opcontext.hh"
#include "libfred/zone/create_zone.hh"
#include "util/random_data_generator.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/asio.hpp>
#include <string>
#include <vector>

namespace Test {
namespace Backend {
namespace Admin {
namespace Zone {

namespace {

constexpr int seconds_per_hour = 60 * 60;
constexpr int expiration_period_min_in_months = 12;
constexpr int expiration_period_max_in_months = 24;
constexpr int ttl_in_seconds = 6 * seconds_per_hour;
constexpr char hostmaster_name[] = "hostmaster_test@nic.cz";
constexpr int refresh_in_seconds  = 4 * seconds_per_hour;
constexpr int update_retr_in_seconds = 2 * seconds_per_hour;
constexpr int expiry_in_seconds = 1 * 7 * 24 * seconds_per_hour;
constexpr int minimum_in_seconds = 3 * seconds_per_hour;
constexpr char ns_fqdn_name[] = "t.ns.nic.cz";

} // namespace Test::Backend::Admin::Zone::{anonymous}

struct ContextHolder
    : virtual instantiate_db_template
{
    LibFred::OperationContextCreator ctx;
};

template <class T>
struct SupplyFixtureCtx : ContextHolder, T
{
    SupplyFixtureCtx()
        : ContextHolder(),
          T(ctx)
    {
        ctx.commit_transaction();
    }
};

struct NonEnumZone
{
    std::string fqdn;

    NonEnumZone(::LibFred::OperationContext& _ctx)
    {
        fqdn = Test::get_nonexistent_value(_ctx, "zone", "fqdn", "text", generate_random_handle);
    }
};

struct EnumZone
{
    std::string fqdn;
    int max_lenght;

    EnumZone()
            : fqdn(""),
              max_lenght(3)
    {
        while (max_lenght != 0)
        {
            fqdn += RandomDataGenerator().xnumstring(1) + ".";
            --max_lenght;
        }
        fqdn += "e164.arpa";
    }
};

struct HasExistingZone {
    NonEnumZone zone;
    int expiration_period_min;
    int expiration_period_max;
    int ttl;
    std::string hostmaster;
    int refresh;
    int update_retr;
    int expiry;
    int minimum;
    std::string ns_fqdn;

    HasExistingZone(LibFred::OperationContext& _ctx)
        : zone(_ctx),
          expiration_period_min(expiration_period_min_in_months),
          expiration_period_max(expiration_period_max_in_months),
          ttl(ttl_in_seconds),
          hostmaster(hostmaster_name),
          refresh(refresh_in_seconds),
          update_retr(update_retr_in_seconds),
          expiry(expiry_in_seconds),
          minimum(minimum_in_seconds),
          ns_fqdn(ns_fqdn_name)
    {
        ::LibFred::Zone::CreateZone(zone.fqdn, expiration_period_min_in_months, expiration_period_max_in_months)
                .exec(_ctx);
    }
};

struct HasEnumZone {
    EnumZone zone;
    int expiration_period_min;
    int expiration_period_max;
    int ttl;
    std::string hostmaster;
    int refresh;
    int update_retr;
    int expiry;
    int minimum;
    std::string ns_fqdn;

    HasEnumZone(LibFred::OperationContext&)
        : zone(),
          expiration_period_min(expiration_period_min_in_months),
          expiration_period_max(expiration_period_max_in_months),
          ttl(ttl_in_seconds),
          hostmaster(hostmaster_name),
          refresh(refresh_in_seconds),
          update_retr(update_retr_in_seconds),
          expiry(expiry_in_seconds),
          minimum(minimum_in_seconds),
          ns_fqdn(ns_fqdn_name)
    {}
};

struct HasNonEnumZone {
    NonEnumZone zone;
    int expiration_period_min;
    int expiration_period_max;
    int ttl;
    std::string hostmaster;
    int refresh;
    int update_retr;
    int expiry;
    int minimum;
    std::string ns_fqdn;

    HasNonEnumZone(LibFred::OperationContext& _ctx)
        : zone(_ctx),
          expiration_period_min(expiration_period_min_in_months),
          expiration_period_max(expiration_period_max_in_months),
          ttl(ttl_in_seconds),
          hostmaster(hostmaster_name),
          refresh(refresh_in_seconds),
          update_retr(update_retr_in_seconds),
          expiry(expiry_in_seconds),
          minimum(minimum_in_seconds),
          ns_fqdn(ns_fqdn_name)
    {}
};

struct HasNonExistentZone {
        NonEnumZone zone;
        std::string nameserver_fqdn;
        std::vector<boost::asio::ip::address> nameserver_ip_addresses;
    HasNonExistentZone(LibFred::OperationContext& _ctx)
        : zone(_ctx),
          nameserver_fqdn(ns_fqdn_name),
          nameserver_ip_addresses()
    {}
};

struct HasMoreRecords {
        NonEnumZone zone;
        unsigned long long zone_id;
        std::string nameserver_fqdn_0;
        std::string nameserver_fqdn_1;
        std::string nameserver_fqdn_2;
        std::vector<boost::asio::ip::address> nameserver_ip_addresses;
    HasMoreRecords(LibFred::OperationContext& _ctx)
        : zone(_ctx),
          nameserver_fqdn_0("t.ns.nic.cz"),
          nameserver_fqdn_1("h.ns.nic.cz"),
          nameserver_fqdn_2("i.ns.nic.cz"),
          nameserver_ip_addresses()
    {
        zone_id = ::LibFred::Zone::CreateZone(
                zone.fqdn,
                expiration_period_min_in_months,
                expiration_period_max_in_months)
                          .exec(_ctx);
    }
};

struct HasEmptyIpAddress {
        NonEnumZone zone;
        unsigned long long zone_id;
        std::string nameserver_fqdn;
        std::vector<boost::asio::ip::address> nameserver_ip_addresses;
    HasEmptyIpAddress(LibFred::OperationContext& _ctx)
        : zone(_ctx),
          nameserver_fqdn(ns_fqdn_name),
          nameserver_ip_addresses()
    {
        zone_id = ::LibFred::Zone::CreateZone(
                zone.fqdn,
                expiration_period_min_in_months,
                expiration_period_max_in_months)
                          .exec(_ctx);
    }
};

struct HasOneIpAddress {
        NonEnumZone zone;
        unsigned long long zone_id;
        std::string nameserver_fqdn;
        std::vector<boost::asio::ip::address> nameserver_ip_addresses;
    HasOneIpAddress(LibFred::OperationContext& _ctx)
        : zone(_ctx),
          nameserver_fqdn(ns_fqdn_name),
          nameserver_ip_addresses()
    {
        nameserver_ip_addresses.emplace_back(boost::asio::ip::address::from_string("2.3.4.5"));
        zone_id = ::LibFred::Zone::CreateZone(
                zone.fqdn,
                expiration_period_min_in_months,
                expiration_period_max_in_months)
                          .exec(_ctx);
    }
};

struct HasMoreIpAddresses {
        NonEnumZone zone;
        unsigned long long zone_id;
        std::string nameserver_fqdn;
        std::vector<boost::asio::ip::address> nameserver_ip_addresses;
    HasMoreIpAddresses(LibFred::OperationContext& _ctx)
        : zone(_ctx),
          nameserver_fqdn(ns_fqdn_name),
          nameserver_ip_addresses()
    {
        nameserver_ip_addresses.emplace_back(boost::asio::ip::address::from_string("2.3.4.5"));
        nameserver_ip_addresses.emplace_back(boost::asio::ip::address::from_string("6.7.8.9"));
        nameserver_ip_addresses.emplace_back(boost::asio::ip::address::from_string("0.0.0.7"));
        zone_id = ::LibFred::Zone::CreateZone(
                zone.fqdn,
                expiration_period_min_in_months,
                expiration_period_max_in_months)
                          .exec(_ctx);
    }
};

} // namespace Test::Backend::Admin::Zone
} // namespace Test::Backend::Admin
} // namespace Test::Backend
} // namespace Test

#endif
