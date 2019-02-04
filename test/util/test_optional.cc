/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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

#include "util/optional_value.hh"

#include <sstream>
#include <stdexcept>
#include <limits>
#include <vector>

namespace
{
typedef int Ta;
typedef unsigned Tb;
typedef std::numeric_limits< Ta > LTa;
typedef std::numeric_limits< Tb > LTb;
typedef Optional< Ta > OTa;
typedef Optional< Tb > OTb;
}

//not using UTF defined main
#define BOOST_TEST_NO_MAIN


#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestOptional)

const std::string server_name = "test-optional";

static bool check_std_logic_error(const std::logic_error &_ex)
{
    return 0 < std::strlen(_ex.what());
}

template < typename T >
void test_the_same_type(const Optional< T > &o0, T v1)
{
    typedef Optional< T > OT;
    OT o1(v1);
    const bool identical = o0.isset() && (o0.get_value() == v1);
    const bool different = !identical;
    BOOST_CHECK((o0 == v1) == identical); // operator==(const Optional<T>&, const T&)
    BOOST_CHECK((v1 == o0) == identical); // operator==(const T&, const Optional<T>&)
    BOOST_CHECK((o0 == o1) == identical); // operator==(const Optional<T>&, const Optional<T>&)
    BOOST_CHECK((o1 == o0) == identical); // operator==(const Optional<T>&, const Optional<T>&)

    BOOST_CHECK((o0 != v1) == different); // operator!=(const Optional<T>&, const T&)
    BOOST_CHECK((v1 != o0) == different); // operator!=(const T&, const Optional<T>&)
    BOOST_CHECK((o0 != o1) == different); // operator!=(const Optional<T>&, const Optional<T>&)
    BOOST_CHECK((o1 != o0) == different); // operator!=(const Optional<T>&, const Optional<T>&)
}

template < typename T >
void test_any_value_of_same_type(const T *values, const T *const values_end)
{
    typedef Optional< T > OT;
    for (const T *pv0 = values; pv0 <= values_end; ++pv0) {
        const bool present = (pv0 < values_end);
        OT o0(present ? OT(*pv0) : OT()); // copy constructor Optional<T>(const Optional<T>&)
        BOOST_CHECK(o0.isset() == present);
        if (present) {
            BOOST_CHECK_NO_THROW (o0.get_value());
            BOOST_CHECK(o0.get_value() == *pv0); // get_value()
            BOOST_CHECK(o0.get_value() == o0.get_value_or_default());
            BOOST_CHECK(o0.get_value_or_default() == *pv0); // get_value_or_default()
        }
        else {
            BOOST_CHECK_EXCEPTION(o0.get_value(), std::logic_error, check_std_logic_error);
            BOOST_CHECK(o0.get_value_or_default() == T()); // get_value_or_default()
        }
        for (const T *pv1 = values; pv1 < values_end; ++pv1) {
            test_the_same_type(o0, *pv1);
        }
    }
}

