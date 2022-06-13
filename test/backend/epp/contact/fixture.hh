/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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

#ifndef FIXTURE_HH_D413188368CF45BEA79CE297BB880BDD
#define FIXTURE_HH_D413188368CF45BEA79CE297BB880BDD

#include "test/setup/fixtures.hh"
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/check_contact_config_data.hh"
#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/contact_change.hh"
#include "src/backend/epp/contact/create_contact_config_data.hh"
#include "src/backend/epp/contact/create_contact_input_data.hh"
#include "src/backend/epp/contact/delete_contact_config_data.hh"
#include "src/backend/epp/contact/info_contact_config_data.hh"
#include "src/backend/epp/contact/transfer_contact_config_data.hh"
#include "src/backend/epp/contact/update_contact_config_data.hh"
#include "src/backend/epp/contact/impl/get_create_contact_data_filter.hh"
#include "src/backend/epp/contact/impl/get_contact_data_share_policy_rules.hh"
#include "src/backend/epp/contact/impl/get_update_contact_data_filter.hh"
#include "src/backend/epp/contact/impl/info_contact.hh"
#include "src/backend/epp/contact/impl/cznic/specific.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "util/optional_value.hh"
#include "util/db/nullable.hh"

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
        : InfoContactConfigData{
              false,
              ::Epp::Contact::Impl::get_contact_data_share_policy_rules(
                      ::Epp::Contact::ConfigDataFilter::get_default<::Epp::Contact::Impl::InfoContact>())}
    { }
};

struct DefaultCreateContactConfigData : ::Epp::Contact::CreateContactConfigData
{
    DefaultCreateContactConfigData()
        : CreateContactConfigData(
                false,
                ::Epp::Contact::Impl::get_create_contact_data_filter(
                        ::Epp::Contact::ConfigDataFilter::get_default<::Epp::Contact::Impl::CzNic::Specific>()))
    {
    }
};

