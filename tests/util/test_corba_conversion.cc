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

BOOST_AUTO_TEST_SUITE_END();
