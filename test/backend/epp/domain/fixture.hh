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

#ifndef FIXTURE_HH_FD001D96A2114C02B77C4A3E5C347AE8
#define FIXTURE_HH_FD001D96A2114C02B77C4A3E5C347AE8

#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/contact/fixture.hh"
#include "test/backend/epp/nsset/fixture.hh"
#include "test/backend/epp/keyset/fixture.hh"
#include "test/backend/epp/util.hh"
#include "test/setup/fixtures.hh"

#include "src/backend/epp/domain/check_domain_config_data.hh"
#include "src/backend/epp/domain/create_domain_config_data.hh"
#include "src/backend/epp/domain/create_domain_input_data.hh"
#include "src/backend/epp/domain/create_domain_localized.hh"
#include "src/backend/epp/domain/delete_domain_config_data.hh"
#include "src/backend/epp/domain/domain_enum_validation.hh"
#include "src/backend/epp/domain/domain_registration_time.hh"
#include "src/backend/epp/domain/info_domain_config_data.hh"
#include "src/backend/epp/domain/renew_domain_config_data.hh"
#include "src/backend/epp/domain/renew_domain_input_data.hh"
#include "src/backend/epp/domain/renew_domain_localized.hh"
#include "src/backend/epp/domain/transfer_domain_config_data.hh"
#include "src/backend/epp/domain/update_domain_config_data.hh"
#include "src/backend/epp/domain/update_domain_input_data.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/nsset_dns_host.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"
#include "util/util.hh"

#include <boost/asio/ip/address.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/optional.hpp>

#include <set>
#include <string>
#include <vector>

namespace Test {
namespace Backend {
namespace Epp {
namespace Domain {

std::vector<std::string> vector_of_Fred_RegistrableObject_Contact_ContactReference_to_vector_of_string(const std::vector<::LibFred::RegistrableObject::Contact::ContactReference>& admin_contacts);

struct DefaultCheckDomainConfigData : ::Epp::Domain::CheckDomainConfigData
{
    DefaultCheckDomainConfigData()
        : CheckDomainConfigData(false)
    {
    }
};

struct DefaultInfoDomainConfigData : ::Epp::Domain::InfoDomainConfigData
{
    DefaultInfoDomainConfigData()
        : InfoDomainConfigData(false)
    {
    }
};

struct DefaultCreateDomainConfigData : ::Epp::Domain::CreateDomainConfigData
{
    DefaultCreateDomainConfigData()
        : CreateDomainConfigData(false)
    {
    }
};

struct DefaultUpdateDomainConfigData : ::Epp::Domain::UpdateDomainConfigData
{
    DefaultUpdateDomainConfigData()
        : UpdateDomainConfigData(
                  false, // rifd_epp_operations_charging,
                  true)  // rifd_epp_update_domain_keyset_clear
    {
    }
};

struct DefaultDeleteDomainConfigData : ::Epp::Domain::DeleteDomainConfigData
{
    DefaultDeleteDomainConfigData()
        : DeleteDomainConfigData(false)
    {
    }
};

struct DefaultTransferDomainConfigData : ::Epp::Domain::TransferDomainConfigData
{
    DefaultTransferDomainConfigData()
        : TransferDomainConfigData(false)
    {
    }
};

struct DefaultRenewDomainConfigData : ::Epp::Domain::RenewDomainConfigData
{
    DefaultRenewDomainConfigData()
        : RenewDomainConfigData(false)
    {
    }
};

struct DefaultCreateDomainInputData : ::Epp::Domain::CreateDomainInputData
{
    DefaultCreateDomainInputData()
        : CreateDomainInputData(
                  "", // _fqdn
                  "", // _registrant
                  "", // _nsset
                  "", // _keyset
                  ::Epp::Domain::DomainRegistrationTime(1, ::Epp::Domain::DomainRegistrationTime::Unit::year), // _period
                  std::vector<std::string>(), // Util::vector_of<std::string>("CONTACT1")("CONTACT2"),
                  boost::optional< ::Epp::Domain::EnumValidationExtension>())
    {
    }
};

struct CreateDomainInputData
{
    ::Epp::Domain::CreateDomainInputData data;

