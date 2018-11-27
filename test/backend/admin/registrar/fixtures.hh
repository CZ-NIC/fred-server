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

#include "src/libfred/object_state/perform_object_state_request.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/create_registrar.hh"
#include "src/libfred/registrar/info_registrar_data.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

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
        const std::string& name = "Updated Name";
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

} // namespace Test::Backend::Admin::Registrar
} // namespace Test::Backend::Admin
} // namespace Test::Backend
} // namespace Test

#endif
