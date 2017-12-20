/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef TEST_INTERFACE_EPP_FIXTURE_681453104385310
#define TEST_INTERFACE_EPP_FIXTURE_681453104385310

#include "test/setup/fixtures.hh"
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/check_contact_config_data.hh"
#include "src/backend/epp/contact/contact_disclose.hh"
#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/contact_change.hh"
#include "src/backend/epp/contact/create_contact_config_data.hh"
#include "src/backend/epp/contact/create_contact_input_data.hh"
#include "src/backend/epp/contact/delete_contact_config_data.hh"
#include "src/backend/epp/contact/info_contact_config_data.hh"
#include "src/backend/epp/contact/transfer_contact_config_data.hh"
#include "src/backend/epp/contact/update_contact_config_data.hh"
#include "src/backend/epp/impl/disclose_policy.hh"
#include "src/backend/epp/session_data.hh"
#include "src/libfred/object_state/create_object_state_request_id.hh"
#include "src/libfred/object_state/get_object_states.hh"
#include "src/libfred/object_state/perform_object_state_request.hh"
#include "src/util/optional_value.hh"
#include "src/util/db/nullable.hh"

namespace Test {
namespace Backend {
namespace Epp {
namespace Contact {

struct DefaultCheckContactConfigData : ::Epp::Contact::CheckContactConfigData
{
    DefaultCheckContactConfigData()
        : CheckContactConfigData(false)
    {
    }
};

struct DefaultInfoContactConfigData : ::Epp::Contact::InfoContactConfigData
{
    DefaultInfoContactConfigData()
        : InfoContactConfigData(false)
    {
    }
};

struct DefaultCreateContactConfigData : ::Epp::Contact::CreateContactConfigData
{
    DefaultCreateContactConfigData()
        : CreateContactConfigData(false)
    {
    }
};

struct DefaultUpdateContactConfigData : ::Epp::Contact::UpdateContactConfigData
{
    DefaultUpdateContactConfigData()
        : UpdateContactConfigData(false, false)
    {
    }
};

struct DefaultDeleteContactConfigData : ::Epp::Contact::DeleteContactConfigData
{
    DefaultDeleteContactConfigData()
        : DeleteContactConfigData(false)
    {
    }
};

struct DefaultTransferContactConfigData : ::Epp::Contact::TransferContactConfigData
{
    DefaultTransferContactConfigData()
        : TransferContactConfigData(false)
    {
    }
};

inline boost::optional< ::Epp::Contact::ContactDisclose > set_all_disclose_flags(bool to_disclose)
{
    if (::Epp::is_the_default_policy_to_disclose() == to_disclose)
    {
        return boost::optional< ::Epp::Contact::ContactDisclose >();
    }
    ::Epp::Contact::ContactDisclose disclose(to_disclose ? ::Epp::Contact::ContactDisclose::Flag::hide
                                                         : ::Epp::Contact::ContactDisclose::Flag::disclose);
    disclose.add< ::Epp::Contact::ContactDisclose::Item::name >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::organization >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::address >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::telephone >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::fax >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::email >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::vat >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::ident >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::notify_email >();
    return disclose;
}

inline ::Epp::Contact::ContactDisclose get_all_items(bool to_disclose = true)
{
    ::Epp::Contact::ContactDisclose disclose(to_disclose ? ::Epp::Contact::ContactDisclose::Flag::disclose
                                                       : ::Epp::Contact::ContactDisclose::Flag::hide);
    disclose.add< ::Epp::Contact::ContactDisclose::Item::name >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::organization >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::address >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::telephone >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::fax >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::email >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::vat >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::ident >();
    disclose.add< ::Epp::Contact::ContactDisclose::Item::notify_email >();
    return disclose;
}

struct DefaultCreateContactInputData : ::Epp::Contact::CreateContactInputData
{
    DefaultCreateContactInputData()
        : CreateContactInputData(::Epp::Contact::ContactData()),
          handle(ValidHandle().handle)
    {
        name = "Jan Novák Jr.";
        organization = "";
        streets.clear();
        streets.reserve(3);
        streets.push_back("ulice 1");
        streets.push_back("ulice 2");
        streets.push_back("ulice 3");
        city = "město";
        state_or_province = "hejtmanství";
        postal_code = "12345";
        country_code = "CZ";
        ::Epp::Contact::CreateContactInputData::Address address;
        address.street1 = "Korešpondenčná";
        address.street2 = "ulica";
        address.street3 = "1";
        address.city = "Korešpondenčné Mesto";
        address.state_or_province = "Korešpondenčné hajtmanstvo";
        address.postal_code = "54321";
        address.country_code = "SK";
        mailing_address = address;
        telephone = "+420 123 456 789";
        fax = "+420 987 654 321";
        email = "jan@novak.novak";
        notify_email = "jan.notify@novak.novak";
        vat = "MyVATstring";
        ident = boost::none;
        authinfopw = "authInfo123";
        disclose = set_all_disclose_flags(true);
    }
    const std::string handle;
};

struct DefaultUpdateContactInputData : ::Epp::Contact::ContactChange
{
    DefaultUpdateContactInputData()
        : ContactChange(),
          handle(ValidHandle().handle)
    {
        name = "Jan Novák";
        organization = "Firma, a. s.";
        streets.clear();
        streets.reserve(3);
        streets.push_back(Nullable<std::string>("Václavské náměstí 1"));
        streets.push_back(Nullable<std::string>("53. patro"));
        streets.push_back(Nullable<std::string>("vpravo"));
        city = "Brno";
        state_or_province = "Morava";
        postal_code = "20000";
        country_code = "CZ";
    }

