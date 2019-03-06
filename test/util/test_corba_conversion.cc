/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
#include "config.h"
#include "libfred/opcontext.hh"
#include "src/bin/cli/read_config_file.hh"
#include "util/util.hh"
#include "util/map_at.hh"
#include "src/util/subprocess.hh"
#include "util/printable.hh"
#include "util/random_data_generator.hh"
#include "src/util/cfg/config_handler_decl.hh"

#include "test/setup/fixtures.hh"

#include "src/util/corba_conversion.hh"

#include "src/bin/corba/corba_conversion_test.hh"
#include "src/bin/corba/util/corba_conversions_isodate.hh"
#include "src/bin/corba/util/corba_conversions_isodatetime.hh"
#include "src/bin/corba/util/corba_conversions_nullableisodate.hh"
#include "src/bin/corba/mojeid/mojeid_corba_conversion.hh"
#include "src/bin/corba/util/corba_conversions_buffer.hh"

#include "src/bin/corba/Buffer.hh"
#include "src/backend/buffer.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <limits>
#include <fstream>
#include <ios>
#include <iterator>
#include <iomanip>
#include <sstream>
#include <utility>

namespace Test {
namespace CorbaConversion {

void unwrap_NullableString(const NullableString *src_ptr, Nullable< std::string > &dst)
{
    if (src_ptr == NULL) {
        dst = Nullable< std::string >();
    }
    else {
        dst = src_ptr->_boxed_in();
    }
}

NullableString_var wrap_Nullable_string(const Nullable< std::string > &src)
{
    return src.isnull() ? NullableString_var()
                        : NullableString_var(new NullableString(src.get_value().c_str()));
}

CORBA::StringSeq_var wrap_into_CORBA_StringSeq(const std::vector< std::string > &src)
{
    CORBA::StringSeq_var result(new CORBA::StringSeq());

    result->length(src.size());
    ::size_t dst_idx = 0;
    for (std::vector< std::string >::const_iterator src_ptr = src.begin() ; src_ptr != src.end();
         ++src_ptr, ++dst_idx)
    {
        result->operator[](dst_idx) = ::CorbaConversion::wrap_string(*src_ptr)._retn();
    }
    return result._retn();
}

StringSeq_var wrap_into_Test_StringSeq(const std::vector< std::string > &src)
{
    StringSeq_var result(new StringSeq());

    result->length(src.size());
    ::size_t dst_idx = 0;
    for (std::vector< std::string >::const_iterator src_ptr = src.begin() ; src_ptr != src.end();
         ++src_ptr, ++dst_idx)
    {
        result->operator[](dst_idx) = ::CorbaConversion::wrap_string(*src_ptr)._retn();
    }
    return result._retn();
}

} // namespace Test::CorbaConversion
} // namespace Test

BOOST_AUTO_TEST_SUITE(TestCorbaConversion)

const std::string server_name = "test-corba-conversion";

BOOST_AUTO_TEST_CASE(test_wrap_string)
{
    static const char *const text = "test";
    const CORBA::String_var sv1 = CorbaConversion::wrap_string(std::string(text))._retn();
    BOOST_CHECK(sv1.in() == std::string(text));
    const std::string ss1 = sv1.in();
    BOOST_CHECK(ss1 == text);
}

BOOST_AUTO_TEST_CASE(test_string_seq_wrap_ref)
{
    static const std::vector< std::string > vs1 = Util::vector_of< std::string >("test1")("test2")("test3");
    CORBA::StringSeq_var ssv1 = Test::CorbaConversion::wrap_into_CORBA_StringSeq(vs1)._retn();

    BOOST_CHECK(ssv1->length() == vs1.size());

    CORBA::String_var sv0;
    sv0 = ssv1[0];
    BOOST_CHECK(sv0.in() == vs1[0]);

    const CORBA::String_var sv1 = ssv1[1];
    BOOST_CHECK(sv1.in() == vs1[1]);

    BOOST_CHECK(ssv1[2].in() == vs1[2]);
}

BOOST_AUTO_TEST_CASE(test_string_seq_wrap_test)
{
    static const std::vector< std::string > vs1 = Util::vector_of< std::string >("test1")("test2")("test3");
    Test::StringSeq_var ssv1 = Test::CorbaConversion::wrap_into_Test_StringSeq(vs1)._retn();

    BOOST_CHECK(ssv1->length() == vs1.size());

    CORBA::String_var sv0;
    sv0 = ssv1[0];
    BOOST_CHECK(sv0.in() == vs1[0]);

    const CORBA::String_var sv1 = ssv1[1];
    BOOST_CHECK(sv1.in() == vs1[1]);

    BOOST_CHECK(ssv1[2].in() == vs1[2]);
}

BOOST_AUTO_TEST_CASE(test_valuetype_string)
{
    static const Nullable< std::string > ns1("test1");
    const Test::NullableString_var tnsv1 = Test::CorbaConversion::wrap_Nullable_string(ns1)._retn();
    BOOST_CHECK(tnsv1->_value() == ns1.get_value());

    Nullable< std::string > ns2;
    Test::CorbaConversion::unwrap_NullableString(tnsv1.in(), ns2);
    BOOST_CHECK(tnsv1->_value() == ns2.get_value());
}

BOOST_AUTO_TEST_CASE(test_mojeid_valuetype_string)
{
    const Nullable< std::string > ns1("test1");
    const Registry::MojeID::NullableString_var tnsv1 = CorbaConversion::wrap_Nullable_string(ns1)._retn();
    BOOST_CHECK(tnsv1->_value() == ns1.get_value());

    Nullable< std::string > ns2;
    CorbaConversion::unwrap_NullableString(tnsv1.in(), ns2);
    BOOST_CHECK(tnsv1->_value() == ns2.get_value());
}

