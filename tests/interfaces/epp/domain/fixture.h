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

#ifndef FIXTURE_H_389DB6E81FF44407AFC3763706918744
#define FIXTURE_H_389DB6E81FF44407AFC3763706918744

#include "src/epp/domain/impl/domain_enum_validation.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "tests/interfaces/epp/util.h"
#include "src/epp/domain/create_domain_localized.h"
#include "src/epp/domain/renew_domain_localized.h"
#include "tests/setup/fixtures.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <boost/asio/ip/address.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/optional.hpp>

#include <string>
#include <vector>
#include <set>

std::vector<std::string> vector_of_Fred_ObjectIdHandlePair_to_vector_of_string(const std::vector<Fred::ObjectIdHandlePair>& admin_contacts);

struct HasInfoRegistrarData : virtual Test::autorollbacking_context {
    Fred::InfoRegistrarData info_registrar_data_;

    HasInfoRegistrarData() {
        const std::string registrar_handle = "REGISTRAR1";
        Fred::CreateRegistrar(registrar_handle).exec(ctx);
        info_registrar_data_ = Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
    }
};

struct HasRegistrarNotInZone : virtual Test::autorollbacking_context {
    Fred::InfoRegistrarData registrar_data_not_in_zone_;

    HasRegistrarNotInZone() {
        const std::string registrar_handle = "REGNOZONE";
        Fred::CreateRegistrar(registrar_handle).exec(ctx);
        registrar_data_not_in_zone_ = Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
    }
};


struct HasDifferentInfoRegistrarData : virtual Test::autorollbacking_context {
    Fred::InfoRegistrarData different_info_registrar_data_;

    HasDifferentInfoRegistrarData() {
        const std::string different_registrar_handle = "REGISTRARX";
        Fred::CreateRegistrar(different_registrar_handle).exec(ctx);
        different_info_registrar_data_ = Fred::InfoRegistrarByHandle(different_registrar_handle).exec(ctx).info_registrar_data;
        ctx.get_conn().exec_params(
            "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                "SELECT $1::bigint, z.id, NOW() "
                    "FROM zone z "
                    "WHERE z.fqdn = $2::text",
            Database::query_param_list(different_info_registrar_data_.id)
            ("cz")
        );
    }
};

struct HasSystemRegistrar : virtual Test::autorollbacking_context {
    Fred::InfoRegistrarData system_registrar_data_;

    HasSystemRegistrar() {
        const std::string registrar_handle = "REGISTRAR0";
        Fred::CreateRegistrar(registrar_handle).set_system(true).exec(ctx);
        system_registrar_data_ = Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
    }
};

struct HasInfoDomainData : HasInfoRegistrarData {
    Fred::InfoDomainData info_domain_data_;
    Fred::InfoDomainData info_enum_domain_data_;

    HasInfoDomainData() {
        namespace ip = boost::asio::ip;

        const std::string registrant_handle = "REGISTRANT1";
        Fred::CreateContact(registrant_handle, info_registrar_data_.handle).exec(ctx);

        const std::string admin_handle = "ADMIN1";
        Fred::CreateContact(admin_handle, info_registrar_data_.handle).exec(ctx);
        const std::vector<std::string> admin_contacts = Util::vector_of<std::string>
            (admin_handle);

        const std::string tech_handle = "TECH1";
        Fred::CreateContact(tech_handle, info_registrar_data_.handle).exec(ctx);

        const std::string nsset_handle = "NSSET1";
        Fred::CreateNsset(nsset_handle, info_registrar_data_.handle)
        .set_tech_contacts(Util::vector_of<std::string>(tech_handle))
        .set_dns_hosts(Util::vector_of<Fred::DnsHost>
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("11.0.0.3"))(ip::address::from_string("11.1.1.3"))))
            (Fred::DnsHost("c.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("11.0.0.4"))(ip::address::from_string("11.1.1.4"))))
            )
        .exec(ctx);

        const std::string keyset_handle = "KEYSET1";
        Fred::CreateKeyset(keyset_handle, info_registrar_data_.handle)
            .set_tech_contacts( boost::assign::list_of(tech_handle) )
            .exec(ctx);

        const std::string fqdn = "derf.cz";
        Fred::CreateDomain(fqdn, info_registrar_data_.handle, registrant_handle).set_nsset(nsset_handle).set_keyset(keyset_handle).set_admin_contacts(admin_contacts).exec(ctx);

        info_domain_data_ = Fred::InfoDomainByHandle(fqdn).exec(ctx, "UTC").info_domain_data;

        const std::string enum_fqdn = "5.5.1.3.5.0.2.4.e164.arpa";
        Fred::CreateDomain(enum_fqdn, info_registrar_data_.handle, registrant_handle)
            .set_enum_validation_expiration(info_domain_data_.creation_time.date() + boost::gregorian::months(3))
            .set_nsset(nsset_handle).set_keyset(keyset_handle).set_admin_contacts(admin_contacts).exec(ctx);

        info_enum_domain_data_ = Fred::InfoDomainByHandle(enum_fqdn).exec(ctx, "UTC").info_domain_data;

        ctx.get_conn().exec_params(
            "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                "SELECT $1::bigint, z.id, NOW() "
                    "FROM zone z "
                    "WHERE z.fqdn = $2::text",
            Database::query_param_list(info_registrar_data_.id)
            ("cz")
        );

        ctx.get_conn().exec_params(
            "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                "SELECT $1::bigint, z.id, NOW() "
                    "FROM zone z "
                    "WHERE z.fqdn = $2::text",
            Database::query_param_list(info_registrar_data_.id)
            ("0.2.4.e164.arpa")
        );
    }
};

