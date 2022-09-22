/*
 * Copyright (C) 2008-2022  CZ.NIC, z. s. p. o.
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

#include "test/backend/epp/keyset/fixture.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/keyset/check_keyset.hh"
#include "src/backend/epp/keyset/create_keyset.hh"
#include "src/backend/epp/keyset/delete_keyset.hh"
#include "src/backend/epp/keyset/impl/limits.hh"
#include "src/backend/epp/keyset/info_keyset.hh"
#include "libfred/registrar/create_registrar.hh"

#include <sstream>
#include <limits>
#include <map>

#include <boost/mpl/assert.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Keyset)
BOOST_AUTO_TEST_SUITE(CreateKeyset)

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
const ::LibFred::InfoRegistrarData& get_registrar(const ObjectsProvider&);

template < >
const ::LibFred::InfoRegistrarData& get_registrar< Registrar::A >(const ObjectsProvider &objects_provider)
{
    return objects_provider.get_registrar_a();
}

template < >
const ::LibFred::InfoRegistrarData& get_registrar< Registrar::B >(const ObjectsProvider &objects_provider)
{
    return objects_provider.get_registrar_b();
}

template < >
const ::LibFred::InfoRegistrarData& get_registrar< Registrar::SYS >(const ObjectsProvider &objects_provider)
{
    return objects_provider.get_sys_registrar();
}

template < Registrar::Enum REGISTRAR >
unsigned long long get_registrar_id(const ObjectsProvider &objects_provider)
{
    return get_registrar< REGISTRAR >(objects_provider).id;
}

template < Registrar::Enum REGISTRAR >
std::string get_registrar_handle(const ObjectsProvider &objects_provider)
{
    return get_registrar< REGISTRAR >(objects_provider).handle;
}

struct KeysetCreateData
{
    std::string keyset_handle;
    unsigned long long registrar_id;
    std::string registrar_handle;
    std::vector< std::string > tech_contacts;
    std::vector< ::Epp::Keyset::DsRecord > ds_records;
    std::vector< ::Epp::Keyset::DnsKey > dns_keys;
};

template < Registrar::Enum REGISTRAR >
KeysetCreateData create_successfully_by(const ObjectsProvider &objects_provider)
{
    try {
        ::LibFred::OperationContextCreator ctx;
        KeysetCreateData data;
        data.registrar_id = get_registrar_id< REGISTRAR >(objects_provider);
        BOOST_REQUIRE(0 < data.registrar_id);
        data.keyset_handle = objects_provider.get_keyset_handle< ::LibFred::Keyset::HandleState::available,
                                                                 ::LibFred::Keyset::HandleState::valid >(ctx);
        data.registrar_handle = get_registrar_handle< REGISTRAR >(objects_provider);
        static const unsigned number_of_contacts = (1 + ::Epp::Keyset::max_number_of_tech_contacts) / 2;
        data.tech_contacts.reserve(number_of_contacts);
        for (unsigned idx = 0; idx < number_of_contacts; ++idx) {
            data.tech_contacts.push_back(objects_provider.get_contact(idx).handle);
        }
        static const unsigned long long logd_request_id = 12345;
        data.dns_keys.push_back(::Epp::Keyset::DnsKey(0, 3, 8, "bla="));
        BOOST_CHECK(is_nondecreasing(::Epp::Keyset::min_number_of_dns_keys, data.dns_keys.size(), ::Epp::Keyset::max_number_of_dns_keys));
        const ::Epp::Keyset::CreateKeysetResult result = ::Epp::Keyset::create_keyset(
                ctx,
                ::Epp::Keyset::CreateKeysetInputData(
                        data.keyset_handle,
                        data.tech_contacts,
                        data.ds_records,
                        data.dns_keys),
                DefaultCreateKeysetConfigData(),
                ::Epp::SessionData(
                        data.registrar_id,
                        ::Epp::SessionLang::en,
                        "",
                        boost::optional<unsigned long long>(logd_request_id)));
        BOOST_REQUIRE(0 < result.id);
        ctx.commit_transaction();
        return data;
    }
    catch (const ::Epp::EppResponseFailure& e) {
        std::cout << "catch: " << e.what() << std::endl;
        throw;
    }
    catch (const std::exception& e) {
        std::cout << "catch: " << e.what() << std::endl;
        throw;
    }
    catch (...) {
        std::cout << "catch: unknown error" << std::endl;
        throw;
    }
}

void check_created_keyset(const KeysetCreateData &data)
{
    try {
        ::LibFred::OperationContextCreator ctx;
        BOOST_REQUIRE(data.registrar_id != 0);
        const ::Epp::Keyset::InfoKeysetOutputData info_data =
            ::Epp::Keyset::info_keyset(
                    ctx,
                    data.keyset_handle,
                    DefaultInfoKeysetConfigData(),
                    ::Epp::Password{},
                    DefaultSessionData().set_registrar_id(data.registrar_id));
        ctx.commit_transaction();
        BOOST_CHECK(info_data.handle == data.keyset_handle);
        BOOST_CHECK(info_data.creating_registrar_handle == data.registrar_handle);
        BOOST_CHECK(info_data.sponsoring_registrar_handle == data.registrar_handle);
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
        for (std::vector< ::Epp::Keyset::DnsKey >::const_iterator dns_key_ptr = data.dns_keys.begin();
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

bool create_by_invalid_registrar_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

void create_by_invalid_registrar(const KeysetCreateData &data)
{
    ::LibFred::OperationContextCreator ctx;
    static const unsigned long long invalid_registrar_id = 0;
    static const unsigned long long logd_request_id = 12346;
    BOOST_CHECK_EXCEPTION(
            ::Epp::Keyset::create_keyset(
                    ctx,
                    ::Epp::Keyset::CreateKeysetInputData(
                            data.keyset_handle,
                            data.tech_contacts,
                            data.ds_records,
                            data.dns_keys),
                    DefaultCreateKeysetConfigData(),
                    ::Epp::SessionData(
                            invalid_registrar_id,
                            ::Epp::SessionLang::en,
                            "",
                            boost::optional<unsigned long long>(logd_request_id))),
            ::Epp::EppResponseFailure,
            create_by_invalid_registrar_exception);
}

bool create_with_correct_data_but_registered_handle_by_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_exists);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(::Epp::has_extended_error_with_param_reason(e.epp_result(), ::Epp::Param::keyset_handle, ::Epp::Reason::existing));
    return true;
}

template < Registrar::Enum REGISTRAR >
void create_with_correct_data_but_registered_handle_by(const KeysetCreateData &data,
                                                       const ObjectsProvider &objects_provider)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE(::LibFred::Keyset::get_handle_registrability(ctx, data.keyset_handle) ==
                  ::LibFred::Keyset::HandleState::registered);
    static const unsigned long long logd_request_id = 12346;
    const unsigned long long registrar_id = get_registrar_id< REGISTRAR >(objects_provider);
    BOOST_CHECK_EXCEPTION(
            ::Epp::Keyset::create_keyset(
                    ctx,
                    ::Epp::Keyset::CreateKeysetInputData(
                            data.keyset_handle,
                            data.tech_contacts,
                            data.ds_records,
                            data.dns_keys),
                    DefaultCreateKeysetConfigData(),
                    ::Epp::SessionData(
                            registrar_id,
                            ::Epp::SessionLang::en,
                            "",
                            boost::optional<unsigned long long>(logd_request_id))),
            ::Epp::EppResponseFailure,
            create_with_correct_data_but_registered_handle_by_exception);
}

template < Registrar::Enum REGISTRAR >
KeysetCreateData create_protected_period_by(const ObjectsProvider &objects_provider)
{
    try {
        const KeysetCreateData data = CreateKeyset::create_successfully_by< REGISTRAR >(objects_provider);
        check_created_keyset(data);
        ::LibFred::OperationContextCreator ctx;
        const unsigned long long delete_result =
                ::Epp::Keyset::delete_keyset(
                        ctx,
                        data.keyset_handle,
                        DefaultDeleteKeysetConfigData(),
                        DefaultSessionData().set_registrar_id(data.registrar_id));
        BOOST_REQUIRE(0 < delete_result);
        ctx.commit_transaction();
        return data;
    }
    catch (const ::Epp::EppResponseFailure& e) {
        std::cout << "catch: " << e.what() << std::endl;
        throw;
    }
    catch (const std::exception& e) {
        std::cout << "catch: " << e.what() << std::endl;
        throw;
    }
    catch (...) {
        std::cout << "catch: unknown error" << std::endl;
        throw;
    }
}

template < Registrar::Enum REGISTRAR >
void check_protected_period_keyset(const std::string &keyset_handle, const ObjectsProvider &objects_provider)
{
    try {
        ::LibFred::OperationContextCreator ctx;
        std::set< std::string > handles;
        handles.insert(keyset_handle);
        const unsigned long long registrar_id = get_registrar_id< REGISTRAR >(objects_provider);
        const std::map<std::string, Nullable< ::Epp::Keyset::KeysetHandleRegistrationObstruction::Enum> > check_result =
                ::Epp::Keyset::check_keyset(
                        ctx,
                        handles,
                        DefaultCheckKeysetConfigData(),
                        DefaultSessionData().set_registrar_id(registrar_id));
        ctx.commit_transaction();
        BOOST_REQUIRE(check_result.size() == 1);
        BOOST_REQUIRE(check_result.count(keyset_handle) == 1);
        BOOST_REQUIRE(!check_result.find(keyset_handle)->second.isnull());
        BOOST_CHECK(check_result.find(keyset_handle)->second.get_value() == ::Epp::Keyset::KeysetHandleRegistrationObstruction::protected_handle);
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

bool create_with_correct_data_but_protected_handle_by_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(::Epp::has_extended_error_with_param_reason(e.epp_result(), ::Epp::Param::keyset_handle, ::Epp::Reason::protected_period));
    return true;
}

template < Registrar::Enum REGISTRAR >
void create_with_correct_data_but_protected_handle_by(const KeysetCreateData &data,
                                                      const ObjectsProvider &objects_provider)
{
    ::LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE(::LibFred::Keyset::get_handle_registrability(ctx, data.keyset_handle) ==
                  ::LibFred::Keyset::HandleState::in_protection_period);
    static const unsigned long long logd_request_id = 12347;
    const unsigned long long registrar_id = get_registrar_id< REGISTRAR >(objects_provider);
    BOOST_CHECK_EXCEPTION(
            ::Epp::Keyset::create_keyset(
                    ctx,
                    ::Epp::Keyset::CreateKeysetInputData(
                            data.keyset_handle,
                            data.tech_contacts,
                            data.ds_records,
                            data.dns_keys),
                    DefaultCreateKeysetConfigData(),
                    ::Epp::SessionData(
                            registrar_id,
                            ::Epp::SessionLang::en,
                            "",
                            boost::optional<unsigned long long>(logd_request_id))),
            ::Epp::EppResponseFailure,
            create_with_correct_data_but_protected_handle_by_exception);
}

bool create_with_correct_data_but_invalid_handle_by_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(::Epp::has_extended_error_with_param_reason(e.epp_result(), ::Epp::Param::keyset_handle, ::Epp::Reason::bad_format_keyset_handle));
    return true;
}

template < Registrar::Enum REGISTRAR >
void create_with_correct_data_but_invalid_handle_by(const KeysetCreateData &data,
                                                    const ObjectsProvider &objects_provider)
{
    const std::string invalid_keyset_handle = data.keyset_handle + "-";
    ::LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE(::LibFred::Keyset::get_handle_syntax_validity(ctx, invalid_keyset_handle) ==
                  ::LibFred::Keyset::HandleState::invalid);
    static const unsigned long long logd_request_id = 12347;
    const unsigned long long registrar_id = get_registrar_id< REGISTRAR >(objects_provider);
    BOOST_CHECK_EXCEPTION(
            ::Epp::Keyset::create_keyset(
                    ctx,
                    ::Epp::Keyset::CreateKeysetInputData(
                            invalid_keyset_handle,
                            data.tech_contacts,
                            data.ds_records,
                            data.dns_keys),
                    DefaultCreateKeysetConfigData(),
                    ::Epp::SessionData(
                            registrar_id,
                            ::Epp::SessionLang::en,
                            "",
                            boost::optional<unsigned long long>(logd_request_id))),
            ::Epp::EppResponseFailure,
            create_with_correct_data_but_invalid_handle_by_exception);
}

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(create, ObjectsProvider)
{
    const KeysetCreateData data_a = CreateKeyset::create_successfully_by< Registrar::A >(*this);
    check_created_keyset(data_a);

    const KeysetCreateData data_b = CreateKeyset::create_successfully_by< Registrar::B >(*this);
    check_created_keyset(data_b);

    const KeysetCreateData data_sys = CreateKeyset::create_successfully_by< Registrar::SYS >(*this);
    check_created_keyset(data_sys);

    create_by_invalid_registrar(data_a);
    create_by_invalid_registrar(data_b);
    create_by_invalid_registrar(data_sys);
    create_by_invalid_registrar(KeysetCreateData());

    create_with_correct_data_but_registered_handle_by< Registrar::A >(data_a, *this);
    create_with_correct_data_but_registered_handle_by< Registrar::B >(data_a, *this);
    create_with_correct_data_but_registered_handle_by< Registrar::SYS >(data_a, *this);
    create_with_correct_data_but_registered_handle_by< Registrar::A >(data_b, *this);
    create_with_correct_data_but_registered_handle_by< Registrar::B >(data_b, *this);
    create_with_correct_data_but_registered_handle_by< Registrar::SYS >(data_b, *this);
    create_with_correct_data_but_registered_handle_by< Registrar::A >(data_sys, *this);
    create_with_correct_data_but_registered_handle_by< Registrar::B >(data_sys, *this);
    create_with_correct_data_but_registered_handle_by< Registrar::SYS >(data_sys, *this);

    const KeysetCreateData protected_data_a = CreateKeyset::create_protected_period_by< Registrar::A >(*this);
    check_protected_period_keyset< Registrar::A >(protected_data_a.keyset_handle, *this);
    const KeysetCreateData protected_data_b = CreateKeyset::create_protected_period_by< Registrar::B >(*this);
    check_protected_period_keyset< Registrar::B >(protected_data_b.keyset_handle, *this);
    const KeysetCreateData protected_data_sys = CreateKeyset::create_protected_period_by< Registrar::SYS >(*this);
    check_protected_period_keyset< Registrar::SYS >(protected_data_sys.keyset_handle, *this);

    create_with_correct_data_but_protected_handle_by< Registrar::A >(protected_data_a, *this);
    create_with_correct_data_but_protected_handle_by< Registrar::B >(protected_data_a, *this);
    create_with_correct_data_but_protected_handle_by< Registrar::SYS >(protected_data_a, *this);
    create_with_correct_data_but_protected_handle_by< Registrar::A >(protected_data_b, *this);
    create_with_correct_data_but_protected_handle_by< Registrar::B >(protected_data_b, *this);
    create_with_correct_data_but_protected_handle_by< Registrar::SYS >(protected_data_b, *this);
    create_with_correct_data_but_protected_handle_by< Registrar::A >(protected_data_sys, *this);
    create_with_correct_data_but_protected_handle_by< Registrar::B >(protected_data_sys, *this);
    create_with_correct_data_but_protected_handle_by< Registrar::SYS >(protected_data_sys, *this);

    create_with_correct_data_but_invalid_handle_by< Registrar::A >(data_a, *this);
    create_with_correct_data_but_invalid_handle_by< Registrar::B >(data_a, *this);
    create_with_correct_data_but_invalid_handle_by< Registrar::SYS >(data_a, *this);
    create_with_correct_data_but_invalid_handle_by< Registrar::A >(data_b, *this);
    create_with_correct_data_but_invalid_handle_by< Registrar::B >(data_b, *this);
    create_with_correct_data_but_invalid_handle_by< Registrar::SYS >(data_b, *this);
    create_with_correct_data_but_invalid_handle_by< Registrar::A >(data_sys, *this);
    create_with_correct_data_but_invalid_handle_by< Registrar::B >(data_sys, *this);
    create_with_correct_data_but_invalid_handle_by< Registrar::SYS >(data_sys, *this);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Keyset/CreateKeyset
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Keyset
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

} // namespace Test
