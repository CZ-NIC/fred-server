/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#include <limits>
#include <fstream>
#include <ios>
#include <iterator>
#include <iomanip>
#include <sstream>
#include <utility>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "config.h"
#include "src/fredlib/opcontext.h"
#include "src/cli_admin/read_config_file.h"
#include "util/util.h"
#include "util/map_at.h"
#include "util/subprocess.h"
#include "util/printable.h"
#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

#include "util/corba_conversion.h"

#include "src/corba/corba_conversion_test.hh"
#include "src/corba/mojeid/mojeid_corba_conversion.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

namespace CorbaConversion
{
    /* DEFAULT_UNWRAPPER redefinition error

    //strange string unwrap
    struct Unwrapper_strange_const_char_ptr_into_std_string
    {
        typedef const char* CORBA_TYPE;
        typedef std::string NON_CORBA_TYPE;
        static void unwrap( CORBA_TYPE ct_in, NON_CORBA_TYPE& nct_out)
        {
            nct_out = "strange";
        }
    };
    template <> struct DEFAULT_UNWRAPPER<
    Unwrapper_strange_const_char_ptr_into_std_string::CORBA_TYPE,
    Unwrapper_strange_const_char_ptr_into_std_string::NON_CORBA_TYPE>
    {
        typedef Unwrapper_strange_const_char_ptr_into_std_string type;
    };
     */

    template < >
    struct DEFAULT_WRAPPER< std::string, Test::NullableString >
    :   Wrapper_value_into_Nullable< std::string, Test::NullableString > { };

    template < >
    struct DEFAULT_UNWRAPPER< Test::NullableString*, Nullable< std::string > >
    :   Unwrapper_ptr_into_Nullable< Test::NullableString, std::string > { };

    template < >
    struct DEFAULT_WRAPPER< std::vector< std::string >, CORBA::StringSeq >
    :   Wrapper_std_vector_into_Seq_of_holders< std::vector< std::string >, CORBA::StringSeq > { };

    template < >
    struct DEFAULT_WRAPPER< std::vector< std::string >, Test::StringSeq >
    :   Wrapper_std_vector_into_Seq_of_holders< std::vector< std::string >, Test::StringSeq > { };

    template < >
    struct DEFAULT_UNWRAPPER< CORBA::StringSeq, std::vector< std::string > >
    :   Unwrapper_Seq_of_holders_into_std_vector< CORBA::StringSeq,
                                                  std::vector< std::string > > { };

    template < >
    struct DEFAULT_UNWRAPPER< Test::StringSeq, std::vector< std::string > >
    :   Unwrapper_Seq_of_holders_into_std_vector< Test::StringSeq,
                                                  std::vector< std::string > > { };
}

BOOST_AUTO_TEST_SUITE(TestCorbaConversion)

const std::string server_name = "test-corba-conversion";


BOOST_AUTO_TEST_CASE(test_string_wrap_by_unwrap_by)
{
    CORBA::String_var sv1;
    CorbaConversion::wrap_into_holder(std::string("test"), sv1);
    BOOST_CHECK(sv1.in() == std::string("test"));
    std::string ss1 = CorbaConversion::unwrap_into_by< CorbaConversion::Unwrapper_const_char_ptr_into_std_string >(static_cast< const char* >(sv1));
    BOOST_CHECK(ss1 == std::string("test"));
}

BOOST_AUTO_TEST_CASE(test_string_wrap_unwrap)
{
    CORBA::String_var sv1;
    std::string ss1("test");
    CorbaConversion::wrap_into_holder(ss1, sv1);
    BOOST_CHECK(std::string(sv1.in()) == ss1);
    std::string ss2;
    CorbaConversion::unwrap(sv1.in(), ss2);
    BOOST_CHECK(std::string(sv1.in()) == ss2);
}

BOOST_AUTO_TEST_CASE(test_string_wrap_into_unwrap_into)
{
    CORBA::String_var sv1;
    CorbaConversion::wrap_into_holder(std::string("test"), sv1);
    BOOST_CHECK(std::string(sv1.in()) == std::string("test"));
    std::string ss1 = CorbaConversion::unwrap_into<std::string>(sv1.in());
    BOOST_CHECK(ss1 == std::string("test"));
}

BOOST_AUTO_TEST_CASE(test_impl_string_wrap_unwrap)
{
    CORBA::String_var sv1;
    std::string ss1("test");
    CorbaConversion::wrap_into_holder(ss1, sv1);
    BOOST_CHECK(std::string(sv1.in()) == ss1);
    std::string ss2;
    CorbaConversion::Unwrapper_const_char_ptr_into_std_string::unwrap(sv1, ss2);
    BOOST_CHECK(std::string(sv1.in()) == ss2);
}

BOOST_AUTO_TEST_CASE(test_string_seq_ref)
{
    CORBA::StringSeq_var ssv1 = new CORBA::StringSeq;
    std::vector< std::string > vs1 = Util::vector_of<std::string>("test1")("test2")("test3");

    CorbaConversion::Wrapper_std_vector_into_Seq_of_holders< std::vector< std::string >, CORBA::StringSeq >
        ::wrap(vs1, ssv1.inout());

    BOOST_CHECK(ssv1->length() == vs1.size());

    CORBA::String_var sv0;
    sv0 = ssv1[0];
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(sv0.in()) == vs1[0]);

    CORBA::String_var sv1 = ssv1[1];
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(sv1.in()) == vs1[1]);

    bool string_seq_element_check = (CorbaConversion::unwrap_into<std::string, const char *>(ssv1[2]) == vs1[2]);
    BOOST_CHECK(string_seq_element_check);

    std::vector<std::string> vs2;

    CorbaConversion::Unwrapper_Seq_of_holders_into_std_vector< CORBA::StringSeq, std::vector< std::string > >
        ::unwrap(ssv1.in(), vs2);

    BOOST_CHECK(vs2.size() == vs1.size());
    BOOST_CHECK(vs2[0] == vs1[0]);
    BOOST_CHECK(vs2[1] == vs1[1]);
    BOOST_CHECK(vs2[2] == vs1[2]);
}

BOOST_AUTO_TEST_CASE(test_string_seq)
{
    CORBA::StringSeq_var ssv1 = new CORBA::StringSeq;
    std::vector<std::string> vs1 = Util::vector_of<std::string>("test1")("test2")("test3");
    CorbaConversion::Wrapper_std_vector_into_Seq_of_holders< std::vector< std::string >,
                                                             CORBA::StringSeq >::wrap(vs1, ssv1);

    BOOST_CHECK(ssv1->length() == vs1.size());

    CORBA::String_var sv0;
    sv0 = ssv1[0];
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(sv0.in()) == vs1[0]);

    CORBA::String_var sv1 = ssv1[1];
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(sv1.in()) == vs1[1]);

    bool string_seq_element_check = (CorbaConversion::unwrap_into<std::string, const char *>(ssv1[2]) == vs1[2]);
    BOOST_CHECK(string_seq_element_check);
}

BOOST_AUTO_TEST_CASE(test_string_seq_wrap_into_corba)
{
    CORBA::StringSeq_var ssv1;
    CorbaConversion::wrap_into_holder(
        std::vector< std::string >(Util::vector_of< std::string >("test1")("test2")("test3")), ssv1);
    std::vector< std::string > vs1;
    CorbaConversion::unwrap_holder(ssv1, vs1);

    BOOST_CHECK(vs1.size() == 3);
    BOOST_CHECK(vs1[0] == "test1");
    BOOST_CHECK(vs1[1] == "test2");
    BOOST_CHECK(vs1[2] == "test3");
}

BOOST_AUTO_TEST_CASE(test_string_seq_wrap_into_test)
{
    Test::StringSeq_var ssv1;
    CorbaConversion::wrap_into_holder(
        std::vector< std::string >(Util::vector_of< std::string >("test1")("test2")("test3")), ssv1);
    std::vector< std::string > vs1;
    CorbaConversion::unwrap_holder(ssv1, vs1);

    BOOST_CHECK(vs1.size() == 3);
    BOOST_CHECK(vs1[0] == "test1");
    BOOST_CHECK(vs1[1] == "test2");
    BOOST_CHECK(vs1[2] == "test3");
}

