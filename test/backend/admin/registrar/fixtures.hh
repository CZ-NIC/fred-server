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

#ifndef FIXTURES_HH_F2C20603D23A42859C090AFDAF38F926
#define FIXTURES_HH_F2C20603D23A42859C090AFDAF38F926

#include "src/backend/admin/registrar/update_zone_access.hh"
#include "src/backend/admin/registrar/update_epp_auth.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/epp_auth/add_registrar_epp_auth.hh"
#include "libfred/registrar/info_registrar_data.hh"
#include "libfred/registrar/zone_access/add_registrar_zone_access.hh"
#include "libfred/zone/create_zone.hh"
#include "util/random_data_generator.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>

namespace Test {
namespace Backend {
namespace Admin {
namespace Registrar {

namespace {
constexpr char address_street1[] = "Fisrt Street 111";
constexpr char address_street2[] = "Second Street 22";
constexpr char address_street3[] = "Third Street 3";
constexpr char address_city[] = "New City";
constexpr char address_state_or_province[] = "New State";
constexpr char address_postal_code[] = "12300";
constexpr char address_country[] = "CZ";
constexpr bool default_system = false;
constexpr bool default_vat_payer = true;

} // namespace Test::Backend::Admin::Registrar::{anonymous}

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

struct NonexistentRegistrar
{
    LibFred::InfoRegistrarData registrar;

    NonexistentRegistrar(::LibFred::OperationContext& _ctx)
    {
        registrar.handle =
                Test::get_nonexistent_value(_ctx, "registrar", "handle", "text", generate_random_handle);
        registrar.system = default_system;
        registrar.vat_payer = default_vat_payer;
    }
};

struct ExistingRegistrar
{
    LibFred::InfoRegistrarData registrar;

    ExistingRegistrar(::LibFred::OperationContext& _ctx)
    {
        registrar = Test::exec(Test::CreateX_factory<LibFred::CreateRegistrar>().make(), _ctx);
        registrar.system = default_system;
        registrar.vat_payer = default_vat_payer;
    }
};

struct NonexistentZone
{
    ::LibFred::Registrar::ZoneAccess::ZoneAccess zone;

    NonexistentZone(::LibFred::OperationContext& _ctx)
    {
        zone.zone_fqdn = Test::get_nonexistent_value(_ctx, "zone", "fqdn", "text", generate_random_handle);
    }
};

struct ExistingZone : NonexistentZone
{
    ExistingZone(::LibFred::OperationContext& _ctx)
            : NonexistentZone(_ctx)
    {
        ::LibFred::Zone::CreateZone(zone.zone_fqdn, 6, 12).exec(_ctx);
    }
};

struct EmptyContactAddress
{
    boost::optional<std::string> street1;
    boost::optional<std::string> street2;
    boost::optional<std::string> street3;
    boost::optional<std::string> city;
    boost::optional<std::string> state_or_province;
    boost::optional<std::string> postal_code;
    boost::optional<std::string> country;
};

struct FullContactAddress : EmptyContactAddress
{
    FullContactAddress()
    {
        street1 = address_street1;
        street2 = address_street2;
        street3 = address_street3;
        city = address_city;
        state_or_province = address_state_or_province;
        postal_code = address_postal_code;
        country = address_country;
    }
};

struct EmptyContactData
{
    boost::optional<std::string> name;
    boost::optional<std::string> telephone;
    boost::optional<std::string> fax;
    boost::optional<std::string> email;
    boost::optional<std::string> url;
};

struct FullContactData : EmptyContactData
{
    FullContactData(const std::string& _handle)
    {
        name = _handle + " von Test";
        telephone = "+420345234123";
        fax = "+420989898989";
        email = _handle + "@gmail.com";
        url = "www." + _handle + ".cz";
    }
};

struct EmptyMerchantInformation
{
    boost::optional<std::string> organization;
    boost::optional<std::string> ico;
    boost::optional<std::string> dic;
    boost::optional<std::string> variable_symbol;
    boost::optional<std::string> payment_memo_regex;
    boost::optional<bool> vat_payer;
};

struct FullMerchantInformation : EmptyMerchantInformation
{
    FullMerchantInformation(const std::string& _handle)
    {
        organization = _handle + ", a. s.";
        ico = "9999999";
        dic = "9999999CZ";
        variable_symbol = "1234567890";
        payment_memo_regex = "reg";
        vat_payer = true;
    }
};

struct HasExistingRegistrarEmpty : ExistingRegistrar
{
    EmptyContactAddress address;
    EmptyContactData contact;
    boost::optional<bool> system;
    EmptyMerchantInformation merchant;

