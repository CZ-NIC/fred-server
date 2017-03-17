/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef FIXTURE_H_F24A408EE3FE45F2898BE15AD792032D
#define FIXTURE_H_F24A408EE3FE45F2898BE15AD792032D

#include "tests/interfaces/epp/util.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/session_data.h"

#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/opcontext.h"

#include <boost/algorithm/string.hpp>

#include <set>
#include <string>
#include <vector>

namespace Test {

template <class T>
struct supply_ctx
    : autorollbacking_context,
      T
{


    supply_ctx()
        : autorollbacking_context(),
          T(ctx)
    {
    }


};

struct SessionData
    : Epp::SessionData
{


    SessionData(const unsigned long long _registrar_id)
        : Epp::SessionData(
                  _registrar_id,
                  Epp::SessionLang::en,
                  "",
                  boost::optional<unsigned long long>(0))
    {
    }


};

struct Session
{
    SessionData data;


    Session(
            Fred::OperationContext& _ctx,
            const unsigned long long _registrar_id)
        : data(_registrar_id)
    {
    }


};

struct SessionWithUnauthenticatedRegistrar
    : Session
{
    static const unsigned long long unauthenticated_registrar_id = 0;


    SessionWithUnauthenticatedRegistrar(Fred::OperationContext& _ctx)
        : Session(_ctx, unauthenticated_registrar_id)
    {
    }


};

struct Registrar
{
    Fred::InfoRegistrarData data;


    Registrar(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle = "REG-TEST")
    {
        Fred::CreateRegistrar(_registrar_handle).exec(_ctx);
        data = Fred::InfoRegistrarByHandle(_registrar_handle).exec(_ctx).info_registrar_data;
        BOOST_REQUIRE(!data.system.get_value_or(false));

        // clang-format off
        _ctx.get_conn().exec_params(
                "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                    "SELECT $1::bigint, z.id, NOW() "
                        "FROM zone z "
                        "WHERE z.fqdn = $2::text",
                Database::query_param_list(data.id)
                ("cz"));
        // clang-format on

        // clang-format off
        _ctx.get_conn().exec_params(
                "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                    "SELECT $1::bigint, z.id, NOW() "
                        "FROM zone z "
                        "WHERE z.fqdn = $2::text",
                Database::query_param_list(data.id)
                ("0.2.4.e164.arpa"));
        // clang-format on
    }


};

struct RegistrarNotInZone
{
    Fred::InfoRegistrarData data;


    RegistrarNotInZone(Fred::OperationContext& _ctx)
    {
        const std::string registrar_handle = "REG-NOZONE";
        Fred::CreateRegistrar(registrar_handle).exec(_ctx);
        data = Fred::InfoRegistrarByHandle(registrar_handle).exec(_ctx).info_registrar_data;
    }


};

struct SystemRegistrar
{
    Fred::InfoRegistrarData data;


    SystemRegistrar(Fred::OperationContext& _ctx)
    {
        const std::string registrar_handle = "REG-SYSTEM";
        Fred::CreateRegistrar(registrar_handle).set_system(true).exec(_ctx);
        data = Fred::InfoRegistrarByHandle(registrar_handle).exec(_ctx).info_registrar_data;
        BOOST_REQUIRE(data.system.get_value_or(false));
    }


};

struct Contact
{
    Fred::InfoContactData data;


    Contact(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _contact_handle = "CONTACT")
    {
        Fred::CreateContact(_contact_handle, _registrar_handle).exec(_ctx);
        data = Fred::InfoContactByHandle(_contact_handle).exec(_ctx).info_contact_data;
    }


};

struct Fqdn
{
    const std::string fqdn;


