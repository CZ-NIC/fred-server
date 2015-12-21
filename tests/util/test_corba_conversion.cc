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
#include <boost/numeric/conversion/converter.hpp>
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

    template <> struct DEFAULT_UNWRAPPER<Test::NullableString*, Nullable<std::string> >
    {
        typedef Unwrapper_NullableString_ptr_into_Nullable_std_string<Test::NullableString*> type;
    };

    template <> struct DEFAULT_WRAPPER<Nullable<std::string>, Test::NullableString_var >
    {
        typedef Wrapper_Nullable_std_string_into_NullableString_var<Test::NullableString, Test::NullableString_var> type;
    };

    //tmpl seq
    /**
     * generic implementation of setting CORBA sequence, previous content of @param cs will be discarded
     * from container with const_iterator, begin(), end() and size() member
     */
    template <class ELEMENT_CONVERSION, class NON_CORBA_CONTAINER, typename CORBA_SEQ>
    struct Wrapper_std_vector_into_Seq
    {
        typedef CORBA_SEQ CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
        {
            ct_out.length(nct_in.size());
            unsigned long long i = 0;
            for(typename NON_CORBA_CONTAINER::const_iterator ci = nct_in.begin() ; ci != nct_in.end(); ++ci,++i)
            {
                ct_out[i] = wrap_by<ELEMENT_CONVERSION>(*ci);
            }
        }
    };

    /**
     * generic implementation of setting non-CORBA container with clear(), reserve() and push_back() member
     * from CORBA sequence, previous content of @param ol will be discarded
     */
    template <class ELEMENT_CONVERSION, typename CORBA_SEQ, class NON_CORBA_CONTAINER>
    struct Unwrapper_Seq_into_std_vector
    {
        typedef CORBA_SEQ CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out )
        {
            nct_out.clear();
            nct_out.reserve(ct_in.length());
            for(unsigned long long i = 0 ; i < ct_in.length();++i)
            {
                nct_out.push_back(unwrap_by<ELEMENT_CONVERSION>(ct_in[i]));
            }

        }
    };

    //tmpl seq var
    template <class ELEMENT_CONVERSION, typename CORBA_SEQ, class NON_CORBA_CONTAINER, typename CORBA_SEQ_VAR>
    struct Wrapper_std_vector_into_Seq_var
    {
        typedef CORBA_SEQ_VAR CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
        {
            ct_out = new CORBA_SEQ;
            Wrapper_std_vector_into_Seq<ELEMENT_CONVERSION, NON_CORBA_CONTAINER, CORBA_SEQ>::wrap(nct_in, ct_out.inout());
        }
    };
    template <> struct DEFAULT_WRAPPER<
        typename Wrapper_std_vector_into_Seq_var <Wrapper_std_string_into_String_var,
            CORBA::StringSeq, std::vector<std::string>, CORBA::StringSeq_var>::NON_CORBA_TYPE,
        typename Wrapper_std_vector_into_Seq_var <Wrapper_std_string_into_String_var,
            CORBA::StringSeq, std::vector<std::string>, CORBA::StringSeq_var>::CORBA_TYPE>
    {
        typedef Wrapper_std_vector_into_Seq_var <Wrapper_std_string_into_String_var,
            CORBA::StringSeq, std::vector<std::string>, CORBA::StringSeq_var> type;
    };

    template <> struct DEFAULT_WRAPPER<
        typename Wrapper_std_vector_into_Seq_var <Wrapper_std_string_into_String_var,
            Test::StringSeq, std::vector<std::string>, Test::StringSeq_var>::NON_CORBA_TYPE,
        typename Wrapper_std_vector_into_Seq_var <Wrapper_std_string_into_String_var,
            Test::StringSeq, std::vector<std::string>, Test::StringSeq_var>::CORBA_TYPE>
    {
        typedef Wrapper_std_vector_into_Seq_var <Wrapper_std_string_into_String_var,
            Test::StringSeq, std::vector<std::string>, Test::StringSeq_var> type;
    };


    template <class ELEMENT_CONVERSION, typename CORBA_SEQ, typename CORBA_SEQ_VAR, class NON_CORBA_CONTAINER>
    struct Unwrapper_Seq_var_into_std_vector
    {
        typedef CORBA_SEQ_VAR CORBA_TYPE;
        typedef NON_CORBA_CONTAINER NON_CORBA_TYPE;

        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out )
        {
            Unwrapper_Seq_into_std_vector<ELEMENT_CONVERSION, CORBA_SEQ, NON_CORBA_CONTAINER>::unwrap(ct_in.in(), nct_out);
        }
    };
    template <> struct DEFAULT_UNWRAPPER<
        typename Unwrapper_Seq_var_into_std_vector <Unwrapper_const_char_ptr_into_std_string,
            CORBA::StringSeq, CORBA::StringSeq_var, std::vector<std::string> >::CORBA_TYPE,
        typename Unwrapper_Seq_var_into_std_vector <Unwrapper_const_char_ptr_into_std_string,
            CORBA::StringSeq, CORBA::StringSeq_var, std::vector<std::string> >::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Seq_var_into_std_vector <Unwrapper_const_char_ptr_into_std_string,
            CORBA::StringSeq, CORBA::StringSeq_var, std::vector<std::string> > type;
    };

    template <> struct DEFAULT_UNWRAPPER<
        typename Unwrapper_Seq_var_into_std_vector <Unwrapper_const_char_ptr_into_std_string,
            Test::StringSeq, Test::StringSeq_var, std::vector<std::string> >::CORBA_TYPE,
        typename Unwrapper_Seq_var_into_std_vector <Unwrapper_const_char_ptr_into_std_string,
            Test::StringSeq, Test::StringSeq_var, std::vector<std::string> >::NON_CORBA_TYPE>
    {
        typedef Unwrapper_Seq_var_into_std_vector <Unwrapper_const_char_ptr_into_std_string,
        Test::StringSeq, Test::StringSeq_var, std::vector<std::string> > type;
    };

    struct Wrapper_short_into_Short
    {
        typedef CORBA::Short CORBA_TYPE;
        typedef short        NON_CORBA_TYPE;

        static void wrap(NON_CORBA_TYPE in_nc, CORBA_TYPE &out_c)
        {
            out_c = in_nc;
        }
    };
    template <> struct DEFAULT_WRAPPER<short, CORBA::Short >
    {
        typedef Wrapper_short_into_Short type;
    };

    struct Unwrapper_Short_into_short
    {
        typedef CORBA::Short CORBA_TYPE;
        typedef short        NON_CORBA_TYPE;

        static void unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out )
        {
            nct_out = ct_in;
        }
    };
    template <> struct DEFAULT_UNWRAPPER<CORBA::Short, short>
    {
        typedef Unwrapper_Short_into_short type;
    };


}