    DefaultUpdateContactInputData& add_additional_data()
    {
        organization = "";
        telephone = "+420 123 456 789";
        fax = "+420 987 654 321";
        email = "jan@novak.novak";
        notify_email = "jan.notify@novak.novak";
        vat = "MyVATstring";
        ident = boost::optional< ::Epp::Contact::ContactIdent >(
                ::Epp::Contact::ContactIdentValueOf< ::Epp::Contact::ContactIdentType::Op >("CZ0123456789"));
        authinfopw = "a6tg85jk57yu97";
        disclose = get_all_items();

        return *this;
    }

    DefaultUpdateContactInputData& drop_mailing_address()
    {
        mailing_address = Nullable< ::Epp::Contact::ContactChange::Address >();
        return *this;
    }

    DefaultUpdateContactInputData& update_mailing_address()
    {
        ::Epp::Contact::ContactChange::Address address;
        address.street1 = "Korespondenční";
        address.street2 = "ulice";
        address.street3 = "2";
        address.city = "Korespondenční Město";
        address.state_or_province = "Korespondenční hajtmanství";
        address.postal_code = "12345";
        address.country_code = "CZ";
        mailing_address = Nullable< ::Epp::Contact::ContactChange::Address >(address);
        return *this;
    }

    const std::string handle;
};

struct Contact
{
    Contact(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _contact_handle = "CONTACT")
    {
        ::LibFred::CreateContact(_contact_handle, _registrar_handle).exec(_ctx);
        data = ::LibFred::InfoContactByHandle(_contact_handle).exec(_ctx).info_contact_data;
    }
    ::LibFred::InfoContactData data;
};

// fixtures

struct HasRegistrarWithSessionAndContact
{
    HasRegistrarWithSessionAndContact(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          contact(_ctx, registrar.data.handle)
    {
    }
    Registrar registrar;
    Session session;
    Contact contact;
};

struct HasRegistrarWithSessionAndContactAndDifferentRegistrar
{
    HasRegistrarWithSessionAndContactAndDifferentRegistrar(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          contact(_ctx, registrar.data.handle),
          different_registrar(_ctx, "REG-TEST2")
    {
    }
    Registrar registrar;
    Session session;
    Contact contact;
    Registrar different_registrar;
};

struct HasRegistrarWithSessionAndContactOfDifferentRegistrar
{
    HasRegistrarWithSessionAndContactOfDifferentRegistrar(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          different_registrar(_ctx, "REG-TEST2"),
          contact_of_different_registrar(_ctx, different_registrar.data.handle)
    {
    }
    Registrar registrar;
    Session session;
    Registrar different_registrar;
    Contact contact_of_different_registrar;
};

struct ContactWithStatusRequest
    : Contact
{
    ContactWithStatusRequest(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _status)
        : Contact(_ctx, _registrar_handle, "contactwith" + boost::algorithm::to_lower_copy(_status)),
          status(_status)
    {
        ObjectWithStatus(_ctx, data.id, _status);
    }
    const std::string status;
};

struct ContactWithStatus
    : ContactWithStatusRequest
{
    ContactWithStatus(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _status)
        : ContactWithStatusRequest(
                  _ctx,
                  _registrar_handle,
                  _status)
    {
        ::LibFred::PerformObjectStateRequest(data.id).exec(_ctx);
    }
};

struct ContactWithServerDeleteProhibited
    : ContactWithStatus
{
    ContactWithServerDeleteProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "serverDeleteProhibited")
    {
    }
};

struct ContactWithStatusServerUpdateProhibited
    : ContactWithStatus
{
    ContactWithStatusServerUpdateProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "serverUpdateProhibited")
    {
    }
};

struct ContactWithStatusServerTransferProhibited
    : ContactWithStatus
{
    ContactWithStatusServerTransferProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "serverTransferProhibited")
    {
    }
};

struct ContactWithStatusRequestServerTransferProhibited
    : ContactWithStatusRequest
{
    ContactWithStatusRequestServerTransferProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatusRequest(_ctx, _registrar_handle, "serverTransferProhibited")
    {
    }
};

struct ContactWithStatusRequestServerUpdateProhibited
    : ContactWithStatusRequest
{
    ContactWithStatusRequestServerUpdateProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatusRequest(_ctx, _registrar_handle, "serverUpdateProhibited")
    {
    }
};

struct ContactWithStatusDeleteCandidate
    : ContactWithStatus
{
    ContactWithStatusDeleteCandidate(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "deleteCandidate")
    {
    }
};

struct ContactWithStatusRequestDeleteCandidate
    : ContactWithStatusRequest
{
    ContactWithStatusRequestDeleteCandidate(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatusRequest(_ctx, _registrar_handle, "deleteCandidate")
    {
    }
};

} // namespace Test::Backend::Epp::Contact
} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test

#endif