BOOST_AUTO_TEST_CASE(test_mojeid_date)
{
    const boost::gregorian::date date(2015,12,10);
    const std::string date_as_string = boost::gregorian::to_iso_extended_string(date);
    BOOST_CHECK(date_as_string == "2015-12-10");

    Registry::IsoDate idl_date;
    CorbaConversion::Util::wrap_boost_gregorian_date_to_IsoDate(date, idl_date);
    BOOST_CHECK(idl_date.value.in() == date_as_string);

    Fred::Backend::MojeIdImplData::Birthdate impl_date;
    CorbaConversion::unwrap_Date_to_Birthdate(idl_date, impl_date);
    BOOST_CHECK(impl_date.value == date_as_string);

    BOOST_CHECK_THROW(CorbaConversion::Util::wrap_boost_gregorian_date_to_IsoDate(boost::gregorian::date(), idl_date),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_mojeid_datetime)
{
    const boost::posix_time::ptime time = boost::posix_time::ptime(boost::gregorian::date(2015,12,10));
    const std::string time_as_string = boost::posix_time::to_iso_extended_string(time) + "Z";
    BOOST_CHECK(time_as_string == "2015-12-10T00:00:00Z");

    Registry::IsoDateTime idl_time = CorbaConversion::Util::wrap_boost_posix_time_ptime_to_IsoDateTime(time);
    BOOST_CHECK(idl_time.value.in() == time_as_string);

    BOOST_CHECK_THROW(CorbaConversion::Util::wrap_boost_posix_time_ptime_to_IsoDateTime(boost::posix_time::ptime()), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_mojeid_nullabledate)
{
    const boost::gregorian::date date(2015,12,10);
    const std::string date_as_string = boost::gregorian::to_iso_extended_string(date);
    BOOST_CHECK(date_as_string == "2015-12-10");
    const Nullable< boost::gregorian::date > nnd(date);

    const Registry::NullableIsoDate_var nd1 = CorbaConversion::Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(nnd)._retn();
    BOOST_CHECK(std::string(nd1->value()) == date_as_string);

    const Nullable< boost::gregorian::date > nd;
    const Registry::NullableIsoDate_var nd2 = CorbaConversion::Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(nd)._retn();
    BOOST_CHECK(nd2.in() == NULL);

    Nullable< Fred::Backend::MojeIdImplData::Birthdate > res1;
    CorbaConversion::unwrap_NullableIsoDate_to_Birthdate(nd1.in(), res1);
    BOOST_CHECK(!res1.isnull());
    BOOST_CHECK(res1.get_value().value == date_as_string);

    Nullable< Fred::Backend::MojeIdImplData::Birthdate > res2;
    CorbaConversion::unwrap_NullableIsoDate_to_Birthdate(nd2.in(), res2);
    BOOST_CHECK(res2.isnull());
}

BOOST_AUTO_TEST_CASE(test_mojeid_address)
{
    Fred::Backend::MojeIdImplData::Address impl_addr;
    impl_addr.street1 = "st1";
    impl_addr.street2 = "st2";
    impl_addr.street3 = "st3";
    impl_addr.city    = "Praha";
    impl_addr.state   = "state";
    impl_addr.country = "Czech Republic";

    Registry::MojeID::Address idl_addr;
    CorbaConversion::wrap_Address(impl_addr, idl_addr);
    BOOST_CHECK(idl_addr.street1.in()           == impl_addr.street1);
    BOOST_CHECK(idl_addr.street2.in()->_value() == impl_addr.street2.get_value());
    BOOST_CHECK(idl_addr.street3.in()->_value() == impl_addr.street3.get_value());
    BOOST_CHECK(idl_addr.city.in()              == impl_addr.city);
    BOOST_CHECK(idl_addr.state.in()->_value()   == impl_addr.state.get_value());
    BOOST_CHECK(idl_addr.country.in()           == impl_addr.country);

    Fred::Backend::MojeIdImplData::Address impl_addr_res;
    CorbaConversion::unwrap_Address(idl_addr, impl_addr_res);
    BOOST_CHECK(impl_addr_res.street1             == impl_addr.street1);
    BOOST_CHECK(impl_addr_res.street2.get_value() == impl_addr.street2.get_value());
    BOOST_CHECK(impl_addr_res.street3.get_value() == impl_addr.street3.get_value());
    BOOST_CHECK(impl_addr_res.city                == impl_addr.city);
    BOOST_CHECK(impl_addr_res.state.get_value()   == impl_addr.state.get_value());
    BOOST_CHECK(impl_addr_res.country             == impl_addr.country);
}

BOOST_AUTO_TEST_CASE(test_nullable_mojeid_address)
{
    Fred::Backend::MojeIdImplData::Address impl_addr;
    impl_addr.street1 = "st1";
    impl_addr.street2 = "st2";
    impl_addr.street3 = "st3";
    impl_addr.city    = "Praha";
    impl_addr.state   = "state";
    impl_addr.country = "Czech Republic";

    Registry::MojeID::NullableAddress_var idl_nullable_addr =
        CorbaConversion::wrap_Nullable_Address(Nullable< Fred::Backend::MojeIdImplData::Address >())._retn();
    BOOST_CHECK(idl_nullable_addr.in() == NULL);

    Nullable< Fred::Backend::MojeIdImplData::Address > impl_nullable_addr(impl_addr);
    BOOST_CHECK(!impl_nullable_addr.isnull());
    CorbaConversion::unwrap_NullableAddress(idl_nullable_addr, impl_nullable_addr);
    BOOST_CHECK(impl_nullable_addr.isnull());

    impl_nullable_addr = impl_addr;
    BOOST_CHECK(!impl_nullable_addr.isnull());
    idl_nullable_addr = CorbaConversion::wrap_Nullable_Address(impl_nullable_addr);
    BOOST_REQUIRE(idl_nullable_addr.in() != NULL);
    BOOST_CHECK(idl_nullable_addr->_value().street1.in()           == impl_addr.street1);
    BOOST_CHECK(idl_nullable_addr->_value().street2.in()->_value() == impl_addr.street2.get_value());
    BOOST_CHECK(idl_nullable_addr->_value().street3.in()->_value() == impl_addr.street3.get_value());
    BOOST_CHECK(idl_nullable_addr->_value().city.in()              == impl_addr.city);
    BOOST_CHECK(idl_nullable_addr->_value().state.in()->_value()   == impl_addr.state.get_value());
    BOOST_CHECK(idl_nullable_addr->_value().country.in()           == impl_addr.country);

    impl_nullable_addr = Nullable< Fred::Backend::MojeIdImplData::Address >();
    BOOST_CHECK(impl_nullable_addr.isnull());
    CorbaConversion::unwrap_NullableAddress(idl_nullable_addr, impl_nullable_addr);
    BOOST_REQUIRE(!impl_nullable_addr.isnull());
    BOOST_CHECK(impl_nullable_addr.get_value().street1             == impl_addr.street1);
    BOOST_CHECK(impl_nullable_addr.get_value().street2.get_value() == impl_addr.street2.get_value());
    BOOST_CHECK(impl_nullable_addr.get_value().street3.get_value() == impl_addr.street3.get_value());
    BOOST_CHECK(impl_nullable_addr.get_value().city                == impl_addr.city);
    BOOST_CHECK(impl_nullable_addr.get_value().state.get_value()   == impl_addr.state.get_value());
    BOOST_CHECK(impl_nullable_addr.get_value().country             == impl_addr.country);
}

BOOST_AUTO_TEST_CASE(test_mojeid_shippingaddress)
{
    Fred::Backend::MojeIdImplData::ShippingAddress impl_addr;
    impl_addr.company_name = "company";
    impl_addr.street1      = "st1";
    impl_addr.street2      = "st2";
    impl_addr.street3      = "st3";
    impl_addr.city         = "Praha";
    impl_addr.state        = "state";
    impl_addr.country      = "Czech Republic";

    Registry::MojeID::ShippingAddress idl_addr;
    CorbaConversion::wrap_ShippingAddress(impl_addr, idl_addr);
    BOOST_CHECK(idl_addr.company_name.in()->_value() == impl_addr.company_name.get_value());
    BOOST_CHECK(idl_addr.street1.in()                == impl_addr.street1);
    BOOST_CHECK(idl_addr.street2.in()->_value()      == impl_addr.street2.get_value());
    BOOST_CHECK(idl_addr.street3.in()->_value()      == impl_addr.street3.get_value());
    BOOST_CHECK(idl_addr.city.in()                   == impl_addr.city);
    BOOST_CHECK(idl_addr.state.in()->_value()        == impl_addr.state.get_value());
    BOOST_CHECK(idl_addr.country.in()                == impl_addr.country);

    Fred::Backend::MojeIdImplData::ShippingAddress impl_addr_res;
    CorbaConversion::unwrap_ShippingAddress(idl_addr, impl_addr_res);
    BOOST_CHECK(impl_addr_res.company_name.get_value() == impl_addr.company_name.get_value());
    BOOST_CHECK(impl_addr_res.street1                  == impl_addr.street1);
    BOOST_CHECK(impl_addr_res.street2.get_value()      == impl_addr.street2.get_value());
    BOOST_CHECK(impl_addr_res.street3.get_value()      == impl_addr.street3.get_value());
    BOOST_CHECK(impl_addr_res.city                     == impl_addr.city);
    BOOST_CHECK(impl_addr_res.state.get_value()        == impl_addr.state.get_value());
    BOOST_CHECK(impl_addr_res.country                  == impl_addr.country);
}

BOOST_AUTO_TEST_CASE(test_nullable_mojeid_shippingaddress)
{
    Fred::Backend::MojeIdImplData::ShippingAddress impl_addr;
    impl_addr.company_name = "company";
    impl_addr.street1      = "st1";
    impl_addr.street2      = "st2";
    impl_addr.street3      = "st3";
    impl_addr.city         = "Praha";
    impl_addr.state        = "state";
    impl_addr.country      = "Czech Republic";

    Registry::MojeID::NullableShippingAddress_var idl_nullable_addr =
        CorbaConversion::wrap_Nullable_ShippingAddress(Nullable< Fred::Backend::MojeIdImplData::ShippingAddress >())._retn();
    BOOST_CHECK(idl_nullable_addr.in() == NULL);

    Nullable< Fred::Backend::MojeIdImplData::ShippingAddress > impl_nullable_addr(impl_addr);
    BOOST_CHECK(!impl_nullable_addr.isnull());
    CorbaConversion::unwrap_NullableShippingAddress(idl_nullable_addr, impl_nullable_addr);
    BOOST_CHECK(impl_nullable_addr.isnull());

    impl_nullable_addr = impl_addr;
    BOOST_CHECK(!impl_nullable_addr.isnull());
    idl_nullable_addr = CorbaConversion::wrap_Nullable_ShippingAddress(impl_nullable_addr);
    BOOST_REQUIRE(idl_nullable_addr.in() != NULL);
    BOOST_CHECK(idl_nullable_addr->_value().company_name.in()->_value() == impl_addr.company_name.get_value());
    BOOST_CHECK(idl_nullable_addr->_value().street1.in()                == impl_addr.street1);
    BOOST_CHECK(idl_nullable_addr->_value().street2.in()->_value()      == impl_addr.street2.get_value());
    BOOST_CHECK(idl_nullable_addr->_value().street3.in()->_value()      == impl_addr.street3.get_value());
    BOOST_CHECK(idl_nullable_addr->_value().city.in()                   == impl_addr.city);
    BOOST_CHECK(idl_nullable_addr->_value().state.in()->_value()        == impl_addr.state.get_value());
    BOOST_CHECK(idl_nullable_addr->_value().country.in()                == impl_addr.country);

    impl_nullable_addr = Nullable< Fred::Backend::MojeIdImplData::ShippingAddress >();
    BOOST_CHECK(impl_nullable_addr.isnull());
    CorbaConversion::unwrap_NullableShippingAddress(idl_nullable_addr, impl_nullable_addr);
    BOOST_REQUIRE(!impl_nullable_addr.isnull());
    BOOST_CHECK(impl_nullable_addr.get_value().company_name.get_value() == impl_addr.company_name.get_value());
    BOOST_CHECK(impl_nullable_addr.get_value().street1                  == impl_addr.street1);
    BOOST_CHECK(impl_nullable_addr.get_value().street2.get_value()      == impl_addr.street2.get_value());
    BOOST_CHECK(impl_nullable_addr.get_value().street3.get_value()      == impl_addr.street3.get_value());
    BOOST_CHECK(impl_nullable_addr.get_value().city                     == impl_addr.city);
    BOOST_CHECK(impl_nullable_addr.get_value().state.get_value()        == impl_addr.state.get_value());
    BOOST_CHECK(impl_nullable_addr.get_value().country                  == impl_addr.country);
}

BOOST_AUTO_TEST_CASE(test_mojeid_validationresult)
{
    Registry::MojeID::ValidationResult idl_value;

    CorbaConversion::wrap_ValidationResult(Fred::Backend::MojeIdImplData::ValidationResult::OK, idl_value);
    BOOST_CHECK(idl_value == Registry::MojeID::OK);

    CorbaConversion::wrap_ValidationResult(Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE, idl_value);
    BOOST_CHECK(idl_value == Registry::MojeID::NOT_AVAILABLE);

    CorbaConversion::wrap_ValidationResult(Fred::Backend::MojeIdImplData::ValidationResult::INVALID, idl_value);
    BOOST_CHECK(idl_value == Registry::MojeID::INVALID);

    CorbaConversion::wrap_ValidationResult(Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED, idl_value);
    BOOST_CHECK(idl_value == Registry::MojeID::REQUIRED);

    const Fred::Backend::MojeIdImplData::ValidationResult::Value out_of_range_value =
        static_cast< Fred::Backend::MojeIdImplData::ValidationResult::Value >(10);
    BOOST_CHECK_THROW(CorbaConversion::wrap_ValidationResult(out_of_range_value, idl_value),
                      CorbaConversion::NotEnumValidationResultValue);
}

BOOST_AUTO_TEST_CASE(test_mojeid_addressvalidationresult)
{
    Fred::Backend::MojeIdImplData::AddressValidationResult addr_err_impl;
    addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::OK;
    addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;

    Registry::MojeID::AddressValidationResult addr_err;
    CorbaConversion::wrap_AddressValidationResult(addr_err_impl, addr_err);

    BOOST_CHECK(addr_err.street1     == Registry::MojeID::OK);
    BOOST_CHECK(addr_err.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(addr_err.postal_code == Registry::MojeID::INVALID);
    BOOST_CHECK(addr_err.country     == Registry::MojeID::REQUIRED);
}

BOOST_AUTO_TEST_CASE(test_mojeid_mandatoryaddressvalidationresult)
{
    Fred::Backend::MojeIdImplData::MandatoryAddressValidationResult addr_err_impl;
    addr_err_impl.address_presence = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    addr_err_impl.street1          = Fred::Backend::MojeIdImplData::ValidationResult::OK;
    addr_err_impl.city             = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    addr_err_impl.postal_code      = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    addr_err_impl.country          = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;

    Registry::MojeID::MandatoryAddressValidationResult addr_err;
    CorbaConversion::wrap_MandatoryAddressValidationResult(addr_err_impl, addr_err);

    BOOST_CHECK(addr_err.address_presence == Registry::MojeID::REQUIRED);
    BOOST_CHECK(addr_err.street1          == Registry::MojeID::OK);
    BOOST_CHECK(addr_err.city             == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(addr_err.postal_code      == Registry::MojeID::INVALID);
    BOOST_CHECK(addr_err.country          == Registry::MojeID::REQUIRED);
}

BOOST_AUTO_TEST_CASE(test_mojeid_message_limit_exceeded)
{
    Fred::Backend::MojeIdImplData::MessageLimitExceeded msg;
    BOOST_CHECK_THROW(CorbaConversion::raise_MESSAGE_LIMIT_EXCEEDED(msg),
                      Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    msg.limit_expire_datetime = boost::posix_time::ptime(boost::gregorian::date(2015,12,10));

    Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED res;
    CorbaConversion::wrap_MessageLimitExceeded(msg, res);
}

BOOST_AUTO_TEST_CASE(test_mojeid_registration_validation_error)
{
    Fred::Backend::MojeIdImplData::MandatoryAddressValidationResult permanent_addr_err_impl;
    permanent_addr_err_impl.address_presence = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    permanent_addr_err_impl.street1          = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    permanent_addr_err_impl.city             = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    permanent_addr_err_impl.postal_code      = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    permanent_addr_err_impl.country          = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::AddressValidationResult mailing_addr_err_impl;
    mailing_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    mailing_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    mailing_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    mailing_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::AddressValidationResult billing_addr_err_impl;
    billing_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    billing_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    billing_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    billing_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;

    Fred::Backend::MojeIdImplData::AddressValidationResult shipping_addr_err_impl;
    shipping_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    shipping_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    shipping_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    shipping_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::AddressValidationResult shipping2_addr_err_impl;
    shipping2_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    shipping2_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    shipping2_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    shipping2_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::AddressValidationResult shipping3_addr_err_impl;
    shipping3_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    shipping3_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    shipping3_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    shipping3_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::RegistrationValidationResult reg_val_err_impl;
    reg_val_err_impl.username     = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    reg_val_err_impl.name         = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    reg_val_err_impl.birth_date   = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    reg_val_err_impl.vat_id_num   = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    reg_val_err_impl.email        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    reg_val_err_impl.notify_email = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    reg_val_err_impl.phone        = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    reg_val_err_impl.fax          = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    reg_val_err_impl.permanent = permanent_addr_err_impl;
    reg_val_err_impl.mailing   = mailing_addr_err_impl;
    reg_val_err_impl.billing   = billing_addr_err_impl;

    reg_val_err_impl.shipping  = shipping_addr_err_impl;
    reg_val_err_impl.shipping2 = shipping2_addr_err_impl;
    reg_val_err_impl.shipping3 = shipping3_addr_err_impl;

    Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR res;
    CorbaConversion::wrap_RegistrationValidationResult(reg_val_err_impl, res);

    BOOST_CHECK(res.username     == Registry::MojeID::INVALID);
    BOOST_CHECK(res.name         == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.birth_date   == Registry::MojeID::INVALID);
    BOOST_CHECK(res.vat_id_num   == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.email        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.notify_email == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.phone        == Registry::MojeID::INVALID);
    BOOST_CHECK(res.fax          == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.permanent.address_presence == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.street1          == Registry::MojeID::INVALID);
    BOOST_CHECK(res.permanent.city             == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.permanent.postal_code      == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.country          == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.mailing.street1     == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.mailing.city        == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.mailing.postal_code == Registry::MojeID::INVALID);
    BOOST_CHECK(res.mailing.country     == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.billing.street1     == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.billing.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.billing.postal_code == Registry::MojeID::INVALID);
    BOOST_CHECK(res.billing.country     == Registry::MojeID::REQUIRED);

    BOOST_CHECK(res.shipping.street1     == Registry::MojeID::INVALID);
    BOOST_CHECK(res.shipping.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.shipping.postal_code == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.shipping.country     == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.shipping2.street1     == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.shipping2.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.shipping2.postal_code == Registry::MojeID::INVALID);
    BOOST_CHECK(res.shipping2.country     == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.shipping3.street1     == Registry::MojeID::INVALID);
    BOOST_CHECK(res.shipping3.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.shipping3.postal_code == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.shipping3.country     == Registry::MojeID::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_update_contact_prepare_validation_error)
{
    Fred::Backend::MojeIdImplData::MandatoryAddressValidationResult permanent_addr_err_impl;
    permanent_addr_err_impl.address_presence = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    permanent_addr_err_impl.street1          = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    permanent_addr_err_impl.city             = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    permanent_addr_err_impl.postal_code      = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    permanent_addr_err_impl.country          = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::AddressValidationResult mailing_addr_err_impl;
    mailing_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    mailing_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    mailing_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    mailing_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::AddressValidationResult billing_addr_err_impl;
    billing_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    billing_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    billing_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    billing_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;

    Fred::Backend::MojeIdImplData::AddressValidationResult shipping_addr_err_impl;
    shipping_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    shipping_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    shipping_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    shipping_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::AddressValidationResult shipping2_addr_err_impl;
    shipping2_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    shipping2_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    shipping2_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    shipping2_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::AddressValidationResult shipping3_addr_err_impl;
    shipping3_addr_err_impl.street1     = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    shipping3_addr_err_impl.city        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    shipping3_addr_err_impl.postal_code = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    shipping3_addr_err_impl.country     = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::UpdateContactPrepareValidationResult upd_val_err_impl;
    upd_val_err_impl.name         = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    upd_val_err_impl.birth_date   = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    upd_val_err_impl.email        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    upd_val_err_impl.notify_email = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    upd_val_err_impl.phone        = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    upd_val_err_impl.fax          = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    upd_val_err_impl.permanent = permanent_addr_err_impl;
    upd_val_err_impl.mailing   = mailing_addr_err_impl;
    upd_val_err_impl.billing   = billing_addr_err_impl;

    upd_val_err_impl.shipping  = shipping_addr_err_impl;
    upd_val_err_impl.shipping2 = shipping2_addr_err_impl;
    upd_val_err_impl.shipping3 = shipping3_addr_err_impl;

    Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR res;
    CorbaConversion::wrap_UpdateContactPrepareValidationResult(upd_val_err_impl,res);

    BOOST_CHECK(res.name         == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.birth_date   == Registry::MojeID::INVALID);
    BOOST_CHECK(res.email        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.notify_email == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.phone        == Registry::MojeID::INVALID);
    BOOST_CHECK(res.fax          == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.permanent.address_presence == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.street1          == Registry::MojeID::INVALID);
    BOOST_CHECK(res.permanent.city             == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.permanent.postal_code      == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.country          == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.mailing.street1     == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.mailing.city        == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.mailing.postal_code == Registry::MojeID::INVALID);
    BOOST_CHECK(res.mailing.country     == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.billing.street1     == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.billing.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.billing.postal_code == Registry::MojeID::INVALID);
    BOOST_CHECK(res.billing.country     == Registry::MojeID::REQUIRED);

    BOOST_CHECK(res.shipping.street1     == Registry::MojeID::INVALID);
    BOOST_CHECK(res.shipping.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.shipping.postal_code == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.shipping.country     == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.shipping2.street1     == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.shipping2.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.shipping2.postal_code == Registry::MojeID::INVALID);
    BOOST_CHECK(res.shipping2.country     == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.shipping3.street1     == Registry::MojeID::INVALID);
    BOOST_CHECK(res.shipping3.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.shipping3.postal_code == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.shipping3.country     == Registry::MojeID::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_create_validation_request_validation_error)
{
    Fred::Backend::MojeIdImplData::MandatoryAddressValidationResult mandatory_addr_err_impl;
    mandatory_addr_err_impl.address_presence = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    mandatory_addr_err_impl.street1          = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    mandatory_addr_err_impl.city             = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    mandatory_addr_err_impl.postal_code      = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    mandatory_addr_err_impl.country          = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;

    Fred::Backend::MojeIdImplData::CreateValidationRequestValidationResult crr_val_err_impl;

    crr_val_err_impl.name         = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    crr_val_err_impl.email        = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    crr_val_err_impl.notify_email = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;
    crr_val_err_impl.phone        = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    crr_val_err_impl.fax          = Fred::Backend::MojeIdImplData::ValidationResult::NOT_AVAILABLE;
    crr_val_err_impl.birth_date   = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    crr_val_err_impl.vat_id_num   = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;

    crr_val_err_impl.permanent = mandatory_addr_err_impl;

    Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR res;
    CorbaConversion::wrap_CreateValidationRequestValidationResult(crr_val_err_impl,res);

    BOOST_CHECK(res.name         == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.email        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.notify_email == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.phone        == Registry::MojeID::INVALID);
    BOOST_CHECK(res.fax          == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.birth_date   == Registry::MojeID::INVALID);
    BOOST_CHECK(res.vat_id_num   == Registry::MojeID::REQUIRED);

    BOOST_CHECK(res.permanent.address_presence == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.street1          == Registry::MojeID::INVALID);
    BOOST_CHECK(res.permanent.city             == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.permanent.postal_code      == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.country          == Registry::MojeID::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_process_registration_request_validation_error)
{
    Fred::Backend::MojeIdImplData::ProcessRegistrationValidationResult prr_val_err_impl;
    prr_val_err_impl.email = Fred::Backend::MojeIdImplData::ValidationResult::INVALID;
    prr_val_err_impl.phone = Fred::Backend::MojeIdImplData::ValidationResult::REQUIRED;

    Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR res;
    CorbaConversion::wrap_ProcessRegistrationValidationResult(prr_val_err_impl,res);

    BOOST_CHECK(res.email == Registry::MojeID::INVALID);
    BOOST_CHECK(res.phone == Registry::MojeID::REQUIRED);
}

BOOST_AUTO_TEST_CASE(test_mojeid_create_contact)
{
    Registry::MojeID::CreateContact cc;

    cc.username = CorbaConversion::wrap_string("username")._retn();
    cc.name = CorbaConversion::wrap_string("first_name last_name")._retn();
    cc.organization = CorbaConversion::wrap_Nullable_string("org")._retn();
    cc.vat_reg_num = CorbaConversion::wrap_Nullable_string("vat_reg_num")._retn();
    cc.birth_date = CorbaConversion::Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(boost::gregorian::date(2015,12,10))._retn();
    cc.id_card_num = CorbaConversion::wrap_Nullable_string("id_card_num")._retn();
    cc.passport_num = CorbaConversion::wrap_Nullable_string("passport_num")._retn();
    cc.ssn_id_num = CorbaConversion::wrap_Nullable_string("ssn_id_num")._retn();
    cc.vat_id_num = CorbaConversion::wrap_Nullable_string("vat_id_num")._retn();
    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "st1";
        addr_impl.street2 = "st2";
        addr_impl.street3 = "st3";
        addr_impl.city    = "Praha";
        addr_impl.state   = "state";
        addr_impl.country = "Czech Republic";
        CorbaConversion::wrap_Address(addr_impl, cc.permanent);
    }
    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "m_st1";
        addr_impl.street2 = "m_st2";
        addr_impl.street3 = "m_st3";
        addr_impl.city    = "m_Praha";
        addr_impl.state   = "m_state";
        addr_impl.country = "m_Czech Republic";

        cc.mailing = CorbaConversion::wrap_Nullable_Address(addr_impl)._retn();
    }
    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "b_st1";
        addr_impl.street2 = "b_st2";
        addr_impl.street3 = "b_st3";
        addr_impl.city    = "b_Praha";
        addr_impl.state   = "b_state";
        addr_impl.country = "b_Czech Republic";

        cc.billing = CorbaConversion::wrap_Nullable_Address(addr_impl)._retn();
    }
    {
        Fred::Backend::MojeIdImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s_company";
        addr_impl.street1      = "s_st1";
        addr_impl.street2      = "s_st2";
        addr_impl.street3      = "s_st3";
        addr_impl.city         = "s_Praha";
        addr_impl.state        = "s_state";
        addr_impl.country      = "s_Czech Republic";

        cc.shipping = CorbaConversion::wrap_Nullable_ShippingAddress(addr_impl)._retn();
    }
    {
        Fred::Backend::MojeIdImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s2_company";
        addr_impl.street1      = "s2_st1";
        addr_impl.street2      = "s2_st2";
        addr_impl.street3      = "s2_st3";
        addr_impl.city         = "s2_Praha";
        addr_impl.state        = "s2_state";
        addr_impl.country      = "s2_Czech Republic";

        cc.shipping2 = CorbaConversion::wrap_Nullable_ShippingAddress(addr_impl)._retn();
    }
    {
        Fred::Backend::MojeIdImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s3_company";
        addr_impl.street1      = "s3_st1";
        addr_impl.street2      = "s3_st2";
        addr_impl.street3      = "s3_st3";
        addr_impl.city         = "s3_Praha";
        addr_impl.state        = "s3_state";
        addr_impl.country      = "s3_Czech Republic";

        cc.shipping3 = CorbaConversion::wrap_Nullable_ShippingAddress(addr_impl)._retn();
    }
    cc.email = CorbaConversion::wrap_string("email")._retn();
    cc.notify_email = CorbaConversion::wrap_Nullable_string("notify_email")._retn();
    cc.telephone = CorbaConversion::wrap_string("telephone")._retn();
    cc.fax = CorbaConversion::wrap_Nullable_string("fax")._retn();

    Fred::Backend::MojeIdImplData::CreateContact cc_impl;
    CorbaConversion::unwrap_CreateContact(cc, cc_impl);

    BOOST_CHECK(cc_impl.username == "username");
    BOOST_CHECK_EQUAL(cc_impl.name, "first_name last_name");
    BOOST_CHECK(cc_impl.organization.get_value() == "org");
    BOOST_CHECK(cc_impl.vat_reg_num.get_value() == "vat_reg_num");
    BOOST_CHECK(boost::gregorian::from_simple_string(cc_impl.birth_date.get_value().value) == boost::gregorian::date(2015,12,10));
    BOOST_CHECK(cc_impl.id_card_num.get_value() == "id_card_num");
    BOOST_CHECK(cc_impl.passport_num.get_value() == "passport_num");
    BOOST_CHECK(cc_impl.ssn_id_num.get_value() == "ssn_id_num");
    BOOST_CHECK(cc_impl.vat_id_num.get_value() == "vat_id_num");

    BOOST_CHECK(cc_impl.permanent.street1 == "st1");
    BOOST_CHECK(cc_impl.permanent.street2.get_value() == "st2");
    BOOST_CHECK(cc_impl.permanent.street3.get_value() == "st3");
    BOOST_CHECK(cc_impl.permanent.city == "Praha");
    BOOST_CHECK(cc_impl.permanent.state.get_value() == "state");
    BOOST_CHECK(cc_impl.permanent.country == "Czech Republic");

    BOOST_CHECK(cc_impl.mailing.get_value().street1 == "m_st1");
    BOOST_CHECK(cc_impl.mailing.get_value().street2.get_value() == "m_st2");
    BOOST_CHECK(cc_impl.mailing.get_value().street3.get_value() == "m_st3");
    BOOST_CHECK(cc_impl.mailing.get_value().city == "m_Praha");
    BOOST_CHECK(cc_impl.mailing.get_value().state.get_value() == "m_state");
    BOOST_CHECK(cc_impl.mailing.get_value().country == "m_Czech Republic");

    BOOST_CHECK(cc_impl.billing.get_value().street1 == "b_st1");
    BOOST_CHECK(cc_impl.billing.get_value().street2.get_value() == "b_st2");
    BOOST_CHECK(cc_impl.billing.get_value().street3.get_value() == "b_st3");
    BOOST_CHECK(cc_impl.billing.get_value().city == "b_Praha");
    BOOST_CHECK(cc_impl.billing.get_value().state.get_value() == "b_state");
    BOOST_CHECK(cc_impl.billing.get_value().country == "b_Czech Republic");

    BOOST_CHECK(cc_impl.shipping.get_value().company_name.get_value() == "s_company");
    BOOST_CHECK(cc_impl.shipping.get_value().street1 == "s_st1");
    BOOST_CHECK(cc_impl.shipping.get_value().street2.get_value() == "s_st2");
    BOOST_CHECK(cc_impl.shipping.get_value().street3.get_value() == "s_st3");
    BOOST_CHECK(cc_impl.shipping.get_value().city == "s_Praha");
    BOOST_CHECK(cc_impl.shipping.get_value().state.get_value() == "s_state");
    BOOST_CHECK(cc_impl.shipping.get_value().country == "s_Czech Republic");

    BOOST_CHECK(cc_impl.shipping2.get_value().company_name.get_value() == "s2_company");
    BOOST_CHECK(cc_impl.shipping2.get_value().street1 == "s2_st1");
    BOOST_CHECK(cc_impl.shipping2.get_value().street2.get_value() == "s2_st2");
    BOOST_CHECK(cc_impl.shipping2.get_value().street3.get_value() == "s2_st3");
    BOOST_CHECK(cc_impl.shipping2.get_value().city == "s2_Praha");
    BOOST_CHECK(cc_impl.shipping2.get_value().state.get_value() == "s2_state");
    BOOST_CHECK(cc_impl.shipping2.get_value().country == "s2_Czech Republic");

    BOOST_CHECK(cc_impl.shipping3.get_value().company_name.get_value() == "s3_company");
    BOOST_CHECK(cc_impl.shipping3.get_value().street1 == "s3_st1");
    BOOST_CHECK(cc_impl.shipping3.get_value().street2.get_value() == "s3_st2");
    BOOST_CHECK(cc_impl.shipping3.get_value().street3.get_value() == "s3_st3");
    BOOST_CHECK(cc_impl.shipping3.get_value().city == "s3_Praha");
    BOOST_CHECK(cc_impl.shipping3.get_value().state.get_value() == "s3_state");
    BOOST_CHECK(cc_impl.shipping3.get_value().country == "s3_Czech Republic");

    BOOST_CHECK(cc_impl.email == "email");
    BOOST_CHECK(cc_impl.notify_email.get_value() == "notify_email");
    BOOST_CHECK(cc_impl.telephone == "telephone");
    BOOST_CHECK(cc_impl.fax.get_value() == "fax");
}

template < class SRC_INT_TYPE, class DST_INT_TYPE,
           bool SRC_IS_SIGNED  = std::numeric_limits< SRC_INT_TYPE >::is_signed,
           bool DST_IS_SIGNED  = std::numeric_limits< DST_INT_TYPE >::is_signed,
           bool DST_IS_SHORTER = std::numeric_limits< DST_INT_TYPE >::digits < std::numeric_limits< SRC_INT_TYPE >::digits >
struct IntConversionTraits;

// ===== bool -> bool =====
// = src: < 0, 1 >        =
// = dst: < 0, 1 >        =
// ========================
template < >
struct IntConversionTraits< bool, bool, false, false, false >
{
    typedef bool type;
    static const ::size_t number_of_values_to_fit = 2;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 0;
    // declaring an array with zero length is illegal in C++ (8.3.4.1)
    static const type out_of_range_value[1];
};

const bool IntConversionTraits< bool, bool, false, false, false >::
    value_to_fit[number_of_values_to_fit] = {
        false,
        true
    };

const bool IntConversionTraits< bool, bool, false, false, false >::
    out_of_range_value[1] = { false };


// ===== bool -> any integer =====
// = src:       < 0, 1 >         =
// = dst: < ......0++++++ >      =
// ===============================
template < class DST_INT_TYPE, bool DST_IS_SIGNED >
struct IntConversionTraits< bool, DST_INT_TYPE, false, DST_IS_SIGNED, false >
{
    typedef bool type;
    static const ::size_t number_of_values_to_fit = 2;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 0;
    // declaring an array with zero length is illegal in C++ (8.3.4.1)
    static const type out_of_range_value[1];
};

template < class DST_INT_TYPE, bool DST_IS_SIGNED >
const bool IntConversionTraits< bool, DST_INT_TYPE, false, DST_IS_SIGNED, false >::
    value_to_fit[number_of_values_to_fit] = {
        false,
        true
    };

template < class DST_INT_TYPE, bool DST_IS_SIGNED >
const bool IntConversionTraits< bool, DST_INT_TYPE, false, DST_IS_SIGNED, false >::
    out_of_range_value[1] = { false };


// ===== shorter unsigned -> longer integer =====
// = src:             < 0+++ >                  =
// = dst: < ............0++++++++++++ >         =
// ==============================================
template < class SRC_INT_TYPE, class DST_INT_TYPE, bool DST_IS_SIGNED >
struct IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, false, DST_IS_SIGNED, false >
{
    typedef SRC_INT_TYPE type;
    typedef std::numeric_limits< SRC_INT_TYPE > shorter_unsigned;
    static const ::size_t number_of_values_to_fit = 4;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 0;
    // declaring an array with zero length is illegal in C++ (8.3.4.1)
    static const type out_of_range_value[1];
};

template < class SRC_INT_TYPE, class DST_INT_TYPE, bool DST_IS_SIGNED >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, false, DST_IS_SIGNED, false >::
    value_to_fit[number_of_values_to_fit] = {
        type(0),
        type(1),
        type(shorter_unsigned::max() - 1),
        type(shorter_unsigned::max())
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE, bool DST_IS_SIGNED >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, false, DST_IS_SIGNED, false >::
    out_of_range_value[1] = { false };


// ===== shorter signed -> longer unsigned =====
// = src: < ---0+++ >                          =
// = dst:    < 0++++++++++++ >                 =
// =============================================
template < class SRC_INT_TYPE, class DST_INT_TYPE >
struct IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, false, false >
{
    typedef SRC_INT_TYPE type;
    typedef std::numeric_limits< SRC_INT_TYPE > shorter_signed;
    static const ::size_t number_of_values_to_fit = 4;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 3;
    static const type out_of_range_value[number_of_out_of_range_values];
};

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, false, false >::
    value_to_fit[number_of_values_to_fit] = {
        type(0),
        type(1),
        type(shorter_signed::max() - 1),
        type(shorter_signed::max())
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, false, false >::
    out_of_range_value[number_of_out_of_range_values] = {
        type(shorter_signed::min()),
        type(shorter_signed::min() + 1),
        type(-1)
    };


// ===== shorter signed -> longer signed =====
// = src:          < ---0+++ >               =
// = dst: < ------------0++++++++++++ >      =
// ===========================================
template < class SRC_INT_TYPE, class DST_INT_TYPE >
struct IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, true, false >
{
    typedef SRC_INT_TYPE type;
    typedef std::numeric_limits< SRC_INT_TYPE > shorter_signed;
    static const ::size_t number_of_values_to_fit = 7;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 0;
    // declaring an array with zero length is illegal in C++ (8.3.4.1)
    static const type out_of_range_value[1];
};

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, true, false >::
    value_to_fit[number_of_values_to_fit] = {
        type(shorter_signed::min()),
        type(shorter_signed::min() + 1),
        type(-1),
        type(0),
        type(1),
        type(shorter_signed::max() - 1),
        type(shorter_signed::max())
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, true, false >::
    out_of_range_value[1] = { false };


// ===== longer unsigned -> bool =====
// = src: < 0++++++++++++ >          =
// = dst: < 0, 1 >                   =
// ===================================
template < class SRC_INT_TYPE >
struct IntConversionTraits< SRC_INT_TYPE, bool, false, false, true >
{
    typedef SRC_INT_TYPE type;
    typedef std::numeric_limits< SRC_INT_TYPE > longer_unsigned;
    typedef std::numeric_limits< bool > shorter_unsigned;
    static const ::size_t number_of_values_to_fit = 2;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 3;
    static const type out_of_range_value[number_of_out_of_range_values];
};

template < class SRC_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, bool, false, false, true >::
    value_to_fit[number_of_values_to_fit] = {
        type(0),
        type(1)
    };

template < class SRC_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, bool, false, false, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        type(type(shorter_unsigned::max()) + 1),
        type(longer_unsigned::max() - 1),
        type(longer_unsigned::max())
    };


// ===== longer signed -> bool =====
// = src: < ---------0+++++++++ >  =
// = dst:          < 0, 1 >        =
// =================================
template < class SRC_INT_TYPE >
struct IntConversionTraits< SRC_INT_TYPE, bool, true, false, true >
{
    typedef SRC_INT_TYPE type;
    typedef std::numeric_limits< SRC_INT_TYPE > longer_signed;
    typedef std::numeric_limits< bool > shorter_unsigned;
    static const ::size_t number_of_values_to_fit = 2;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 7;
    static const type out_of_range_value[number_of_out_of_range_values];
};

template < class SRC_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, bool, true, false, true >::
    value_to_fit[number_of_values_to_fit] = {
        type(0),
        type(1)
    };

template < class SRC_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, bool, true, false, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        type(longer_signed::min()),
        type(longer_signed::min() + 1),
        type(-type(shorter_unsigned::max()) - 1),
        type(-type(shorter_unsigned::max())),
        type(type(shorter_unsigned::max()) + 1),
        type(longer_signed::max() - 1),
        type(longer_signed::max())
    };


// ===== longer unsigned -> shorter integer =====
// = src:    < 0++++++++++++ >                  =
// = dst: < ---0+++ >                           =
// ==============================================
template < class SRC_INT_TYPE, class DST_INT_TYPE, bool DST_IS_SIGNED >
struct IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, false, DST_IS_SIGNED, true >
{
    typedef SRC_INT_TYPE type;
    typedef std::numeric_limits< SRC_INT_TYPE > longer_unsigned;
    typedef std::numeric_limits< DST_INT_TYPE > shorter;
    static const ::size_t number_of_values_to_fit = 4;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 3;
    static const type out_of_range_value[number_of_out_of_range_values];
};

template < class SRC_INT_TYPE, class DST_INT_TYPE, bool DST_IS_SIGNED >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, false, DST_IS_SIGNED, true >::
    value_to_fit[number_of_values_to_fit] = {
        type(0),
        type(1),
        type(shorter::max() - 1),
        type(shorter::max())
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE, bool DST_IS_SIGNED >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, false, DST_IS_SIGNED, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        type(type(shorter::max()) + 1),
        type(longer_unsigned::max() - 1),
        type(longer_unsigned::max())
    };

// ===== longer signed -> shorter unsigned =====
// = src: < ------------0++++++++++++ >        =
// = dst:             < 0+++ >                 =
// =============================================
template < class SRC_INT_TYPE, class DST_INT_TYPE >
struct IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, false, true >
{
    typedef SRC_INT_TYPE type;
    typedef std::numeric_limits< SRC_INT_TYPE > longer_signed;
    typedef std::numeric_limits< DST_INT_TYPE > shorter_unsigned;
    static const ::size_t number_of_values_to_fit = 4;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 9;
    static const type out_of_range_value[number_of_out_of_range_values];
};

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, false, true >::
    value_to_fit[number_of_values_to_fit] = {
        type(0),
        type(1),
        type(shorter_unsigned::max() - 1),
        type(shorter_unsigned::max())
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, false, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        type(longer_signed::min()),
        type(longer_signed::min() + 1),
        type(-type(shorter_unsigned::max()) - 1),
        type(-type(shorter_unsigned::max())),
        type(-type(shorter_unsigned::max()) + 1),
        type(-1),
        type(type(shorter_unsigned::max()) + 1),
        type(longer_signed::max() - 1),
        type(longer_signed::max())
    };


// ===== longer signed -> shorter signed =====
// = src: < ------------0++++++++++++ >      =
// = dst:          < ---0+++ >               =
// ===========================================
template < class SRC_INT_TYPE, class DST_INT_TYPE >
struct IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, true, true >
{
    typedef SRC_INT_TYPE type;
    typedef std::numeric_limits< SRC_INT_TYPE > longer_signed;
    typedef std::numeric_limits< DST_INT_TYPE > shorter_signed;
    static const ::size_t number_of_values_to_fit = 7;
    static const type value_to_fit[number_of_values_to_fit];
    static const ::size_t number_of_out_of_range_values = 6;
    static const type out_of_range_value[number_of_out_of_range_values];
};

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, true, true >::
    value_to_fit[number_of_values_to_fit] = {
        type(shorter_signed::min()),
        type(shorter_signed::min() + 1),
        type(-1),
        type(0),
        type(1),
        type(shorter_signed::max() - 1),
        type(shorter_signed::max())
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, true, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        type(longer_signed::min()),
        type(longer_signed::min() + 1),
        type(type(shorter_signed::min()) - 1),
        type(type(shorter_signed::max()) + 1),
        type(longer_signed::max() - 1),
        type(longer_signed::max())
    };


template < class SRC_INT_TYPE, class DST_INT_TYPE >
void integer_conversion_test(::size_t &sum_cnt, ::size_t &sum_fit_cnt)
{
    typedef SRC_INT_TYPE src_type;
    typedef DST_INT_TYPE dst_type;
    typedef IntConversionTraits< src_type, dst_type > traits;
    for (::size_t idx = 0; idx < traits::number_of_values_to_fit; ++idx) {
        const src_type src_value = traits::value_to_fit[idx];
        dst_type dst_value;
        CorbaConversion::int_to_int(src_value, dst_value);
        BOOST_CHECK(src_type(dst_value) == src_value);
        BOOST_CHECK(dst_value == dst_type(src_value));
        src_type src_value_back;
        CorbaConversion::int_to_int(dst_value, src_value_back);
        BOOST_CHECK(src_value_back == src_value);
    }
    sum_cnt += traits::number_of_values_to_fit;
    sum_fit_cnt += traits::number_of_values_to_fit;

    for (::size_t idx = 0; idx < traits::number_of_out_of_range_values; ++idx) {
        const src_type src_value = traits::out_of_range_value[idx];
        dst_type dst_value;
        BOOST_CHECK_THROW((CorbaConversion::int_to_int(src_value, dst_value)),
                          CorbaConversion::IntegralConversionOutOfRange);
    }
    sum_cnt += traits::number_of_out_of_range_values;
}


#define TEST_INT_CONVERSION_FOR_GIVEN_TYPES(SRC_INT_TYPE, DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM) \
    integer_conversion_test< SRC_INT_TYPE, DST_INT_TYPE >(CNT_SUM, FIT_CNT_SUM)

#define TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM) \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(bool,               DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(char,               DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(unsigned char,      DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(signed char,        DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(wchar_t,            DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(short,              DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(unsigned short,     DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(int,                DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(unsigned int,       DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(long,               DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(unsigned long,      DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(long long,          DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM); \
    TEST_INT_CONVERSION_FOR_GIVEN_TYPES(unsigned long long, DST_INT_TYPE, CNT_SUM, FIT_CNT_SUM)

BOOST_AUTO_TEST_CASE(test_basic_integer_conversion)
{
    ::size_t     cnt_sum = 0;
    ::size_t fit_cnt_sum = 0;
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(bool,               cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(char,               cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(unsigned char,      cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(signed char,        cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(wchar_t,            cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(short,              cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(unsigned short,     cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(int,                cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(unsigned int,       cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(long,               cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(unsigned long,      cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(long long,          cnt_sum, fit_cnt_sum);
    TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO(unsigned long long, cnt_sum, fit_cnt_sum);
    BOOST_CHECK(    cnt_sum == 1215);
    BOOST_CHECK(fit_cnt_sum ==  773);
}

#undef TEST_CONVERSION_FOR_ANY_INT_TYPE_INTO
#undef TEST_INT_CONVERSION_FOR_GIVEN_TYPES

BOOST_AUTO_TEST_CASE(test_mojeid_update_contact)
{
    Registry::MojeID::UpdateContact uc;

    uc.name = CorbaConversion::wrap_string("first_name last_name")._retn();
    uc.organization = CorbaConversion::wrap_Nullable_string("org")._retn();
    uc.vat_reg_num = CorbaConversion::wrap_Nullable_string("vat_reg_num")._retn();
    uc.birth_date = CorbaConversion::Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(boost::gregorian::date(2015,12,10))._retn();
    uc.id_card_num = CorbaConversion::wrap_Nullable_string("id_card_num")._retn();
    uc.passport_num = CorbaConversion::wrap_Nullable_string("passport_num")._retn();
    uc.ssn_id_num = CorbaConversion::wrap_Nullable_string("ssn_id_num")._retn();
    uc.vat_id_num = CorbaConversion::wrap_Nullable_string("vat_id_num")._retn();
    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "st1";
        addr_impl.street2 = "st2";
        addr_impl.street3 = "st3";
        addr_impl.city    = "Praha";
        addr_impl.state   = "state";
        addr_impl.country = "Czech Republic";
        CorbaConversion::wrap_Address(addr_impl, uc.permanent);
    }
    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "m_st1";
        addr_impl.street2 = "m_st2";
        addr_impl.street3 = "m_st3";
        addr_impl.city    = "m_Praha";
        addr_impl.state   = "m_state";
        addr_impl.country = "m_Czech Republic";

        uc.mailing = CorbaConversion::wrap_Nullable_Address(addr_impl)._retn();
    }
    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "b_st1";
        addr_impl.street2 = "b_st2";
        addr_impl.street3 = "b_st3";
        addr_impl.city    = "b_Praha";
        addr_impl.state   = "b_state";
        addr_impl.country = "b_Czech Republic";

        uc.billing = CorbaConversion::wrap_Nullable_Address(addr_impl)._retn();
    }
    {
        Fred::Backend::MojeIdImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s_company";
        addr_impl.street1      = "s_st1";
        addr_impl.street2      = "s_st2";
        addr_impl.street3      = "s_st3";
        addr_impl.city         = "s_Praha";
        addr_impl.state        = "s_state";
        addr_impl.country      = "s_Czech Republic";

        uc.shipping = CorbaConversion::wrap_Nullable_ShippingAddress(addr_impl)._retn();
    }
    {
        Fred::Backend::MojeIdImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s2_company";
        addr_impl.street1      = "s2_st1";
        addr_impl.street2      = "s2_st2";
        addr_impl.street3      = "s2_st3";
        addr_impl.city         = "s2_Praha";
        addr_impl.state        = "s2_state";
        addr_impl.country      = "s2_Czech Republic";

        uc.shipping2 = CorbaConversion::wrap_Nullable_ShippingAddress(addr_impl)._retn();
    }
    {
        Fred::Backend::MojeIdImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s3_company";
        addr_impl.street1      = "s3_st1";
        addr_impl.street2      = "s3_st2";
        addr_impl.street3      = "s3_st3";
        addr_impl.city         = "s3_Praha";
        addr_impl.state        = "s3_state";
        addr_impl.country      = "s3_Czech Republic";

        uc.shipping3 = CorbaConversion::wrap_Nullable_ShippingAddress(addr_impl)._retn();
    }
    uc.email = CorbaConversion::wrap_string("email")._retn();
    uc.notify_email = CorbaConversion::wrap_Nullable_string("notify_email")._retn();
    uc.telephone = CorbaConversion::wrap_Nullable_string("telephone")._retn();
    uc.fax = CorbaConversion::wrap_Nullable_string("fax")._retn();

    Fred::Backend::MojeIdImplData::UpdateContact uc_impl;
    CorbaConversion::unwrap_UpdateContact(uc, uc_impl);

    BOOST_CHECK_EQUAL(uc_impl.name, "first_name last_name");
    BOOST_CHECK(uc_impl.organization.get_value() == "org");
    BOOST_CHECK(uc_impl.vat_reg_num.get_value() == "vat_reg_num");
    BOOST_CHECK(boost::gregorian::from_simple_string(uc_impl.birth_date.get_value().value) == boost::gregorian::date(2015,12,10));
    BOOST_CHECK(uc_impl.id_card_num.get_value() == "id_card_num");
    BOOST_CHECK(uc_impl.passport_num.get_value() == "passport_num");
    BOOST_CHECK(uc_impl.ssn_id_num.get_value() == "ssn_id_num");
    BOOST_CHECK(uc_impl.vat_id_num.get_value() == "vat_id_num");


    BOOST_CHECK(uc_impl.permanent.street1 == "st1");
    BOOST_CHECK(uc_impl.permanent.street2.get_value() == "st2");
    BOOST_CHECK(uc_impl.permanent.street3.get_value() == "st3");
    BOOST_CHECK(uc_impl.permanent.city == "Praha");
    BOOST_CHECK(uc_impl.permanent.state.get_value() == "state");
    BOOST_CHECK(uc_impl.permanent.country == "Czech Republic");

    BOOST_CHECK(uc_impl.mailing.get_value().street1 == "m_st1");
    BOOST_CHECK(uc_impl.mailing.get_value().street2.get_value() == "m_st2");
    BOOST_CHECK(uc_impl.mailing.get_value().street3.get_value() == "m_st3");
    BOOST_CHECK(uc_impl.mailing.get_value().city == "m_Praha");
    BOOST_CHECK(uc_impl.mailing.get_value().state.get_value() == "m_state");
    BOOST_CHECK(uc_impl.mailing.get_value().country == "m_Czech Republic");

    BOOST_CHECK(uc_impl.billing.get_value().street1 == "b_st1");
    BOOST_CHECK(uc_impl.billing.get_value().street2.get_value() == "b_st2");
    BOOST_CHECK(uc_impl.billing.get_value().street3.get_value() == "b_st3");
    BOOST_CHECK(uc_impl.billing.get_value().city == "b_Praha");
    BOOST_CHECK(uc_impl.billing.get_value().state.get_value() == "b_state");
    BOOST_CHECK(uc_impl.billing.get_value().country == "b_Czech Republic");

    BOOST_CHECK(uc_impl.shipping.get_value().company_name.get_value() == "s_company");
    BOOST_CHECK(uc_impl.shipping.get_value().street1 == "s_st1");
    BOOST_CHECK(uc_impl.shipping.get_value().street2.get_value() == "s_st2");
    BOOST_CHECK(uc_impl.shipping.get_value().street3.get_value() == "s_st3");
    BOOST_CHECK(uc_impl.shipping.get_value().city == "s_Praha");
    BOOST_CHECK(uc_impl.shipping.get_value().state.get_value() == "s_state");
    BOOST_CHECK(uc_impl.shipping.get_value().country == "s_Czech Republic");

    BOOST_CHECK(uc_impl.shipping2.get_value().company_name.get_value() == "s2_company");
    BOOST_CHECK(uc_impl.shipping2.get_value().street1 == "s2_st1");
    BOOST_CHECK(uc_impl.shipping2.get_value().street2.get_value() == "s2_st2");
    BOOST_CHECK(uc_impl.shipping2.get_value().street3.get_value() == "s2_st3");
    BOOST_CHECK(uc_impl.shipping2.get_value().city == "s2_Praha");
    BOOST_CHECK(uc_impl.shipping2.get_value().state.get_value() == "s2_state");
    BOOST_CHECK(uc_impl.shipping2.get_value().country == "s2_Czech Republic");

    BOOST_CHECK(uc_impl.shipping3.get_value().company_name.get_value() == "s3_company");
    BOOST_CHECK(uc_impl.shipping3.get_value().street1 == "s3_st1");
    BOOST_CHECK(uc_impl.shipping3.get_value().street2.get_value() == "s3_st2");
    BOOST_CHECK(uc_impl.shipping3.get_value().street3.get_value() == "s3_st3");
    BOOST_CHECK(uc_impl.shipping3.get_value().city == "s3_Praha");
    BOOST_CHECK(uc_impl.shipping3.get_value().state.get_value() == "s3_state");
    BOOST_CHECK(uc_impl.shipping3.get_value().country == "s3_Czech Republic");

    BOOST_CHECK(uc_impl.email == "email");
    BOOST_CHECK(uc_impl.notify_email.get_value() == "notify_email");
    BOOST_CHECK(uc_impl.telephone.get_value() == "telephone");
    BOOST_CHECK(uc_impl.fax.get_value() == "fax");
}

BOOST_AUTO_TEST_CASE(test_mojeid_set_contact)
{
    Registry::MojeID::UpdateTransferContact sc;

    sc.organization = CorbaConversion::wrap_Nullable_string("org")._retn();
    sc.vat_reg_num = CorbaConversion::wrap_Nullable_string("vat_reg_num")._retn();
    sc.birth_date = CorbaConversion::Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(boost::gregorian::date(2015,12,10))._retn();
    sc.vat_id_num = CorbaConversion::wrap_Nullable_string("vat_id_num")._retn();
    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "st1";
        addr_impl.street2 = "st2";
        addr_impl.street3 = "st3";
        addr_impl.city    = "Praha";
        addr_impl.state   = "state";
        addr_impl.country = "Czech Republic";
        CorbaConversion::wrap_Address(addr_impl, sc.permanent);
    }
    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "m_st1";
        addr_impl.street2 = "m_st2";
        addr_impl.street3 = "m_st3";
        addr_impl.city    = "m_Praha";
        addr_impl.state   = "m_state";
        addr_impl.country = "m_Czech Republic";

        sc.mailing = CorbaConversion::wrap_Nullable_Address(addr_impl)._retn();
    }

    sc.email = CorbaConversion::wrap_string("email")._retn();
    sc.notify_email = CorbaConversion::wrap_Nullable_string("notify_email")._retn();
    sc.telephone = CorbaConversion::wrap_string("telephone")._retn();
    sc.fax = CorbaConversion::wrap_Nullable_string("fax")._retn();

    Fred::Backend::MojeIdImplData::UpdateTransferContact sc_impl;
    CorbaConversion::unwrap_UpdateTransferContact(sc, sc_impl);

    BOOST_CHECK(sc_impl.organization.get_value() == "org");
    BOOST_CHECK(sc_impl.vat_reg_num.get_value() == "vat_reg_num");
    BOOST_CHECK(boost::gregorian::from_simple_string(sc_impl.birth_date.get_value().value) == boost::gregorian::date(2015,12,10));
    BOOST_CHECK(sc_impl.vat_id_num.get_value() == "vat_id_num");

    BOOST_CHECK(sc_impl.permanent.street1 == "st1");
    BOOST_CHECK(sc_impl.permanent.street2.get_value() == "st2");
    BOOST_CHECK(sc_impl.permanent.street3.get_value() == "st3");
    BOOST_CHECK(sc_impl.permanent.city == "Praha");
    BOOST_CHECK(sc_impl.permanent.state.get_value() == "state");
    BOOST_CHECK(sc_impl.permanent.country == "Czech Republic");

    BOOST_CHECK(sc_impl.mailing.get_value().street1 == "m_st1");
    BOOST_CHECK(sc_impl.mailing.get_value().street2.get_value() == "m_st2");
    BOOST_CHECK(sc_impl.mailing.get_value().street3.get_value() == "m_st3");
    BOOST_CHECK(sc_impl.mailing.get_value().city == "m_Praha");
    BOOST_CHECK(sc_impl.mailing.get_value().state.get_value() == "m_state");
    BOOST_CHECK(sc_impl.mailing.get_value().country == "m_Czech Republic");

    BOOST_CHECK(sc_impl.email == "email");
    BOOST_CHECK(sc_impl.notify_email.get_value() == "notify_email");
    BOOST_CHECK(sc_impl.telephone == "telephone");
    BOOST_CHECK(sc_impl.fax.get_value() == "fax");
}

BOOST_AUTO_TEST_CASE(test_mojeid_info_contact)
{

    Fred::Backend::MojeIdImplData::InfoContact impl_ic;

    impl_ic.id = 5;
    impl_ic.name = "first_name last_name";
    impl_ic.organization = "org";
    impl_ic.vat_reg_num = "vat_reg_num";
    {
        Fred::Backend::MojeIdImplData::Birthdate birthdate;
        birthdate.value = boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,10));
        impl_ic.birth_date = birthdate;
    }
    impl_ic.id_card_num = "id_card_num";
    impl_ic.passport_num = "passport_num";
    impl_ic.ssn_id_num = "ssn_id_num";
    impl_ic.vat_id_num = "vat_id_num";

    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "st1";
        addr_impl.street2 = "st2";
        addr_impl.street3 = "st3";
        addr_impl.city = "Praha";
        addr_impl.state = "state";
        addr_impl.country = "Czech Republic";

        impl_ic.permanent = addr_impl;
    }

    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "m_st1";
        addr_impl.street2 = "m_st2";
        addr_impl.street3 = "m_st3";
        addr_impl.city = "m_Praha";
        addr_impl.state = "m_state";
        addr_impl.country = "m_Czech Republic";

        impl_ic.mailing = addr_impl;
    }
    {
        Fred::Backend::MojeIdImplData::Address addr_impl;
        addr_impl.street1 = "b_st1";
        addr_impl.street2 = "b_st2";
        addr_impl.street3 = "b_st3";
        addr_impl.city = "b_Praha";
        addr_impl.state = "b_state";
        addr_impl.country = "b_Czech Republic";

        impl_ic.billing = addr_impl;
    }
    {
        Fred::Backend::MojeIdImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s_company";
        addr_impl.street1 = "s_st1";
        addr_impl.street2 = "s_st2";
        addr_impl.street3 = "s_st3";
        addr_impl.city = "s_Praha";
        addr_impl.state = "s_state";
        addr_impl.country = "s_Czech Republic";

        impl_ic.shipping = addr_impl;
    }
    {
        Fred::Backend::MojeIdImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s2_company";
        addr_impl.street1 = "s2_st1";
        addr_impl.street2 = "s2_st2";
        addr_impl.street3 = "s2_st3";
        addr_impl.city = "s2_Praha";
        addr_impl.state = "s2_state";
        addr_impl.country = "s2_Czech Republic";

        impl_ic.shipping2 = addr_impl;
    }
    {
        Fred::Backend::MojeIdImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s3_company";
        addr_impl.street1 = "s3_st1";
        addr_impl.street2 = "s3_st2";
        addr_impl.street3 = "s3_st3";
        addr_impl.city = "s3_Praha";
        addr_impl.state = "s3_state";
        addr_impl.country = "s3_Czech Republic";

        impl_ic.shipping3 = addr_impl;
    }

    impl_ic.email = "email";
    impl_ic.notify_email = "notify_email";
    impl_ic.telephone = "telephone";
    impl_ic.fax = "fax";


    const Registry::MojeID::InfoContact_var idl_ic_ptr = CorbaConversion::wrap_InfoContact(impl_ic)._retn();
    BOOST_REQUIRE(idl_ic_ptr.operator->() != NULL);

    BOOST_CHECK(idl_ic_ptr->id == 5);
    BOOST_CHECK(idl_ic_ptr->name.in()                       == impl_ic.name);
    BOOST_CHECK(idl_ic_ptr->organization->_value()          == impl_ic.organization.get_value());
    BOOST_CHECK(idl_ic_ptr->vat_reg_num->_value()           == impl_ic.vat_reg_num.get_value());
    BOOST_CHECK(idl_ic_ptr->birth_date->_value().value.in() == impl_ic.birth_date.get_value().value);
    BOOST_CHECK(idl_ic_ptr->id_card_num->_value()           == impl_ic.id_card_num.get_value());
    BOOST_CHECK(idl_ic_ptr->passport_num->_value()          == impl_ic.passport_num.get_value());
    BOOST_CHECK(idl_ic_ptr->ssn_id_num->_value()            == impl_ic.ssn_id_num.get_value());
    BOOST_CHECK(idl_ic_ptr->vat_id_num->_value()            == impl_ic.vat_id_num.get_value());


    BOOST_CHECK(idl_ic_ptr->permanent.street1.in() == impl_ic.permanent.street1);
    BOOST_CHECK(idl_ic_ptr->permanent.street2.in()->_value() == impl_ic.permanent.street2.get_value());
    BOOST_CHECK(idl_ic_ptr->permanent.street3.in()->_value() == impl_ic.permanent.street3.get_value());
    BOOST_CHECK(idl_ic_ptr->permanent.city.in() == impl_ic.permanent.city);
    BOOST_CHECK(idl_ic_ptr->permanent.state.in()->_value() == impl_ic.permanent.state.get_value());
    BOOST_CHECK(idl_ic_ptr->permanent.country.in() == impl_ic.permanent.country);

    Nullable< Fred::Backend::MojeIdImplData::Address > mailing;
    CorbaConversion::unwrap_NullableAddress(idl_ic_ptr->mailing.in(), mailing);
    BOOST_REQUIRE(!mailing.isnull());

    BOOST_CHECK(mailing.get_value().street1 == impl_ic.mailing.get_value().street1);
    BOOST_CHECK(mailing.get_value().street2.get_value() == impl_ic.mailing.get_value().street2.get_value());
    BOOST_CHECK(mailing.get_value().street3.get_value() == impl_ic.mailing.get_value().street3.get_value());
    BOOST_CHECK(mailing.get_value().city == impl_ic.mailing.get_value().city);
    BOOST_CHECK(mailing.get_value().state.get_value() == impl_ic.mailing.get_value().state.get_value());
    BOOST_CHECK(mailing.get_value().country == impl_ic.mailing.get_value().country);

    Nullable< Fred::Backend::MojeIdImplData::Address > billing;
    CorbaConversion::unwrap_NullableAddress(idl_ic_ptr->billing.in(), billing);
    BOOST_REQUIRE(!billing.isnull());

    BOOST_CHECK(billing.get_value().street1 == impl_ic.billing.get_value().street1);
    BOOST_CHECK(billing.get_value().street2.get_value() == impl_ic.billing.get_value().street2.get_value());
    BOOST_CHECK(billing.get_value().street3.get_value() == impl_ic.billing.get_value().street3.get_value());
    BOOST_CHECK(billing.get_value().city == impl_ic.billing.get_value().city);
    BOOST_CHECK(billing.get_value().state.get_value() == impl_ic.billing.get_value().state.get_value());
    BOOST_CHECK(billing.get_value().country == impl_ic.billing.get_value().country);

    Nullable< Fred::Backend::MojeIdImplData::ShippingAddress > shipping;
    CorbaConversion::unwrap_NullableShippingAddress(idl_ic_ptr->shipping.in(), shipping);
    BOOST_REQUIRE(!shipping.isnull());

    BOOST_CHECK(shipping.get_value().company_name.get_value() == impl_ic.shipping.get_value().company_name.get_value());
    BOOST_CHECK(shipping.get_value().street1 == impl_ic.shipping.get_value().street1);
    BOOST_CHECK(shipping.get_value().street2.get_value() == impl_ic.shipping.get_value().street2.get_value());
    BOOST_CHECK(shipping.get_value().street3.get_value() == impl_ic.shipping.get_value().street3.get_value());
    BOOST_CHECK(shipping.get_value().city == impl_ic.shipping.get_value().city);
    BOOST_CHECK(shipping.get_value().state.get_value() == impl_ic.shipping.get_value().state.get_value());
    BOOST_CHECK(shipping.get_value().country == impl_ic.shipping.get_value().country);

    Nullable< Fred::Backend::MojeIdImplData::ShippingAddress > shipping2;
    CorbaConversion::unwrap_NullableShippingAddress(idl_ic_ptr->shipping2.in(), shipping2);
    BOOST_REQUIRE(!shipping2.isnull());

    BOOST_CHECK(shipping2.get_value().company_name.get_value() == impl_ic.shipping2.get_value().company_name.get_value());
    BOOST_CHECK(shipping2.get_value().street1 == impl_ic.shipping2.get_value().street1);
    BOOST_CHECK(shipping2.get_value().street2.get_value() == impl_ic.shipping2.get_value().street2.get_value());
    BOOST_CHECK(shipping2.get_value().street3.get_value() == impl_ic.shipping2.get_value().street3.get_value());
    BOOST_CHECK(shipping2.get_value().city == impl_ic.shipping2.get_value().city);
    BOOST_CHECK(shipping2.get_value().state.get_value() == impl_ic.shipping2.get_value().state.get_value());
    BOOST_CHECK(shipping2.get_value().country == impl_ic.shipping2.get_value().country);

    Nullable< Fred::Backend::MojeIdImplData::ShippingAddress > shipping3;
    CorbaConversion::unwrap_NullableShippingAddress(idl_ic_ptr->shipping3.in(), shipping3);
    BOOST_REQUIRE(!shipping3.isnull());

    BOOST_CHECK(shipping3.get_value().company_name.get_value() == impl_ic.shipping3.get_value().company_name.get_value());
    BOOST_CHECK(shipping3.get_value().street1 == impl_ic.shipping3.get_value().street1);
    BOOST_CHECK(shipping3.get_value().street2.get_value() == impl_ic.shipping3.get_value().street2.get_value());
    BOOST_CHECK(shipping3.get_value().street3.get_value() == impl_ic.shipping3.get_value().street3.get_value());
    BOOST_CHECK(shipping3.get_value().city == impl_ic.shipping3.get_value().city);
    BOOST_CHECK(shipping3.get_value().state.get_value() == impl_ic.shipping3.get_value().state.get_value());
    BOOST_CHECK(shipping3.get_value().country == impl_ic.shipping3.get_value().country);

    BOOST_CHECK(idl_ic_ptr->email.in() == impl_ic.email);
    BOOST_CHECK(idl_ic_ptr->notify_email.in()->_value() == impl_ic.notify_email.get_value());
    BOOST_CHECK(idl_ic_ptr->telephone.in()->_value() == impl_ic.telephone.get_value());
    BOOST_CHECK(idl_ic_ptr->fax.in()->_value() == impl_ic.fax.get_value());
}

BOOST_AUTO_TEST_CASE(test_mojeid_contact_state_info)
{

    Fred::Backend::MojeIdImplData::ContactStateInfo impl_info;

    impl_info.contact_id = 6;
    impl_info.mojeid_activation_datetime = boost::posix_time::ptime(boost::gregorian::date(2015,12,10));
    impl_info.identification_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,12));
    impl_info.validation_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,13));
    impl_info.linked_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,14));

    Registry::MojeID::ContactStateInfo idl_info;
    CorbaConversion::wrap_ContactStateInfo(impl_info, idl_info);

    BOOST_CHECK(idl_info.contact_id == impl_info.contact_id);
    BOOST_CHECK(idl_info.mojeid_activation_datetime.value.in() == boost::posix_time::to_iso_extended_string(impl_info.mojeid_activation_datetime) + "Z");
    BOOST_CHECK(idl_info.identification_date->_value().value.in() == boost::gregorian::to_iso_extended_string(impl_info.identification_date.get_value()));
    BOOST_CHECK(idl_info.validation_date->_value().value.in() == boost::gregorian::to_iso_extended_string(impl_info.validation_date.get_value()));
    BOOST_CHECK(idl_info.linked_date->_value().value.in() == boost::gregorian::to_iso_extended_string(impl_info.linked_date.get_value()));
}

BOOST_AUTO_TEST_CASE(test_mojeid_contact_state_info_list)
{
    Fred::Backend::MojeIdImplData::ContactStateInfoList impl_list;
    Registry::MojeID::ContactStateInfoList_var idl_list_ptr = CorbaConversion::wrap_ContactStateInfoList(impl_list)._retn();
    BOOST_REQUIRE(idl_list_ptr.operator->() != NULL);
    BOOST_CHECK(idl_list_ptr->length() == 0);

    {
        Fred::Backend::MojeIdImplData::ContactStateInfo info_impl;
        info_impl.contact_id = 5;
        info_impl.mojeid_activation_datetime = boost::posix_time::ptime(boost::gregorian::date(2015,12,10));
        info_impl.identification_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,12));
        info_impl.validation_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,13));
        info_impl.linked_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,14));
        impl_list.push_back(info_impl);
    }
    {
        Fred::Backend::MojeIdImplData::ContactStateInfo info_impl;
        info_impl.contact_id = 6;
        info_impl.mojeid_activation_datetime = boost::posix_time::ptime(boost::gregorian::date(2016,12,10));
        info_impl.identification_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2016,12,12));
        info_impl.validation_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2016,12,13));
        info_impl.linked_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2016,12,14));
        impl_list.push_back(info_impl);
    }

    idl_list_ptr = CorbaConversion::wrap_ContactStateInfoList(impl_list)._retn();
    BOOST_REQUIRE(idl_list_ptr.operator ->() != NULL);
    BOOST_REQUIRE(idl_list_ptr->length() == 2);

    for (::size_t idx = 0; idx < idl_list_ptr->length(); ++idx) {
        BOOST_CHECK(idl_list_ptr[idx].contact_id == impl_list[idx].contact_id);
        BOOST_CHECK(idl_list_ptr[idx].mojeid_activation_datetime.value.in() == boost::posix_time::to_iso_extended_string(impl_list[idx].mojeid_activation_datetime) + "Z");
        BOOST_CHECK(idl_list_ptr[idx].identification_date->_value().value.in() == boost::gregorian::to_iso_extended_string(impl_list[idx].identification_date.get_value()));
        BOOST_CHECK(idl_list_ptr[idx].validation_date->_value().value.in() == boost::gregorian::to_iso_extended_string(impl_list[idx].validation_date.get_value()));
        BOOST_CHECK(idl_list_ptr[idx].linked_date->_value().value.in() == boost::gregorian::to_iso_extended_string(impl_list[idx].linked_date.get_value()));
    }
}