    HasExistingRegistrarEmpty(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx)
    {
    }
};

struct HasNonexistentRegistrarMin : NonexistentRegistrar
{
    EmptyContactAddress address;
    EmptyContactData contact;
    boost::optional<bool> system;
    EmptyMerchantInformation merchant;

    HasNonexistentRegistrarMin(::LibFred::OperationContext& _ctx)
            : NonexistentRegistrar(_ctx),
              system(false)
    {
    }
};

struct  HasNonexistentRegistrarMax : NonexistentRegistrar
{
    FullContactAddress address;
    FullContactData contact;
    boost::optional<bool> system;
    FullMerchantInformation merchant;

    HasNonexistentRegistrarMax(::LibFred::OperationContext& _ctx)
            : NonexistentRegistrar(_ctx),
              contact(registrar.handle),
              system(false),
              merchant(registrar.handle)
    {
        registrar.street1 = address.street1.get();
        registrar.street2 = address.street2.get();
        registrar.street3 = address.street3.get();
        registrar.city = address.city.get();
        registrar.stateorprovince = address.state_or_province.get();
        registrar.postalcode = address.postal_code.get();
        registrar.country = address.country.get();

        registrar.name = contact.name.get();
        registrar.telephone = contact.telephone.get();
        registrar.fax = contact.fax.get();
        registrar.email = contact.email.get();
        registrar.url = contact.url.get();

        registrar.system = system.get();

        registrar.organization = merchant.organization.get();
        registrar.ico = merchant.ico.get();
        registrar.dic = merchant.dic.get();
        registrar.variable_symbol = merchant.variable_symbol.get();
        registrar.vat_payer = merchant.vat_payer.get();
        registrar.payment_memo_regex = merchant.payment_memo_regex.get();
    }
};

struct HasSystemRegistrar : NonexistentRegistrar
{
    FullContactAddress address;
    EmptyContactData contact;
    boost::optional<bool> system;
    EmptyMerchantInformation merchant;

    HasSystemRegistrar(::LibFred::OperationContext& _ctx)
            : NonexistentRegistrar(_ctx),
              system(true)
    {
        registrar.street1 = address.street1.get();
        registrar.street2 = address.street2.get();
        registrar.street3 = address.street3.get();
        registrar.city = address.city.get();
        registrar.stateorprovince = address.state_or_province.get();
        registrar.postalcode = address.postal_code.get();
        registrar.country = address.country.get();

        registrar.system = system.get();
    }
};

struct HasExistingRegistrarMin : ExistingRegistrar
{
    EmptyContactAddress address;
    EmptyContactData contact;
    boost::optional<bool> system;
    EmptyMerchantInformation merchant;

    HasExistingRegistrarMin(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx)
    {
        const std::string name = "Updated Name";
        registrar.name = name;
        contact.name = name;
    }
};

struct HasExistingRegistrarMax : ExistingRegistrar
{
    FullContactAddress address;
    FullContactData contact;
    boost::optional<bool> system;
    FullMerchantInformation merchant;

    HasExistingRegistrarMax(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              contact(registrar.handle),
              system(false),
              merchant(registrar.handle)
    {
        registrar.street1 = address.street1.get();
        registrar.street2 = address.street2.get();
        registrar.street3 = address.street3.get();
        registrar.city = address.city.get();
        registrar.stateorprovince = address.state_or_province.get();
        registrar.postalcode = address.postal_code.get();
        registrar.country = address.country.get();

        registrar.name = contact.name.get();
        registrar.telephone = contact.telephone.get();
        registrar.fax = contact.fax.get();
        registrar.email = contact.email.get();
        registrar.url = contact.url.get();

        registrar.system = system.get();

        registrar.organization = merchant.organization.get();
        registrar.ico = merchant.ico.get();
        registrar.dic = merchant.dic.get();
        registrar.variable_symbol = merchant.variable_symbol.get();
        registrar.vat_payer = merchant.vat_payer.get();
        registrar.payment_memo_regex = merchant.payment_memo_regex.get();
    }
};

struct HasZoneAccessWithNonexistentRegistrar : NonexistentRegistrar
{
    ExistingZone zone;
    ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses accesses;