struct HasInfoDomainDataAndDifferentInfoRegistrarData : HasInfoDomainData, HasDifferentInfoRegistrarData { };

struct HasInfoDomainDataWithInfoRegistrarDataOfRegistrarWithoutZoneAccess : HasInfoRegistrarData {
    Fred::InfoDomainData info_domain_data_;

    HasInfoDomainDataWithInfoRegistrarDataOfRegistrarWithoutZoneAccess() {
        const std::string fqdn = "derf.cz";
        const std::string registrant = "TNARTSIGER1";
        Fred::CreateContact(registrant, info_registrar_data_.handle).exec(ctx);
        Fred::CreateDomain(fqdn, info_registrar_data_.handle, registrant).exec(ctx);
        info_domain_data_ = Fred::InfoDomainByHandle(fqdn).exec(ctx, "UTC").info_domain_data;
    }
};

struct HasFqdnOfBlacklistedDomain : HasInfoRegistrarData {
    std::string blacklisted_domain_fqdn_;
    HasFqdnOfBlacklistedDomain() {
        blacklisted_domain_fqdn_ = std::string("blacklisted-domain.cz");
        ctx.get_conn().exec_params(
            "INSERT INTO domain_blacklist (regexp, valid_from, reason) "
                "VALUES ($1::text, NOW(), '')",
            Database::query_param_list(blacklisted_domain_fqdn_)
        );
    }
};

struct HasInfoDomainDataOfRegisteredBlacklistedDomain : HasInfoDomainData {
    HasInfoDomainDataOfRegisteredBlacklistedDomain() {
        ctx.get_conn().exec_params(
            "INSERT INTO domain_blacklist (regexp, valid_from, reason) "
                "VALUES ($1::text, NOW(), '')",
            Database::query_param_list(info_domain_data_.fqdn)
        );
    }
};

struct HasInfoDomainDataWithStatusRequest : HasInfoDomainData {
    const std::string status_;