BOOST_AUTO_TEST_SUITE(TestCorbaConversion)

const std::string server_name = "test-corba-conversion";


BOOST_AUTO_TEST_CASE(test_string_wrap_by_unwrap_by)
{
    CORBA::String_var sv1 = CorbaConversion::wrap_by<CorbaConversion::Wrapper_std_string_into_String_var>("test");
    BOOST_CHECK(std::string(sv1.in()) == std::string("test"));
    std::string ss1 = CorbaConversion::unwrap_by<CorbaConversion::Unwrapper_const_char_ptr_into_std_string>(sv1);
    BOOST_CHECK(ss1 == std::string("test"));
}

BOOST_AUTO_TEST_CASE(test_string_wrap_unwrap)
{
    CORBA::String_var sv1;
    std::string ss1("test");
    CorbaConversion::wrap(ss1, sv1);
    BOOST_CHECK(std::string(sv1.in()) == ss1);
    std::string ss2;
    CorbaConversion::unwrap(sv1.in(), ss2);
    BOOST_CHECK(std::string(sv1.in()) == ss2);
}

BOOST_AUTO_TEST_CASE(test_string_wrap_into_unwrap_into)
{
    CORBA::String_var sv1 = CorbaConversion::wrap_into<CORBA::String_var>(std::string("test"));
    BOOST_CHECK(std::string(sv1.in()) == std::string("test"));
    std::string ss1 = CorbaConversion::unwrap_into<std::string>(sv1.in());
    BOOST_CHECK(ss1 == std::string("test"));
}

BOOST_AUTO_TEST_CASE(test_impl_string_wrap_unwrap)
{
    CORBA::String_var sv1;
    std::string ss1("test");
    CorbaConversion::Wrapper_std_string_into_String_var::wrap(ss1, sv1);
    BOOST_CHECK(std::string(sv1.in()) == ss1);
    std::string ss2;
    CorbaConversion::Unwrapper_const_char_ptr_into_std_string::unwrap(sv1, ss2);
    BOOST_CHECK(std::string(sv1.in()) == ss2);
}

BOOST_AUTO_TEST_CASE(test_string_seq_ref)
{
    CORBA::StringSeq_var ssv1 = new CORBA::StringSeq;
    std::vector<std::string> vs1 = Util::vector_of<std::string>("test1")("test2")("test3");

    CorbaConversion::Wrapper_std_vector_into_Seq<CorbaConversion::Wrapper_std_string_into_String_var,
        std::vector<std::string>,CORBA::StringSeq>::wrap(vs1, ssv1.inout());

    BOOST_CHECK(ssv1->length() == vs1.size());

    CORBA::String_var sv0;
    sv0 = ssv1[0];
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(sv0.in()) == vs1[0]);

    CORBA::String_var sv1 = ssv1[1];
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(sv1.in()) == vs1[1]);

    bool string_seq_element_check = (CorbaConversion::unwrap_into<std::string, const char *>(ssv1[2]) == vs1[2]);
    BOOST_CHECK(string_seq_element_check);

    std::vector<std::string> vs2;

    CorbaConversion::Unwrapper_Seq_into_std_vector<CorbaConversion::Unwrapper_const_char_ptr_into_std_string,
        CORBA::StringSeq, std::vector<std::string> >::unwrap(ssv1.in(), vs2);

    BOOST_CHECK(vs2.size() == vs1.size());
    BOOST_CHECK(vs2[0] == vs1[0]);
    BOOST_CHECK(vs2[1] == vs1[1]);
    BOOST_CHECK(vs2[2] == vs1[2]);
}

BOOST_AUTO_TEST_CASE(test_string_seq)
{
    CORBA::StringSeq_var ssv1;
    std::vector<std::string> vs1 = Util::vector_of<std::string>("test1")("test2")("test3");

    CorbaConversion::Wrapper_std_vector_into_Seq_var <CorbaConversion::Wrapper_std_string_into_String_var,
        CORBA::StringSeq, std::vector<std::string>, CORBA::StringSeq_var>::wrap(vs1, ssv1);

    BOOST_CHECK(ssv1->length() == vs1.size());

    CORBA::String_var sv0;
    sv0 = ssv1[0];
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(sv0.in()) == vs1[0]);

    CORBA::String_var sv1 = ssv1[1];
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(sv1.in()) == vs1[1]);

    bool string_seq_element_check = (CorbaConversion::unwrap_into<std::string, const char *>(ssv1[2]) == vs1[2]);
    BOOST_CHECK(string_seq_element_check);

    std::vector<std::string> vs2;

    CorbaConversion::Unwrapper_Seq_var_into_std_vector <CorbaConversion::Unwrapper_const_char_ptr_into_std_string,
        CORBA::StringSeq, CORBA::StringSeq_var, std::vector<std::string> >::unwrap(ssv1, vs2);

    BOOST_CHECK(vs2.size() == vs1.size());
    BOOST_CHECK(vs2[0] == vs1[0]);
    BOOST_CHECK(vs2[1] == vs1[1]);
    BOOST_CHECK(vs2[2] == vs1[2]);
}

BOOST_AUTO_TEST_CASE(test_string_seq_wrap_into_corba)
{
    CORBA::StringSeq_var ssv1 = CorbaConversion::wrap_into<CORBA::StringSeq_var>(
        std::vector<std::string>(Util::vector_of<std::string>("test1")("test2")("test3")));
    std::vector<std::string> vs1 = CorbaConversion::unwrap_into<std::vector<std::string> > (ssv1);

    BOOST_CHECK(vs1.size() == 3);
    BOOST_CHECK(vs1[0] == "test1");
    BOOST_CHECK(vs1[1] == "test2");
    BOOST_CHECK(vs1[2] == "test3");
}