    HasZoneAccessWithNonexistentRegistrar(::LibFred::OperationContext& _ctx)
            : NonexistentRegistrar(_ctx),
              zone(_ctx)
    {
        zone.zone.from_date = boost::gregorian::day_clock::local_day();
        accesses.zone_accesses.push_back(zone.zone);
        accesses.registrar_handle = registrar.handle;
    }
};

struct HasZoneAccessWithNonexistentZone : ExistingRegistrar
{
    NonexistentZone zone;
    ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses accesses;

    HasZoneAccessWithNonexistentZone(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              zone(_ctx)
    {
        zone.zone.from_date = boost::gregorian::day_clock::local_day();
        accesses.zone_accesses.push_back(zone.zone);
        accesses.registrar_handle = registrar.handle;
    }
};

struct HasNonexistentZoneAccess : ExistingRegistrar
{
    ExistingZone zone;
    ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses accesses;

    HasNonexistentZoneAccess(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              zone(_ctx)
    {
        zone.zone.id =
                Test::get_nonexistent_value(_ctx, "registrarinvoice", "id", "bigint", generate_random_bigserial);
        zone.zone.from_date = boost::gregorian::day_clock::local_day();
        accesses.zone_accesses.push_back(zone.zone);
        accesses.registrar_handle = registrar.handle;
    }
};

struct HasNoUpdateData : ExistingRegistrar
{
    ExistingZone zone;
    ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses accesses;

    HasNoUpdateData(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              zone(_ctx)
    {
        zone.zone.id = 1;
        accesses.zone_accesses.push_back(zone.zone);
        accesses.registrar_handle = registrar.handle;
    }
};

struct HasMissingParams : ExistingRegistrar
{
    ExistingZone zone;
    ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses accesses;

    HasMissingParams(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              zone(_ctx)
    {
        accesses.zone_accesses.push_back(zone.zone);
        accesses.registrar_handle = registrar.handle;
    }
};

struct HasZoneAccessEmpty : ExistingRegistrar
{
    ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses accesses;

    HasZoneAccessEmpty(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx)
    {
        accesses.registrar_handle = registrar.handle;
    }
};

struct HasAddZoneAccessMax : ExistingRegistrar
{
    ExistingZone zone;
    ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses accesses;

    HasAddZoneAccessMax(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              zone(_ctx)
    {
        zone.zone.from_date = boost::gregorian::day_clock::local_day();
        zone.zone.to_date = zone.zone.from_date + boost::gregorian::weeks_duration(3);
        accesses.zone_accesses.push_back(zone.zone);
        accesses.registrar_handle = registrar.handle;
    }
};

struct HasUpdateZoneAccessMax : ExistingRegistrar
{
    ExistingZone zone;
    ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses accesses;

    HasUpdateZoneAccessMax(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              zone(_ctx)
    {
        boost::gregorian::date today = boost::gregorian::day_clock::local_day();
        zone.zone.id = ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(
                        registrar.handle,
                        zone.zone.zone_fqdn,
                        today)
                .exec(_ctx);
        zone.zone.from_date = today + boost::gregorian::date_duration(1);
        zone.zone.to_date = zone.zone.from_date + boost::gregorian::weeks_duration(3);
        accesses.zone_accesses.push_back(zone.zone);
        accesses.registrar_handle = registrar.handle;
    }
};

struct HasMoreZoneAccesses : ExistingRegistrar
{
    ExistingZone zone;
    ::LibFred::Registrar::ZoneAccess::RegistrarZoneAccesses accesses;

    HasMoreZoneAccesses(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              zone(_ctx)
    {
        boost::gregorian::date today = boost::gregorian::day_clock::local_day();
        for (unsigned i = 0; i < 2; ++i)
        {
            zone.zone.from_date = today;
            accesses.zone_accesses.push_back(zone.zone);
        }
        for (unsigned i = 0; i < 3; ++i)
        {
            zone.zone.id =
                    ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(
                            registrar.handle,
                            zone.zone.zone_fqdn,
                            today)
                    .exec(_ctx);
            if (i % 2 == 1 || i == 0)
            {
                zone.zone.from_date = today - boost::gregorian::date_duration(i + 1);
            }
            if (i % 2 == 0)
            {
                zone.zone.to_date = today + boost::gregorian::weeks_duration(3);
            }
            accesses.zone_accesses.push_back(zone.zone);
        }
        accesses.registrar_handle = registrar.handle;
    }
};

struct NonexistentEppAuth
{
    ::Admin::Registrar::EppAuthRecord auth;

