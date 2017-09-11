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

#include "tests/setup/fixtures.h"
#include "tests/interfaces/epp/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/contact/check_contact_config_data.h"
#include "src/epp/contact/contact_disclose.h"
#include "src/epp/contact/contact_data.h"
#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/create_contact_config_data.h"
#include "src/epp/contact/create_contact_input_data.h"
#include "src/epp/contact/delete_contact_config_data.h"
#include "src/epp/contact/info_contact_config_data.h"
#include "src/epp/contact/transfer_contact_config_data.h"
#include "src/epp/contact/update_contact_config_data.h"
#include "src/epp/impl/disclose_policy.h"
#include "src/epp/session_data.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

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
    std::string handle;

    DefaultCreateContactInputData()
        : CreateContactInputData(::Epp::Contact::ContactData()),
          handle(ValidHandle().handle)
    {
        name = "Jan Novak Jr.";
        organization = "";
        streets.clear();
        streets.reserve(3);
        streets.push_back("ulice 1");
        streets.push_back("ulice 2");
        streets.push_back("ulice 3");
        city = "mesto";
        state_or_province = "hejtmanstvi";
        postal_code = "12345";
        country_code = "CZ";
        telephone = "+420 123 456 789";
        fax = "+420 987 654 321";
        email = "jan@novak.novak";
        notify_email = "jan.notify@novak.novak";
        vat = "MyVATstring";
        ident = boost::none;
        authinfopw = "authInfo123";
        disclose = set_all_disclose_flags(true);
    }
};

struct DefaultUpdateContactInputData : ::Epp::Contact::ContactChange
{
    DefaultUpdateContactInputData()
        : ContactChange(),
          handle(ValidHandle().handle)
    {
        name = "Jan Novak";
        organization = "Firma, a. s.";
        streets.clear();
        streets.reserve(3);
        streets.push_back(Nullable<std::string>("Vaclavske namesti 1"));
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

    std::string handle;
};

struct Contact
{
    Contact(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _contact_handle = "CONTACT")
    {
        Fred::CreateContact(_contact_handle, _registrar_handle).exec(_ctx);
        data = Fred::InfoContactByHandle(_contact_handle).exec(_ctx).info_contact_data;
    }
    Fred::InfoContactData data;
};


// fixtures

struct HasRegistrarWithSessionAndContact
{
    Registrar registrar;
    Session session;
    Contact contact;


    HasRegistrarWithSessionAndContact(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          contact(_ctx, registrar.data.handle)
    {
    }


};

struct HasRegistrarWithSessionAndContactAndDifferentRegistrar
{
    Registrar registrar;
    Session session;
    Contact contact;
    Registrar different_registrar;


    HasRegistrarWithSessionAndContactAndDifferentRegistrar(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          contact(_ctx, registrar.data.handle),
          different_registrar(_ctx, "REG-TEST2")
    {
    }


};

struct HasRegistrarWithSessionAndContactOfDifferentRegistrar
{
    Registrar registrar;
    Session session;
    Registrar different_registrar;
    Contact contact_of_different_registrar;


    HasRegistrarWithSessionAndContactOfDifferentRegistrar(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          different_registrar(_ctx, "REG-TEST2"),
          contact_of_different_registrar(_ctx, different_registrar.data.handle)
    {
    }


};

struct ContactWithStatusRequest
    : Contact
{
    const std::string status;


    ContactWithStatusRequest(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _status)
        : Contact(_ctx, _registrar_handle, "contactwith" + boost::algorithm::to_lower_copy(_status)),
          status(_status)
    {
        ObjectWithStatus(_ctx, data.id, _status);
    }


};

struct ContactWithStatus
    : ContactWithStatusRequest
{


    ContactWithStatus(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _status)
        : ContactWithStatusRequest(
                  _ctx,
                  _registrar_handle,
                  _status)
    {
        Fred::PerformObjectStateRequest(data.id).exec(_ctx);
    }


};

struct ContactWithServerDeleteProhibited
    : ContactWithStatus
{


    ContactWithServerDeleteProhibited(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "serverDeleteProhibited")
    {
    }


};

struct ContactWithStatusServerUpdateProhibited
    : ContactWithStatus
{


    ContactWithStatusServerUpdateProhibited(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "serverUpdateProhibited")
    {
    }


};

struct ContactWithStatusServerTransferProhibited
    : ContactWithStatus
{


    ContactWithStatusServerTransferProhibited(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "serverTransferProhibited")
    {
    }


};

struct ContactWithStatusRequestServerTransferProhibited
    : ContactWithStatusRequest
{


    ContactWithStatusRequestServerTransferProhibited(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatusRequest(_ctx, _registrar_handle, "serverTransferProhibited")
    {
    }


};

struct ContactWithStatusRequestServerUpdateProhibited
    : ContactWithStatusRequest
{


    ContactWithStatusRequestServerUpdateProhibited(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatusRequest(_ctx, _registrar_handle, "serverUpdateProhibited")
    {
    }


};


struct ContactWithStatusDeleteCandidate
    : ContactWithStatus
{


    ContactWithStatusDeleteCandidate(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "deleteCandidate")
    {
    }


};


struct ContactWithStatusRequestDeleteCandidate
    : ContactWithStatusRequest
{


    ContactWithStatusRequestDeleteCandidate(
            Fred::OperationContext& _ctx,
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