BOOST_AUTO_TEST_CASE(test_string_seq_wrap_into_test)
{
    Test::StringSeq_var ssv1 = CorbaConversion::wrap_into<Test::StringSeq_var>(
        std::vector<std::string>(Util::vector_of<std::string>("test1")("test2")("test3")));
    std::vector<std::string> vs1 = CorbaConversion::unwrap_into<std::vector<std::string> > (ssv1);

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
    Nullable<std::string> ns1 ("test1");
    Test::NullableString_var tnsv1 = CorbaConversion::wrap_into<Test::NullableString_var>(ns1);
    BOOST_CHECK(std::string(tnsv1->_value()) == ns1.get_value());

    Nullable<std::string> ns2 = CorbaConversion::unwrap_into<Nullable<std::string> >(tnsv1.in());
    BOOST_CHECK(std::string(tnsv1->_value()) == ns2.get_value());
}

BOOST_AUTO_TEST_CASE(test_mojeid_valuetype_string)
{
    Nullable<std::string> ns1 ("test1");
    Registry::MojeID::NullableString_var tnsv1 = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(ns1);
    BOOST_CHECK(std::string(tnsv1->_value()) == ns1.get_value());

    Nullable<std::string> ns2 = CorbaConversion::unwrap_into<Nullable<std::string> >(tnsv1.in());
    BOOST_CHECK(std::string(tnsv1->_value()) == ns2.get_value());
}

BOOST_AUTO_TEST_CASE(test_mojeid_date)
{
    Registry::MojeID::Date_var mojeid_date = CorbaConversion::wrap_into<Registry::MojeID::Date_var>(boost::gregorian::date(2015,12,10));
    mojeid_date->value;
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(mojeid_date->value.in()) == "2015-12-10");

    BOOST_CHECK(CorbaConversion::unwrap_into<boost::gregorian::date>(mojeid_date.in()) == boost::gregorian::date(2015,12,10));

    BOOST_CHECK_THROW(CorbaConversion::wrap_into<Registry::MojeID::Date_var>(boost::gregorian::date()), CorbaConversion::ArgumentIsSpecial);
    BOOST_CHECK_THROW(CorbaConversion::unwrap_into<boost::gregorian::date>(
        CorbaConversion::wrap_into<Registry::MojeID::Date_var>(boost::gregorian::date()).in()), CorbaConversion::ArgumentIsSpecial);
}

BOOST_AUTO_TEST_CASE(test_mojeid_datetime)
{
    Registry::MojeID::DateTime_var mojeid_datetime = CorbaConversion::wrap_into<Registry::MojeID::DateTime_var>(boost::posix_time::ptime(boost::gregorian::date(2015,12,10)));
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(mojeid_datetime->value.in()) == "2015-12-10T00:00:00");
    BOOST_CHECK(CorbaConversion::unwrap_into<boost::posix_time::ptime>(mojeid_datetime.in()) == boost::posix_time::ptime(boost::gregorian::date(2015,12,10)));

    BOOST_CHECK_THROW(CorbaConversion::wrap_into<Registry::MojeID::DateTime_var>(boost::posix_time::ptime()), CorbaConversion::ArgumentIsSpecial);
    BOOST_CHECK_THROW(CorbaConversion::unwrap_into<boost::posix_time::ptime>(
        CorbaConversion::wrap_into<Registry::MojeID::DateTime_var>(boost::posix_time::ptime()).in()), CorbaConversion::ArgumentIsSpecial);
}

BOOST_AUTO_TEST_CASE(test_mojeid_nullabledate)
{
    Registry::MojeID::NullableDate_var nd1 = CorbaConversion::wrap_into<Registry::MojeID::NullableDate_var>(Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,10)));
    BOOST_CHECK(std::string(nd1->value()) == "2015-12-10");
    Registry::MojeID::NullableDate_var nd2 = CorbaConversion::wrap_into<Registry::MojeID::NullableDate_var>(Nullable<boost::gregorian::date>());
    BOOST_CHECK(nd2.in() == NULL);

    Nullable<boost::gregorian::date> res1 = CorbaConversion::unwrap_into<Nullable<boost::gregorian::date> >(nd1.operator->());
    BOOST_CHECK(!res1.isnull());
    BOOST_CHECK(res1.get_value() == boost::gregorian::date(2015,12,10));
    Nullable<boost::gregorian::date> res2 = CorbaConversion::unwrap_into<Nullable<boost::gregorian::date> >(nd2.operator->());
    BOOST_CHECK(res2.isnull());
}