//basic types
BOOST_AUTO_TEST_CASE(test_short)
{
    short a =1;
    CORBA::Short ca = CorbaConversion::wrap_into<CORBA::Short>(a);
    BOOST_CHECK(a == ca);

    BOOST_CHECK(CorbaConversion::unwrap_into<short>(CORBA::Short(1)) == 1);

    //error
    //BOOST_CHECK(CorbaConversion::wrap_into<CORBA::Short>(2ull) == 2);
}

BOOST_AUTO_TEST_CASE(test_valuetype_string)
{
    const Nullable< std::string > ns1("test1");
    Test::NullableString_var tnsv1;
    CorbaConversion::wrap_nullable_into_holder(ns1, tnsv1);
    BOOST_CHECK(std::string(tnsv1->_value()) == ns1.get_value());

    Nullable< std::string > ns2;
    CorbaConversion::unwrap_holder(tnsv1, ns2);
    BOOST_CHECK(std::string(tnsv1->_value()) == ns2.get_value());
}

BOOST_AUTO_TEST_CASE(test_mojeid_valuetype_string)
{
    Nullable<std::string> ns1 ("test1");
    Registry::MojeID::NullableString_var tnsv1;
    CorbaConversion::wrap_nullable_into_holder(ns1, tnsv1);
    BOOST_CHECK(std::string(tnsv1->_value()) == ns1.get_value());

    Nullable<std::string> ns2 = CorbaConversion::unwrap_into<Nullable<std::string> >(tnsv1.in());
    BOOST_CHECK(std::string(tnsv1->_value()) == ns2.get_value());
}

BOOST_AUTO_TEST_CASE(test_mojeid_date)
{
    Registry::MojeID::Date mojeid_date;
    CorbaConversion::wrap(boost::gregorian::date(2015,12,10), mojeid_date);
    BOOST_CHECK(CorbaConversion::unwrap_into< std::string >(mojeid_date.value.in()) == "2015-12-10");

    BOOST_CHECK(CorbaConversion::unwrap_into< Registry::MojeIDImplData::Date >(mojeid_date).value ==
                boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,10)));

    BOOST_CHECK_THROW(CorbaConversion::wrap_into< Registry::MojeID::Date >(boost::gregorian::date()), CorbaConversion::ArgumentIsSpecial);
}

BOOST_AUTO_TEST_CASE(test_mojeid_datetime)
{
    Registry::MojeID::DateTime_var mojeid_datetime;
    CorbaConversion::wrap_into_holder(boost::posix_time::ptime(boost::gregorian::date(2015,12,10)), mojeid_datetime);
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(mojeid_datetime->value.in()) == "2015-12-10T00:00:00");
    BOOST_CHECK(CorbaConversion::unwrap_into<boost::posix_time::ptime>(mojeid_datetime.in()) == boost::posix_time::ptime(boost::gregorian::date(2015,12,10)));

    BOOST_CHECK_THROW(CorbaConversion::wrap_into< Registry::MojeID::DateTime >(boost::posix_time::ptime()), CorbaConversion::ArgumentIsSpecial);
    BOOST_CHECK_THROW(CorbaConversion::unwrap_into< boost::posix_time::ptime >(
        CorbaConversion::wrap_into< Registry::MojeID::DateTime >(boost::posix_time::ptime())), CorbaConversion::ArgumentIsSpecial);
}

BOOST_AUTO_TEST_CASE(test_mojeid_nullabledate)
{
    static const Nullable< boost::gregorian::date > d(boost::gregorian::date(2015,12,10));
    Registry::MojeID::NullableDate_var nd1;
    CorbaConversion::wrap_nullable_into_holder(d, nd1);
    BOOST_CHECK(std::string(nd1->value()) == "2015-12-10");
    Registry::MojeID::NullableDate_var nd2;
    CorbaConversion::wrap_nullable_into_holder(Nullable< boost::gregorian::date >(), nd2);
    BOOST_CHECK(nd2.in() == NULL);

    Nullable< Registry::MojeIDImplData::Date > res1 = CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(nd1.operator->());
    BOOST_CHECK(!res1.isnull());
    BOOST_CHECK(res1.get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,10)));
    Nullable< Registry::MojeIDImplData::Date > res2 = CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(nd2.operator->());
    BOOST_CHECK(res2.isnull());
}

BOOST_AUTO_TEST_CASE(test_mojeid_address)
{
    Registry::MojeIDImplData::Address addr_impl;
    addr_impl.street1 = "st1";
    addr_impl.street2 = "st2";
    addr_impl.street3 = "st3";
    addr_impl.city = "Praha";
    addr_impl.state = "state";
    addr_impl.country = "Czech Republic";

    Registry::MojeID::Address_var addr;
    CorbaConversion::wrap_into_holder(addr_impl, addr);
    BOOST_CHECK(std::string(addr->street1.in()) == "st1");
    BOOST_CHECK(std::string(addr->street2.in()->_value()) == "st2");
    BOOST_CHECK(std::string(addr->street3.in()->_value()) == "st3");
    BOOST_CHECK(std::string(addr->city.in()) == "Praha");
    BOOST_CHECK(std::string(addr->state.in()->_value()) == "state");
    BOOST_CHECK(std::string(addr->country.in()) == "Czech Republic");

    Registry::MojeIDImplData::Address addr_res = CorbaConversion::unwrap_into<Registry::MojeIDImplData::Address>(addr.in());
    BOOST_CHECK(addr_res.street1 == "st1");
    BOOST_CHECK(addr_res.street2.get_value() == "st2");
    BOOST_CHECK(addr_res.street3.get_value() == "st3");
    BOOST_CHECK(addr_res.city == "Praha");
    BOOST_CHECK(addr_res.state.get_value() == "state");
    BOOST_CHECK(addr_res.country == "Czech Republic");
}


BOOST_AUTO_TEST_CASE(test_nullable_mojeid_address)
{
    Registry::MojeID::NullableAddress_var nullable_addr_holder;
    CorbaConversion::wrap_nullable_into_holder(Nullable< Registry::MojeIDImplData::Address >(), nullable_addr_holder);
    BOOST_CHECK(nullable_addr_holder.in() == NULL);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::Address> >(static_cast<Registry::MojeID::NullableAddress*>(NULL)).isnull());

    Registry::MojeIDImplData::Address addr_impl;
    addr_impl.street1 = "st1";
    addr_impl.street2 = "st2";
    addr_impl.street3 = "st3";
    addr_impl.city = "Praha";
    addr_impl.state = "state";
    addr_impl.country = "Czech Republic";

    Nullable<Registry::MojeIDImplData::Address> nullable_addr(addr_impl);
    BOOST_CHECK(!nullable_addr.isnull());

    Registry::MojeID::NullableAddress_var addr;
    CorbaConversion::wrap_nullable_into_holder(nullable_addr, addr);
    BOOST_REQUIRE(addr.in() != NULL);
    BOOST_CHECK(std::string(addr->_value().street1.in()) == "st1");
    BOOST_CHECK(std::string(addr->_value().street2.in()->_value()) == "st2");
    BOOST_CHECK(std::string(addr->_value().street3.in()->_value()) == "st3");
    BOOST_CHECK(std::string(addr->_value().city.in()) == "Praha");
    BOOST_CHECK(std::string(addr->_value().state.in()->_value()) == "state");
    BOOST_CHECK(std::string(addr->_value().country.in()) == "Czech Republic");

    Nullable<Registry::MojeIDImplData::Address> addr_res = CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::Address> >(addr.in());
    BOOST_REQUIRE(!addr_res.isnull());
    BOOST_CHECK(addr_res.get_value().street1 == "st1");
    BOOST_CHECK(addr_res.get_value().street2.get_value() == "st2");
    BOOST_CHECK(addr_res.get_value().street3.get_value() == "st3");
    BOOST_CHECK(addr_res.get_value().city == "Praha");
    BOOST_CHECK(addr_res.get_value().state.get_value() == "state");
    BOOST_CHECK(addr_res.get_value().country == "Czech Republic");
}

