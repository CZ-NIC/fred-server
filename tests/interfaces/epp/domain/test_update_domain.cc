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
 *  @file
 */

#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/domain/impl/domain_enum_validation.h"
#include "src/epp/domain/update_domain.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"
#include "src/fredlib/domain/info_domain.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <boost/mpl/assert.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(UpdateDomain)

bool fail_invalid_registrar_id_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_invalid_registrar_id, HasInfoDomainData)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::update_domain(
            ctx,
            Epp::Domain::UpdateDomainInputData(
                info_domain_data_.fqdn,
                Optional<std::string>(), // registrant_chg
                Optional<std::string>(), // authinfopw_chg
                Optional<Nullable<std::string> >(), // nsset_chg
                Optional<Nullable<std::string> >(), // keyset_chg
                std::vector<std::string>(), // admin_contacts_add
                std::vector<std::string>(), // admin_contacts_rem
                std::vector<std::string>(), // tmpcontacts_rem
                std::vector<Epp::Domain::EnumValidationExtension>()), // enum_validation_list
            0, // invalid_registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_invalid_registrar_id_exception
    );
}
bool fail_nonexistent_handle_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_domain_does_not_exist, HasInfoDomainDataOfNonexistentDomain)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::update_domain(
            ctx,
            Epp::Domain::UpdateDomainInputData(
                info_domain_data_.fqdn,
                Optional<std::string>(), // registrant_chg
                Optional<std::string>(), // authinfopw_chg
                Optional<Nullable<std::string> >(), // nsset_chg
                Optional<Nullable<std::string> >(), // keyset_chg
                std::vector<std::string>(), // admin_contacts_add
                std::vector<std::string>(), // admin_contacts_rem
                std::vector<std::string>(), // tmpcontacts_rem
                std::vector<Epp::Domain::EnumValidationExtension>()), // enum_validation_list
            info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_nonexistent_handle_exception
    );
}

bool fail_enum_domain_does_not_exist_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_enum_domain_does_not_exist, HasInfoDomainDataOfNonexistentEnumDomain)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::domain_update_impl(
            ctx,
            info_enum_domain_data_.fqdn,
            Optional<std::string>(), // registrant_chg
            Optional<std::string>(), // auth_info_pw_chg
            Optional<Nullable<std::string> >(), // nsset_chg
            Optional<Nullable<std::string> >(), // keyset_chg
            std::vector<std::string>(), // admin_contacts_add
            std::vector<std::string>(), // admin_contacts_rem
            std::vector<std::string>(), // tmpcontacts_rem
            std::vector<Epp::ENUMValidationExtension>(), // enum_validation_list
            info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_enum_domain_does_not_exist_exception
    );
}

bool fail_enum_domain_does_not_exist_wih_valexdate_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_enum_domain_does_not_exist_wih_valexdate, HasInfoDomainDataOfNonexistentEnumDomain)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::domain_update_impl(
            ctx,
            info_enum_domain_data_.fqdn,
            Optional<std::string>(), // registrant_chg
            Optional<std::string>(), // auth_info_pw_chg
            Optional<Nullable<std::string> >(), // nsset_chg
            Optional<Nullable<std::string> >(), // keyset_chg
            std::vector<std::string>(), // admin_contacts_add
            std::vector<std::string>(), // admin_contacts_rem
            std::vector<std::string>(), // tmpcontacts_rem
            std::vector<Epp::ENUMValidationExtension>(1,
                Epp::ENUMValidationExtension(
                    info_enum_domain_data_.enum_domain_validation.get_value().validation_expiration
                    + boost::gregorian::months(1)
                    , false)), // enum_validation_list
            info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_enum_domain_does_not_exist_wih_valexdate_exception
    );
}

bool fail_invalid_zone_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_invalid_zone, HasInfoDomainDataOfNonexistentDomain)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::domain_update_impl(
            ctx,
            info_domain_data_.fqdn + "c",
            Optional<std::string>(), // registrant_chg
            Optional<std::string>(), // auth_info_pw_chg
            Optional<Nullable<std::string> >(), // nsset_chg
            Optional<Nullable<std::string> >(), // keyset_chg
            std::vector<std::string>(), // admin_contacts_add
            std::vector<std::string>(), // admin_contacts_rem
            std::vector<std::string>(), // tmpcontacts_rem
            std::vector<Epp::ENUMValidationExtension>(), // enum_validation_list
            info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_invalid_zone_exception
    );
}