    HasInfoDomainDataWithStatusRequest(const std::string& _status)
    : status_(_status)
    {
        ctx.get_conn().exec_params(
            "UPDATE enum_object_states SET manual = 'true'::bool WHERE name = $1::text",
            Database::query_param_list(_status)
        );

        const std::set<std::string> statuses = boost::assign::list_of(_status);

        Fred::CreateObjectStateRequestId(info_domain_data_.id, statuses).exec(ctx);

        // ensure object has only request, not the state itself
        {
            std::vector<std::string> object_states_before;
            {
                BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(info_domain_data_.id).exec(ctx) ) {
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
        Fred::PerformObjectStateRequest(info_domain_data_.id).exec(ctx);
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
struct HasInfoDomainDataWithDifferentInfoRegistrarDataAndServerTransferProhibited : HasDifferentInfoRegistrarData, HasObjectWithStatus {
    HasInfoDomainDataWithDifferentInfoRegistrarDataAndServerTransferProhibited ()
    : HasDifferentInfoRegistrarData(),
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

struct HasInfoDomainDataOfNonexistentDomain : HasInfoDomainData {
    HasInfoDomainDataOfNonexistentDomain() {
        const std::string nonexistent_domain_fqdn = "nonexistent-domain.cz";
        info_domain_data_.fqdn = nonexistent_domain_fqdn;
    }
};

struct HasInfoDomainDataOfNonexistentEnumDomain : HasInfoDomainData {
    HasInfoDomainDataOfNonexistentEnumDomain() {
        const std::string nonexistent_domain_fqdn = "5.1.3.5.0.2.4.e164.arpa";
        info_enum_domain_data_.fqdn = nonexistent_domain_fqdn;
    }
};

struct HasDomainData : HasInfoRegistrarData {
    const std::string contact1;
    const std::string contact2;
    const std::string contact3;
    const std::string contact4;
    const std::string contact5;

    const std::string nsset1;
    const std::string nsset2;

    const std::string keyset1;
    const std::string keyset2;

    const std::string fqdn1;
    const std::string fqdn2;
    const std::string enum_fqdn;

    const std::string authinfopw1;
    const std::string authinfopw2;

    Epp::Domain::CreateDomainInputData domain1_create_input_data;
    Epp::Domain::CreateDomainInputData domain2_create_input_data;

    Epp::Domain::RenewDomainInputData domain1_renew_input_data;

    boost::optional<Epp::Domain::RenewDomainInputData> domain2_renew_input_data;


    HasDomainData()
    : contact1("TESTCONTACT1")
    , contact2("TESTCONTACT2")
    , contact3("TESTCONTACT3")
    , contact4("TESTCONTACT4")
    , contact5("TESTCONTACT5")

    , nsset1("TESTNSSET1")
    , nsset2("TESTNSSET2")

    , keyset1("TESTKEYSET1")
    , keyset2("TESTKEYSET2")

    , fqdn1("testdomain1.cz")
    , fqdn2("testdomain2.cz")
    , enum_fqdn("5.5.1.3.5.0.2.4.e164.arpa")

    , authinfopw1("transferheslo")
    , authinfopw2("transferheslo")

    , domain1_create_input_data (fqdn1, contact1, nsset1, keyset1, boost::optional<std::string>(authinfopw1),
        Epp::Domain::DomainRegistrationTime(1, Epp::DomainRegistrationTime::Unit::year),
        Util::vector_of<std::string>(contact2)(contact3),
        std::vector<Epp::Domain::EnumValidationExtension>())

    , domain2_create_input_data (fqdn2, contact1, nsset1, keyset1, boost::optional<std::string>(authinfopw2),
            Epp::Domain::DomainRegistrationTime(1, Epp::DomainRegistrationTime::Unit::year),
            Util::vector_of<std::string>(contact2)(contact3),
            std::vector<Epp::Domain::EnumValidationExtension>())

    , domain1_renew_input_data(fqdn1, std::string(""),
            Epp::Domain::DomainRegistrationTime(1,Epp::Domain::DomainRegistrationTime::Unit::year),
            std::vector<Epp::Domain::EnumValidationExtension>())

    {
        Fred::CreateContact(contact1, info_registrar_data_.handle).exec(ctx);
        Fred::CreateContact(contact2, info_registrar_data_.handle).exec(ctx);
        Fred::CreateContact(contact3, info_registrar_data_.handle).exec(ctx);
        Fred::CreateContact(contact4, info_registrar_data_.handle).exec(ctx);
        Fred::CreateContact(contact5, info_registrar_data_.handle).exec(ctx);

        Fred::CreateNsset(nsset1, info_registrar_data_.handle).exec(ctx);
        Fred::CreateNsset(nsset2, info_registrar_data_.handle).exec(ctx);

        Fred::CreateKeyset(keyset1, info_registrar_data_.handle).exec(ctx);
        Fred::CreateKeyset(keyset2, info_registrar_data_.handle).exec(ctx);

        Fred::CreateDomain::Result create_result_fqdn2 =
        Fred::CreateDomain(fqdn2, info_registrar_data_.handle, contact1)
        .set_admin_contacts(Util::vector_of<std::string>(contact2)(contact3))
        .set_nsset(nsset1).set_keyset(keyset1).exec(ctx);

        Fred::CreateDomain(enum_fqdn, info_registrar_data_.handle, contact1)
            .set_enum_validation_expiration(create_result_fqdn2.creation_time.date() + boost::gregorian::months(3))
            .set_nsset(nsset1).set_keyset(keyset1).set_admin_contacts(Util::vector_of<std::string>(contact2)(contact3)).exec(ctx);

        domain2_renew_input_data = Epp::Domain::RenewDomainInputData(fqdn2,
                boost::gregorian::to_iso_extended_string(Fred::InfoDomainByHandle(fqdn2).exec(ctx).info_domain_data.expiration_date),
                Epp::Domain::DomainRegistrationTime(1,Epp::Domain::DomainRegistrationTime::Unit::year),std::vector<Epp::Domain::EnumValidationExtension>());

        ctx.get_conn().exec_params(
            "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                "SELECT $1::bigint, z.id, NOW() "
                    "FROM zone z "
                    "WHERE z.fqdn = $2::text",
            Database::query_param_list(info_registrar_data_.id)
            ("cz")
        );

        ctx.get_conn().exec_params(
            "INSERT INTO registrarinvoice (registrarid, zone, fromdate) "
                "SELECT $1::bigint, z.id, NOW() "
                    "FROM zone z "
                    "WHERE z.fqdn = $2::text",
            Database::query_param_list(info_registrar_data_.id)
            ("0.2.4.e164.arpa")
        );
    }
};


struct HasDomainDataAndRegistrar : HasDomainData, HasRegistrarNotInZone {};

struct HasDomainDataAndSystemRegistrar : HasDomainData, HasSystemRegistrar {};

struct HasInfoDomainDataOfDomainWithInvalidFqdn : HasInfoDomainData {
    HasInfoDomainDataOfDomainWithInvalidFqdn() {
        const std::string domain_invalid_fqdn = "!domain.cz";
        info_domain_data_.fqdn = domain_invalid_fqdn;
    }
};

struct HasDataForUpdateDomain : HasInfoDomainData {
    const std::string new_registrant_handle_;
    const std::string new_auth_info_pw_;
    const std::string new_nsset_handle_;
    const std::string new_keyset_handle_;
    Optional<std::string> registrant_chg_;
    Optional<std::string> authinfopw_chg_;
    Optional<Nullable<std::string> > nsset_chg_;
    Optional<Nullable<std::string> > keyset_chg_;
    std::vector<std::string> admin_contacts_add_;
    std::vector<std::string> admin_contacts_rem_;
    std::vector<std::string> tmpcontacts_rem_;
    std::vector<Epp::Domain::EnumValidationExtension> enum_validation_list_;

    HasDataForUpdateDomain() {
        : new_registrant_handle_("REGISTRANT2"),
          new_auth_info_pw_(" auth info "),
          new_nsset_handle_("NSSET2"),
          new_keyset_handle_("KEYSET2")
    {
        namespace ip = boost::asio::ip;

        Fred::CreateContact(new_registrant_handle_, info_registrar_data_.handle).exec(ctx);

        const std::string admin_handle2 = "ADMIN2";
        Fred::CreateContact(admin_handle2, info_registrar_data_.handle).exec(ctx);
        const std::string admin_handle3 = "ADMIN3";
        Fred::CreateContact(admin_handle3, info_registrar_data_.handle).exec(ctx);

        const std::string tech_handle = "TECH2";
        Fred::CreateContact(tech_handle, info_registrar_data_.handle).exec(ctx);

        Fred::CreateNsset(new_nsset_handle_, info_registrar_data_.handle)
        .set_tech_contacts(Util::vector_of<std::string>(tech_handle))
        .set_dns_hosts(Util::vector_of<Fred::DnsHost>
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("11.0.0.3"))(ip::address::from_string("11.1.1.3"))))
            (Fred::DnsHost("c.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("11.0.0.4"))(ip::address::from_string("11.1.1.4"))))
            )
        .exec(ctx);

        Fred::CreateKeyset(new_keyset_handle_, info_registrar_data_.handle)
            .set_tech_contacts( boost::assign::list_of(tech_handle) )
            .exec(ctx);


        registrant_chg_ = Optional<std::string>(new_registrant_handle_);
        authinfopw_chg_ = Optional<std::string>(new_authinfopw_);

        nsset_chg_ = Optional<Nullable<std::string> >(new_nsset_handle_);
        keyset_chg_ = Optional<Nullable<std::string> >(new_keyset_handle_);

        admin_contacts_add_ = Util::vector_of<std::string>
            (admin_handle2)
            (admin_handle3);

        admin_contacts_rem_ = vector_of_Fred_ObjectIdHandlePair_to_vector_of_string(info_domain_data_.admin_contacts);

        tmpcontacts_rem_ = Util::vector_of<std::string>
            ("WHATEVER");

    }
};

#endif
