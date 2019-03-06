/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/domain/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/domain/domain_enum_validation.hh"
#include "src/backend/epp/domain/update_domain.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"

#include <boost/mpl/assert.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(UpdateDomain)


bool fail_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::update_domain(
                    ctx,
                    DefaultUpdateDomainInputData(),
                    DefaultUpdateDomainConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            fail_invalid_registrar_id_exception);
}
bool fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_domain_does_not_exist, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::update_domain(
                    ctx,
                    UpdateDomainInputData(NonexistentFqdn().fqdn).data,
                    DefaultUpdateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_nonexistent_handle_exception);
}

bool fail_enum_domain_does_not_exist_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_enum_domain_does_not_exist, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::update_domain(
                    ctx,
                    UpdateDomainInputData(NonexistentEnumFqdn().fqdn).data,
                    DefaultUpdateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_enum_domain_does_not_exist_exception);
}

bool fail_enum_domain_does_not_exist_wih_valexdate_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_enum_domain_does_not_exist_with_valexdate, supply_ctx<HasRegistrarWithSession>)
{
    NonexistentEnumDomain nonexistent_enum_domain(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::update_domain(
                    ctx,
                    ::Epp::Domain::UpdateDomainInputData(
                            nonexistent_enum_domain.data.fqdn,
                            Optional<std::string>(),            // registrant_chg
                            Optional<std::string>(),            // auth_info_pw_chg
                            Optional<Nullable<std::string> >(), // nsset_chg
                            Optional<Nullable<std::string> >(), // keyset_chg
                            std::vector<std::string>(),         // admin_contacts_add
                            std::vector<std::string>(),         // admin_contacts_rem
                            std::vector<std::string>(),         // tmpcontacts_rem
                            boost::optional< ::Epp::Domain::EnumValidationExtension>(
                                    1,
                                    ::Epp::Domain::EnumValidationExtension(
                                            nonexistent_enum_domain.data.enum_domain_validation.get_value().validation_expiration + boost::gregorian::months(1),
                                            false))), // enum_validation_list
                    DefaultUpdateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_enum_domain_does_not_exist_wih_valexdate_exception);
}

bool fail_invalid_zone_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_invalid_zone, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::update_domain(
                    ctx,
                    UpdateDomainInputData(domain.data.fqdn + "c").data,
                    DefaultUpdateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_invalid_zone_exception);
}

bool fail_wrong_registrar_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_wrong_registrar, supply_ctx<HasRegistrarWithSessionAndDomainOfDifferentRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::update_domain(
                    ctx,
                    UpdateDomainInputData(domain_of_different_registrar.data.fqdn).data,
                    DefaultUpdateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_wrong_registrar_exception);
}

bool fail_registrar_without_zone_access_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_registrar_without_zone_access, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::update_domain(
                    ctx,
                    UpdateDomainInputData(domain.data.fqdn).data,
                    DefaultUpdateDomainConfigData(),
                    Session(ctx, RegistrarNotInZone(ctx).data.id).data),
            ::Epp::EppResponseFailure,
            fail_registrar_without_zone_access_exception);
}
bool fail_prohibiting_status_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_prohibiting_status, supply_ctx<HasRegistrarWithSession>)
{
    const DomainWithStatusRequestServerUpdateProhibited domain_with_status_request_server_update_prohibited(ctx, registrar.data.handle);

    ::LibFred::PerformObjectStateRequest(domain_with_status_request_server_update_prohibited.data.id).exec(ctx);
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::update_domain(
                    ctx,
                    UpdateDomainInputData(domain_with_status_request_server_update_prohibited.data.fqdn).data,
                    DefaultUpdateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_prohibiting_status_exception);
}
bool fail_invalid_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_invalid_handle, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::update_domain(
                    ctx,
                    UpdateDomainInputData(InvalidFqdn().fqdn).data,
                    DefaultUpdateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_invalid_handle_exception);
}