bool fail_wrong_registrar_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_wrong_registrar, HasInfoDomainDataAndDifferentInfoRegistrarData)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::update_domain(
            ctx,
            Epp::Domain::UpdateDomainInputData(
                info_domain_data_.fqdn,
                Optional<std::string>(), // registrant_chg
                Optional<std::string>(), // authinfopw_chg
                Optional<Nullable<std::string> >(), // nsset_chg
                Optional<Nullable<std::string> >(), // keyset_chg
                std::vector<std::string>(), // admin_contacts_add
                std::vector<std::string>(), // admin_contacts_rem
                std::vector<std::string>(), // tmpcontacts_rem
                std::vector<Epp::Domain::EnumValidationExtension>()), // enum_validation_list
            different_info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_wrong_registrar_exception
    );
}
bool fail_registrar_without_zone_access_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authorization_error);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_registrar_without_zone_access, HasInfoDomainDataWithInfoRegistrarDataOfRegistrarWithoutZoneAccess)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::update_domain(
            ctx,
            Epp::Domain::UpdateDomainInputData(
                info_domain_data_.fqdn,
                Optional<std::string>(), // registrant_chg
                Optional<std::string>(), // authinfopw_chg
                Optional<Nullable<std::string> >(), // nsset_chg
                Optional<Nullable<std::string> >(), // keyset_chg
                std::vector<std::string>(), // admin_contacts_add
                std::vector<std::string>(), // admin_contacts_rem
                std::vector<std::string>(), // tmpcontacts_rem
                std::vector<Epp::Domain::EnumValidationExtension>()), // enum_validation_list
            info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_registrar_without_zone_access_exception
    );
}
bool fail_prohibiting_status_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_prohibiting_status, HasInfoDomainDataWithServerUpdateProhibited)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::update_domain(
            ctx,
            Epp::Domain::UpdateDomainInputData(
                info_domain_data_.fqdn,
                Optional<std::string>(), // registrant_chg
                Optional<std::string>(), // authinfopw_chg
                Optional<Nullable<std::string> >(), // nsset_chg
                Optional<Nullable<std::string> >(), // keyset_chg
                std::vector<std::string>(), // admin_contacts_add
                std::vector<std::string>(), // admin_contacts_rem
                std::vector<std::string>(), // tmpcontacts_rem
                std::vector<Epp::Domain::EnumValidationExtension>()), // enum_validation_list
            info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_prohibiting_status_exception
    );
}
bool fail_invalid_handle_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_invalid_handle, HasInfoDomainDataOfDomainWithInvalidFqdn)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Domain::update_domain(
            ctx,
            Epp::Domain::UpdateDomainInputData(
                info_domain_data_.fqdn,
                Optional<std::string>(), // registrant_chg
                Optional<std::string>(), // authinfopw_chg
                Optional<Nullable<std::string> >(), // nsset_chg
                Optional<Nullable<std::string> >(), // keyset_chg
                std::vector<std::string>(), // admin_contacts_add
                std::vector<std::string>(), // admin_contacts_rem
                std::vector<std::string>(), // tmpcontacts_rem
                std::vector<Epp::Domain::EnumValidationExtension>()), // enum_validation_list
            info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_invalid_handle_exception
    );
}