BOOST_AUTO_TEST_CASE(test_mojeid_nullableboolean)
{
    Registry::MojeID::NullableBoolean_var nb1 = CorbaConversion::wrap_into<Registry::MojeID::NullableBoolean_var>(Nullable<bool>(true));
    BOOST_REQUIRE(nb1.in() != NULL);
    BOOST_CHECK(nb1->_value() == true);
    Registry::MojeID::NullableBoolean_var nb2 = CorbaConversion::wrap_into<Registry::MojeID::NullableBoolean_var>(Nullable<bool>());
    BOOST_CHECK(nb2.in() == NULL);

    Nullable<bool> res1 = CorbaConversion::unwrap_into<Nullable<bool> >(nb1.in());
    BOOST_CHECK(!res1.isnull());
    BOOST_CHECK(res1.get_value() == true);
    Nullable<bool> res2 = CorbaConversion::unwrap_into<Nullable<bool> >(nb2.in());
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

    Registry::MojeID::Address_var addr = CorbaConversion::wrap_into<Registry::MojeID::Address_var>(addr_impl);
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
    BOOST_CHECK(CorbaConversion::wrap_into<Registry::MojeID::NullableAddress_var>(Nullable<Registry::MojeIDImplData::Address>()).in() == NULL);
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

    Registry::MojeID::NullableAddress_var addr = CorbaConversion::wrap_into<Registry::MojeID::NullableAddress_var>(nullable_addr);
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

    Registry::MojeID::ShippingAddress_var addr = CorbaConversion::wrap_into<Registry::MojeID::ShippingAddress_var>(addr_impl);
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
    BOOST_CHECK(CorbaConversion::wrap_into<Registry::MojeID::NullableShippingAddress_var>(Nullable<Registry::MojeIDImplData::ShippingAddress>()).in() == NULL);
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

    Registry::MojeID::NullableShippingAddress_var addr = CorbaConversion::wrap_into<Registry::MojeID::NullableShippingAddress_var>(nullable_addr);
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

BOOST_AUTO_TEST_CASE(test_mojeid_validationerror)
{
    BOOST_CHECK_THROW(CorbaConversion::unwrap_into<Registry::MojeIDImplData::ValidationError::EnumType>(
        static_cast<Registry::MojeID::ValidationError>(10)), CorbaConversion::NotEnumValidationErrorValue);
    BOOST_CHECK_THROW(CorbaConversion::wrap_into<Registry::MojeID::ValidationError>(
        static_cast<Registry::MojeIDImplData::ValidationError::EnumType>(10)), CorbaConversion::NotEnumValidationErrorValue);

    BOOST_CHECK(Registry::MojeIDImplData::ValidationError::INVALID
        == CorbaConversion::unwrap_into<Registry::MojeIDImplData::ValidationError::EnumType>(Registry::MojeID::INVALID));
    BOOST_CHECK(Registry::MojeID::INVALID
        == CorbaConversion::wrap_into<Registry::MojeID::ValidationError>(Registry::MojeIDImplData::ValidationError::INVALID));
}

BOOST_AUTO_TEST_CASE(test_mojeid_nullablevalidationerror)
{
    Registry::MojeID::NullableValidationError_var err_value =
    CorbaConversion::wrap_into<Registry::MojeID::NullableValidationError_var>(
        Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
            Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE));

    BOOST_CHECK(Registry::MojeID::NOT_AVAILABLE == err_value->_value());

    BOOST_CHECK(Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE)
        == CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(err_value.in()));

    BOOST_CHECK(CorbaConversion::wrap_into<Registry::MojeID::NullableValidationError_var>(
        Nullable<Registry::MojeIDImplData::ValidationError::EnumType>()).operator ->() == NULL);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        static_cast<Registry::MojeID::NullableValidationError*>(NULL)).isnull());
}