BOOST_FIXTURE_TEST_CASE(nsset_change_should_clear_keyset, supply_ctx<HasRegistrarWithSession>)
{
    FullDomain domain(ctx, registrar.data.handle);

    const std::string nsset_handle = "NSSET2";
    ::LibFred::CreateNsset(nsset_handle, registrar.data.handle).exec(ctx);
    const Optional<Nullable<std::string>> nsset_chg = Nullable<std::string>(nsset_handle);

    BOOST_REQUIRE(!domain.data.keyset.isnull());

    const ::Epp::Domain::UpdateDomainConfigData update_domain_config_data(
            false, // rifd_epp_operations_charging
            true); // rifd_epp_update_domain_keyset_clear

    ::Epp::Domain::update_domain(
            ctx,
            ::Epp::Domain::UpdateDomainInputData(
                    domain.data.fqdn,
                    Optional<std::string>(), // registrant_chg
                    Optional<std::string>(), // authinfopw_chg
                    nsset_chg, // nsset_chg
                    Optional<Nullable<std::string>>(), // keyset_chg
                    std::vector<std::string>(), // admin_contacts_add
                    std::vector<std::string>(), // admin_contacts_rem
                    std::vector<std::string>(), // tmpcontacts_rem
                    boost::optional< ::Epp::Domain::EnumValidationExtension >()), // enum_validation
            update_domain_config_data,
            session.data);
    const ::LibFred::InfoDomainData info_domain_data = ::LibFred::InfoDomainById(domain.data.id).exec(ctx, "UTC").info_domain_data;

    BOOST_CHECK(!info_domain_data.nsset.isnull());
    if (!info_domain_data.nsset.isnull())
    {
        BOOST_CHECK_EQUAL(info_domain_data.nsset.get_value().handle, nsset_handle);
    }
    BOOST_CHECK(info_domain_data.keyset.isnull());
}

BOOST_FIXTURE_TEST_CASE(nsset_change_should_not_clear_keyset, supply_ctx<HasRegistrarWithSession>)
{
    FullDomain domain(ctx, registrar.data.handle);

    const Optional<Nullable<std::string> > nsset_chg = Optional<Nullable<std::string> >(Nullable<std::string>());

    BOOST_REQUIRE(!domain.data.keyset.isnull());

    ::Epp::Domain::UpdateDomainConfigData update_domain_config_data(
            false, // rifd_epp_operations_charging
            false); // rifd_epp_update_domain_keyset_clear

    ::Epp::Domain::update_domain(
        ctx,
        ::Epp::Domain::UpdateDomainInputData(
            domain.data.fqdn,
            Optional<std::string>(), // registrant_chg
            Optional<std::string>(), // authinfopw_chg
            nsset_chg, // nsset_chg,
            Optional<Nullable<std::string> >(), // keyset_chg
            std::vector<std::string>(), // admin_contacts_add
            std::vector<std::string>(), // admin_contacts_rem
            std::vector<std::string>(), // tmpcontacts_rem
            boost::optional< ::Epp::Domain::EnumValidationExtension>()), // enum_validation
        update_domain_config_data,
        session.data
    );

    ::LibFred::InfoDomainData info_domain_data = ::LibFred::InfoDomainById(domain.data.id).exec(ctx, "UTC").info_domain_data;

    BOOST_CHECK(info_domain_data.nsset.isnull());
    BOOST_CHECK_EQUAL(info_domain_data.keyset.isnull(), update_domain_config_data.rifd_epp_update_domain_keyset_clear);
}

bool fail_tmpcontacts_rem_not_empty_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_tmpcontacts_rem_not_empty, supply_ctx<HasDataForUpdateDomain>)
{
    BOOST_REQUIRE(!tmpcontacts_rem_.empty());

    BOOST_CHECK_EXCEPTION(
        ::Epp::Domain::update_domain(
            ctx,
            ::Epp::Domain::UpdateDomainInputData(
                domain.data.fqdn,
                Optional<std::string>(), // registrant_chg
                Optional<std::string>(), // authinfopw_chg
                Optional<Nullable<std::string> >(), // nsset_chg
                Optional<Nullable<std::string> >(), // keyset_chg
                std::vector<std::string>(), // admin_contacts_add
                std::vector<std::string>(), // admin_contacts_rem
                tmpcontacts_rem_, // tmpcontacts_rem
                boost::optional< ::Epp::Domain::EnumValidationExtension>()), // enum_validation
            DefaultUpdateDomainConfigData(),
            session.data
        ),
        ::Epp::EppResponseFailure,
        fail_tmpcontacts_rem_not_empty_exception
    );
}