BOOST_AUTO_TEST_CASE(test_mojeid_buffer)
{
    Fred::Backend::Buffer impl_buffer("test");
    Registry::Buffer_var idl_buffer_ptr = CorbaConversion::Util::wrap_Buffer(impl_buffer);
    BOOST_REQUIRE(idl_buffer_ptr.operator ->() != NULL);
    BOOST_CHECK(std::string(reinterpret_cast< const char* >(idl_buffer_ptr->data.get_buffer()),
                            idl_buffer_ptr->data.length()) == impl_buffer.data);
}

BOOST_AUTO_TEST_CASE(test_mojeid_constact_handle_list)
{
    const Fred::Backend::MojeIdImplData::ContactHandleList impl_list = Util::vector_of< std::string >("test1")("test2")("test3");
    const Registry::MojeID::ContactHandleList_var ssv1 = CorbaConversion::wrap_ContactHandleList(impl_list)._retn();
    BOOST_REQUIRE(ssv1.operator ->() != NULL);
    BOOST_REQUIRE(ssv1->length() == 3);
    for (::size_t idx = 0; idx < ssv1->length(); ++idx) {
        BOOST_CHECK(ssv1.in()[idx].in() == impl_list[idx]);
    }
}

BOOST_AUTO_TEST_CASE(test_nullable_iso_date)
{
    const boost::gregorian::date valid_boost_date(2018, 3, 14);
    const std::string valid_boost_date_as_string = boost::gregorian::to_iso_extended_string(valid_boost_date);
    BOOST_CHECK(valid_boost_date_as_string == "2018-03-14");
    const Nullable<boost::gregorian::date> nullable_valid_boost_date(valid_boost_date);

    const Registry::NullableIsoDate_var nullable_valid_iso_date = CorbaConversion::Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(nullable_valid_boost_date)._retn();
    BOOST_CHECK(std::string(nullable_valid_iso_date->value()) == valid_boost_date_as_string);

    const Nullable<boost::gregorian::date> nullable_null_date;
    const Registry::NullableIsoDate_var nullable_null_iso_date = CorbaConversion::Util::wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(nullable_null_date)._retn();
    BOOST_CHECK(nullable_null_iso_date.in() == NULL);

    Nullable<boost::gregorian::date> res1;
    CorbaConversion::Util::unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(nullable_valid_iso_date.in(), res1);
    BOOST_CHECK(!res1.isnull());
    BOOST_CHECK(res1.get_value() == valid_boost_date);

    Nullable<boost::gregorian::date> res1x = CorbaConversion::Util::unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(nullable_valid_iso_date.in());
    BOOST_CHECK(!res1x.isnull());
    BOOST_CHECK(res1x.get_value() == valid_boost_date);

    Nullable<boost::gregorian::date> res2;
    CorbaConversion::Util::unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(nullable_null_iso_date.in(), res2);
    BOOST_CHECK(res2.isnull());

    Nullable<boost::gregorian::date> res2x = CorbaConversion::Util::unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(nullable_null_iso_date.in());
    BOOST_CHECK(res2x.isnull());
}

BOOST_AUTO_TEST_SUITE_END();