BOOST_FIXTURE_TEST_CASE(nsset_change_should_clear_keyset, HasInfoDomainData)
{
    const Optional<Nullable<std::string> > nsset_chg = Optional<Nullable<std::string> >(Nullable<std::string>());
    const bool rifd_epp_update_domain_keyset_clear = true;

    Epp::Domain::update_domain(
        ctx,
        Epp::Domain::UpdateDomainInputData(
            info_domain_data_.fqdn,
            Optional<std::string>(), // registrant_chg
            Optional<std::string>(), // authinfopw_chg
            nsset_chg, // nsset_chg
            Optional<Nullable<std::string> >(), // keyset_chg
            std::vector<std::string>(), // admin_contacts_add
            std::vector<std::string>(), // admin_contacts_rem
            std::vector<std::string>(), // tmpcontacts_rem
            std::vector<Epp::Domain::EnumValidationExtension>()), // enum_validation_list
        info_registrar_data_.id, // registrar_id
        Optional<unsigned long long>(), // logd_request_id
        rifd_epp_update_domain_keyset_clear // rifd_epp_update_domain_keyset_clear
    );

    BOOST_CHECK(Fred::InfoDomainById(info_domain_data_.id).exec(ctx, "UTC").info_domain_data.nsset.isnull());
    BOOST_CHECK_EQUAL(Fred::InfoDomainById(info_domain_data_.id).exec(ctx, "UTC").info_domain_data.keyset.isnull(), rifd_epp_update_domain_keyset_clear);
}

BOOST_FIXTURE_TEST_CASE(nsset_change_should_not_clear_keyset, HasInfoDomainData)
{
    const Optional<Nullable<std::string> > nsset_chg = Optional<Nullable<std::string> >(Nullable<std::string>());
    const bool rifd_epp_update_domain_keyset_clear = false;

    Epp::Domain::update_domain(
        ctx,
        Epp::Domain::UpdateDomainInputData(
            info_domain_data_.fqdn,
            Optional<std::string>(), // registrant_chg
            Optional<std::string>(), // authinfopw_chg
            nsset_chg, // nsset_chg,
            Optional<Nullable<std::string> >(), // keyset_chg
            std::vector<std::string>(), // admin_contacts_add
            std::vector<std::string>(), // admin_contacts_rem
            std::vector<std::string>(), // tmpcontacts_rem
            std::vector<Epp::Domain::EnumValidationExtension>()), // enum_validation_list
        info_registrar_data_.id, // registrar_id
        Optional<unsigned long long>(), // logd_request_id
        rifd_epp_update_domain_keyset_clear // rifd_epp_update_domain_keyset_clear
    );

    Fred::InfoDomainData info_domain_data = Fred::InfoDomainById(info_domain_data_.id).exec(ctx, "UTC").info_domain_data;

    BOOST_CHECK(info_domain_data.nsset.isnull());
    BOOST_CHECK_EQUAL(info_domain_data.keyset.isnull(), rifd_epp_update_domain_keyset_clear);
}

bool fail_tmpcontacts_rem_not_empty_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_tmpcontacts_rem_not_empty, HasDataForUpdateDomain)
{
    BOOST_REQUIRE(!tmpcontacts_rem_.empty());

    BOOST_CHECK_EXCEPTION(
        Epp::Domain::update_domain(
            ctx,
            Epp::Domain::UpdateDomainInputData(
                info_domain_data_.fqdn,
                Optional<std::string>(), // registrant_chg
                Optional<std::string>(), // authinfopw_chg
                Optional<Nullable<std::string> >(), // nsset_chg
                Optional<Nullable<std::string> >(), // keyset_chg
                std::vector<std::string>(), // admin_contacts_add
                std::vector<std::string>(), // admin_contacts_rem
                tmpcontacts_rem_, // tmpcontacts_rem
                std::vector<Epp::Domain::EnumValidationExtension>()), // enum_validation_list
            info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            true // rifd_epp_update_domain_keyset_clear
        ),
        Epp::EppResponseFailure,
        fail_tmpcontacts_rem_not_empty_exception
    );
}

BOOST_FIXTURE_TEST_CASE(fail_existing_fqdn_but_spaces, HasDataForDomainUpdate)
{
    BOOST_REQUIRE(admin_contacts_add_.size() == 2);
    BOOST_REQUIRE(admin_contacts_rem_.size() == 1);
    const bool rifd_epp_update_domain_keyset_clear = false;

    const std::string fqdn_with_spaces("  " + info_domain_data_.fqdn + "  ");

    BOOST_CHECK_THROW(
        Epp::Domain::update_domain(
            ctx,
            fqdn_with_spaces,
            Optional<std::string>(new_registrant_handle_),
            Optional<std::string>(new_auth_info_pw_),
            Optional<Nullable<std::string> >(new_nsset_handle_),
            Optional<Nullable<std::string> >(new_keyset_handle_),
            admin_contacts_add_,
            admin_contacts_rem_,
            std::vector<std::string>(), // tmpcontacts_rem
            std::vector<Epp::ENUMValidationExtension>(), // enum_validation_list
            info_registrar_data_.id, // registrar_id
            Optional<unsigned long long>(), // logd_request_id
            rifd_epp_update_domain_keyset_clear // rifd_epp_update_domain_keyset_clear
        ),
        Epp::ObjectDoesNotExist
    );
}