    Fqdn(const std::string& _fqdn)
        : fqdn(_fqdn)
    {
    }


};

struct InvalidFqdn
    : Fqdn
{


    InvalidFqdn()
        : Fqdn("invalid!fqdn.cz")
    {
    }


};

struct InvalidEnumFqdn
    : Fqdn
{


    InvalidEnumFqdn()
        : Fqdn("5.1.3.5.0.2.4.e164.arpa")
    {
    }


};

struct NonexistentFqdn
    : Fqdn
{


    NonexistentFqdn()
        : Fqdn("nonexistentdomain.cz")
    {
    }


};

struct NonexistentEnumFqdn
    : Fqdn
{


    NonexistentEnumFqdn()
        : Fqdn("5.1.3.5.0.2.4.e164.arpa")
    {
    }


};

struct BlacklistedFqdn
    : Fqdn
{


    BlacklistedFqdn(Fred::OperationContext& _ctx)
        : Fqdn("blacklistedfqdn.cz")
    {
        _ctx.get_conn().exec_params(
                "INSERT INTO domain_blacklist (regexp, valid_from, reason) "
                "VALUES ($1::text, NOW(), '')",
                Database::query_param_list(fqdn));
    }


};

struct Domain
{
    Contact registrant;
    Fred::InfoDomainData data;


    Domain(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _fqdn = "freddy.cz",
            const std::string& _registrant_handle = "REGISTRANT")
        : registrant(_ctx, _registrar_handle, _registrant_handle)
    {
        Fred::CreateDomain(_fqdn, _registrar_handle, registrant.data.handle).exec(_ctx);
        data = Fred::InfoDomainByHandle(_fqdn).exec(_ctx, "UTC").info_domain_data;
    }


};

struct EnumDomain
{
    Contact registrant;
    Fred::InfoDomainData data;