    NonexistentEppAuth(::LibFred::OperationContext& _ctx)
    {
        auth.certificate_fingerprint =
                Test::get_nonexistent_value(_ctx, "registraracl", "cert", "text", generate_random_handle);
    }
};

struct ExistingEppAuth : NonexistentEppAuth
{
    ExistingEppAuth(::LibFred::OperationContext& _ctx, const std::string& _registrar_handle)
            : NonexistentEppAuth(_ctx)
    {
        auth.plain_password = RandomDataGenerator().xstring(20);
        auth.id = ::LibFred::Registrar::EppAuth::AddRegistrarEppAuth(_registrar_handle,
                    auth.certificate_fingerprint,
                    auth.plain_password)
                .exec(_ctx);
    }
};

struct HasEppAuthWithNonexistentRegistrar : NonexistentRegistrar
{
    NonexistentEppAuth epp_auth;
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasEppAuthWithNonexistentRegistrar(::LibFred::OperationContext& _ctx)
            : NonexistentRegistrar(_ctx),
              epp_auth(_ctx)
    {
        epp_auth_data.registrar_handle = registrar.handle;
        epp_auth.auth.plain_password = RandomDataGenerator().xstring(20);
        epp_auth_data.epp_auth_records.push_back(epp_auth.auth);
    }
};

struct HasEppAuthMissingParams : ExistingRegistrar
{
    NonexistentEppAuth epp_auth;
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasEppAuthMissingParams(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              epp_auth(_ctx)
    {
        epp_auth_data.registrar_handle = registrar.handle;
        epp_auth_data.epp_auth_records.push_back(epp_auth.auth);
    }
};

struct HasAddDuplicateCert : ExistingRegistrar
{
    ExistingEppAuth epp_auth;
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasAddDuplicateCert(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              epp_auth(_ctx, registrar.handle)
    {
        epp_auth.auth.id = 0;
        epp_auth_data.registrar_handle = registrar.handle;
        epp_auth_data.epp_auth_records.push_back(epp_auth.auth);
    }
};

struct HasUpdateDuplicateCert : ExistingRegistrar
{
    ExistingEppAuth epp_auth;
    ExistingEppAuth auth_for_update;
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasUpdateDuplicateCert(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              epp_auth(_ctx, registrar.handle),
              auth_for_update(_ctx, registrar.handle)
    {
        epp_auth_data.registrar_handle = registrar.handle;
        epp_auth_data.epp_auth_records.push_back(epp_auth.auth);
        auth_for_update.auth.certificate_fingerprint = epp_auth.auth.certificate_fingerprint;
        epp_auth_data.epp_auth_records.push_back(auth_for_update.auth);
    }
};

struct HasCloneDuplicateCert : ExistingRegistrar
{
    ExistingEppAuth epp_auth;
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasCloneDuplicateCert(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              epp_auth(_ctx, registrar.handle)
    {
        epp_auth.auth.new_certificate_fingerprint = epp_auth.auth.certificate_fingerprint;
        epp_auth_data.registrar_handle = registrar.handle;
        epp_auth_data.epp_auth_records.push_back(epp_auth.auth);
    }
};

struct HasEppAuthNoUpdateData : ExistingRegistrar
{
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasEppAuthNoUpdateData(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx)
    {
        ::Admin::Registrar::EppAuthRecord epp_auth;
        epp_auth.id = 1;
        epp_auth_data.epp_auth_records.push_back(epp_auth);
    }
};

struct HasNoexistentEppAuth : ExistingRegistrar
{
    NonexistentEppAuth epp_auth;
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasNoexistentEppAuth(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              epp_auth(_ctx)
    {
        epp_auth.auth.id =
                Test::get_nonexistent_value(_ctx, "registraracl", "id", "bigint", generate_random_bigserial);
        epp_auth_data.registrar_handle = registrar.handle;
        epp_auth_data.epp_auth_records.push_back(epp_auth.auth);
    }
};

struct HasEmptyEppAuth : ExistingRegistrar
{
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasEmptyEppAuth(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx)
    {
    }
};

struct HasEppAuthAddWithDelete : ExistingRegistrar
{
    NonexistentEppAuth nonexistent_auth;
    ExistingEppAuth existing_auth;
    ExistingEppAuth auth_for_delete;
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasEppAuthAddWithDelete(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              nonexistent_auth(_ctx),
              existing_auth(_ctx, registrar.handle),
              auth_for_delete(_ctx, registrar.handle)
    {
        epp_auth_data.registrar_handle = registrar.handle;
        nonexistent_auth.auth.plain_password = RandomDataGenerator().xstring(10);
        epp_auth_data.epp_auth_records.push_back(nonexistent_auth.auth);
        epp_auth_data.epp_auth_records.push_back(existing_auth.auth);
    }
};

struct HasEppAuthUpdateWithDelete : ExistingRegistrar
{
    ExistingEppAuth auth_for_delete;
    ExistingEppAuth update_pass;
    ExistingEppAuth update_cert;
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasEppAuthUpdateWithDelete(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              auth_for_delete(_ctx, registrar.handle),
              update_pass(_ctx, registrar.handle),
              update_cert(_ctx, registrar.handle)
    {
        update_pass.auth.plain_password = RandomDataGenerator().xstring(10);
        update_cert.auth.certificate_fingerprint =
                Test::get_nonexistent_value(_ctx, "registraracl", "cert", "text", generate_random_handle);
        epp_auth_data.registrar_handle = registrar.handle;
        epp_auth_data.epp_auth_records.push_back(update_pass.auth);
        epp_auth_data.epp_auth_records.push_back(update_cert.auth);
    }
};

struct HasEppAuthCloneWithDelete : ExistingRegistrar
{
    ExistingEppAuth auth_for_delete;
    NonexistentEppAuth new_epp_auth;
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasEppAuthCloneWithDelete(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx),
              auth_for_delete(_ctx, registrar.handle),
              new_epp_auth(_ctx)
    {
        new_epp_auth.auth.plain_password = RandomDataGenerator().xstring(10);
        new_epp_auth.auth.new_certificate_fingerprint =
                Test::get_nonexistent_value(_ctx, "registraracl", "cert", "text", generate_random_handle);
        epp_auth_data.registrar_handle = registrar.handle;
        epp_auth_data.epp_auth_records.push_back(new_epp_auth.auth);
    }
};

struct HasMoreEppAuthData : ExistingRegistrar
{
    ::Admin::Registrar::EppAuthData epp_auth_data;