BOOST_AUTO_TEST_CASE(test_mojeid_shippingaddress)
{
    Registry::MojeIDImplData::ShippingAddress addr_impl;
    addr_impl.company_name = "company";
    addr_impl.street1 = "st1";
    addr_impl.street2 = "st2";
    addr_impl.street3 = "st3";
    addr_impl.city = "Praha";
    addr_impl.state = "state";
    addr_impl.country = "Czech Republic";

    Registry::MojeID::ShippingAddress_var addr;
    CorbaConversion::wrap_into_holder(addr_impl, addr);
    BOOST_CHECK(std::string(addr->company_name.in()->_value()) == "company");
    BOOST_CHECK(std::string(addr->street1.in()) == "st1");
    BOOST_CHECK(std::string(addr->street2.in()->_value()) == "st2");
    BOOST_CHECK(std::string(addr->street3.in()->_value()) == "st3");
    BOOST_CHECK(std::string(addr->city.in()) == "Praha");
    BOOST_CHECK(std::string(addr->state.in()->_value()) == "state");
    BOOST_CHECK(std::string(addr->country.in()) == "Czech Republic");

    Registry::MojeIDImplData::ShippingAddress addr_res = CorbaConversion::unwrap_into<Registry::MojeIDImplData::ShippingAddress>(addr.in());
    BOOST_CHECK(addr_res.company_name.get_value() == "company");
    BOOST_CHECK(addr_res.street1 == "st1");
    BOOST_CHECK(addr_res.street2.get_value() == "st2");
    BOOST_CHECK(addr_res.street3.get_value() == "st3");
    BOOST_CHECK(addr_res.city == "Praha");
    BOOST_CHECK(addr_res.state.get_value() == "state");
    BOOST_CHECK(addr_res.country == "Czech Republic");
}

BOOST_AUTO_TEST_CASE(test_nullable_mojeid_shippingaddress)
{
    Registry::MojeID::NullableShippingAddress_var nullable_value_holder;
    CorbaConversion::wrap_nullable_into_holder(Nullable< Registry::MojeIDImplData::ShippingAddress >(), nullable_value_holder);
    BOOST_CHECK(nullable_value_holder.in() == NULL);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ShippingAddress> >(static_cast<Registry::MojeID::NullableShippingAddress*>(NULL)).isnull());

    Registry::MojeIDImplData::ShippingAddress addr_impl;
    addr_impl.company_name = "company";
    addr_impl.street1 = "st1";
    addr_impl.street2 = "st2";
    addr_impl.street3 = "st3";
    addr_impl.city = "Praha";
    addr_impl.state = "state";
    addr_impl.country = "Czech Republic";

    Nullable<Registry::MojeIDImplData::ShippingAddress> nullable_addr(addr_impl);
    BOOST_CHECK(!nullable_addr.isnull());

    Registry::MojeID::NullableShippingAddress_var addr;
    CorbaConversion::wrap_nullable_into_holder(nullable_addr, addr);
    BOOST_REQUIRE(addr.in() != NULL);
    BOOST_CHECK(std::string(addr->_value().company_name.in()->_value()) == "company");
    BOOST_CHECK(std::string(addr->_value().street1.in()) == "st1");
    BOOST_CHECK(std::string(addr->_value().street2.in()->_value()) == "st2");
    BOOST_CHECK(std::string(addr->_value().street3.in()->_value()) == "st3");
    BOOST_CHECK(std::string(addr->_value().city.in()) == "Praha");
    BOOST_CHECK(std::string(addr->_value().state.in()->_value()) == "state");
    BOOST_CHECK(std::string(addr->_value().country.in()) == "Czech Republic");

    Nullable<Registry::MojeIDImplData::ShippingAddress> addr_res = CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ShippingAddress> >(addr.in());
    BOOST_REQUIRE(!addr_res.isnull());
    BOOST_CHECK(addr_res.get_value().company_name.get_value() == "company");
    BOOST_CHECK(addr_res.get_value().street1 == "st1");
    BOOST_CHECK(addr_res.get_value().street2.get_value() == "st2");
    BOOST_CHECK(addr_res.get_value().street3.get_value() == "st3");
    BOOST_CHECK(addr_res.get_value().city == "Praha");
    BOOST_CHECK(addr_res.get_value().state.get_value() == "state");
    BOOST_CHECK(addr_res.get_value().country == "Czech Republic");
}


/**
 * Exception if argument value is not enum ValidationError value
 */
class NotEnumValidationErrorValue : public std::invalid_argument
{
public:
    NotEnumValidationErrorValue() : std::invalid_argument(
        "argument value is not enum ValidationError value") {}
    virtual ~NotEnumValidationErrorValue() throw() {}
};

BOOST_AUTO_TEST_CASE(test_mojeid_validationresult)
{
    BOOST_CHECK_THROW(CorbaConversion::wrap_into<Registry::MojeID::ValidationResult>(
        Registry::MojeIDImplData::ValidationResult(10)), CorbaConversion::NotEnumValidationResultValue);

    static const Registry::MojeIDImplData::ValidationResult value = Registry::MojeID::INVALID;
    BOOST_CHECK(CorbaConversion::wrap_into< Registry::MojeID::ValidationResult >(value) == value);
}