template < typename T0, typename T1 >
void test_convertible_types(T0 v0, T1 v1)
{
    typedef Optional< T0 > OT0;
    typedef Optional< T1 > OT1;

    std::vector<T0> t0_v;
    t0_v.push_back(v0);
    t0_v.push_back(v1);

    std::vector<T1> t1_v;
    t1_v.push_back(v0);
    t1_v.push_back(v1);

    std::vector<OT0> o0i;
    o0i.push_back(v0);  // init constructor Optional< T0 >(const T0&)
    o0i.push_back(v1);  // init constructor Optional< T0 >(const T1&)

    std::vector<OT1> o1i;
    o1i.push_back(v0);  // init constructor Optional< T1 >(const T0&)
    o1i.push_back(v1);  // init constructor Optional< T1 >(const T1&)

    std::vector<OT0> o0c;
    o0c.push_back(o0i[0]);  // copy constructor Optional< T0 >(const Optional< T0 >&)
    o0c.push_back(o0i[1]);  // copy constructor Optional< T0 >(const Optional< T0 >&)
    o0c.push_back(o1i[0]);  // copy constructor Optional< T0 >(const Optional< T1 >&)
    o0c.push_back(o1i[1]);  // copy constructor Optional< T0 >(const Optional< T1 >&)

    std::vector<OT1> o1c;
    o1c.push_back(o0i[0]);  // copy constructor Optional< T1 >(const Optional< T0 >&)
    o1c.push_back(o0i[1]);  // copy constructor Optional< T1 >(const Optional< T0 >&)
    o1c.push_back(o1i[0]);  // copy constructor Optional< T1 >(const Optional< T1 >&)
    o1c.push_back(o1i[1]);  // copy constructor Optional< T1 >(const Optional< T1 >&)

    for (int variant = 0; variant < 2; ++variant) {
        switch (variant) { // 0..1
        case 1:
            o0i[0] = v0; // Optional< T0 >::operator=(const T0&)
            o0i[1] = v1; // Optional< T0 >::operator=(const T1&)
            o1i[0] = v0; // Optional< T1 >::operator=(const T0&)
            o1i[1] = v1; // Optional< T1 >::operator=(const T1&)
            o0c[0] = o0i[0]; // Optional< T0 >::operator=(const Optional< T0 >&)
            o0c[1] = o0i[1];
            o0c[2] = o1i[0]; // Optional< T0 >::operator=(const Optional< T1 >&)
            o0c[3] = o1i[1];
            o1c[0] = o0i[0]; // Optional< T1 >::operator=(const Optional< T0 >&)
            o1c[1] = o0i[1];
            o1c[2] = o1i[0]; // Optional< T1 >::operator=(const Optional< T1 >&)
            o1c[3] = o1i[1];
            break;
        }
        for (int idx = 0; idx <= 1; ++idx) {
            BOOST_CHECK(o0i[idx]     == t0_v[idx]); // operator==(const Optional< T0 >&, const T0&)const
            BOOST_CHECK(o1i[idx]     == t1_v[idx]); // operator==(const Optional< T1 >&, const T1&)const
            BOOST_CHECK(o0c[idx]     == t0_v[idx]); // operator==(const Optional< T0 >&, const T0&)const
            BOOST_CHECK(o0c[2 + idx] == t0_v[idx]); // operator==(const Optional< T0 >&, const T0&)const
            BOOST_CHECK(o1c[idx]     == t1_v[idx]); // operator==(const Optional< T1 >&, const T1&)const
            BOOST_CHECK(o1c[2 + idx] == t1_v[idx]); // operator==(const Optional< T1 >&, const T1&)const

            BOOST_CHECK(!(o0i[idx]     != t0_v[idx])); // operator!=(const Optional< T0 >&, const T0&)const
            BOOST_CHECK(!(o1i[idx]     != t1_v[idx])); // operator!=(const Optional< T1 >&, const T1&)const
            BOOST_CHECK(!(o0c[idx]     != t0_v[idx])); // operator!=(const Optional< T0 >&, const T0&)const
            BOOST_CHECK(!(o0c[2 + idx] != t0_v[idx])); // operator!=(const Optional< T0 >&, const T0&)const
            BOOST_CHECK(!(o1c[idx]     != t1_v[idx])); // operator!=(const Optional< T1 >&, const T1&)const
            BOOST_CHECK(!(o1c[2 + idx] != t1_v[idx])); // operator!=(const Optional< T1 >&, const T1&)const

            BOOST_CHECK(t0_v[idx] == o0i[idx]);     // operator==(const T0&, const Optional< T0 >&)const
            BOOST_CHECK(t1_v[idx] == o1i[idx]);     // operator==(const T1&, const Optional< T1 >&)const
            BOOST_CHECK(t0_v[idx] == o0c[idx]);     // operator==(const T0&, const Optional< T0 >&)const
            BOOST_CHECK(t0_v[idx] == o0c[2 + idx]); // operator==(const T0&, const Optional< T0 >&)const
            BOOST_CHECK(t1_v[idx] == o1c[idx]);     // operator==(const T1&, const Optional< T1 >&)const
            BOOST_CHECK(t1_v[idx] == o1c[2 + idx]); // operator==(const T1&, const Optional< T1 >&)const

            BOOST_CHECK(!(t0_v[idx] != o0i[idx]));     // operator!=(const T0&, const Optional< T0 >&)const
            BOOST_CHECK(!(t1_v[idx] != o1i[idx]));     // operator!=(const T1&, const Optional< T1 >&)const
            BOOST_CHECK(!(t0_v[idx] != o0c[idx]));     // operator!=(const T0&, const Optional< T0 >&)const
            BOOST_CHECK(!(t0_v[idx] != o0c[2 + idx])); // operator!=(const T0&, const Optional< T0 >&)const
            BOOST_CHECK(!(t1_v[idx] != o1c[idx]));     // operator!=(const T1&, const Optional< T1 >&)const
            BOOST_CHECK(!(t1_v[idx] != o1c[2 + idx])); // operator!=(const T1&, const Optional< T1 >&)const
        }
    }
}

