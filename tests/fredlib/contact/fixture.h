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

#ifndef FIXTURE_H_EDBE867A47014435BE73002E2F5D3723
#define FIXTURE_H_EDBE867A47014435BE73002E2F5D3723

#include "tests/fredlib/contact/test_merge_contact_fixture.h"
#include "tests/fredlib/contact/util.h"

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


struct ObjectWithStatus
{

    ObjectWithStatus(
            Fred::OperationContext& _ctx,
            unsigned long long _object_id,
            const std::string& _status)
    {
        _ctx.get_conn().exec_params(
                "UPDATE enum_object_states SET manual = 'true'::bool WHERE name = $1::text",
                Database::query_param_list(_status));

        const std::set<std::string> statuses = boost::assign::list_of(_status);

        Fred::CreateObjectStateRequestId(_object_id, statuses).exec(_ctx);

        // ensure object has only request, not the state itself
        {
            std::vector<std::string> object_states_before;
            {
                BOOST_FOREACH (
                        const Fred::ObjectStateData& state,
                        Fred::GetObjectStates(_object_id).exec(_ctx))
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
            Fred::OperationContext& _ctx,
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

struct ContactWithContactPassedManualVerification
    : ContactWithStatus
{


    ContactWithContactPassedManualVerification(
            Fred::OperationContext& _ctx,
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
            Fred::OperationContext& _ctx)
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
            Fred::OperationContext& _ctx)
        : registrar(_ctx),
          contact_with_contact_passed_manual_verification(_ctx, registrar.data.handle)
    {
    }


};



} // namespace Test::LibFred::Contact
} // namespace Test::LibFred
} // namespace Test

#endif