BOOST_AUTO_TEST_CASE(test_mojeid_addressvalidationresult)
{
    Registry::MojeIDImplData::AddressValidationResult addr_err_impl;
    addr_err_impl.street1     = Registry::MojeID::INVALID;
    addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    addr_err_impl.postal_code = Registry::MojeID::REQUIRED;
    addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeID::AddressValidationResult addr_err;
    CorbaConversion::wrap(addr_err_impl, addr_err);

    BOOST_CHECK(addr_err.street1     == Registry::MojeID::INVALID);
    BOOST_CHECK(addr_err.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(addr_err.postal_code == Registry::MojeID::REQUIRED);
    BOOST_CHECK(addr_err.country     == Registry::MojeID::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_mandatoryaddressvalidationresult)
{
    Registry::MojeIDImplData::MandatoryAddressValidationResult addr_err_impl;
    addr_err_impl.address_presence = Registry::MojeID::REQUIRED;
    addr_err_impl.street1          = Registry::MojeID::INVALID;
    addr_err_impl.city             = Registry::MojeID::NOT_AVAILABLE;
    addr_err_impl.postal_code      = Registry::MojeID::REQUIRED;
    addr_err_impl.country          = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeID::MandatoryAddressValidationResult addr_err;
    CorbaConversion::wrap(addr_err_impl, addr_err);

    BOOST_CHECK(addr_err.address_presence == Registry::MojeID::REQUIRED);
    BOOST_CHECK(addr_err.street1          == Registry::MojeID::INVALID);
    BOOST_CHECK(addr_err.city             == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(addr_err.postal_code      == Registry::MojeID::REQUIRED);
    BOOST_CHECK(addr_err.country          == Registry::MojeID::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_shippingaddressvalidationresult)
{
    Registry::MojeIDImplData::ShippingAddressValidationResult addr_err_impl;
    addr_err_impl.street1     = Registry::MojeID::INVALID;
    addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    addr_err_impl.postal_code = Registry::MojeID::REQUIRED;
    addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeID::ShippingAddressValidationResult addr_err;
    CorbaConversion::wrap(addr_err_impl, addr_err);

    BOOST_CHECK(addr_err.street1     == Registry::MojeID::INVALID);
    BOOST_CHECK(addr_err.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(addr_err.postal_code == Registry::MojeID::REQUIRED);
    BOOST_CHECK(addr_err.country     == Registry::MojeID::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_message_limit_exceeded)
{
    BOOST_CHECK_THROW(CorbaConversion::raise< Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED >(
        Registry::MojeIDImplData::MessageLimitExceeded()), Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::MessageLimitExceeded msg;
    msg.limit_expire_date = boost::gregorian::date(2015,12,10);
    msg.limit_count = 11;
    msg.limit_days = 12;

    Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED res = CorbaConversion::wrap_into<
                    Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED>(msg);

    BOOST_CHECK(CorbaConversion::unwrap_into< Registry::MojeIDImplData::Date >(
        res.limit_expire_date).value == boost::gregorian::to_iso_extended_string(msg.limit_expire_date));
    BOOST_CHECK(res.limit_count == msg.limit_count);
    BOOST_CHECK(res.limit_days == msg.limit_days);
}

BOOST_AUTO_TEST_CASE(test_mojeid_registration_validation_error)
{
    Registry::MojeIDImplData::RegistrationValidationResult ex;
    ex.first_name = Registry::MojeIDImplData::ValidationResult(10);
    BOOST_CHECK_THROW(CorbaConversion::raise<
        Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR >(ex),
        Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::AddressValidationResult permanent_addr_err_impl;
    permanent_addr_err_impl.street1     = Registry::MojeID::INVALID;
    permanent_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    permanent_addr_err_impl.postal_code = Registry::MojeID::REQUIRED;
    permanent_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::AddressValidationResult mailing_addr_err_impl;
    mailing_addr_err_impl.street1     = Registry::MojeID::NOT_AVAILABLE;
    mailing_addr_err_impl.city        = Registry::MojeID::REQUIRED;
    mailing_addr_err_impl.postal_code = Registry::MojeID::INVALID;
    mailing_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::AddressValidationResult billing_addr_err_impl;
    billing_addr_err_impl.street1     = Registry::MojeID::REQUIRED;
    billing_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    billing_addr_err_impl.postal_code = Registry::MojeID::INVALID;
    billing_addr_err_impl.country     = Registry::MojeID::REQUIRED;

    Registry::MojeIDImplData::ShippingAddressValidationResult shipping_addr_err_impl;
    shipping_addr_err_impl.street1     = Registry::MojeID::INVALID;
    shipping_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    shipping_addr_err_impl.postal_code = Registry::MojeID::REQUIRED;
    shipping_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::ShippingAddressValidationResult shipping2_addr_err_impl;
    shipping2_addr_err_impl.street1     = Registry::MojeID::REQUIRED;
    shipping2_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    shipping2_addr_err_impl.postal_code = Registry::MojeID::INVALID;
    shipping2_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::ShippingAddressValidationResult shipping3_addr_err_impl;
    shipping3_addr_err_impl.street1     = Registry::MojeID::INVALID;
    shipping3_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    shipping3_addr_err_impl.postal_code = Registry::MojeID::REQUIRED;
    shipping3_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::RegistrationValidationResult reg_val_err_impl;

    reg_val_err_impl.username     = Registry::MojeID::INVALID;
    reg_val_err_impl.first_name   = Registry::MojeID::NOT_AVAILABLE;
    reg_val_err_impl.last_name    = Registry::MojeID::REQUIRED;
    reg_val_err_impl.birth_date   = Registry::MojeID::INVALID;
    reg_val_err_impl.email        = Registry::MojeID::NOT_AVAILABLE;
    reg_val_err_impl.notify_email = Registry::MojeID::REQUIRED;
    reg_val_err_impl.phone        = Registry::MojeID::INVALID;
    reg_val_err_impl.fax          = Registry::MojeID::NOT_AVAILABLE;

    reg_val_err_impl.permanent = permanent_addr_err_impl;
    reg_val_err_impl.mailing = mailing_addr_err_impl;
    reg_val_err_impl.billing = billing_addr_err_impl;

    reg_val_err_impl.shipping = shipping_addr_err_impl;
    reg_val_err_impl.shipping2 = shipping2_addr_err_impl;
    reg_val_err_impl.shipping3 = shipping3_addr_err_impl;

    Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR res;
    CorbaConversion::wrap(reg_val_err_impl,res);

    BOOST_CHECK(res.username     == Registry::MojeID::INVALID);
    BOOST_CHECK(res.first_name   == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.last_name    == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.birth_date   == Registry::MojeID::INVALID);
    BOOST_CHECK(res.email        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.notify_email == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.phone        == Registry::MojeID::INVALID);
    BOOST_CHECK(res.fax          == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.permanent.street1     == Registry::MojeID::INVALID);
    BOOST_CHECK(res.permanent.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.permanent.postal_code == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.country     == Registry::MojeID::NOT_AVAILABLE);

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
    Registry::MojeIDImplData::UpdateContactPrepareValidationResult ex;
    ex.first_name = Registry::MojeIDImplData::ValidationResult(10);
    BOOST_CHECK_THROW(CorbaConversion::raise<
        Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR >(ex),
        Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::AddressValidationResult permanent_addr_err_impl;
    permanent_addr_err_impl.street1     = Registry::MojeID::INVALID;
    permanent_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    permanent_addr_err_impl.postal_code = Registry::MojeID::REQUIRED;
    permanent_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::AddressValidationResult mailing_addr_err_impl;
    mailing_addr_err_impl.street1     = Registry::MojeID::NOT_AVAILABLE;
    mailing_addr_err_impl.city        = Registry::MojeID::REQUIRED;
    mailing_addr_err_impl.postal_code = Registry::MojeID::INVALID;
    mailing_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::AddressValidationResult billing_addr_err_impl;
    billing_addr_err_impl.street1     = Registry::MojeID::REQUIRED;
    billing_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    billing_addr_err_impl.postal_code = Registry::MojeID::INVALID;
    billing_addr_err_impl.country     = Registry::MojeID::REQUIRED;

    Registry::MojeIDImplData::ShippingAddressValidationResult shipping_addr_err_impl;
    shipping_addr_err_impl.street1     = Registry::MojeID::INVALID;
    shipping_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    shipping_addr_err_impl.postal_code = Registry::MojeID::REQUIRED;
    shipping_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::ShippingAddressValidationResult shipping2_addr_err_impl;
    shipping2_addr_err_impl.street1     = Registry::MojeID::REQUIRED;
    shipping2_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    shipping2_addr_err_impl.postal_code = Registry::MojeID::INVALID;
    shipping2_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::ShippingAddressValidationResult shipping3_addr_err_impl;
    shipping3_addr_err_impl.street1     = Registry::MojeID::INVALID;
    shipping3_addr_err_impl.city        = Registry::MojeID::NOT_AVAILABLE;
    shipping3_addr_err_impl.postal_code = Registry::MojeID::REQUIRED;
    shipping3_addr_err_impl.country     = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::UpdateContactPrepareValidationResult upd_val_err_impl;

    upd_val_err_impl.first_name   = Registry::MojeID::NOT_AVAILABLE;
    upd_val_err_impl.last_name    = Registry::MojeID::REQUIRED;
    upd_val_err_impl.birth_date   = Registry::MojeID::INVALID;
    upd_val_err_impl.email        = Registry::MojeID::NOT_AVAILABLE;
    upd_val_err_impl.notify_email = Registry::MojeID::REQUIRED;
    upd_val_err_impl.phone        = Registry::MojeID::INVALID;
    upd_val_err_impl.fax          = Registry::MojeID::NOT_AVAILABLE;

    upd_val_err_impl.permanent = permanent_addr_err_impl;
    upd_val_err_impl.mailing = mailing_addr_err_impl;
    upd_val_err_impl.billing = billing_addr_err_impl;

    upd_val_err_impl.shipping = shipping_addr_err_impl;
    upd_val_err_impl.shipping2 = shipping2_addr_err_impl;
    upd_val_err_impl.shipping3 = shipping3_addr_err_impl;

    Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR res;
    CorbaConversion::wrap(upd_val_err_impl,res);

    BOOST_CHECK(res.first_name   == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.last_name    == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.birth_date   == Registry::MojeID::INVALID);
    BOOST_CHECK(res.email        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.notify_email == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.phone        == Registry::MojeID::INVALID);
    BOOST_CHECK(res.fax          == Registry::MojeID::NOT_AVAILABLE);

    BOOST_CHECK(res.permanent.street1     == Registry::MojeID::INVALID);
    BOOST_CHECK(res.permanent.city        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.permanent.postal_code == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.country     == Registry::MojeID::NOT_AVAILABLE);

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
    Registry::MojeIDImplData::CreateValidationRequestValidationResult ex;
    ex.first_name = Registry::MojeIDImplData::ValidationResult(10);
    BOOST_CHECK_THROW(CorbaConversion::raise<
        Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR >(ex),
        Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::MandatoryAddressValidationResult mandatory_addr_err_impl;
    mandatory_addr_err_impl.address_presence = Registry::MojeID::REQUIRED;
    mandatory_addr_err_impl.street1          = Registry::MojeID::INVALID;
    mandatory_addr_err_impl.city             = Registry::MojeID::NOT_AVAILABLE;
    mandatory_addr_err_impl.postal_code      = Registry::MojeID::REQUIRED;
    mandatory_addr_err_impl.country          = Registry::MojeID::NOT_AVAILABLE;

    Registry::MojeIDImplData::CreateValidationRequestValidationResult crr_val_err_impl;

    crr_val_err_impl.first_name   = Registry::MojeID::NOT_AVAILABLE;
    crr_val_err_impl.last_name    = Registry::MojeID::REQUIRED;
    crr_val_err_impl.email        = Registry::MojeID::NOT_AVAILABLE;
    crr_val_err_impl.notify_email = Registry::MojeID::REQUIRED;
    crr_val_err_impl.phone        = Registry::MojeID::INVALID;
    crr_val_err_impl.fax          = Registry::MojeID::NOT_AVAILABLE;
    crr_val_err_impl.ssn          = Registry::MojeID::INVALID;

    crr_val_err_impl.permanent = mandatory_addr_err_impl;

    Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR res;
    CorbaConversion::wrap(crr_val_err_impl,res);

    BOOST_CHECK(res.first_name   == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.last_name    == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.email        == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.notify_email == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.phone        == Registry::MojeID::INVALID);
    BOOST_CHECK(res.fax          == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.ssn          == Registry::MojeID::INVALID);

    BOOST_CHECK(res.permanent.address_presence == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.street1          == Registry::MojeID::INVALID);
    BOOST_CHECK(res.permanent.city             == Registry::MojeID::NOT_AVAILABLE);
    BOOST_CHECK(res.permanent.postal_code      == Registry::MojeID::REQUIRED);
    BOOST_CHECK(res.permanent.country          == Registry::MojeID::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_process_registration_request_validation_error)
{
    Registry::MojeIDImplData::ProcessRegistrationValidationResult ex;
    ex.email = Registry::MojeIDImplData::ValidationResult(10);
    BOOST_CHECK_THROW(CorbaConversion::raise<
        Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR >(ex),
        Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::ProcessRegistrationValidationResult prr_val_err_impl;

    prr_val_err_impl.email = Registry::MojeID::INVALID;
    prr_val_err_impl.phone = Registry::MojeID::REQUIRED;

    Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR res;
    CorbaConversion::wrap(prr_val_err_impl,res);

    BOOST_CHECK(res.email == Registry::MojeID::INVALID);
    BOOST_CHECK(res.phone == Registry::MojeID::REQUIRED);
}

BOOST_AUTO_TEST_CASE(test_mojeid_create_contact)
{
    Registry::MojeID::CreateContact_var cc = new Registry::MojeID::CreateContact;

    CorbaConversion::wrap_into_holder(std::string("username"), cc->username);
    CorbaConversion::wrap_into_holder(std::string("first_name"), cc->first_name);
    CorbaConversion::wrap_into_holder(std::string("last_name"), cc->last_name);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("org"), cc->organization);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("vat_reg_num"), cc->vat_reg_num);
    CorbaConversion::wrap_nullable_into_holder(Nullable< boost::gregorian::date >(boost::gregorian::date(2015,12,10)),
                                               cc->birth_date);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("id_card_num"), cc->id_card_num);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("passport_num"), cc->passport_num);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("ssn_id_num"), cc->ssn_id_num);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("vat_id_num"), cc->vat_id_num);
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "st1";
        addr_impl.street2 = "st2";
        addr_impl.street3 = "st3";
        addr_impl.city = "Praha";
        addr_impl.state = "state";
        addr_impl.country = "Czech Republic";
        CorbaConversion::wrap(addr_impl, cc->permanent);
    }
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "m_st1";
        addr_impl.street2 = "m_st2";
        addr_impl.street3 = "m_st3";
        addr_impl.city = "m_Praha";
        addr_impl.state = "m_state";
        addr_impl.country = "m_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, cc->mailing);
    }
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "b_st1";
        addr_impl.street2 = "b_st2";
        addr_impl.street3 = "b_st3";
        addr_impl.city = "b_Praha";
        addr_impl.state = "b_state";
        addr_impl.country = "b_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, cc->billing);
    }
    {
        Registry::MojeIDImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s_company";
        addr_impl.street1 = "s_st1";
        addr_impl.street2 = "s_st2";
        addr_impl.street3 = "s_st3";
        addr_impl.city = "s_Praha";
        addr_impl.state = "s_state";
        addr_impl.country = "s_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, cc->shipping);
    }
    {
        Registry::MojeIDImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s2_company";
        addr_impl.street1 = "s2_st1";
        addr_impl.street2 = "s2_st2";
        addr_impl.street3 = "s2_st3";
        addr_impl.city = "s2_Praha";
        addr_impl.state = "s2_state";
        addr_impl.country = "s2_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, cc->shipping2);
    }
    {
        Registry::MojeIDImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s3_company";
        addr_impl.street1 = "s3_st1";
        addr_impl.street2 = "s3_st2";
        addr_impl.street3 = "s3_st3";
        addr_impl.city = "s3_Praha";
        addr_impl.state = "s3_state";
        addr_impl.country = "s3_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, cc->shipping3);
    }
    CorbaConversion::wrap_into_holder(std::string("email"), cc->email);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("notify_email"), cc->notify_email);
    CorbaConversion::wrap_into_holder(std::string("telephone"), cc->telephone);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("fax"), cc->fax);

    Registry::MojeIDImplData::CreateContact cc_impl = CorbaConversion::unwrap_into<
        Registry::MojeIDImplData::CreateContact>(cc.in());

    BOOST_CHECK(cc_impl.username == "username");
    BOOST_CHECK(cc_impl.first_name == "first_name");
    BOOST_CHECK(cc_impl.last_name == "last_name");
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
    static const type out_of_range_value[number_of_out_of_range_values];
};

const bool IntConversionTraits< bool, bool, false, false, false >::
    value_to_fit[number_of_values_to_fit] = {
        false,
        true
    };


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
    static const type out_of_range_value[number_of_out_of_range_values];
};