BOOST_FIXTURE_TEST_CASE(ok, HasDataForUpdateDomain)
{
    BOOST_REQUIRE(admin_contacts_add_.size() == 2);
    BOOST_REQUIRE(admin_contacts_rem_.size() == 1);
    const bool rifd_epp_update_domain_keyset_clear = false;

    Epp::Domain::update_domain(
        ctx,
        Epp::Domain::UpdateDomainInputData(
            info_domain_data_.fqdn,
            registrant_chg_, // registrant_chg
            authinfopw_chg_, // authinfopw_chg
            nsset_chg_, // nsset_chg
            keyset_chg_, // keyset_chg
            admin_contacts_add_, // admin_contacts_add
            admin_contacts_rem_, // admin_contacts_rem
            std::vector<std::string>(), // tmpcontacts_rem
            enum_validation_list_), // enum_validation_list
        info_registrar_data_.id, // registrar_id
        Optional<unsigned long long>(), // logd_request_id
        rifd_epp_update_domain_keyset_clear // rifd_epp_update_domain_keyset_clear
    );

    Fred::InfoDomainData info_domain_data = Fred::InfoDomainByHandle(info_domain_data_.fqdn).exec(ctx, "UTC").info_domain_data;

    BOOST_CHECK_EQUAL(info_domain_data.roid, info_domain_data_.roid);
    BOOST_CHECK_EQUAL(info_domain_data.fqdn, info_domain_data_.fqdn);

    BOOST_CHECK_EQUAL(info_domain_data.registrant.handle, registrant_chg_);

    BOOST_CHECK_EQUAL(info_domain_data.nsset.get_value_or_default().handle, nsset_chg_.get_value_or_default());
    BOOST_CHECK_EQUAL(info_domain_data.keyset.get_value_or_default().handle, keyset_chg_.get_value_or_default());
    // states
    BOOST_CHECK_EQUAL(info_domain_data.sponsoring_registrar_handle, info_domain_data_.sponsoring_registrar_handle);
    BOOST_CHECK_EQUAL(info_domain_data.create_registrar_handle, info_domain_data_.create_registrar_handle);

    BOOST_CHECK_EQUAL(info_domain_data.update_registrar_handle, info_registrar_data_.handle);

    BOOST_CHECK_EQUAL(info_domain_data.creation_time, info_domain_data_.creation_time);
    //BOOST_CHECK_EQUAL(info_domain_data.last_update, info_domain_data_.update_time);
    BOOST_CHECK_EQUAL(info_domain_data.transfer_time, info_domain_data_.transfer_time);
    BOOST_CHECK_EQUAL(info_domain_data.expiration_date, info_domain_data_.expiration_date);
    BOOST_CHECK_EQUAL(info_domain_data.authinfopw, authinfopw_chg_);

    std::vector<std::string> info_domain_data_admin_contacts = vector_of_Fred_ObjectIdHandlePair_to_vector_of_string(info_domain_data.admin_contacts);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        info_domain_data_admin_contacts.begin(),
        info_domain_data_admin_contacts.end(),
        admin_contacts_add_.begin(),
        admin_contacts_add_.end()
    );

    BOOST_CHECK_EQUAL(
       info_domain_data.enum_domain_validation.get_value_or_default().validation_expiration,
       info_domain_data_.enum_domain_validation.get_value_or_default().validation_expiration
   );

    BOOST_CHECK_EQUAL(
       info_domain_data.enum_domain_validation.get_value_or_default().publish,
       info_domain_data_.enum_domain_validation.get_value_or_default().publish
   );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