    HasMoreEppAuthData(::LibFred::OperationContext& _ctx)
            : ExistingRegistrar(_ctx)
    {
        ExistingEppAuth auth_for_delete(_ctx, registrar.handle);

        NonexistentEppAuth auth_for_add(_ctx);
        auth_for_add.auth.plain_password = RandomDataGenerator().xstring(10);
        epp_auth_data.epp_auth_records.push_back(auth_for_add.auth);

        NonexistentEppAuth auth_for_clone(_ctx);
        auth_for_clone.auth.plain_password = RandomDataGenerator().xstring(10);
        auth_for_clone.auth.new_certificate_fingerprint =
                Test::get_nonexistent_value(_ctx, "registraracl", "cert", "text", generate_random_handle);
        epp_auth_data.epp_auth_records.push_back(auth_for_clone.auth);

        ExistingEppAuth pass_for_update(_ctx, registrar.handle);
        pass_for_update.auth.plain_password = RandomDataGenerator().xstring(10);
        epp_auth_data.epp_auth_records.push_back(pass_for_update.auth);

        ExistingEppAuth cert_for_update(_ctx, registrar.handle);
        cert_for_update.auth.certificate_fingerprint =
                Test::get_nonexistent_value(_ctx, "registraracl", "cert", "text", generate_random_handle);
        epp_auth_data.epp_auth_records.push_back(cert_for_update.auth);

        ExistingEppAuth update_all(_ctx, registrar.handle);
        update_all.auth.plain_password = RandomDataGenerator().xstring(10);
        update_all.auth.certificate_fingerprint =
                Test::get_nonexistent_value(_ctx, "registraracl", "cert", "text", generate_random_handle);
        epp_auth_data.epp_auth_records.push_back(update_all.auth);

        epp_auth_data.registrar_handle = registrar.handle;
    }
};

} // namespace Test::Backend::Admin::Registrar
} // namespace Test::Backend::Admin
} // namespace Test::Backend
} // namespace Test

#endif