template < class DST_INT_TYPE, bool DST_IS_SIGNED >
const bool IntConversionTraits< bool, DST_INT_TYPE, false, DST_IS_SIGNED, false >::
    value_to_fit[number_of_values_to_fit] = {
        false,
        true
    };


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
    static const type out_of_range_value[number_of_out_of_range_values];
};

template < class SRC_INT_TYPE, class DST_INT_TYPE, bool DST_IS_SIGNED >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, false, DST_IS_SIGNED, false >::
    value_to_fit[number_of_values_to_fit] = {
        0,
        1,
        shorter_unsigned::max() - 1,
        shorter_unsigned::max()
    };


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
        0,
        1,
        shorter_signed::max() - 1,
        shorter_signed::max()
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, false, false >::
    out_of_range_value[number_of_out_of_range_values] = {
        shorter_signed::min(),
        shorter_signed::min() + 1,
        -1
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
    static const type out_of_range_value[number_of_out_of_range_values];
};

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, true, false >::
    value_to_fit[number_of_values_to_fit] = {
        shorter_signed::min(),
        shorter_signed::min() + 1,
       -1,
        0,
        1,
        shorter_signed::max() - 1,
        shorter_signed::max()
    };


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
        0,
        1
    };

template < class SRC_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, bool, false, false, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        type(shorter_unsigned::max()) + 1,
        longer_unsigned::max() - 1,
        longer_unsigned::max()
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
        0,
        1
    };

