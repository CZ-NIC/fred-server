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

#ifndef FIXTURE_HH_82D19B17078B4964B185BFF5A6F2207E
#define FIXTURE_HH_82D19B17078B4964B185BFF5A6F2207E

#include "test/backend/epp/util.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/session_data.hh"

#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opcontext.hh"

#include <boost/algorithm/string.hpp>

#include <set>
#include <string>
#include <vector>

namespace Test {
namespace Backend {
namespace Epp {

template <class T>
struct supply_ctx : autorollbacking_context, T
{
    supply_ctx()
        : autorollbacking_context(),
          T(ctx)
    {
    }
};

struct DefaultSessionData : public ::Epp::SessionData
{
    DefaultSessionData()
        : SessionData(0, ::Epp::SessionLang::en, "", boost::optional<unsigned long long>(0))
    {
    }

    DefaultSessionData& set_registrar_id(unsigned long long _registrar_id)
    {
        registrar_id = _registrar_id;
        return *this;
    }
};

struct SessionData
    : ::Epp::SessionData
{
    SessionData(const unsigned long long _registrar_id)
        : ::Epp::SessionData(
                  _registrar_id,
                  ::Epp::SessionLang::en,
                  "",
                  boost::optional<unsigned long long>(0))
    {
    }
};

struct Session
{
    SessionData data;

    Session(
            ::LibFred::OperationContext& _ctx,
            const unsigned long long _registrar_id)
        : data(_registrar_id)
    {
    }
};

struct SessionWithUnauthenticatedRegistrar
    : Session
{
    static const unsigned long long unauthenticated_registrar_id = 0;

    SessionWithUnauthenticatedRegistrar(::LibFred::OperationContext& _ctx)
        : Session(_ctx, unauthenticated_registrar_id)
    {
    }
};

struct Registrar
{
    ::LibFred::InfoRegistrarData data;

    Registrar(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle = "REG-TEST")
    {
        ::LibFred::CreateRegistrar(_registrar_handle).exec(_ctx);
        data = ::LibFred::InfoRegistrarByHandle(_registrar_handle).exec(_ctx).info_registrar_data;
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
    ::LibFred::InfoRegistrarData data;

    RegistrarNotInZone(::LibFred::OperationContext& _ctx)
    {
        const std::string registrar_handle = "REG-NOZONE";
        ::LibFred::CreateRegistrar(registrar_handle).exec(_ctx);
        data = ::LibFred::InfoRegistrarByHandle(registrar_handle).exec(_ctx).info_registrar_data;
    }
};

struct SystemRegistrar
{
    ::LibFred::InfoRegistrarData data;