    EnumDomain(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _enum_fqdn = "5.5.1.3.5.0.2.4.e164.arpa",
            const std::string& _registrant_handle = "REGISTRANT")
        : registrant(_ctx, _registrar_handle, _registrant_handle)
    {

        const std::string tmp_fqdn = "tmpdomain.cz";

        Fred::CreateDomain(tmp_fqdn, _registrar_handle, registrant.data.handle).exec(_ctx);
        data = Fred::InfoDomainByHandle(tmp_fqdn).exec(_ctx, "UTC").info_domain_data;

        Fred::CreateDomain(_enum_fqdn, _registrar_handle, registrant.data.handle)
        .set_enum_validation_expiration(data.creation_time.date() + boost::gregorian::months(3))
        // .set_nsset(nsset_handle)
        // .set_keyset(keyset_handle)
        // .set_admin_contacts(admin_contacts)
        .exec(_ctx);

        data = Fred::InfoDomainByHandle(_enum_fqdn).exec(_ctx, "UTC").info_domain_data;
    }


};

struct NonexistentEnumDomain
    : EnumDomain
{


    NonexistentEnumDomain(
            Fred::OperationContext& _ctx,
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
            Fred::OperationContext& _ctx,
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
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _status)
        : Domain(_ctx, _registrar_handle, "domainwith" + boost::algorithm::to_lower_copy(_status) + ".cz"),
          status(_status)
    {
        _ctx.get_conn().exec_params(
                "UPDATE enum_object_states SET manual = 'true'::bool WHERE name = $1::text",
                Database::query_param_list(_status)
                );

        const std::set<std::string> statuses = boost::assign::list_of(_status);

        Fred::CreateObjectStateRequestId(data.id, statuses).exec(_ctx);

        // ensure object has only request, not the state itself
        {
            std::vector<std::string> object_states_before;
            {
                BOOST_FOREACH(
                        const Fred::ObjectStateData & state,
                        Fred::GetObjectStates(data.id).exec(_ctx)) {
                    object_states_before.push_back(state.state_name);
                }
            }

            BOOST_CHECK(
                    std::find(object_states_before.begin(), object_states_before.end(), _status) ==
                    object_states_before.end());
        }
    }


};

struct DomainWithStatus
    : DomainWithStatusRequest
{


    DomainWithStatus(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _status)
        : DomainWithStatusRequest(
                  _ctx,
                  _registrar_handle,
                  _status)
    {
        Fred::PerformObjectStateRequest(data.id).exec(_ctx);
    }


};

struct DomainWithServerDeleteProhibited
    : DomainWithStatus
{


    DomainWithServerDeleteProhibited(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatus(_ctx, _registrar_handle, "serverDeleteProhibited")
    {
    }


};

struct DomainWithServerUpdateProhibited
    : DomainWithStatus
{


    DomainWithServerUpdateProhibited(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatus(_ctx, _registrar_handle, "serverUpdateProhibited")
    {
    }


};

struct DomainWithServerTransferProhibited
    : DomainWithStatus
{


    DomainWithServerTransferProhibited(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatus(_ctx, _registrar_handle, "serverTransferProhibited")
    {
    }


};

struct DomainWithStatusRequestAndServerTransferProhibited
    : DomainWithStatusRequest
{


    DomainWithStatusRequestAndServerTransferProhibited(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatusRequest(_ctx, _registrar_handle, "serverTransferProhibited")
    {
    }


};

struct DomainWithServerUpdateProhoibitedRequest
    : DomainWithStatusRequest
{


    DomainWithServerUpdateProhoibitedRequest(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatusRequest(_ctx, _registrar_handle, "serverUpdateProhibited")
    {
    }


};

struct DomainWithServerUpdateProhibitedRequest
    : DomainWithStatusRequest
{


    DomainWithServerUpdateProhibitedRequest(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : DomainWithStatusRequest(_ctx, _registrar_handle, "serverUpdateProhibited")
    {
    }


};

struct FullDomain
{
    Contact registrant;
    Fred::InfoDomainData data;


    FullDomain(
            Fred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _fqdn = "freddy.cz",
            const std::string& _registrant_handle = "REGISTRANT")
        : registrant(_ctx, _registrar_handle, _registrant_handle)
    {
        Fred::CreateContact("CONTACT1", _registrar_handle).exec(_ctx);

        Fred::CreateNsset("NSSET1", _registrar_handle).exec(_ctx);

        Fred::CreateKeyset("KEYSET1", _registrar_handle).exec(_ctx);

        Fred::CreateDomain("fulldomain.cz", _registrar_handle, _registrant_handle)
                .set_admin_contacts(Util::vector_of<std::string>("CONTACT1"))
                .set_nsset(std::string("NSSET1"))
                .set_keyset(std::string("KEYSET1"))
                .exec(_ctx);

        data = Fred::InfoDomainByHandle("fulldomain.cz").exec(_ctx, "UTC").info_domain_data;
    }


};


namespace Fixture {

struct HasSessionWithUnauthenticatedRegistrar
{
    SessionWithUnauthenticatedRegistrar session_with_unauthenticated_registrar;


    HasSessionWithUnauthenticatedRegistrar(Fred::OperationContext& _ctx)
        : session_with_unauthenticated_registrar(_ctx)
    {
    }


};

struct HasSessionWithAuthenticatedRegistrar
{
    Registrar registrar;
    SessionData session_data_with_authenticated_registrar;


    HasSessionWithAuthenticatedRegistrar(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session_data_with_authenticated_registrar(registrar.data.id)
    {
    }


};

struct HasRegistrarWithSession
{
    Registrar registrar;
    Session session;


    HasRegistrarWithSession(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id)
    {
    }


};

struct HasSystemRegistrarWithSession
{
    SystemRegistrar system_registrar;
    Session session;


    HasSystemRegistrarWithSession(Fred::OperationContext& _ctx)
        : system_registrar(_ctx),
          session(_ctx, system_registrar.data.id)
    {
    }


};

struct HasRegistrarNotInZoneWithSession
{
    RegistrarNotInZone registrar_not_in_zone;
    Session session;


    HasRegistrarNotInZoneWithSession(Fred::OperationContext& _ctx)
        : registrar_not_in_zone(_ctx),
          session(_ctx, registrar_not_in_zone.data.id)
    {
    }


};

struct HasSystemRegistrarWithSessionAndDomain
{
    SystemRegistrar system_registrar;
    Session session;
    Domain domain;


    HasSystemRegistrarWithSessionAndDomain(Fred::OperationContext& _ctx)
        : system_registrar(_ctx),
          session(_ctx, system_registrar.data.id),
          domain(_ctx, system_registrar.data.handle)

    {
    }


};

struct HasSystemRegistrarWithSessionAndBlacklistedDomain
{
    SystemRegistrar system_registrar;
    Session session;
    BlacklistedDomain blacklisted_domain;


    HasSystemRegistrarWithSessionAndBlacklistedDomain(Fred::OperationContext& _ctx)
        : system_registrar(_ctx),
          session(_ctx, system_registrar.data.id),
          blacklisted_domain(_ctx, system_registrar.data.handle)
    {
    }


};

struct HasRegistrarWithSessionAndBlacklistedDomain
{
    Registrar registrar;
    Session session;
    BlacklistedDomain blacklisted_domain;


    HasRegistrarWithSessionAndBlacklistedDomain(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          blacklisted_domain(_ctx, registrar.data.handle)

    {
    }


};

struct HasRegistrarWithSessionAndNonexistentFqdn
{
    Registrar registrar;
    Session session;
    NonexistentFqdn nonexistent_fqdn;


    HasRegistrarWithSessionAndNonexistentFqdn(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          nonexistent_fqdn()

    {
    }


};

struct HasRegistrarWithSessionAndNonexistentEnumFqdn
{
    Registrar registrar;
    Session session;
    NonexistentEnumFqdn nonexistent_enum_fqdn;


    HasRegistrarWithSessionAndNonexistentEnumFqdn(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          nonexistent_enum_fqdn()

    {
    }


};

struct HasRegistrarWithSessionAndBlacklistedFqdn
{
    Registrar registrar;
    Session session;
    BlacklistedFqdn blacklisted_fqdn;


    HasRegistrarWithSessionAndBlacklistedFqdn(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          blacklisted_fqdn(_ctx)

    {
    }


};

struct HasRegistrarNotInZoneWithSessionAndDomain
{
    SystemRegistrar registrar_not_in_zone;
    Session session;
    Domain domain;


    HasRegistrarNotInZoneWithSessionAndDomain(Fred::OperationContext& _ctx)
        : registrar_not_in_zone(_ctx),
          session(_ctx, registrar_not_in_zone.data.id),
          domain(_ctx, registrar_not_in_zone.data.handle)

    {
    }


};

struct HasRegistrarWithSessionAndDomain
{
    Registrar registrar;
    Session session;
    Domain domain;


    HasRegistrarWithSessionAndDomain(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          domain(_ctx, registrar.data.handle)
    {
    }


};

struct HasRegistrarWithSessionAndDomainAndDifferentRegistrar
{
    Registrar registrar;
    Session session;
    Domain domain;
    Registrar different_registrar;


    HasRegistrarWithSessionAndDomainAndDifferentRegistrar(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          domain(_ctx, registrar.data.handle),
          different_registrar(_ctx, "REG-TEST2")
    {
    }


};

struct HasSystemRegistrarWithSessionAndDifferentRegistrar
{
    SystemRegistrar system_registrar;
    Session session;
    Registrar different_registrar;


    HasSystemRegistrarWithSessionAndDifferentRegistrar(Fred::OperationContext& _ctx)
        : system_registrar(_ctx),
          session(_ctx, system_registrar.data.id),
          different_registrar(_ctx, "REG-TEST2")
    {
    }


};

struct HasRegistrarWithSessionAndDifferentRegistrar
{
    Registrar registrar;
    Session session;
    Registrar different_registrar;


    HasRegistrarWithSessionAndDifferentRegistrar(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
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


    HasRegistrarWithSessionAndDomainOfDifferentRegistrar(Fred::OperationContext& _ctx)
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


    HasDomainWithdServerTransferProhibitedAndDifferentRegistrar(Fred::OperationContext& _ctx)
        : different_registrar(_ctx, "REG-TEST2"),
          domain_with_server_transfer_prohibited(_ctx, different_registrar.data.handle + "DIFFERENT")
    {
        BOOST_REQUIRE(
                domain_with_server_transfer_prohibited.data.sponsoring_registrar_handle !=
                different_registrar.data.handle);
    }


};

} // namespace Test::Fixture

// void print_epp_response_failure(const Epp::EppResponseFailure& e) {
//    BOOST_TEST_MESSAGE("EPP result code: " << e.epp_result().epp_result_code());
//    for (std::set<Epp::EppExtendedError>::const_iterator extended_error = e.epp_result().extended_errors()->begin();
//            extended_error != e.epp_result().extended_errors()->end();
//            ++extended_error)
//    {
//        BOOST_TEST_MESSAGE("EPP extended error param: " << e.epp_result().extended_errors()->begin()->param());
//        BOOST_TEST_MESSAGE("EPP extended error position: " << e.epp_result().extended_errors()->begin()->position());
//        BOOST_TEST_MESSAGE("EPP extended error reason: " << e.epp_result().extended_errors()->begin()->reason());
//    }
// }

} // namespace Test

#endif