template < class SRC_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, bool, true, false, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        longer_signed::min(),
        longer_signed::min() + 1,
       -type(shorter_unsigned::max()) - 1,
       -type(shorter_unsigned::max()),
        type(shorter_unsigned::max()) + 1,
        longer_signed::max() - 1,
        longer_signed::max()
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
        0,
        1,
        shorter::max() - 1,
        shorter::max()
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE, bool DST_IS_SIGNED >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, false, DST_IS_SIGNED, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        type(shorter::max()) + 1,
        longer_unsigned::max() - 1,
        longer_unsigned::max()
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
        0,
        1,
        shorter_unsigned::max() - 1,
        shorter_unsigned::max()
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, false, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        longer_signed::min(),
        longer_signed::min() + 1,
       -type(shorter_unsigned::max()) - 1,
       -type(shorter_unsigned::max()),
       -type(shorter_unsigned::max()) + 1,
       -1,
        type(shorter_unsigned::max()) + 1,
        longer_signed::max() - 1,
        longer_signed::max()
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
        shorter_signed::min(),
        shorter_signed::min() + 1,
       -1,
        0,
        1,
        shorter_signed::max() - 1,
        shorter_signed::max()
    };

template < class SRC_INT_TYPE, class DST_INT_TYPE >
const SRC_INT_TYPE IntConversionTraits< SRC_INT_TYPE, DST_INT_TYPE, true, true, true >::
    out_of_range_value[number_of_out_of_range_values] = {
        longer_signed::min(),
        longer_signed::min() + 1,
        type(shorter_signed::min()) - 1,
        type(shorter_signed::max()) + 1,
        longer_signed::max() - 1,
        longer_signed::max()
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
        CorbaConversion::wrap(src_value, dst_value);
        BOOST_CHECK(src_type(dst_value) == src_value);
        BOOST_CHECK(dst_value == dst_type(src_value));
        src_type src_value_back;
        CorbaConversion::unwrap(dst_value, src_value_back);
        BOOST_CHECK(src_value_back == src_value);
    }
    sum_cnt += traits::number_of_values_to_fit;
    sum_fit_cnt += traits::number_of_values_to_fit;

    for (::size_t idx = 0; idx < traits::number_of_out_of_range_values; ++idx) {
        const src_type src_value = traits::out_of_range_value[idx];
        dst_type dst_value;
        BOOST_CHECK_THROW((CorbaConversion::wrap(src_value, dst_value)),
                          CorbaConversion::IntegralConversionOutOfRange);
        BOOST_CHECK_THROW((CorbaConversion::unwrap(src_value, dst_value)),
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
    Registry::MojeID::UpdateContact_var uc = new Registry::MojeID::UpdateContact;

    CorbaConversion::wrap(5ull, uc->id);
    CorbaConversion::wrap_into_holder(std::string("first_name"), uc->first_name);
    CorbaConversion::wrap_into_holder(std::string("last_name"), uc->last_name);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("org"), uc->organization);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("vat_reg_num"), uc->vat_reg_num);
    CorbaConversion::wrap_nullable_into_holder(Nullable< boost::gregorian::date >(boost::gregorian::date(2015,12,10)),
                                               uc->birth_date);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("id_card_num"), uc->id_card_num);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("passport_num"), uc->passport_num);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("ssn_id_num"), uc->ssn_id_num);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("vat_id_num"), uc->vat_id_num);
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "st1";
        addr_impl.street2 = "st2";
        addr_impl.street3 = "st3";
        addr_impl.city = "Praha";
        addr_impl.state = "state";
        addr_impl.country = "Czech Republic";
        CorbaConversion::wrap(addr_impl, uc->permanent);
    }
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "m_st1";
        addr_impl.street2 = "m_st2";
        addr_impl.street3 = "m_st3";
        addr_impl.city = "m_Praha";
        addr_impl.state = "m_state";
        addr_impl.country = "m_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, uc->mailing);
    }
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "b_st1";
        addr_impl.street2 = "b_st2";
        addr_impl.street3 = "b_st3";
        addr_impl.city = "b_Praha";
        addr_impl.state = "b_state";
        addr_impl.country = "b_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, uc->billing);
    }
    {
        Registry::MojeIDImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s_company";
        addr_impl.street1 = "s_st1";
        addr_impl.street2 = "s_st2";
        addr_impl.street3 = "s_st3";
        addr_impl.city = "s_Praha";
        addr_impl.state = "s_state";
        addr_impl.country = "s_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, uc->shipping);
    }
    {
        Registry::MojeIDImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s2_company";
        addr_impl.street1 = "s2_st1";
        addr_impl.street2 = "s2_st2";
        addr_impl.street3 = "s2_st3";
        addr_impl.city = "s2_Praha";
        addr_impl.state = "s2_state";
        addr_impl.country = "s2_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, uc->shipping2);
    }
    {
        Registry::MojeIDImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s3_company";
        addr_impl.street1 = "s3_st1";
        addr_impl.street2 = "s3_st2";
        addr_impl.street3 = "s3_st3";
        addr_impl.city = "s3_Praha";
        addr_impl.state = "s3_state";
        addr_impl.country = "s3_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, uc->shipping3);
    }
    CorbaConversion::wrap_into_holder(std::string("email"), uc->email);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("notify_email"), uc->notify_email);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("telephone"), uc->telephone);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("fax"), uc->fax);

    Registry::MojeIDImplData::UpdateContact uc_impl = CorbaConversion::unwrap_into<
        Registry::MojeIDImplData::UpdateContact>(uc.in());

    BOOST_CHECK(uc_impl.id == 5);
    BOOST_CHECK(uc_impl.first_name == std::string("first_name"));
    BOOST_CHECK(uc_impl.last_name == std::string("last_name"));
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
    Registry::MojeID::SetContact_var sc = new Registry::MojeID::SetContact;

    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("org"), sc->organization);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("vat_reg_num"), sc->vat_reg_num);
    CorbaConversion::wrap_nullable_into_holder(Nullable< boost::gregorian::date >(boost::gregorian::date(2015,12,10)),
                                               sc->birth_date);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("vat_id_num"), sc->vat_id_num);
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "st1";
        addr_impl.street2 = "st2";
        addr_impl.street3 = "st3";
        addr_impl.city = "Praha";
        addr_impl.state = "state";
        addr_impl.country = "Czech Republic";
        CorbaConversion::wrap(addr_impl, sc->permanent);
    }
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "m_st1";
        addr_impl.street2 = "m_st2";
        addr_impl.street3 = "m_st3";
        addr_impl.city = "m_Praha";
        addr_impl.state = "m_state";
        addr_impl.country = "m_Czech Republic";

        CorbaConversion::wrap_into_holder(addr_impl, sc->mailing);
    }

    CorbaConversion::wrap_into_holder(std::string("email"), sc->email);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("notify_email"), sc->notify_email);
    CorbaConversion::wrap_into_holder(std::string("telephone"), sc->telephone);
    CorbaConversion::wrap_nullable_into_holder(Nullable< std::string >("fax"), sc->fax);

    Registry::MojeIDImplData::SetContact sc_impl = CorbaConversion::unwrap_into<
        Registry::MojeIDImplData::SetContact>(sc.in());

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

    Registry::MojeIDImplData::InfoContact info_contact_impl;

    info_contact_impl.id = 5;
    info_contact_impl.first_name = "first_name";
    info_contact_impl.last_name ="last_name";
    info_contact_impl.organization = "org";
    info_contact_impl.vat_reg_num = "vat_reg_num";
    {
        Registry::MojeIDImplData::Date birthdate;
        birthdate.value = boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,10));
        info_contact_impl.birth_date = birthdate;
    }
    info_contact_impl.id_card_num = "id_card_num";
    info_contact_impl.passport_num = "passport_num";
    info_contact_impl.ssn_id_num = "ssn_id_num";
    info_contact_impl.vat_id_num = "vat_id_num";

    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "st1";
        addr_impl.street2 = "st2";
        addr_impl.street3 = "st3";
        addr_impl.city = "Praha";
        addr_impl.state = "state";
        addr_impl.country = "Czech Republic";

        info_contact_impl.permanent = addr_impl;
    }

    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "m_st1";
        addr_impl.street2 = "m_st2";
        addr_impl.street3 = "m_st3";
        addr_impl.city = "m_Praha";
        addr_impl.state = "m_state";
        addr_impl.country = "m_Czech Republic";

        info_contact_impl.mailing = Nullable<Registry::MojeIDImplData::Address>(addr_impl);
    }
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "b_st1";
        addr_impl.street2 = "b_st2";
        addr_impl.street3 = "b_st3";
        addr_impl.city = "b_Praha";
        addr_impl.state = "b_state";
        addr_impl.country = "b_Czech Republic";

        info_contact_impl.billing = Nullable<Registry::MojeIDImplData::Address>(addr_impl);
    }
    {
        Registry::MojeIDImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s_company";
        addr_impl.street1 = "s_st1";
        addr_impl.street2 = "s_st2";
        addr_impl.street3 = "s_st3";
        addr_impl.city = "s_Praha";
        addr_impl.state = "s_state";
        addr_impl.country = "s_Czech Republic";

        info_contact_impl.shipping = Nullable<Registry::MojeIDImplData::ShippingAddress>(addr_impl);
    }
    {
        Registry::MojeIDImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s2_company";
        addr_impl.street1 = "s2_st1";
        addr_impl.street2 = "s2_st2";
        addr_impl.street3 = "s2_st3";
        addr_impl.city = "s2_Praha";
        addr_impl.state = "s2_state";
        addr_impl.country = "s2_Czech Republic";

        info_contact_impl.shipping2 = Nullable<Registry::MojeIDImplData::ShippingAddress>(addr_impl);
    }
    {
        Registry::MojeIDImplData::ShippingAddress addr_impl;
        addr_impl.company_name = "s3_company";
        addr_impl.street1 = "s3_st1";
        addr_impl.street2 = "s3_st2";
        addr_impl.street3 = "s3_st3";
        addr_impl.city = "s3_Praha";
        addr_impl.state = "s3_state";
        addr_impl.country = "s3_Czech Republic";

        info_contact_impl.shipping3 = Nullable<Registry::MojeIDImplData::ShippingAddress>(addr_impl);
    }

    info_contact_impl.email = "email";
    info_contact_impl.notify_email = "notify_email";
    info_contact_impl.telephone = "telephone";
    info_contact_impl.fax = "fax";


    Registry::MojeID::InfoContact_var info_contact;
    CorbaConversion::wrap_into_holder(info_contact_impl, info_contact);

    BOOST_REQUIRE(info_contact.operator->() != NULL);
    BOOST_CHECK(info_contact->id == 5);
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(
        info_contact->first_name.in()) == std::string("first_name"));
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(
        info_contact->last_name.in()) == std::string("last_name"));
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->organization.in()).get_value() == "org");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->vat_reg_num.in()).get_value() == "vat_reg_num");
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        info_contact->birth_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,10)));
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->id_card_num.in()).get_value() == "id_card_num");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->passport_num.in()).get_value() == "passport_num");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->ssn_id_num.in()).get_value() == "ssn_id_num");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->vat_id_num.in()).get_value() == "vat_id_num");


    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(
        info_contact->permanent.street1.in()) == "st1");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->permanent.street2.in()).get_value() == "st2");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->permanent.street3.in()).get_value() == "st3");
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(
        info_contact->permanent.city.in()) == "Praha");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->permanent.state.in()).get_value() == "state");
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(
        info_contact->permanent.country.in()) == "Czech Republic");

    Registry::MojeIDImplData::Address mailing = CorbaConversion::unwrap_into<
        Nullable<Registry::MojeIDImplData::Address> >(info_contact->mailing.in()).get_value();

    BOOST_CHECK(mailing.street1 == "m_st1");
    BOOST_CHECK(mailing.street2.get_value() == "m_st2");
    BOOST_CHECK(mailing.street3.get_value() == "m_st3");
    BOOST_CHECK(mailing.city == "m_Praha");
    BOOST_CHECK(mailing.state.get_value() == "m_state");
    BOOST_CHECK(mailing.country == "m_Czech Republic");

    Registry::MojeIDImplData::Address billing = CorbaConversion::unwrap_into<
            Nullable<Registry::MojeIDImplData::Address> >(info_contact->billing.in()).get_value();

    BOOST_CHECK(billing.street1 == "b_st1");
    BOOST_CHECK(billing.street2.get_value() == "b_st2");
    BOOST_CHECK(billing.street3.get_value() == "b_st3");
    BOOST_CHECK(billing.city == "b_Praha");
    BOOST_CHECK(billing.state.get_value() == "b_state");
    BOOST_CHECK(billing.country == "b_Czech Republic");

    Registry::MojeIDImplData::ShippingAddress shipping = CorbaConversion::unwrap_into<
            Nullable<Registry::MojeIDImplData::ShippingAddress> >(info_contact->shipping.in()).get_value();

    BOOST_CHECK(shipping.company_name.get_value() == "s_company");
    BOOST_CHECK(shipping.street1 == "s_st1");
    BOOST_CHECK(shipping.street2.get_value() == "s_st2");
    BOOST_CHECK(shipping.street3.get_value() == "s_st3");
    BOOST_CHECK(shipping.city == "s_Praha");
    BOOST_CHECK(shipping.state.get_value() == "s_state");
    BOOST_CHECK(shipping.country == "s_Czech Republic");

    Registry::MojeIDImplData::ShippingAddress shipping2 = CorbaConversion::unwrap_into<
            Nullable<Registry::MojeIDImplData::ShippingAddress> >(info_contact->shipping2.in()).get_value();

    BOOST_CHECK(shipping2.company_name.get_value() == "s2_company");
    BOOST_CHECK(shipping2.street1 == "s2_st1");
    BOOST_CHECK(shipping2.street2.get_value() == "s2_st2");
    BOOST_CHECK(shipping2.street3.get_value() == "s2_st3");
    BOOST_CHECK(shipping2.city == "s2_Praha");
    BOOST_CHECK(shipping2.state.get_value() == "s2_state");
    BOOST_CHECK(shipping2.country == "s2_Czech Republic");

    Registry::MojeIDImplData::ShippingAddress shipping3 = CorbaConversion::unwrap_into<
            Nullable<Registry::MojeIDImplData::ShippingAddress> >(info_contact->shipping3.in()).get_value();

    BOOST_CHECK(shipping3.company_name.get_value() == "s3_company");
    BOOST_CHECK(shipping3.street1 == "s3_st1");
    BOOST_CHECK(shipping3.street2.get_value() == "s3_st2");
    BOOST_CHECK(shipping3.street3.get_value() == "s3_st3");
    BOOST_CHECK(shipping3.city == "s3_Praha");
    BOOST_CHECK(shipping3.state.get_value() == "s3_state");
    BOOST_CHECK(shipping3.country == "s3_Czech Republic");

    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(
        info_contact->email.in()) == "email");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->notify_email.in()).get_value() == "notify_email");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->telephone.in()).get_value() == "telephone");
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        info_contact->fax.in()).get_value() == "fax");

}

