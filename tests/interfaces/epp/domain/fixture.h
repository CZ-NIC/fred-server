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

/**
 *  @file fixture.h
 *  <++>
 */

#ifndef TESTS_INTERFACES_EPP_DOMAIN_FIXTURE_H
#define TESTS_INTERFACES_EPP_DOMAIN_FIXTURE_H

#include "tests/setup/fixtures.h"
#include "tests/interfaces/epp/util.h"

#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/registrar/info_registrar.h"

struct HasInfoRegistrarData : virtual Test::autocommitting_context {
    Fred::InfoRegistrarData info_registrar_data;

    HasInfoRegistrarData() {
        const std::string registrar_handle = "REGISTRAR1";
        Fred::CreateRegistrar(registrar_handle).exec(ctx);
        info_registrar_data = Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
    }
};

struct HasDifferentInfoRegistrarData : virtual Test::autocommitting_context {
    Fred::InfoRegistrarData different_info_registrar_data;

    HasDifferentInfoRegistrarData() {
        const std::string different_registrar_handle = "REGISTRAR2";
        Fred::CreateRegistrar(different_registrar_handle).exec(ctx);
        different_info_registrar_data = Fred::InfoRegistrarByHandle(different_registrar_handle).exec(ctx).info_registrar_data;
    }
};

struct HasInfoDomainData : HasInfoRegistrarData {
    Fred::InfoDomainData info_domain_data;

    HasInfoDomainData() {
        const std::string fqdn = "fred.cz";
        const std::string registrant = "REGISTRANT1";
        Fred::CreateDomain(fqdn, info_registrar_data.handle, registrant).exec(ctx);
        info_domain_data = Fred::InfoDomainByHandle(fqdn).exec(ctx).info_domain_data;
    }
};

struct HasInfoDomainDataAndDifferentInfoRegistrarData : HasInfoDomainData, HasDifferentInfoRegistrarData { };

struct HasInfoDomainDataWithStatusRequest : HasInfoDomainData {
    const std::string status;

    HasInfoDomainDataWithStatusRequest(const std::string& _status)
    : status(_status)
    {
        ctx.get_conn().exec_params(
            "UPDATE enum_object_states SET manual = 'true'::bool WHERE name = $1::text",
            Database::query_param_list(_status)
        );

        const std::set<std::string> statuses = boost::assign::list_of(_status);

        Fred::CreateObjectStateRequestId(info_domain_data.id, statuses).exec(ctx);

        // ensure object has only request, not the state itself
        {
            std::vector<std::string> object_states_before;
            {
                BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(info_domain_data.id).exec(ctx) ) {
                    object_states_before.push_back(state.state_name);
                }
            }

            BOOST_CHECK(
                std::find( object_states_before.begin(), object_states_before.end(), _status )
                ==
                object_states_before.end()
            );
        }
    }
};

struct HasObjectWithStatus : HasInfoDomainDataWithStatusRequest {
    HasObjectWithStatus(const std::string& _status)
    : HasInfoDomainDataWithStatusRequest(_status)
    {
        Fred::PerformObjectStateRequest(info_domain_data.id).exec(ctx);
    }
};

struct HasInfoDomainDataWithServerDeleteProhibited : HasObjectWithStatus {
    HasInfoDomainDataWithServerDeleteProhibited()
    : HasObjectWithStatus("serverDeleteProhibited")
    { }
};

struct HasInfoDomainDataWithServerUpdateProhibited : HasObjectWithStatus {
    HasInfoDomainDataWithServerUpdateProhibited()
    : HasObjectWithStatus("serverUpdateProhibited")
    { }
};

struct HasInfoDomainDataWithServerTransferProhibited : HasObjectWithStatus {
    HasInfoDomainDataWithServerTransferProhibited()
    : HasObjectWithStatus("serverTransferProhibited")
    { }
};
struct HasInfoDomainDataWithDifferentInfoRegistrarDataAndServerTransferProhibited : HasInfoDomainDataAndDifferentInfoRegistrarData, HasObjectWithStatus {
    HasInfoDomainDataWithDifferentInfoRegistrarDataAndServerTransferProhibited ()
    : HasInfoDomainDataAndDifferentInfoRegistrarData(),
      HasObjectWithStatus("serverTransferProhibited")
    { }
};

struct HasInfoDomainDataWithStatusRequestAndServerTransferProhibited : HasInfoDomainDataWithStatusRequest {
    HasInfoDomainDataWithStatusRequestAndServerTransferProhibited()
    : HasInfoDomainDataWithStatusRequest("serverTransferProhibited")
    { }
};

struct HasDomainWithServerUpdateProhoibitedRequest : HasInfoDomainDataWithStatusRequest {
    HasDomainWithServerUpdateProhoibitedRequest()
    : HasInfoDomainDataWithStatusRequest("serverUpdateProhibited")
    { }
};

struct HasInfoDomainDataWithServerUpdateProhibitedRequest : HasInfoDomainDataWithStatusRequest {
    HasInfoDomainDataWithServerUpdateProhibitedRequest()
    : HasInfoDomainDataWithStatusRequest("serverUpdateProhibited")
    { }
};

#endif