    CreateDomainInputData(
            const std::string& _fqdn,
            const std::string& _registrant,
            const std::string& _nsset,
            const std::string& _keyset,
            const std::vector<std::string>& _admin_contacts_add)
        : data(
                  _fqdn,
                  _registrant,
                  _nsset,
                  _keyset,
                  ::Epp::Domain::DomainRegistrationTime(1, ::Epp::Domain::DomainRegistrationTime::Unit::year), // _period
                  _admin_contacts_add,
                  boost::optional< ::Epp::Domain::EnumValidationExtension>())
    {
    }
};

struct DefaultRenewDomainInputData : ::Epp::Domain::RenewDomainInputData
{
    DefaultRenewDomainInputData()
        : RenewDomainInputData(
            "", // _fqdn
            "", // _current_exdate
            ::Epp::Domain::DomainRegistrationTime(1, ::Epp::Domain::DomainRegistrationTime::Unit::year), // _period
            boost::optional< ::Epp::Domain::EnumValidationExtension>())
    {
    }
};

struct RenewDomainInputData
{
    ::Epp::Domain::RenewDomainInputData data;

    RenewDomainInputData(
            const std::string& _fqdn)
        : data(
                  _fqdn, // _fqdn
                  "", // _current_exdate
                  ::Epp::Domain::DomainRegistrationTime(1, ::Epp::Domain::DomainRegistrationTime::Unit::year), // _period
                  boost::optional< ::Epp::Domain::EnumValidationExtension>())
    {
    }

    RenewDomainInputData(
            const std::string& _fqdn,
            const std::string& _current_exdate,
            const ::Epp::Domain::DomainRegistrationTime& _period,
            const boost::optional< ::Epp::Domain::EnumValidationExtension>& _enum_validation_extension)
        : data(
                  _fqdn,
                  _current_exdate,
                  _period,
                  _enum_validation_extension)
    {
    }
};

struct DefaultUpdateDomainInputData : ::Epp::Domain::UpdateDomainInputData
{
    DefaultUpdateDomainInputData()
        : UpdateDomainInputData(
                  "domain.cz",
                  Optional<std::string>(), // registrant_chg
                  Optional<std::string>(), // authinfopw_chg
                  Optional<Nullable<std::string> >(), // nsset_chg
                  Optional<Nullable<std::string> >(), // keyset_chg
                  std::vector<std::string>(), // admin_contacts_add
                  std::vector<std::string>(), // admin_contacts_rem
                  std::vector<std::string>(), // tmpcontacts_rem
                  boost::optional< ::Epp::Domain::EnumValidationExtension>()) // enum_validation
    {
    }
};

struct UpdateDomainInputData
{
    ::Epp::Domain::UpdateDomainInputData data;

    UpdateDomainInputData(
            const std::string& _fqdn)
        : data(
                _fqdn,
                Optional<std::string>("a"), // registrant_chg
                Optional<std::string>("b"), // authinfopw_chg
                Optional<Nullable<std::string> >("c"), // nsset_chg
                Optional<Nullable<std::string> >("d"), // keyset_chg
                std::vector<std::string>(), // admin_contacts_add
                std::vector<std::string>(), // admin_contacts_rem
                std::vector<std::string>(), // tmpcontacts_rem
                boost::optional< ::Epp::Domain::EnumValidationExtension>()) // enum_validation
    {
    }
};

struct Domain
{
    Contact::Contact registrant;
    ::LibFred::InfoDomainData data;

    Domain(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _fqdn = "freddy.cz",
            const std::string& _registrant_handle = "REGISTRANT")
        : registrant(_ctx, _registrar_handle, _registrant_handle)
    {
        ::LibFred::CreateDomain(_fqdn, _registrar_handle, registrant.data.handle).exec(_ctx);
        data = ::LibFred::InfoDomainByFqdn(_fqdn).exec(_ctx, "UTC").info_domain_data;
    }
};

struct EnumDomain
{
    Contact::Contact registrant;
    ::LibFred::InfoDomainData data;