    SystemRegistrar(::LibFred::OperationContext& _ctx)
    {
        const std::string registrar_handle = "REG-SYSTEM";
        ::LibFred::CreateRegistrar(registrar_handle).set_system(true).exec(_ctx);
        data = ::LibFred::InfoRegistrarByHandle(registrar_handle).exec(_ctx).info_registrar_data;
        BOOST_REQUIRE(data.system.get_value_or(false));
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

struct ValidFqdn
    : Fqdn
{
    ValidFqdn()
        : Fqdn("validfqdn.cz")
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
    BlacklistedFqdn(::LibFred::OperationContext& _ctx)
        : Fqdn("blacklistedfqdn.cz")
    {
        _ctx.get_conn().exec_params(
                "INSERT INTO domain_blacklist (regexp, valid_from, reason) "
                "VALUES ($1::text, NOW(), '')",
                Database::query_param_list(fqdn));
    }
};

struct Handle
{
    const std::string handle;

    Handle(const std::string& _handle)
        : handle(_handle)
    {
    }
};

struct ValidHandle
    : Handle
{
    ValidHandle()
        : Handle("validhandle")
    {
    }
};

struct InvalidHandle
    : Handle
{
    InvalidHandle()
        : Handle("invalid!handle")
    {
    }
};

struct NonexistentHandle
    : Handle
{
    NonexistentHandle()
        : Handle("nonexistenthandle")
    {
    }
};

// fixtures

struct HasSessionWithUnauthenticatedRegistrar
{
    SessionWithUnauthenticatedRegistrar session_with_unauthenticated_registrar;

    HasSessionWithUnauthenticatedRegistrar(::LibFred::OperationContext& _ctx)
        : session_with_unauthenticated_registrar(_ctx)
    {
    }
};

struct HasSessionWithAuthenticatedRegistrar
{
    Registrar registrar;
    SessionData session_data_with_authenticated_registrar;

    HasSessionWithAuthenticatedRegistrar(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session_data_with_authenticated_registrar(registrar.data.id)
    {
    }
};

struct HasRegistrarWithSession
{
    Registrar registrar;
    Session session;

    HasRegistrarWithSession(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id)
    {
    }
};

struct HasSystemRegistrarWithSession
{
    SystemRegistrar system_registrar;
    Session session;

    HasSystemRegistrarWithSession(::LibFred::OperationContext& _ctx)
        : system_registrar(_ctx),
          session(_ctx, system_registrar.data.id)
    {
    }
};

struct HasRegistrarNotInZoneWithSession
{
    RegistrarNotInZone registrar_not_in_zone;
    Session session;

    HasRegistrarNotInZoneWithSession(::LibFred::OperationContext& _ctx)
        : registrar_not_in_zone(_ctx),
          session(_ctx, registrar_not_in_zone.data.id)
    {
    }
};

struct HasRegistrarWithSessionAndNonexistentFqdn
{
    Registrar registrar;
    Session session;
    NonexistentFqdn nonexistent_fqdn;

    HasRegistrarWithSessionAndNonexistentFqdn(::LibFred::OperationContext& _ctx)
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

    HasRegistrarWithSessionAndNonexistentEnumFqdn(::LibFred::OperationContext& _ctx)
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

    HasRegistrarWithSessionAndBlacklistedFqdn(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          blacklisted_fqdn(_ctx)
    {
    }
};

struct HasSystemRegistrarWithSessionAndDifferentRegistrar
{
    SystemRegistrar system_registrar;
    Session session;
    Registrar different_registrar;

    HasSystemRegistrarWithSessionAndDifferentRegistrar(::LibFred::OperationContext& _ctx)
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

    HasRegistrarWithSessionAndDifferentRegistrar(::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          different_registrar(_ctx, "REG-TEST2")
    {
    }
};

struct ObjectWithStatus
{
    ObjectWithStatus(
            ::LibFred::OperationContext& _ctx,
            unsigned long long _object_id,
            const std::string& _status)
    {
        _ctx.get_conn().exec_params(
                "UPDATE enum_object_states SET manual = 'true'::bool WHERE name = $1::text",
                Database::query_param_list(_status));

        const std::set<std::string> statuses = boost::assign::list_of(_status);

        ::LibFred::CreateObjectStateRequestId(_object_id, statuses).exec(_ctx);

        // ensure object has only request, not the state itself
        {
            std::vector<std::string> object_states_before;
            {
                BOOST_FOREACH (
                        const ::LibFred::ObjectStateData& state,
                        ::LibFred::GetObjectStates(_object_id).exec(_ctx))
                {
                    object_states_before.push_back(state.state_name);
                }
            }

            BOOST_CHECK(
                    std::find(object_states_before.begin(), object_states_before.end(), _status) ==
                    object_states_before.end());
        }
    }
};

// move to cc file if needed
//void print_epp_response_failure(const ::Epp::EppResponseFailure& e) {
//   for (std::vector< ::Epp::EppResultFailure>::const_iterator epp_result = e.epp_results().begin();
//        epp_result != e.epp_results().end();
//        ++epp_result)
//   {
//       BOOST_TEST_MESSAGE("EPP result code: " << epp_result->epp_result_code());
//       for (std::set< ::Epp::EppExtendedError>::const_iterator extended_error = epp_result->extended_errors()->begin();
//            extended_error != epp_result->extended_errors()->end();
//            ++extended_error)
//       {
//           BOOST_TEST_MESSAGE("  EPP extended error param: " << extended_error->param());
//           BOOST_TEST_MESSAGE("  EPP extended error position: " << extended_error->position());
//           BOOST_TEST_MESSAGE("  EPP extended error reason: " << extended_error->reason());
//       }
//   }
//}

} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test

#endif