BOOST_AUTO_TEST_CASE(test_mojeid_addressvalidationerror)
{
    Registry::MojeIDImplData::AddressValidationError addr_err_impl;
    addr_err_impl.street1 = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::INVALID);
    addr_err_impl.city = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    addr_err_impl.postal_code = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::REQUIRED);
    addr_err_impl.country = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Registry::MojeID::AddressValidationError_var addr_err = CorbaConversion::wrap_into<
        Registry::MojeID::AddressValidationError_var>(addr_err_impl);

    BOOST_REQUIRE(addr_err.operator ->() != NULL);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->street1.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->city.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->postal_code.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->country.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Registry::MojeIDImplData::AddressValidationError addr_err_impl_res = CorbaConversion::unwrap_into<
        Registry::MojeIDImplData::AddressValidationError>(addr_err.in());

    BOOST_CHECK(addr_err_impl_res.street1.get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(addr_err_impl_res.city.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(addr_err_impl_res.postal_code.get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(addr_err_impl_res.country.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_nullableaddressvalidationerror)
{
    Registry::MojeIDImplData::AddressValidationError addr_err_impl;
    addr_err_impl.street1 = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::INVALID);
    addr_err_impl.city = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    addr_err_impl.postal_code = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::REQUIRED);
    addr_err_impl.country = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Registry::MojeID::NullableAddressValidationError_var addr_err = CorbaConversion::wrap_into<
        Registry::MojeID::NullableAddressValidationError_var>(
            Nullable<Registry::MojeIDImplData::AddressValidationError>(addr_err_impl));

    BOOST_REQUIRE(addr_err.operator ->() != NULL);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->street1()).get_value() ==  Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->city()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->postal_code()).get_value() ==  Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->country()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Nullable<Registry::MojeIDImplData::AddressValidationError> addr_err_impl_res = CorbaConversion::unwrap_into<
                    Nullable<Registry::MojeIDImplData::AddressValidationError> >(addr_err.in());

    BOOST_CHECK(addr_err_impl_res.get_value().street1.get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(addr_err_impl_res.get_value().city.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(addr_err_impl_res.get_value().postal_code.get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(addr_err_impl_res.get_value().country.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_mandatoryaddressvalidationerror)
{
    Registry::MojeIDImplData::MandatoryAddressValidationError addr_err_impl;
    addr_err_impl.address_presence = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
            Registry::MojeIDImplData::ValidationError::REQUIRED);
    addr_err_impl.street1 = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::INVALID);
    addr_err_impl.city = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    addr_err_impl.postal_code = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::REQUIRED);
    addr_err_impl.country = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Registry::MojeID::MandatoryAddressValidationError_var addr_err = CorbaConversion::wrap_into<
        Registry::MojeID::MandatoryAddressValidationError_var>(addr_err_impl);

    BOOST_REQUIRE(addr_err.operator ->() != NULL);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
            addr_err->address_presence.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->street1.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->city.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->postal_code.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->country.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Registry::MojeIDImplData::MandatoryAddressValidationError addr_err_impl_res = CorbaConversion::unwrap_into<
        Registry::MojeIDImplData::MandatoryAddressValidationError>(addr_err.in());

    BOOST_CHECK(addr_err_impl_res.address_presence.get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(addr_err_impl_res.street1.get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(addr_err_impl_res.city.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(addr_err_impl_res.postal_code.get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(addr_err_impl_res.country.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_nullablemandatoryaddressvalidationerror)
{
    Registry::MojeIDImplData::MandatoryAddressValidationError addr_err_impl;
    addr_err_impl.address_presence = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::REQUIRED);
    addr_err_impl.street1 = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::INVALID);
    addr_err_impl.city = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    addr_err_impl.postal_code = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::REQUIRED);
    addr_err_impl.country = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Registry::MojeID::NullableMandatoryAddressValidationError_var addr_err = CorbaConversion::wrap_into<
        Registry::MojeID::NullableMandatoryAddressValidationError_var>(
            Nullable<Registry::MojeIDImplData::MandatoryAddressValidationError>(addr_err_impl));

    BOOST_REQUIRE(addr_err.operator ->() != NULL);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->address_presence()).get_value() ==  Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->street1()).get_value() ==  Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->city()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->postal_code()).get_value() ==  Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->country()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Nullable<Registry::MojeIDImplData::MandatoryAddressValidationError> addr_err_impl_res = CorbaConversion::unwrap_into<
                    Nullable<Registry::MojeIDImplData::MandatoryAddressValidationError> >(addr_err.in());

    BOOST_CHECK(addr_err_impl_res.get_value().address_presence.get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(addr_err_impl_res.get_value().street1.get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(addr_err_impl_res.get_value().city.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(addr_err_impl_res.get_value().postal_code.get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(addr_err_impl_res.get_value().country.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_shippingaddressvalidationerror)
{
    Registry::MojeIDImplData::ShippingAddressValidationError addr_err_impl;
    addr_err_impl.street1 = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::INVALID);
    addr_err_impl.city = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    addr_err_impl.postal_code = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::REQUIRED);
    addr_err_impl.country = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Registry::MojeID::ShippingAddressValidationError_var addr_err = CorbaConversion::wrap_into<
        Registry::MojeID::ShippingAddressValidationError_var>(addr_err_impl);

    BOOST_REQUIRE(addr_err.operator ->() != NULL);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->street1.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->city.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->postal_code.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->country.in()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Registry::MojeIDImplData::ShippingAddressValidationError addr_err_impl_res = CorbaConversion::unwrap_into<
        Registry::MojeIDImplData::ShippingAddressValidationError>(addr_err.in());

    BOOST_CHECK(addr_err_impl_res.street1.get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(addr_err_impl_res.city.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(addr_err_impl_res.postal_code.get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(addr_err_impl_res.country.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_nullableshippingaddressvalidationerror)
{
    Registry::MojeIDImplData::ShippingAddressValidationError addr_err_impl;
    addr_err_impl.street1 = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::INVALID);
    addr_err_impl.city = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    addr_err_impl.postal_code = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::REQUIRED);
    addr_err_impl.country = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Registry::MojeID::NullableShippingAddressValidationError_var addr_err = CorbaConversion::wrap_into<
        Registry::MojeID::NullableShippingAddressValidationError_var>(
            Nullable<Registry::MojeIDImplData::ShippingAddressValidationError>(addr_err_impl));

    BOOST_REQUIRE(addr_err.operator ->() != NULL);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->street1()).get_value() ==  Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->city()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->postal_code()).get_value() ==  Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        addr_err->country()).get_value() ==  Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    Nullable<Registry::MojeIDImplData::ShippingAddressValidationError> addr_err_impl_res = CorbaConversion::unwrap_into<
                    Nullable<Registry::MojeIDImplData::ShippingAddressValidationError> >(addr_err.in());

    BOOST_CHECK(addr_err_impl_res.get_value().street1.get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(addr_err_impl_res.get_value().city.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(addr_err_impl_res.get_value().postal_code.get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(addr_err_impl_res.get_value().country.get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_message_limit_exceeded)
{
    BOOST_CHECK_THROW(CorbaConversion::wrap_into<Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED>(
        Registry::MojeIDImplData::MessageLimitExceeded()), Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::MessageLimitExceeded msg;
    msg.limit_expire_date = boost::gregorian::date(2015,12,10);
    msg.limit_count = 11;
    msg.limit_days = 12;

    Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED res = CorbaConversion::wrap_into<
                    Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED>(msg);

    BOOST_CHECK(CorbaConversion::unwrap_into<boost::gregorian::date>(
        res.limit_expire_date) == msg.limit_expire_date);
    BOOST_CHECK(res.limit_count == msg.limit_count);
    BOOST_CHECK(res.limit_days == msg.limit_days);
}

BOOST_AUTO_TEST_CASE(test_mojeid_registration_validation_error)
{
    Registry::MojeIDImplData::RegistrationValidationError ex;
    ex.first_name = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        static_cast<Registry::MojeIDImplData::ValidationError::EnumType>(10));
    BOOST_CHECK_THROW(CorbaConversion::wrap_into<
        Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR>(ex),
        Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::AddressValidationError permanent_addr_err_impl;
    permanent_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::INVALID;
    permanent_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    permanent_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::REQUIRED;
    permanent_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::AddressValidationError mailing_addr_err_impl;
    mailing_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    mailing_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::REQUIRED;
    mailing_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::INVALID;
    mailing_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::AddressValidationError billing_addr_err_impl;
    billing_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::REQUIRED;
    billing_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    billing_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::INVALID;
    billing_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::REQUIRED;

    Registry::MojeIDImplData::ShippingAddressValidationError shipping_addr_err_impl;
    shipping_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::INVALID;
    shipping_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    shipping_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::REQUIRED;
    shipping_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::ShippingAddressValidationError shipping2_addr_err_impl;
    shipping2_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::REQUIRED;
    shipping2_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    shipping2_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::INVALID;
    shipping2_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::ShippingAddressValidationError shipping3_addr_err_impl;
    shipping3_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::INVALID;
    shipping3_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    shipping3_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::REQUIRED;
    shipping3_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::RegistrationValidationError reg_val_err_impl;

    reg_val_err_impl.username = Registry::MojeIDImplData::ValidationError::INVALID;
    reg_val_err_impl.first_name = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    reg_val_err_impl.last_name = Registry::MojeIDImplData::ValidationError::REQUIRED;
    reg_val_err_impl.birth_date = Registry::MojeIDImplData::ValidationError::INVALID;
    reg_val_err_impl.email = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    reg_val_err_impl.notify_email = Registry::MojeIDImplData::ValidationError::REQUIRED;
    reg_val_err_impl.phone = Registry::MojeIDImplData::ValidationError::INVALID;
    reg_val_err_impl.fax = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    reg_val_err_impl.permanent = permanent_addr_err_impl;
    reg_val_err_impl.mailing = mailing_addr_err_impl;
    reg_val_err_impl.billing = billing_addr_err_impl;

    reg_val_err_impl.shipping = shipping_addr_err_impl;
    reg_val_err_impl.shipping2 = shipping2_addr_err_impl;
    reg_val_err_impl.shipping3 = shipping3_addr_err_impl;

    Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR res;
    CorbaConversion::wrap(reg_val_err_impl,res);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.username.in()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.first_name.in()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.last_name.in()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.birth_date.in()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.email.in()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.notify_email.in()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.phone.in()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.fax.in()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->street1()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.mailing->street1()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.mailing->city()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.mailing->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.mailing->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.billing->street1()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.billing->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.billing->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.billing->country()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping->street1()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping2->street1()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping2->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping2->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping2->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping3->street1()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping3->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping3->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping3->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_update_contact_prepare_validation_error)
{
    Registry::MojeIDImplData::UpdateContactPrepareValidationError ex;
    ex.first_name = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        static_cast<Registry::MojeIDImplData::ValidationError::EnumType>(10));
    BOOST_CHECK_THROW(CorbaConversion::wrap_into<
        Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR>(ex),
        Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::AddressValidationError permanent_addr_err_impl;
    permanent_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::INVALID;
    permanent_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    permanent_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::REQUIRED;
    permanent_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::AddressValidationError mailing_addr_err_impl;
    mailing_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    mailing_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::REQUIRED;
    mailing_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::INVALID;
    mailing_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::AddressValidationError billing_addr_err_impl;
    billing_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::REQUIRED;
    billing_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    billing_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::INVALID;
    billing_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::REQUIRED;

    Registry::MojeIDImplData::ShippingAddressValidationError shipping_addr_err_impl;
    shipping_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::INVALID;
    shipping_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    shipping_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::REQUIRED;
    shipping_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::ShippingAddressValidationError shipping2_addr_err_impl;
    shipping2_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::REQUIRED;
    shipping2_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    shipping2_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::INVALID;
    shipping2_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::ShippingAddressValidationError shipping3_addr_err_impl;
    shipping3_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::INVALID;
    shipping3_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    shipping3_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::REQUIRED;
    shipping3_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::UpdateContactPrepareValidationError upd_val_err_impl;

    upd_val_err_impl.first_name = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    upd_val_err_impl.last_name = Registry::MojeIDImplData::ValidationError::REQUIRED;
    upd_val_err_impl.birth_date = Registry::MojeIDImplData::ValidationError::INVALID;
    upd_val_err_impl.email = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    upd_val_err_impl.notify_email = Registry::MojeIDImplData::ValidationError::REQUIRED;
    upd_val_err_impl.phone = Registry::MojeIDImplData::ValidationError::INVALID;
    upd_val_err_impl.fax = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    upd_val_err_impl.permanent = permanent_addr_err_impl;
    upd_val_err_impl.mailing = mailing_addr_err_impl;
    upd_val_err_impl.billing = billing_addr_err_impl;

    upd_val_err_impl.shipping = shipping_addr_err_impl;
    upd_val_err_impl.shipping2 = shipping2_addr_err_impl;
    upd_val_err_impl.shipping3 = shipping3_addr_err_impl;

    Registry::MojeID::Server::UPDATE_CONTACT_PREPARE_VALIDATION_ERROR res;
    CorbaConversion::wrap(upd_val_err_impl,res);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.first_name.in()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.last_name.in()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.birth_date.in()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.email.in()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.notify_email.in()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.phone.in()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.fax.in()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->street1()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.mailing->street1()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.mailing->city()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.mailing->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.mailing->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.billing->street1()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.billing->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.billing->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.billing->country()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping->street1()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping2->street1()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping2->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping2->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping2->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping3->street1()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping3->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping3->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.shipping3->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);

}

BOOST_AUTO_TEST_CASE(test_mojeid_create_validation_request_validation_error)
{
    Registry::MojeIDImplData::CreateValidationRequestValidationError ex;
    ex.first_name = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        static_cast<Registry::MojeIDImplData::ValidationError::EnumType>(10));
    BOOST_CHECK_THROW(CorbaConversion::wrap_into<
        Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR>(ex),
        Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::MandatoryAddressValidationError mandatory_addr_err_impl;
    mandatory_addr_err_impl.address_presence = Registry::MojeIDImplData::ValidationError::REQUIRED;
    mandatory_addr_err_impl.street1 = Registry::MojeIDImplData::ValidationError::INVALID;
    mandatory_addr_err_impl.city = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    mandatory_addr_err_impl.postal_code = Registry::MojeIDImplData::ValidationError::REQUIRED;
    mandatory_addr_err_impl.country = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;

    Registry::MojeIDImplData::CreateValidationRequestValidationError crr_val_err_impl;

    crr_val_err_impl.first_name = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    crr_val_err_impl.last_name = Registry::MojeIDImplData::ValidationError::REQUIRED;
    crr_val_err_impl.email = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    crr_val_err_impl.notify_email = Registry::MojeIDImplData::ValidationError::REQUIRED;
    crr_val_err_impl.phone = Registry::MojeIDImplData::ValidationError::INVALID;
    crr_val_err_impl.fax = Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE;
    crr_val_err_impl.ssn = Registry::MojeIDImplData::ValidationError::INVALID;

    crr_val_err_impl.permanent = mandatory_addr_err_impl;

    Registry::MojeID::Server::CREATE_VALIDATION_REQUEST_VALIDATION_ERROR res;
    CorbaConversion::wrap(crr_val_err_impl,res);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.first_name.in()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.last_name.in()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.email.in()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.notify_email.in()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.phone.in()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.fax.in()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.ssn.in()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->address_presence()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->street1()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->city()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->postal_code()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.permanent->country()).get_value() == Registry::MojeIDImplData::ValidationError::NOT_AVAILABLE);
}

BOOST_AUTO_TEST_CASE(test_mojeid_process_registration_request_validation_error)
{
    Registry::MojeIDImplData::ProcessRegistrationValidationError ex;
    ex.email = Nullable<Registry::MojeIDImplData::ValidationError::EnumType>(
        static_cast<Registry::MojeIDImplData::ValidationError::EnumType>(10));
    BOOST_CHECK_THROW(CorbaConversion::wrap_into<
        Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR>(ex),
        Registry::MojeID::Server::INTERNAL_SERVER_ERROR);

    Registry::MojeIDImplData::ProcessRegistrationValidationError prr_val_err_impl;

    prr_val_err_impl.email = Registry::MojeIDImplData::ValidationError::INVALID;
    prr_val_err_impl.phone = Registry::MojeIDImplData::ValidationError::REQUIRED;

    Registry::MojeID::Server::PROCESS_REGISTRATION_VALIDATION_ERROR res;
    CorbaConversion::wrap(prr_val_err_impl,res);

    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.email.in()).get_value() == Registry::MojeIDImplData::ValidationError::INVALID);
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<Registry::MojeIDImplData::ValidationError::EnumType> >(
        res.phone.in()).get_value() == Registry::MojeIDImplData::ValidationError::REQUIRED);
}

