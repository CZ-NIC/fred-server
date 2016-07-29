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

#include "tests/interfaces/epp/keyset/fixture.h"
#include "src/epp/parameter_errors.h"
#include "src/epp/keyset/create.h"
#include "src/epp/keyset/info.h"
#include "src/epp/keyset/limits.h"
#include "src/fredlib/registrar/create_registrar.h"

#include <sstream>
#include <limits>
#include <map>

#include <boost/mpl/assert.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Keyset)

namespace {

template < class T0, class T1 >
bool is_nondecreasing(T0 a0, T1 a1)
{
    BOOST_MPL_ASSERT_MSG(std::numeric_limits< T0 >::is_signed == std::numeric_limits< T1 >::is_signed,
                         do_not_mix_signed_and_unsigned_types, (T0, T1));
    return a0 <= a1;
}

template < class T0, class T1, class T2 >
bool is_nondecreasing(T0 a0, T1 a1, T2 a2)
{
    return is_nondecreasing(a0, a1) && is_nondecreasing(a1, a2);
}

struct Registrar
{
    enum Enum
    {
        A,
        B,
        SYS
    };
};

template < Registrar::Enum REGISTRAR >
const Fred::InfoRegistrarData& get_registrar(const Test::ObjectsProvider&);

template < >
const Fred::InfoRegistrarData& get_registrar< Registrar::A >(const Test::ObjectsProvider &objects_provider)
{
    return objects_provider.get_registrar_a();
}

template < >
const Fred::InfoRegistrarData& get_registrar< Registrar::B >(const Test::ObjectsProvider &objects_provider)
{
    return objects_provider.get_registrar_b();
}

template < >
const Fred::InfoRegistrarData& get_registrar< Registrar::SYS >(const Test::ObjectsProvider &objects_provider)
{
    return objects_provider.get_sys_registrar();
}

template < Registrar::Enum REGISTRAR >
unsigned long long get_registrar_id(const Test::ObjectsProvider &objects_provider)
{
    return get_registrar< REGISTRAR >(objects_provider).id;
}

template < Registrar::Enum REGISTRAR >
std::string get_registrar_handle(const Test::ObjectsProvider &objects_provider)
{
    return get_registrar< REGISTRAR >(objects_provider).handle;
}

struct KeySetCreateData
{
    std::string keyset_handle;
    unsigned long long registrar_id;
    std::string registrar_handle;
    Optional< std::string > auth_info_pw;
    std::vector< std::string > tech_contacts;
    std::vector< Epp::KeySet::DsRecord > ds_records;
    std::vector< Epp::KeySet::DnsKey > dns_keys;
};

template < Registrar::Enum REGISTRAR >
KeySetCreateData create_successfully_by(const Test::ObjectsProvider &objects_provider)
{
    try {
        Fred::OperationContextCreator ctx;
        KeySetCreateData data;
        data.registrar_id = get_registrar_id< REGISTRAR >(objects_provider);
        BOOST_REQUIRE(0 < data.registrar_id);
        data.keyset_handle = objects_provider.get_keyset_handle< Fred::KeySet::HandleState::available,
                                                                 Fred::KeySet::HandleState::valid >(ctx);
        data.registrar_handle = get_registrar_handle< REGISTRAR >(objects_provider);
        static const unsigned number_of_contacts = (1 + Epp::KeySet::max_number_of_tech_contacts) / 2;
        data.tech_contacts.reserve(number_of_contacts);
        for (unsigned idx = 0; idx < number_of_contacts; ++idx) {
            data.tech_contacts.push_back(objects_provider.get_contact(idx).handle);
        }
        static const unsigned long long logd_request_id = 12345;
        data.dns_keys.push_back(Epp::KeySet::DnsKey(0, 3, 1, "bla="));
        BOOST_CHECK(is_nondecreasing(Epp::KeySet::min_number_of_dns_keys, data.dns_keys.size(), Epp::KeySet::max_number_of_dns_keys));
        const Epp::KeysetCreateResult result = Epp::keyset_create(
            ctx,
            data.keyset_handle,
            data.auth_info_pw,
            data.tech_contacts,
            data.ds_records,
            data.dns_keys,
            data.registrar_id,
            logd_request_id);
        BOOST_REQUIRE(0 < result.id);
        ctx.commit_transaction();
        return data;
    }
    catch (const Epp::AuthErrorServerClosingConnection&) {
        std::cout << "catch: AuthErrorServerClosingConnection" << std::endl;
        throw;
    }
    catch (const Epp::ParameterErrors&) {
        std::cout << "catch: ParameterErrors" << std::endl;
        throw;
    }
    catch (const Fred::CreateKeyset::Exception&) {
        std::cout << "catch: Fred::CreateKeyset::Exception" << std::endl;
        throw;
    }
    catch (const std::exception &e) {
        std::cout << "catch: " << e.what() << std::endl;
        throw;
    }
    catch (...) {
        std::cout << "catch: unknown error" << std::endl;
        throw;
    }
}

void check_created_keyset(const KeySetCreateData &data)
{
    try {
        Fred::OperationContextCreator ctx;
        const Epp::KeysetInfoData info_data = Epp::keyset_info(ctx, data.keyset_handle, data.registrar_id);
        ctx.commit_transaction();
        BOOST_CHECK(info_data.handle == data.keyset_handle);
        BOOST_CHECK(info_data.creating_registrar_handle == data.registrar_handle);
        BOOST_CHECK(info_data.sponsoring_registrar_handle == data.registrar_handle);
        BOOST_REQUIRE(!info_data.auth_info_pw.isnull());
        BOOST_CHECK(!info_data.auth_info_pw.get_value().empty());
        BOOST_CHECK(!data.auth_info_pw.isset() || (info_data.auth_info_pw.get_value() == data.auth_info_pw.get_value()));
        BOOST_CHECK(info_data.tech_contacts.size() == data.tech_contacts.size());
        BOOST_CHECK(!info_data.tech_contacts.empty());
        for (std::vector< std::string >::const_iterator tech_contact_ptr = data.tech_contacts.begin();
             tech_contact_ptr != data.tech_contacts.end(); ++tech_contact_ptr)
        {
            BOOST_CHECK(info_data.tech_contacts.count(*tech_contact_ptr) == 1);
        }
        BOOST_CHECK(info_data.ds_records.empty());
        BOOST_CHECK(data.ds_records.empty());
        BOOST_CHECK(info_data.dns_keys.size() == data.dns_keys.size());
        BOOST_CHECK(!info_data.dns_keys.empty());
        for (std::vector< Epp::KeySet::DnsKey >::const_iterator dns_key_ptr = data.dns_keys.begin();
             dns_key_ptr != data.dns_keys.end(); ++dns_key_ptr)
        {
            BOOST_CHECK(info_data.dns_keys.count(*dns_key_ptr) == 1);
        }
        return;
    }
    catch (const std::exception &e) {
        std::cout << "catch: " << e.what() << std::endl;
        throw;
    }
    catch (...) {
        std::cout << "catch: unknown error" << std::endl;
        throw;
    }
}

void create_by_invalid_registrar(const KeySetCreateData &data)
{
    Fred::OperationContextCreator ctx;
    static const unsigned long long invalid_registrar_id = 0;
    static const unsigned long long logd_request_id = 12346;
    BOOST_CHECK_THROW(
        Epp::keyset_create(
            ctx,
            data.keyset_handle,
            data.auth_info_pw,
            data.tech_contacts,
            data.ds_records,
            data.dns_keys,
            invalid_registrar_id,
            logd_request_id),
        Epp::AuthErrorServerClosingConnection);
}

bool equal(const Epp::ParameterErrors &a, const Epp::ParameterErrors &b)
{
    const std::set< Epp::Error > errors_a = a.get_set_of_error();
    const std::set< Epp::Error > errors_b = b.get_set_of_error();
    if (errors_a.size() != errors_b.size()) {
        return false;
    }
    for (std::set< Epp::Error >::const_iterator error_a_ptr = errors_a.begin();
         error_a_ptr != errors_a.end(); ++error_a_ptr)
    {
        if (errors_b.count(*error_a_ptr) != 1) {
            return false;
        }
    }
    return true;
}

void create_with_correct_data_but_registered_handle(const KeySetCreateData &data)
{
    Fred::OperationContextCreator ctx;
    BOOST_REQUIRE(Fred::KeySet::get_handle_registrability(ctx, data.keyset_handle) ==
                  Fred::KeySet::HandleState::registered);
    static const unsigned long long logd_request_id = 12346;
    try {
        Epp::keyset_create(
            ctx,
            data.keyset_handle,
            data.auth_info_pw,
            data.tech_contacts,
            data.ds_records,
            data.dns_keys,
            data.registrar_id,
            logd_request_id);
        BOOST_CHECK(false);
    }
    catch (const Epp::ParameterErrors &e) {
        Epp::ParameterErrors expected_errors;
        expected_errors.add_scalar_parameter_error(Epp::Param::keyset_handle, Epp::Reason::existing);
        BOOST_CHECK(equal(e, expected_errors));
    }
}

}

BOOST_FIXTURE_TEST_CASE(create, Test::ObjectsProvider)
{
    const KeySetCreateData data_a = Keyset::create_successfully_by< Registrar::A >(*this);
    check_created_keyset(data_a);

    const KeySetCreateData data_b = Keyset::create_successfully_by< Registrar::B >(*this);
    check_created_keyset(data_b);

    const KeySetCreateData data_sys = Keyset::create_successfully_by< Registrar::SYS >(*this);
    check_created_keyset(data_sys);

    create_by_invalid_registrar(data_a);
    create_by_invalid_registrar(data_b);
    create_by_invalid_registrar(data_sys);
    create_by_invalid_registrar(KeySetCreateData());

    create_with_correct_data_but_registered_handle(data_a);
    create_with_correct_data_but_registered_handle(data_b);
    create_with_correct_data_but_registered_handle(data_sys);
}

BOOST_AUTO_TEST_SUITE_END();