struct DefaultUpdateContactConfigData : ::Epp::Contact::UpdateContactConfigData
{
    DefaultUpdateContactConfigData()
        : UpdateContactConfigData(
                false,
                false,
                ::Epp::Contact::Impl::get_update_contact_data_filter(
                        ::Epp::Contact::ConfigDataFilter::get_default<::Epp::Contact::Impl::CzNic::Specific>()))
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

struct DefaultCreateContactInputData : ::Epp::Contact::CreateContactInputData
{
    DefaultCreateContactInputData()
        : CreateContactInputData(::Epp::Contact::ContactData()),
          handle(ValidHandle().handle)
    {
        this->name = ::Epp::Contact::make_public_data(std::string("Jan Novák Jr."));
        this->organization = ::Epp::Contact::make_public_data(std::string(""));
        ::Epp::Contact::CreateContactInputData::Address address;
        address.street[0] = "ulice 1";
        address.street[1] = "ulice 2";
        address.street[2] = "ulice 3";
        address.city = "město";
        address.state_or_province = "hejtmanství";
        address.postal_code = "12345";
        address.country_code = "CZ";
        this->address = ::Epp::Contact::make_public_data(address);
        address.street[0] = "Korešpondenčná";
        address.street[1] = "ulica";
        address.street[2] = "1";
        address.city = "Korešpondenčné Mesto";
        address.state_or_province = "Korešpondenčné hajtmanstvo";
        address.postal_code = "54321";
        address.country_code = "SK";
        this->mailing_address = address;
        this->telephone = ::Epp::Contact::make_public_data(std::string("+420 123 456 789"));
        this->fax = ::Epp::Contact::make_public_data(std::string("+420 987 654 321"));
        this->email = ::Epp::Contact::make_public_data(std::string("jan@novak.novak"));
        this->notify_email = ::Epp::Contact::make_public_data(std::string("jan.notify@novak.novak"));
        this->vat = ::Epp::Contact::make_public_data(std::string("MyVATstring"));
        this->ident = ::Epp::Contact::make_public_data(boost::optional<::Epp::Contact::ContactIdent>());
    }
    const std::string handle;
};

template <typename T, typename S>
auto make_deletable(const S& src)
{
    return ::Epp::UpdateOperation::set_value(T(src));
}

template <typename T, typename S>
auto make_updateable(const S& src)
{
    return ::Epp::UpdateOperation::set_value(T(src));
}

inline auto make_deletable_unchanged()
{
    return ::Epp::UpdateOperation::no_operation();
}

inline auto make_updateable_unchanged()
{
    return ::Epp::UpdateOperation::no_operation();
}

inline auto make_all_public()
{
    ::Epp::Contact::ContactChange::Publishability disclose;
    disclose.name = ::Epp::Contact::PrivacyPolicy::show;
    disclose.organization = ::Epp::Contact::PrivacyPolicy::show;
    disclose.address = ::Epp::Contact::PrivacyPolicy::show;
    disclose.telephone = ::Epp::Contact::PrivacyPolicy::show;
    disclose.fax = ::Epp::Contact::PrivacyPolicy::show;
    disclose.email = ::Epp::Contact::PrivacyPolicy::show;
    disclose.notify_email = ::Epp::Contact::PrivacyPolicy::show;
    disclose.vat = ::Epp::Contact::PrivacyPolicy::show;
    disclose.ident = ::Epp::Contact::PrivacyPolicy::show;
    return disclose;
}

struct DefaultUpdateContactInputData : ::Epp::Contact::ContactChange
{
    DefaultUpdateContactInputData()
        : ContactChange(),
          handle(ValidHandle().handle)
    {
        this->name = make_deletable<std::string>("Jan Novák");
        this->organization = make_deletable<std::string>("Firma, a. s.");
        this->address.street[0] = make_deletable<std::string>("Václavské náměstí 1");
        this->address.street[1] = make_deletable<std::string>("53. patro");
        this->address.street[2] = make_deletable<std::string>("vpravo");
        this->address.city = make_deletable<std::string>("Brno");
        this->address.state_or_province = make_deletable<std::string>("Morava");
        this->address.postal_code = make_deletable<std::string>("20000");
        this->address.country_code = make_updateable<std::string>("CZ");
        this->mailing_address = make_deletable_unchanged();
        this->telephone = make_deletable_unchanged();
        this->fax = make_deletable_unchanged();
        this->email = make_deletable_unchanged();
        this->notify_email = make_deletable_unchanged();
        this->vat = make_deletable_unchanged();
        this->ident = make_deletable_unchanged();
        this->authinfopw = make_deletable_unchanged();
        this->disclose = make_updateable_unchanged();
    }
    DefaultUpdateContactInputData& add_additional_data()
    {
        this->organization = make_deletable<std::string>("");
        this->telephone = make_deletable<std::string>("+420 123 456 789");
        this->fax = make_deletable<std::string>("+420.987654321");
        this->email = make_deletable<std::string>("jan@novak.novak");
        this->notify_email = make_deletable<std::string>("jan.notify@novak.novak");
        this->vat = make_deletable<std::string>("MyVATstring");
        this->ident = make_deletable<::Epp::Contact::ContactIdent>(
                ::Epp::Contact::ContactIdentValueOf<::Epp::Contact::ContactIdentType::Op>("CZ0123456789"));
        this->authinfopw = make_deletable<std::string>("a6tg85jk57yu97");
        this->disclose = make_updateable<::Epp::Contact::ContactChange::Publishability>(make_all_public());
        return *this;
    }
    DefaultUpdateContactInputData& change_address()
    {
        this->address.street[0] = make_deletable<std::string>("Václavské náměstí 1");
        this->address.street[1] = make_deletable<std::string>("53. patro");
        this->address.street[2] = make_deletable<std::string>("vpravo");
        this->address.city = make_deletable<std::string>("Brno");
        this->address.state_or_province = make_deletable<std::string>("Morava");
        this->address.postal_code = make_deletable<std::string>("20000");
        this->address.country_code = make_updateable<std::string>("CZ");
        return *this;
    }
    DefaultUpdateContactInputData& drop_mailing_address()
    {
        this->mailing_address = ::Epp::UpdateOperation::delete_value();
        return *this;
    }
    DefaultUpdateContactInputData& update_mailing_address()
    {
        ::Epp::Contact::ContactChange::Address address;
        address.street[0] = "Korespondenční";
        address.street[1] = "ulice";
        address.street[2] = "2";
        address.city = "Korespondenční Město";
        address.state_or_province = "Korespondenční hajtmanství";
        address.postal_code = "12345";
        address.country_code = "CZ";
        this->mailing_address = ::Epp::UpdateOperation::set_value(address);
        return *this;
    }
    const std::string handle;
};

struct Contact
{
    Contact(::LibFred::OperationContext& _ctx,
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

struct ContactWithStatusServerDeleteProhibited
    : ContactWithStatus
{
    ContactWithStatusServerDeleteProhibited(
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