BOOST_AUTO_TEST_CASE(test_mojeid_create_contact)
{
    Registry::MojeID::CreateContact_var cc = new Registry::MojeID::CreateContact;

    cc->username = CorbaConversion::wrap_into<CORBA::String_var>(std::string("username"));
    cc->first_name = CorbaConversion::wrap_into<CORBA::String_var>(std::string("first_name"));
    cc->last_name = CorbaConversion::wrap_into<CORBA::String_var>(std::string("last_name"));
    cc->organization = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
        Nullable<std::string>("org"));
    cc->vat_reg_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
        Nullable<std::string>("vat_reg_num"));
    cc->birth_date = CorbaConversion::wrap_into<Registry::MojeID::NullableDate_var>(
        Nullable<boost::gregorian::date>(boost::gregorian::date(2015,12,10)));
    cc->id_card_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
        Nullable<std::string>("id_card_num"));
    cc->passport_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
        Nullable<std::string>("passport_num"));
    cc->ssn_id_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
        Nullable<std::string>("ssn_id_num"));
    cc->vat_id_num = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
        Nullable<std::string>("vat_id_num"));
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "st1";
        addr_impl.street2 = "st2";
        addr_impl.street3 = "st3";
        addr_impl.city = "Praha";
        addr_impl.state = "state";
        addr_impl.country = "Czech Republic";
        cc->permanent = CorbaConversion::wrap_into<Registry::MojeID::Address_var>(addr_impl);
    }
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "m_st1";
        addr_impl.street2 = "m_st2";
        addr_impl.street3 = "m_st3";
        addr_impl.city = "m_Praha";
        addr_impl.state = "m_state";
        addr_impl.country = "m_Czech Republic";

        cc->mailing = CorbaConversion::wrap_into<Registry::MojeID::NullableAddress_var>(
            Nullable<Registry::MojeIDImplData::Address>(addr_impl));
    }
    {
        Registry::MojeIDImplData::Address addr_impl;
        addr_impl.street1 = "b_st1";
        addr_impl.street2 = "b_st2";
        addr_impl.street3 = "b_st3";
        addr_impl.city = "b_Praha";
        addr_impl.state = "b_state";
        addr_impl.country = "b_Czech Republic";

        cc->billing = CorbaConversion::wrap_into<Registry::MojeID::NullableAddress_var>(
            Nullable<Registry::MojeIDImplData::Address>(addr_impl));
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

        cc->shipping = CorbaConversion::wrap_into<Registry::MojeID::NullableShippingAddress_var>(
            Nullable<Registry::MojeIDImplData::ShippingAddress>(addr_impl));
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

        cc->shipping2 = CorbaConversion::wrap_into<Registry::MojeID::NullableShippingAddress_var>(
            Nullable<Registry::MojeIDImplData::ShippingAddress>(addr_impl));
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

        cc->shipping3 = CorbaConversion::wrap_into<Registry::MojeID::NullableShippingAddress_var>(
            Nullable<Registry::MojeIDImplData::ShippingAddress>(addr_impl));
    }
    cc->email = CorbaConversion::wrap_into<CORBA::String_var>(std::string("email"));
    cc->notify_email = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
        Nullable<std::string>("notify_email"));
    cc->telephone = CorbaConversion::wrap_into<CORBA::String_var>(std::string("telephone"));
    cc->fax = CorbaConversion::wrap_into<Registry::MojeID::NullableString_var>(
        Nullable<std::string>("fax"));

    Registry::MojeIDImplData::CreateContact cc_impl = CorbaConversion::unwrap_into<
        Registry::MojeIDImplData::CreateContact>(cc.in());

    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(
        cc->username.in()) == std::string("username"));
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(
        cc->first_name.in()) == std::string("first_name"));
    BOOST_CHECK(CorbaConversion::unwrap_into<std::string>(
        cc->last_name.in()) == std::string("last_name"));
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        cc->organization.in()) == Nullable<std::string>("org"));
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        cc->vat_reg_num.in()) == Nullable<std::string>("vat_reg_num"));
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<boost::gregorian::date> >(
        cc->birth_date.in()).get_value() == boost::gregorian::date(2015,12,10));
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        cc->id_card_num.in()) == Nullable<std::string>("id_card_num"));
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        cc->passport_num.in()) == Nullable<std::string>("passport_num"));
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        cc->ssn_id_num.in()) == Nullable<std::string>("ssn_id_num"));
    BOOST_CHECK(CorbaConversion::unwrap_into<Nullable<std::string> >(
        cc->vat_id_num.in()) == Nullable<std::string>("vat_id_num"));

    BOOST_REQUIRE(cc.operator ->() != NULL);
    BOOST_CHECK(std::string(cc->permanent.street1.in()) == "st1");
    BOOST_CHECK(std::string(cc->permanent.street2.in()->_value()) == "st2");
    BOOST_CHECK(std::string(cc->permanent.street3.in()->_value()) == "st3");
    BOOST_CHECK(std::string(cc->permanent.city.in()) == "Praha");
    BOOST_CHECK(std::string(cc->permanent.state.in()->_value()) == "state");
    BOOST_CHECK(std::string(cc->permanent.country.in()) == "Czech Republic");

    Registry::MojeIDImplData::Address permanent_addr_res = CorbaConversion::unwrap_into<
                    Registry::MojeIDImplData::Address>(cc->permanent);
    BOOST_CHECK(permanent_addr_res.street1 == "st1");
    BOOST_CHECK(permanent_addr_res.street2.get_value() == "st2");
    BOOST_CHECK(permanent_addr_res.street3.get_value() == "st3");
    BOOST_CHECK(permanent_addr_res.city == "Praha");
    BOOST_CHECK(permanent_addr_res.state.get_value() == "state");
    BOOST_CHECK(permanent_addr_res.country == "Czech Republic");

    Nullable<Registry::MojeIDImplData::Address> mailing_addr_res = CorbaConversion::unwrap_into<
        Nullable<Registry::MojeIDImplData::Address> >(cc->mailing.in());

    BOOST_CHECK(mailing_addr_res.get_value().street1 == "m_st1");
    BOOST_CHECK(mailing_addr_res.get_value().street2.get_value() == "m_st2");
    BOOST_CHECK(mailing_addr_res.get_value().street3.get_value() == "m_st3");
    BOOST_CHECK(mailing_addr_res.get_value().city == "m_Praha");
    BOOST_CHECK(mailing_addr_res.get_value().state.get_value() == "m_state");
    BOOST_CHECK(mailing_addr_res.get_value().country == "m_Czech Republic");

    Nullable<Registry::MojeIDImplData::Address> billing_addr_res = CorbaConversion::unwrap_into<
        Nullable<Registry::MojeIDImplData::Address> >(cc->billing.in());

    BOOST_CHECK(billing_addr_res.get_value().street1 == "b_st1");
    BOOST_CHECK(billing_addr_res.get_value().street2.get_value() == "b_st2");
    BOOST_CHECK(billing_addr_res.get_value().street3.get_value() == "b_st3");
    BOOST_CHECK(billing_addr_res.get_value().city == "b_Praha");
    BOOST_CHECK(billing_addr_res.get_value().state.get_value() == "b_state");
    BOOST_CHECK(billing_addr_res.get_value().country == "b_Czech Republic");

    Nullable<Registry::MojeIDImplData::ShippingAddress> shipping_addr_res = CorbaConversion::unwrap_into<
        Nullable<Registry::MojeIDImplData::ShippingAddress> >(cc->shipping.in());

    BOOST_CHECK(shipping_addr_res.get_value().company_name.get_value() == "s_company");
    BOOST_CHECK(shipping_addr_res.get_value().street1 == "s_st1");
    BOOST_CHECK(shipping_addr_res.get_value().street2.get_value() == "s_st2");
    BOOST_CHECK(shipping_addr_res.get_value().street3.get_value() == "s_st3");
    BOOST_CHECK(shipping_addr_res.get_value().city == "s_Praha");
    BOOST_CHECK(shipping_addr_res.get_value().state.get_value() == "s_state");
    BOOST_CHECK(shipping_addr_res.get_value().country == "s_Czech Republic");

    Nullable<Registry::MojeIDImplData::ShippingAddress> shipping2_addr_res = CorbaConversion::unwrap_into<
        Nullable<Registry::MojeIDImplData::ShippingAddress> >(cc->shipping2.in());

    BOOST_CHECK(shipping2_addr_res.get_value().company_name.get_value() == "s2_company");
    BOOST_CHECK(shipping2_addr_res.get_value().street1 == "s2_st1");
    BOOST_CHECK(shipping2_addr_res.get_value().street2.get_value() == "s2_st2");
    BOOST_CHECK(shipping2_addr_res.get_value().street3.get_value() == "s2_st3");
    BOOST_CHECK(shipping2_addr_res.get_value().city == "s2_Praha");
    BOOST_CHECK(shipping2_addr_res.get_value().state.get_value() == "s2_state");
    BOOST_CHECK(shipping2_addr_res.get_value().country == "s2_Czech Republic");

    Nullable<Registry::MojeIDImplData::ShippingAddress> shipping3_addr_res = CorbaConversion::unwrap_into<
        Nullable<Registry::MojeIDImplData::ShippingAddress> >(cc->shipping3.in());

    BOOST_CHECK(shipping3_addr_res.get_value().company_name.get_value() == "s3_company");
    BOOST_CHECK(shipping3_addr_res.get_value().street1 == "s3_st1");
    BOOST_CHECK(shipping3_addr_res.get_value().street2.get_value() == "s3_st2");
    BOOST_CHECK(shipping3_addr_res.get_value().street3.get_value() == "s3_st3");
    BOOST_CHECK(shipping3_addr_res.get_value().city == "s3_Praha");
    BOOST_CHECK(shipping3_addr_res.get_value().state.get_value() == "s3_state");
    BOOST_CHECK(shipping3_addr_res.get_value().country == "s3_Czech Republic");

    BOOST_CHECK(std::string(cc->email.in()) == "email");
    BOOST_CHECK(std::string(cc->notify_email.in()->_value()) == "notify_email");
    BOOST_CHECK(std::string(cc->telephone.in()) == "telephone");
    BOOST_CHECK(std::string(cc->fax.in()->_value()) == "fax");
}

BOOST_AUTO_TEST_CASE(test_basic_numeric_conversion)
{
    unsigned long long a = 1;
    short b = -1;

    CorbaConversion::boostNumericTypeConvertor(a, b);
    BOOST_CHECK( a == 1);
    BOOST_CHECK( b == 1);

    CorbaConversion::boostNumericTypeConvertor(1.00, b);
    BOOST_CHECK( b == 1);

    BOOST_CHECK_THROW((CorbaConversion::boostNumericTypeConvertor(-1, a)), CorbaConversion::NumericConversionOutOfRange);
    BOOST_CHECK_THROW(CorbaConversion::boostNumericTypeConvertor(0.1, b), CorbaConversion::NumericConversionPrecisionLoss);
}


BOOST_AUTO_TEST_SUITE_END();