    EnumDomain(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _enum_fqdn = "5.5.1.3.5.0.2.4.e164.arpa",
            const std::string& _registrant_handle = "REGISTRANT")
        : registrant(_ctx, _registrar_handle, _registrant_handle)
    {
        const std::string tmp_fqdn = "tmpdomain.cz";

        ::LibFred::CreateDomain(tmp_fqdn, _registrar_handle, registrant.data.handle).exec(_ctx);
        data = ::LibFred::InfoDomainByFqdn(tmp_fqdn).exec(_ctx, "UTC").info_domain_data;

        ::LibFred::CreateDomain(_enum_fqdn, _registrar_handle, registrant.data.handle)
        .set_enum_validation_expiration(data.creation_time.date() + boost::gregorian::months(3))
        // .set_nsset(nsset_handle)
        // .set_keyset(keyset_handle)
        // .set_admin_contacts(admin_contacts)
        .exec(_ctx);

        data = ::LibFred::InfoDomainByFqdn(_enum_fqdn).exec(_ctx, "UTC").info_domain_data;
    }
};

struct NonexistentEnumDomain
    : EnumDomain
{
    NonexistentEnumDomain(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _registrant_handle = "REGISTRANT")
        : EnumDomain(
                  _ctx,
                  _registrar_handle,
                  "5.5.1.3.5.0.2.4.e164.arpa",
                  _registrant_handle)
    {
        data.fqdn = NonexistentEnumFqdn().fqdn;
    }
};

struct BlacklistedDomain
    : Domain
{
    BlacklistedDomain(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _registrant_handle = "REGISTRANT")
        : Domain(_ctx, _registrar_handle, "blacklisteddomain.cz", _registrant_handle)
    {
        _ctx.get_conn().exec_params(
                "INSERT INTO domain_blacklist (regexp, valid_from, reason) "
                "VALUES ($1::text, NOW(), '')",
                Database::query_param_list(data.fqdn));
    }
};

struct DomainWithStatusRequest
    : Domain
{
    const std::string status;

    DomainWithStatusRequest(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _status)
        : Domain(_ctx, _registrar_handle, "domainwith" + boost::algorithm::to_lower_copy(_status) + ".cz"),
          status(_status)
    {
        ObjectWithStatus(_ctx, data.id, _status);
    }
};

struct DomainWithStatus
    : DomainWithStatusRequest
{
    DomainWithStatus(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _status)
        : DomainWithStatusRequest(
                  _ctx,
                  _registrar_handle,
                  _status)
    {
        ::LibFred::PerformObjectStateRequest(data.id).exec(_ctx);
    }
};

struct DomainWithStatusServerDeleteProhibited
    : DomainWithStatus
{
    DomainWithStatusServerDeleteProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatus(_ctx, _registrar_handle, "serverDeleteProhibited")
    {
    }
};

struct DomainWithStatusServerUpdateProhibited
    : DomainWithStatus
{
    DomainWithStatusServerUpdateProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatus(_ctx, _registrar_handle, "serverUpdateProhibited")
    {
    }
};

struct DomainWithServerTransferProhibited
    : DomainWithStatus
{
    DomainWithServerTransferProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatus(_ctx, _registrar_handle, "serverTransferProhibited")
    {
    }
};

struct DomainWithStatusRequestServerTransferProhibited
    : DomainWithStatusRequest
{
    DomainWithStatusRequestServerTransferProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatusRequest(_ctx, _registrar_handle, "serverTransferProhibited")
    {
    }
};

struct DomainWithStatusRequestServerUpdateProhibited
    : DomainWithStatusRequest
{
    DomainWithStatusRequestServerUpdateProhibited(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatusRequest(_ctx, _registrar_handle, "serverUpdateProhibited")
    {
    }
};

struct FullDomain
{
    Contact::Contact registrant;
    ::LibFred::InfoDomainData data;

    FullDomain(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle
            )
        : registrant(_ctx, _registrar_handle, "REGISTRANT")
    {
        ::LibFred::CreateContact("CONTACT1", _registrar_handle).exec(_ctx);

        ::LibFred::CreateNsset("NSSET1", _registrar_handle).exec(_ctx);

        ::LibFred::CreateKeyset("KEYSET1", _registrar_handle).exec(_ctx);

        ::LibFred::CreateDomain("fulldomain.cz", _registrar_handle, "REGISTRANT")
                .set_admin_contacts(Util::vector_of<std::string>("CONTACT1"))
                .set_nsset(std::string("NSSET1"))
                .set_keyset(std::string("KEYSET1"))
                .exec(_ctx);

        data = ::LibFred::InfoDomainByFqdn("fulldomain.cz").exec(_ctx, "UTC").info_domain_data;
    }
};

// fixtures

struct HasSystemRegistrarWithSessionAndDomain
{
    SystemRegistrar system_registrar;
    Session session;
    Domain domain;

    HasSystemRegistrarWithSessionAndDomain(::LibFred::OperationContext& _ctx)
        : system_registrar(_ctx),
          session(_ctx, system_registrar.data.id),
          domain(_ctx, system_registrar.data.handle)
    {
    }
};

struct HasRegistrarWithSessionAndDomain
{
    Registrar registrar;
    Session session;
    Domain domain;

    HasRegistrarWithSessionAndDomain(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          domain(_ctx, registrar.data.handle)
    {
    }
};

struct HasRegistrarWithSessionAndBlacklistedDomain
{
    Registrar registrar;
    Session session;
    BlacklistedDomain blacklisted_domain;

    HasRegistrarWithSessionAndBlacklistedDomain(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          blacklisted_domain(_ctx, registrar.data.handle)
    {
    }
};

struct HasSystemRegistrarWithSessionAndBlacklistedDomain
{
    SystemRegistrar system_registrar;
    Session session;
    BlacklistedDomain blacklisted_domain;

    HasSystemRegistrarWithSessionAndBlacklistedDomain(::LibFred::OperationContext& _ctx)
        : system_registrar(_ctx),
          session(_ctx, system_registrar.data.id),
          blacklisted_domain(_ctx, system_registrar.data.handle)
    {
    }
};

struct HasRegistrarNotInZoneWithSessionAndDomain
{
    SystemRegistrar registrar_not_in_zone;
    Session session;
    Domain domain;

    HasRegistrarNotInZoneWithSessionAndDomain(::LibFred::OperationContext& _ctx)
        : registrar_not_in_zone(_ctx),
          session(_ctx, registrar_not_in_zone.data.id),
          domain(_ctx, registrar_not_in_zone.data.handle)
    {
    }
};

struct HasRegistrarWithSessionAndDomainAndDifferentRegistrar
{
    Registrar registrar;
    Session session;
    Domain domain;
    Registrar different_registrar;

    HasRegistrarWithSessionAndDomainAndDifferentRegistrar(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          domain(_ctx, registrar.data.handle),
          different_registrar(_ctx, "REG-TEST2")
    {
    }
};

struct HasRegistrarWithSessionAndDomainOfDifferentRegistrar
{
    Registrar registrar;
    Session session;
    Registrar different_registrar;
    Domain domain_of_different_registrar;

    HasRegistrarWithSessionAndDomainOfDifferentRegistrar(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          different_registrar(_ctx, "REG-TEST2"),
          domain_of_different_registrar(_ctx, different_registrar.data.handle)
    {
    }
};

struct HasDomainWithdServerTransferProhibitedAndDifferentRegistrar
{
    Registrar different_registrar;
    DomainWithServerTransferProhibited domain_with_server_transfer_prohibited;

    HasDomainWithdServerTransferProhibitedAndDifferentRegistrar(::LibFred::OperationContext& _ctx)
        : different_registrar(_ctx, "REG-TEST2"),
          domain_with_server_transfer_prohibited(_ctx, different_registrar.data.handle + "DIFFERENT")
    {
        BOOST_REQUIRE(
                domain_with_server_transfer_prohibited.data.sponsoring_registrar_handle !=
                different_registrar.data.handle);
    }
};

struct HasRegistrarWithSessionAndCreateDomainInputData
{
    Registrar registrar;
    Session session;
    Contact::Contact registrant;
    Nsset::Nsset nsset;
    Keyset::Keyset keyset;
    Contact::Contact contact1;
    Contact::Contact contact2;
    CreateDomainInputData create_domain_input_data;

    HasRegistrarWithSessionAndCreateDomainInputData(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          registrant(_ctx, registrar.data.handle),
          nsset(_ctx, registrar.data.handle),
          keyset(_ctx, registrar.data.handle),
          contact1(_ctx, registrar.data.handle, "CONTACT1"),
          contact2(_ctx, registrar.data.handle, "CONTACT2"),
          create_domain_input_data("newdomain.cz",
                  registrant.data.handle,
                  nsset.data.handle,
                  keyset.handle,
                  Util::vector_of<std::string>(contact1.data.handle)(contact2.data.handle))
    {
    }
};

struct HasRegistrarWithSessionAndRenewDomainInputData
{
    Registrar registrar;
    Session session;
    Contact::Contact registrant;
    RenewDomainInputData renew_domain_input_data;

    HasRegistrarWithSessionAndRenewDomainInputData(::LibFred::OperationContext& _ctx)
            : registrar(_ctx),
              session(_ctx, registrar.data.id),
              registrant(_ctx, registrar.data.handle),
              renew_domain_input_data("domainforrenewal.cz")
    {
    }
};

struct HasRegistrarWithSessionAndDomainAndRenewDomainInputData
{
    Registrar registrar;
    Session session;
    Contact::Contact registrant;
    Domain domain;
    RenewDomainInputData renew_domain_input_data;

    HasRegistrarWithSessionAndDomainAndRenewDomainInputData(::LibFred::OperationContext & _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          registrant(_ctx, registrar.data.handle),
          domain(_ctx, registrar.data.handle),
          renew_domain_input_data(
              domain.data.fqdn,
              boost::gregorian::to_iso_extended_string(domain.data.expiration_date),
              ::Epp::Domain::DomainRegistrationTime(1, ::Epp::Domain::DomainRegistrationTime::Unit::year),
              boost::optional< ::Epp::Domain::EnumValidationExtension>())
    {
    }
};

struct HasRegistrarWithSessionAndCreateDomainInputDataAndRenewDomainInputData
{
    Registrar registrar;
    Session session;
    Contact::Contact registrant;
    Nsset::Nsset nsset;
    Keyset::Keyset keyset;
    Contact::Contact contact1;
    Contact::Contact contact2;
    CreateDomainInputData create_domain_input_data;
    RenewDomainInputData renew_domain_input_data;

    HasRegistrarWithSessionAndCreateDomainInputDataAndRenewDomainInputData(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          registrant(_ctx, registrar.data.handle),
          nsset(_ctx, registrar.data.handle),
          keyset(_ctx, registrar.data.handle),
          contact1(_ctx, registrar.data.handle, "CONTACT1"),
          contact2(_ctx, registrar.data.handle, "CONTACT2"),
          create_domain_input_data(
              "nonexistentdomain.cz",
              registrant.data.handle,
              nsset.data.handle,
              keyset.handle,
              Util::vector_of<std::string>(contact1.data.handle)(contact2.data.handle)),
          renew_domain_input_data(
              create_domain_input_data.data.fqdn,
              "",
              ::Epp::Domain::DomainRegistrationTime(1, ::Epp::Domain::DomainRegistrationTime::Unit::year),
              boost::optional< ::Epp::Domain::EnumValidationExtension>())
    {
    }
};

struct HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData
{
    Registrar registrar;
    Session session;
    Contact::Contact registrant;
    Nsset::Nsset nsset;
    Keyset::Keyset keyset;
    Contact::Contact contact1;
    Contact::Contact contact2;
    Domain domain;
    CreateDomainInputData create_domain_input_data;
    RenewDomainInputData renew_domain_input_data;

    HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          registrant(_ctx, registrar.data.handle),
          nsset(_ctx, registrar.data.handle),
          keyset(_ctx, registrar.data.handle),
          contact1(_ctx, registrar.data.handle, "CONTACT1"),
          contact2(_ctx, registrar.data.handle, "CONTACT2"),
          domain(_ctx, registrar.data.handle),
          create_domain_input_data(
                  domain.data.fqdn,
                  registrant.data.handle,
                  nsset.data.handle,
                  keyset.handle,
                  Util::vector_of<std::string>(contact1.data.handle)(contact2.data.handle)),
          renew_domain_input_data(
                  domain.data.fqdn,
                  boost::gregorian::to_iso_extended_string(domain.data.expiration_date),
                  ::Epp::Domain::DomainRegistrationTime(1, ::Epp::Domain::DomainRegistrationTime::Unit::year),
                  boost::optional< ::Epp::Domain::EnumValidationExtension>())
    {
    }
};

struct HasDataForUpdateDomain
{
    Registrar registrar;
    Session session;
    FullDomain domain;

    const std::string new_registrant_handle_;
    const std::string new_authinfopw_;
    const std::string new_nsset_handle_;
    const std::string new_keyset_handle_;
    Optional<std::string> registrant_chg_;
    Optional<std::string> authinfopw_chg_;
    Optional<Nullable<std::string> > nsset_chg_;
    Optional<Nullable<std::string> > keyset_chg_;
    std::vector<std::string> admin_contacts_add_;
    std::vector<std::string> admin_contacts_rem_;
    std::vector<std::string> tmpcontacts_rem_;
    boost::optional< ::Epp::Domain::EnumValidationExtension> enum_validation_;

    HasDataForUpdateDomain(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          domain(_ctx, registrar.data.handle),
          new_registrant_handle_("REGISTRANT2"),
          new_authinfopw_(" auth info "),
          new_nsset_handle_("NSSET2"),
          new_keyset_handle_("KEYSET2")
    {
        namespace ip = boost::asio::ip;

        ::LibFred::CreateContact(new_registrant_handle_, registrar.data.handle).exec(_ctx);

        const std::string admin_handle2 = "CONTACT2";
        ::LibFred::CreateContact(admin_handle2, registrar.data.handle).exec(_ctx);
        const std::string admin_handle3 = "CONTACT3";
        ::LibFred::CreateContact(admin_handle3, registrar.data.handle).exec(_ctx);

        const std::string tech_handle = "TECH2";
        ::LibFred::CreateContact(tech_handle, registrar.data.handle).exec(_ctx);

        ::LibFred::CreateNsset(new_nsset_handle_, registrar.data.handle)
                .set_tech_contacts(Util::vector_of<std::string>(tech_handle))
                .set_dns_hosts(
                        Util::vector_of<::LibFred::DnsHost>
                        (::LibFred::DnsHost("a.ns.nic.cz",
                                Util::vector_of<ip::address>
                                        (ip::address::from_string("11.0.0.3"))
                                        (ip::address::from_string("11.1.1.3"))))
                        (::LibFred::DnsHost("c.ns.nic.cz",
                                Util::vector_of<ip::address>
                                     (ip::address::from_string("11.0.0.4"))
                                     (ip::address::from_string("11.1.1.4")))))
                .exec(_ctx);

        ::LibFred::CreateKeyset(new_keyset_handle_, registrar.data.handle)
                .set_tech_contacts(boost::assign::list_of(tech_handle))
                .exec(_ctx);


        registrant_chg_ = Optional<std::string>(new_registrant_handle_);
        authinfopw_chg_ = Optional<std::string>(new_authinfopw_);

        nsset_chg_ = Optional<Nullable<std::string> >(new_nsset_handle_);
        keyset_chg_ = Optional<Nullable<std::string> >(new_keyset_handle_);

        admin_contacts_add_ = Util::vector_of<std::string>
                                  (admin_handle2)
                                  (admin_handle3);

        admin_contacts_rem_ = vector_of_Fred_RegistrableObject_Contact_ContactReference_to_vector_of_string(
                domain.data.admin_contacts);

        tmpcontacts_rem_ = Util::vector_of<std::string>
                               ("WHATEVER");
    }
};

} // namespace Test::Backend::Epp::Domain
} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test

#endif