BOOST_AUTO_TEST_CASE(test_mojeid_contact_state_info)
{

    Registry::MojeIDImplData::ContactStateInfo info_impl;

    info_impl.contact_id = 6;
    info_impl.mojeid_activation_datetime = boost::posix_time::ptime(boost::gregorian::date(2015,12,10));
    info_impl.conditionally_identification_date = boost::gregorian::date(2015,12,11);
    info_impl.identification_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,12));
    info_impl.validation_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,13));
    info_impl.linked_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,14));

    Registry::MojeID::ContactStateInfo_var info;
    CorbaConversion::wrap_into_holder(info_impl, info);

    BOOST_CHECK(info->contact_id == 6);
    BOOST_CHECK(CorbaConversion::unwrap_into<boost::posix_time::ptime>(
        info->mojeid_activation_datetime) == boost::posix_time::ptime(boost::gregorian::date(2015,12,10)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Registry::MojeIDImplData::Date >(
        info->conditionally_identification_date).value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,11)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        info->identification_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,12)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        info->validation_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,13)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        info->linked_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,14)));
}


BOOST_AUTO_TEST_CASE(test_string_octet_seq_tmpl)
{
    Registry::MojeID::BufferValue_var out_seq_var = new Registry::MojeID::BufferValue;
    CorbaConversion::Wrapper_container_into_OctetSeq< std::string, Registry::MojeID::BufferValue >
        ::wrap(std::string("test"), out_seq_var.inout());
    BOOST_REQUIRE(out_seq_var.operator ->() != NULL);
    BOOST_CHECK(std::string(reinterpret_cast<const char *>(out_seq_var->get_buffer()),
        out_seq_var->length()) == "test");

    std::string out_str;
    CorbaConversion::Unwrapper_OctetSeq_into_container< Registry::MojeID::BufferValue, std::string >
        ::unwrap(out_seq_var.in(), out_str);
    BOOST_CHECK(out_str == "test");
}