BOOST_AUTO_TEST_CASE( test_all_values )
{
    {
        const OTa not_present; // default constructor
        BOOST_CHECK(!not_present.isset());
        BOOST_CHECK_EXCEPTION(not_present.get_value(), std::logic_error, check_std_logic_error);
        BOOST_CHECK_NO_THROW (not_present.get_value_or_default());
    }
    static const Ta va[] = {
        LTa::min(), LTa::min() + 1, -1, 0, 1, LTa::max() - 1, LTa::max()};
    static const Ta *const va_end = va + sizeof(va) / sizeof(*va);
    test_any_value_of_same_type(va, va_end);

    {
        const OTb not_present; // default constructor
        BOOST_CHECK(!not_present.isset());
        BOOST_CHECK_EXCEPTION(not_present.get_value(), std::logic_error, check_std_logic_error);
        BOOST_CHECK_NO_THROW (not_present.get_value_or_default());
    }
    static const Tb vb[] = {
        LTb::min(), LTb::min() + 1, LTb::max() - 1, LTb::max()};
    static const Tb *const vb_end = vb + sizeof(vb) / sizeof(*vb);
    test_any_value_of_same_type(vb, vb_end);

    for (const Ta *pva = va; pva < va_end; ++pva) {
        for (const Tb *pvb = vb; pvb < vb_end; ++pvb) {
            test_convertible_types(*pva, *pvb);
        }
    }
}

BOOST_AUTO_TEST_CASE( test_get_value_or )
{
    BOOST_CHECK_EQUAL( Optional<int>().get_value_or(0), 0);
    BOOST_CHECK_EQUAL( Optional<int>().get_value_or(42), 42);
    BOOST_CHECK_EQUAL( Optional<std::string>().get_value_or(""), "");
    BOOST_CHECK_EQUAL( Optional<std::string>().get_value_or("aBc"), "aBc");

    BOOST_CHECK_EQUAL( Optional<int>(13).get_value_or(0), 13);
    BOOST_CHECK_EQUAL( Optional<int>(144).get_value_or(42), 144);
    BOOST_CHECK_EQUAL( Optional<std::string>("Prague").get_value_or(""), "Prague");
    BOOST_CHECK_EQUAL( Optional<std::string>("CZ.NIC").get_value_or("aBc"), "CZ.NIC");
}

#if 0
void unable_to_compile()
{
    int a = 5;
    unsigned b = 7;
    Optional< int > oa = a;
    Optional< unsigned > ob = b;
    oa == b; // error: no match for ‘operator==’ in ‘oa == b’
    b == oa; // error: no match for ‘operator==’ in ‘b == oa’
    ob == a; // error: no match for ‘operator==’ in ‘ob == a’
    a == ob; // error: no match for ‘operator==’ in ‘a == ob’
    oa != b; // error: no match for ‘operator!=’ in ‘oa != b’
    b != oa; // error: no match for ‘operator!=’ in ‘b != oa’
    ob != a; // error: no match for ‘operator!=’ in ‘ob != a’
    a != ob; // error: no match for ‘operator!=’ in ‘a != ob’
    oa == ob; // error: no match for ‘operator==’ in ‘oa == ob’
    ob == oa; // error: no match for ‘operator==’ in ‘ob == oa’
    oa != ob; // error: no match for ‘operator!=’ in ‘oa != ob’
    ob != oa; // error: no match for ‘operator!=’ in ‘ob != oa’
}
#endif

BOOST_AUTO_TEST_SUITE_END();//TestOptional