bool fail_existing_fqdn_but_spaces_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_existing_fqdn_but_spaces, supply_ctx<HasDataForUpdateDomain>)
{
    BOOST_REQUIRE(admin_contacts_add_.size() == 2);
    BOOST_REQUIRE(admin_contacts_rem_.size() == 1);

    const std::string fqdn_with_spaces("  " + domain.data.fqdn + "  ");

    BOOST_CHECK_EXCEPTION(
        ::Epp::Domain::update_domain(
            ctx,
            ::Epp::Domain::UpdateDomainInputData(
                fqdn_with_spaces,
                Optional<std::string>(new_registrant_handle_),
                Optional<std::string>(new_authinfopw_),
                Optional<Nullable<std::string> >(new_nsset_handle_),
                Optional<Nullable<std::string> >(new_keyset_handle_),
                admin_contacts_add_,
                admin_contacts_rem_,
                std::vector<std::string>(), // tmpcontacts_rem
                boost::optional< ::Epp::Domain::EnumValidationExtension>()), // enum_validation
            ::Epp::Domain::UpdateDomainConfigData(
                    false,  // rifd_epp_operations_charging
                    false), // rifd_epp_update_domain_keyset_clear
            session.data
        ),
        ::Epp::EppResponseFailure,
        fail_existing_fqdn_but_spaces_exception
    );
}

BOOST_FIXTURE_TEST_CASE(ok, supply_ctx<HasDataForUpdateDomain>)
{
    BOOST_REQUIRE(admin_contacts_add_.size() == 2);
    BOOST_REQUIRE(admin_contacts_rem_.size() == 1);

    ::Epp::Domain::update_domain(
        ctx,
        ::Epp::Domain::UpdateDomainInputData(
            domain.data.fqdn,
            registrant_chg_, // registrant_chg
            authinfopw_chg_, // authinfopw_chg
            nsset_chg_, // nsset_chg
            keyset_chg_, // keyset_chg
            admin_contacts_add_, // admin_contacts_add
            admin_contacts_rem_, // admin_contacts_rem
            std::vector<std::string>(), // tmpcontacts_rem
            enum_validation_), // enum_validation_list
        ::Epp::Domain::UpdateDomainConfigData(
                false,  // rifd_epp_operations_charging
                false), // rifd_epp_update_domain_keyset_clear
        session.data
    );

    ::LibFred::InfoDomainData domain_data_after_update = ::LibFred::InfoDomainByFqdn(domain.data.fqdn).exec(ctx, "UTC").info_domain_data;

    BOOST_CHECK_EQUAL(domain_data_after_update.roid, domain.data.roid);
    BOOST_CHECK_EQUAL(domain_data_after_update.fqdn, domain.data.fqdn);

    BOOST_CHECK_EQUAL(domain_data_after_update.registrant.handle, registrant_chg_);

    BOOST_CHECK_EQUAL(domain_data_after_update.nsset.get_value_or_default().handle, nsset_chg_.get_value_or_default());
    BOOST_CHECK_EQUAL(domain_data_after_update.keyset.get_value_or_default().handle, keyset_chg_.get_value_or_default());
    // states
    BOOST_CHECK_EQUAL(domain_data_after_update.sponsoring_registrar_handle, domain.data.sponsoring_registrar_handle);
    BOOST_CHECK_EQUAL(domain_data_after_update.create_registrar_handle, domain.data.create_registrar_handle);

    BOOST_CHECK_EQUAL(domain_data_after_update.update_registrar_handle, registrar.data.handle);

    BOOST_CHECK_EQUAL(domain_data_after_update.creation_time, domain.data.creation_time);
    //BOOST_CHECK_EQUAL(domain_data_after_update.last_update, domain.data.update_time);
    BOOST_CHECK_EQUAL(domain_data_after_update.transfer_time, domain.data.transfer_time);
    BOOST_CHECK_EQUAL(domain_data_after_update.expiration_date, domain.data.expiration_date);
    BOOST_CHECK_EQUAL(domain_data_after_update.authinfopw, authinfopw_chg_);

    std::vector<std::string> domain_data_after_update_admin_contacts =
            vector_of_Fred_ObjectIdHandlePair_to_vector_of_string(domain_data_after_update.admin_contacts);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        domain_data_after_update_admin_contacts.begin(),
        domain_data_after_update_admin_contacts.end(),
        admin_contacts_add_.begin(),
        admin_contacts_add_.end()
    );

    BOOST_CHECK_EQUAL(
       domain_data_after_update.enum_domain_validation.get_value_or_default().validation_expiration,
       domain.data.enum_domain_validation.get_value_or_default().validation_expiration
   );

    BOOST_CHECK_EQUAL(
       domain_data_after_update.enum_domain_validation.get_value_or_default().publish,
       domain.data.enum_domain_validation.get_value_or_default().publish
   );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