BOOST_AUTO_TEST_CASE(test_empty_octet_seq_tmpl)
{
    Registry::MojeID::BufferValue_var out_seq_var = new Registry::MojeID::BufferValue;
    CorbaConversion::Wrapper_container_into_OctetSeq< std::string, Registry::MojeID::BufferValue >
        ::wrap(std::string(), out_seq_var.inout());
    BOOST_REQUIRE(out_seq_var.operator ->() != NULL);
    BOOST_CHECK(out_seq_var->length() == 0);

    std::string out_str;
    CorbaConversion::Unwrapper_OctetSeq_into_container< Registry::MojeID::BufferValue, std::string >
        ::unwrap(out_seq_var.in(), out_str);
    BOOST_CHECK(out_str.empty());

    std::vector<unsigned char> out_data;
    CorbaConversion::Unwrapper_OctetSeq_into_container< Registry::MojeID::BufferValue, std::vector< unsigned char > >
        ::unwrap(out_seq_var.in(), out_data);
    BOOST_CHECK(out_data.empty());
}

BOOST_AUTO_TEST_CASE(test_vector_octet_seq_tmpl)
{
    const char* data = "test";
    Registry::MojeID::BufferValue_var out_seq_var = new Registry::MojeID::BufferValue;
    CorbaConversion::Wrapper_container_into_OctetSeq< std::vector< unsigned char >, Registry::MojeID::BufferValue >
        ::wrap(std::vector< unsigned char >(data, data + 4), out_seq_var.inout());
    BOOST_REQUIRE(out_seq_var.operator ->() != NULL);
    BOOST_CHECK(std::string(reinterpret_cast<const char *>(out_seq_var->get_buffer()),
        out_seq_var->length()) == "test");

    BOOST_CHECK(std::vector<unsigned char>(
        reinterpret_cast<const char *>(out_seq_var->get_buffer()),
        reinterpret_cast<const char *>(out_seq_var->get_buffer()) + out_seq_var->length()).size()
        == out_seq_var->length());

    std::vector<unsigned char> out_data;
    CorbaConversion::Unwrapper_OctetSeq_into_container< Registry::MojeID::BufferValue,
        std::vector< unsigned char > >::unwrap(out_seq_var.in(), out_data);
    BOOST_CHECK(std::string(out_data.begin(), out_data.end()) == "test");
}

BOOST_AUTO_TEST_CASE(test_mojeid_contact_state_info_list)
{
    std::vector<Registry::MojeIDImplData::ContactStateInfo> data;
    Registry::MojeID::ContactStateInfoList_var res;

    CorbaConversion::wrap_into_holder(data, res);
    BOOST_REQUIRE(res.operator ->() != NULL);
    BOOST_CHECK(res->length() == 0);

    {
        Registry::MojeIDImplData::ContactStateInfo info_impl;
        info_impl.contact_id = 5;
        info_impl.mojeid_activation_datetime = boost::posix_time::ptime(boost::gregorian::date(2015,12,10));
        info_impl.conditionally_identification_date = boost::gregorian::date(2015,12,11);
        info_impl.identification_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,12));
        info_impl.validation_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,13));
        info_impl.linked_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,14));
        data.push_back(info_impl);
    }
    {
        Registry::MojeIDImplData::ContactStateInfo info_impl;
        info_impl.contact_id = 6;
        info_impl.mojeid_activation_datetime = boost::posix_time::ptime(boost::gregorian::date(2016,12,10));
        info_impl.conditionally_identification_date = boost::gregorian::date(2016,12,11);
        info_impl.identification_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2016,12,12));
        info_impl.validation_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2016,12,13));
        info_impl.linked_date = Nullable<boost::gregorian::date>(boost::gregorian::date(2016,12,14));
        data.push_back(info_impl);
    }

    CorbaConversion::wrap_into_holder(data, res);
    BOOST_REQUIRE(res.operator ->() != NULL);
    BOOST_REQUIRE(res->length() == 2);

    BOOST_CHECK(res[0].contact_id == 5);
    BOOST_CHECK(CorbaConversion::unwrap_into<boost::posix_time::ptime>(
        res[0].mojeid_activation_datetime) == boost::posix_time::ptime(boost::gregorian::date(2015,12,10)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Registry::MojeIDImplData::Date >(
        res[0].conditionally_identification_date).value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,11)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        res[0].identification_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,12)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        res[0].validation_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,13)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        res[0].linked_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2015,12,14)));

    BOOST_CHECK(res[1].contact_id == 6);
    BOOST_CHECK(CorbaConversion::unwrap_into<boost::posix_time::ptime>(
        res[1].mojeid_activation_datetime) == boost::posix_time::ptime(boost::gregorian::date(2016,12,10)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Registry::MojeIDImplData::Date >(
        res[1].conditionally_identification_date).value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2016,12,11)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        res[1].identification_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2016,12,12)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        res[1].validation_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2016,12,13)));
    BOOST_CHECK(CorbaConversion::unwrap_into< Nullable< Registry::MojeIDImplData::Date > >(
        res[1].linked_date.in()).get_value().value == boost::gregorian::to_iso_extended_string(boost::gregorian::date(2016,12,14)));
}

BOOST_AUTO_TEST_CASE(test_mojeid_buffer)
{
    Registry::MojeID::BufferValue_var out_seq_var;
    CorbaConversion::wrap_into_holder(std::string("test"), out_seq_var);
    BOOST_REQUIRE(out_seq_var.operator ->() != NULL);
    BOOST_CHECK(std::string(reinterpret_cast<const char *>(out_seq_var->get_buffer()),
        out_seq_var->length()) == "test");

}

BOOST_AUTO_TEST_CASE(test_mojeid_constact_handle_list)
{
    Registry::MojeID::ContactHandleList_var ssv1;
    CorbaConversion::wrap_into_holder(std::vector< std::string >(Util::vector_of< std::string >("test1")("test2")("test3")),
                                      ssv1);
    BOOST_REQUIRE(ssv1.operator ->() != NULL);
    BOOST_REQUIRE(ssv1->length() == 3);
    BOOST_CHECK(std::string(ssv1[0]) == "test1");
    BOOST_CHECK(std::string(ssv1[1]) == "test2");
    BOOST_CHECK(std::string(ssv1[2]) == "test3");

}

BOOST_AUTO_TEST_SUITE_END();
