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

#ifndef FIXTURE_HH_DDB7D8EEA4534803A96FA30593FE5869
#define FIXTURE_HH_DDB7D8EEA4534803A96FA30593FE5869

#include "libfred/opcontext.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "test/libfred/contact/util.hh"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {
namespace LibFred {
namespace Contact {


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

        _ctx.get_conn().exec_params(
                // clang-format off
                "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                    "SELECT $1::bigint, z.id, NOW() "
                        "FROM zone z "
                        "WHERE z.fqdn = $2::text",
                // clang-format on
                Database::query_param_list(data.id)("cz"));

        _ctx.get_conn().exec_params(
                // clang-format off
                "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                    "SELECT $1::bigint, z.id, NOW() "
                        "FROM zone z "
                        "WHERE z.fqdn = $2::text",
                // clang-format on
                Database::query_param_list(data.id)("0.2.4.e164.arpa"));
    }


};


struct Contact
{
    ::LibFred::InfoContactData data;


    Contact(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _contact_handle = "CONTACT")
    {
        ::LibFred::CreateContact(_contact_handle, _registrar_handle).exec(_ctx);
        data = ::LibFred::InfoContactByHandle(_contact_handle).exec(_ctx).info_contact_data;
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
                // clang-format off
                "UPDATE enum_object_states "
                   "SET manual = 'true'::bool "
                 "WHERE name = $1::text",
                // clang-format on
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

struct ContactWithStatusRequest
    : Contact
{
    const std::string status;


    ContactWithStatusRequest(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle,
            const std::string& _status)
        : Contact(_ctx, _registrar_handle, "CONTACTWITH" + boost::algorithm::to_upper_copy(_status)),
          status(_status)
    {
        ObjectWithStatus(_ctx, data.id, _status);
    }


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

struct ContactWithContactPassedManualVerification
    : ContactWithStatus
{


    ContactWithContactPassedManualVerification(
            ::LibFred::OperationContext& _ctx,
            const std::string& _registrar_handle)
        : ContactWithStatus(_ctx, _registrar_handle, "contactPassedManualVerification")
    {
    }


};


// fixtures

struct HasRegistrarWithContact
{
    Registrar registrar;
    Contact contact;


    HasRegistrarWithContact(
            ::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          contact(_ctx, registrar.data.handle)
    {
    }


};


struct HasRegistrarWithContactWithPassedManualVerification
{
    Registrar registrar;
    ContactWithContactPassedManualVerification contact_with_contact_passed_manual_verification;


    HasRegistrarWithContactWithPassedManualVerification(
            ::LibFred::OperationContext& _ctx)
        : registrar(_ctx),
          contact_with_contact_passed_manual_verification(_ctx, registrar.data.handle)
    {
    }


};



} // namespace Test::LibFred::Contact
} // namespace Test::LibFred
} // namespace Test

#endif
