/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#include "util/db/nullable.hh"

#include <sstream>
#include <stdexcept>
#include <limits>

namespace
{
typedef int Ta;
typedef unsigned Tb;
typedef std::numeric_limits< Ta > LTa;
typedef std::numeric_limits< Tb > LTb;
typedef Nullable< Ta > NTa;
typedef Nullable< Tb > NTb;
}

//not using UTF defined main
#define BOOST_TEST_NO_MAIN


#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestNullable)

const std::string server_name = "test-nullable";

static bool check_std_logic_error(const std::logic_error &_ex)
{
    return 0 < std::strlen(_ex.what());
}

template < typename T >
void test_the_same_type(const Nullable< T > &n0, T v1)
{
    typedef Nullable< T > NT;
    NT n1(v1);
    const bool identical = !n0.isnull() && (n0.get_value() == v1);
    const bool different = !identical;
    BOOST_CHECK((n0 == v1) == identical); // operator==(const Nullable<T>&, const T&)
    BOOST_CHECK((v1 == n0) == identical); // operator==(const T&, const Nullable<T>&)
    BOOST_CHECK((n0 == n1) == identical); // operator==(const Nullable<T>&, const Nullable<T>&)
    BOOST_CHECK((n1 == n0) == identical); // operator==(const Nullable<T>&, const Nullable<T>&)

    BOOST_CHECK((n0 != v1) == different); // operator!=(const Nullable<T>&, const T&)
    BOOST_CHECK((v1 != n0) == different); // operator!=(const T&, const Nullable<T>&)
    BOOST_CHECK((n0 != n1) == different); // operator!=(const Nullable<T>&, const Nullable<T>&)
    BOOST_CHECK((n1 != n0) == different); // operator!=(const Nullable<T>&, const Nullable<T>&)
}

template < typename T >
void test_any_value_of_same_type(const T *values, const T *const values_end)
{
    typedef Nullable< T > NT;
    for (const T *pv0 = values; pv0 <= values_end; ++pv0) {
        const bool is_null = (pv0 == values_end);
        NT n0(is_null ? NT() : NT(*pv0)); // copy constructor Nullable<T>(const Nullable<T>&)
        BOOST_CHECK(n0.isnull() == is_null);
        if (is_null) {
            BOOST_CHECK_EXCEPTION(n0.get_value(), std::logic_error, check_std_logic_error);
            BOOST_CHECK(n0.get_value_or_default() == T()); // get_value_or_default()
        }
        else {
            BOOST_CHECK_NO_THROW (n0.get_value());
            BOOST_CHECK(n0.get_value() == *pv0); // get_value()
            BOOST_CHECK(n0.get_value() == n0.get_value_or_default());
            BOOST_CHECK(n0.get_value_or_default() == *pv0); // get_value_or_default()
        }
        for (const T *pv1 = values; pv1 < values_end; ++pv1) {
            test_the_same_type(n0, *pv1);
        }
    }
}

template < typename T0, typename T1 >
void test_convertible_types(T0 v0, T1 v1)
{
    typedef Nullable< T0 > NT0;
    typedef Nullable< T1 > NT1;

    std::vector<T0> t0_v;
    t0_v.push_back(v0);
    t0_v.push_back(v1);

    std::vector<T1> t1_v;
    t1_v.push_back(v0);
    t1_v.push_back(v1);

    std::vector<NT0> n0i;
    n0i.push_back(v0);  // init constructor Nullable< T0 >(const T0&)
    n0i.push_back(v1);  // init constructor Nullable< T0 >(const T1&)

    std::vector<NT1> n1i;
    n1i.push_back(v0);  // init constructor Nullable< T1 >(const T0&)
    n1i.push_back(v1);  // init constructor Nullable< T1 >(const T1&)

    std::vector<NT0> n0c;
    n0c.push_back(n0i[0]);  // copy constructor Nullable< T0 >(const Nullable< T0 >&)
    n0c.push_back(n0i[1]);  // copy constructor Nullable< T0 >(const Nullable< T0 >&)
    n0c.push_back(n1i[0]);  // copy constructor Nullable< T0 >(const Nullable< T1 >&)
    n0c.push_back(n1i[1]);  // copy constructor Nullable< T0 >(const Nullable< T1 >&)

    std::vector<NT1> n1c;
    n1c.push_back(n0i[0]);  // copy constructor Nullable< T1 >(const Nullable< T0 >&)
    n1c.push_back(n0i[1]);  // copy constructor Nullable< T1 >(const Nullable< T0 >&)
    n1c.push_back(n1i[0]);  // copy constructor Nullable< T1 >(const Nullable< T1 >&)
    n1c.push_back(n1i[1]);  // copy constructor Nullable< T1 >(const Nullable< T1 >&)

    for (int variant = 0; variant < 2; ++variant) {
        switch (variant) { // 0..1
        case 1:
            n0i[0] = v0; // Nullable< T0 >::operator=(const T0&)
            n0i[1] = v1; // Nullable< T0 >::operator=(const T1&)
            n1i[0] = v0; // Nullable< T1 >::operator=(const T0&)
            n1i[1] = v1; // Nullable< T1 >::operator=(const T1&)
            n0c[0] = n0i[0]; // Nullable< T0 >::operator=(const Nullable< T0 >&)
            n0c[1] = n0i[1];
            n0c[2] = n1i[0]; // Nullable< T0 >::operator=(const Nullable< T1 >&)
            n0c[3] = n1i[1];
            n1c[0] = n0i[0]; // Nullable< T1 >::operator=(const Nullable< T0 >&)
            n1c[1] = n0i[1];
            n1c[2] = n1i[0]; // Nullable< T1 >::operator=(const Nullable< T1 >&)
            n1c[3] = n1i[1];
            break;
        }
        for (int idx = 0; idx <= 1; ++idx) {
            BOOST_CHECK(n0i[idx]     == t0_v[idx]); // operator==(const Nullable< T0 >&, const T0&)const
            BOOST_CHECK(n1i[idx]     == t1_v[idx]); // operator==(const Nullable< T1 >&, const T1&)const
            BOOST_CHECK(n0c[idx]     == t0_v[idx]); // operator==(const Nullable< T0 >&, const T0&)const
            BOOST_CHECK(n0c[2 + idx] == t0_v[idx]); // operator==(const Nullable< T0 >&, const T0&)const
            BOOST_CHECK(n1c[idx]     == t1_v[idx]); // operator==(const Nullable< T1 >&, const T1&)const
            BOOST_CHECK(n1c[2 + idx] == t1_v[idx]); // operator==(const Nullable< T1 >&, const T1&)const

            BOOST_CHECK(!(n0i[idx]     != t0_v[idx])); // operator!=(const Nullable< T0 >&, const T0&)const
            BOOST_CHECK(!(n1i[idx]     != t1_v[idx])); // operator!=(const Nullable< T1 >&, const T1&)const
            BOOST_CHECK(!(n0c[idx]     != t0_v[idx])); // operator!=(const Nullable< T0 >&, const T0&)const
            BOOST_CHECK(!(n0c[2 + idx] != t0_v[idx])); // operator!=(const Nullable< T0 >&, const T0&)const
            BOOST_CHECK(!(n1c[idx]     != t1_v[idx])); // operator!=(const Nullable< T1 >&, const T1&)const
            BOOST_CHECK(!(n1c[2 + idx] != t1_v[idx])); // operator!=(const Nullable< T1 >&, const T1&)const

            BOOST_CHECK(t0_v[idx] == n0i[idx]);     // operator==(const T0&, const Nullable< T0 >&)const
            BOOST_CHECK(t1_v[idx] == n1i[idx]);     // operator==(const T1&, const Nullable< T1 >&)const
            BOOST_CHECK(t0_v[idx] == n0c[idx]);     // operator==(const T0&, const Nullable< T0 >&)const
            BOOST_CHECK(t0_v[idx] == n0c[2 + idx]); // operator==(const T0&, const Nullable< T0 >&)const
            BOOST_CHECK(t1_v[idx] == n1c[idx]);     // operator==(const T1&, const Nullable< T1 >&)const
            BOOST_CHECK(t1_v[idx] == n1c[2 + idx]); // operator==(const T1&, const Nullable< T1 >&)const

            BOOST_CHECK(!(t0_v[idx] != n0i[idx]));     // operator!=(const T0&, const Nullable< T0 >&)const
            BOOST_CHECK(!(t1_v[idx] != n1i[idx]));     // operator!=(const T1&, const Nullable< T1 >&)const
            BOOST_CHECK(!(t0_v[idx] != n0c[idx]));     // operator!=(const T0&, const Nullable< T0 >&)const
            BOOST_CHECK(!(t0_v[idx] != n0c[2 + idx])); // operator!=(const T0&, const Nullable< T0 >&)const
            BOOST_CHECK(!(t1_v[idx] != n1c[idx]));     // operator!=(const T1&, const Nullable< T1 >&)const
            BOOST_CHECK(!(t1_v[idx] != n1c[2 + idx])); // operator!=(const T1&, const Nullable< T1 >&)const
        }
    }
}

BOOST_AUTO_TEST_CASE( test_all_values )
{
    {
        const NTa null; // default constructor
        BOOST_CHECK(null.isnull());
        BOOST_CHECK_EXCEPTION(null.get_value(), std::logic_error, check_std_logic_error);
        BOOST_CHECK_NO_THROW (null.get_value_or_default());
    }
    static const Ta va[] = {
        LTa::min(), LTa::min() + 1, -1, 0, 1, LTa::max() - 1, LTa::max()};
    static const Ta *const va_end = va + sizeof(va) / sizeof(*va);
    test_any_value_of_same_type(va, va_end);

    {
        const NTb null; // default constructor
        BOOST_CHECK(null.isnull());
        BOOST_CHECK_EXCEPTION(null.get_value(), std::logic_error, check_std_logic_error);
        BOOST_CHECK_NO_THROW (null.get_value_or_default());
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
    BOOST_CHECK_EQUAL( Nullable<int>().get_value_or(0), 0);
    BOOST_CHECK_EQUAL( Nullable<int>().get_value_or(42), 42);
    BOOST_CHECK_EQUAL( Nullable<std::string>().get_value_or(""), "");
    BOOST_CHECK_EQUAL( Nullable<std::string>().get_value_or("aBc"), "aBc");

    BOOST_CHECK_EQUAL( Nullable<int>(13).get_value_or(0), 13);
    BOOST_CHECK_EQUAL( Nullable<int>(144).get_value_or(42), 144);
    BOOST_CHECK_EQUAL( Nullable<std::string>("Prague").get_value_or(""), "Prague");
    BOOST_CHECK_EQUAL( Nullable<std::string>("CZ.NIC").get_value_or("aBc"), "CZ.NIC");
}

struct MyDummyException {};

BOOST_AUTO_TEST_CASE( test_get_value_or_throw )
{
    BOOST_CHECK_THROW( Nullable<int>().get_value_or_throw<MyDummyException>( ), MyDummyException);
    /* For lack of other types I am throwing int, std::string, etc. (But I am safely catching as well! No type was harmed during testing.) */
    BOOST_CHECK_THROW( Nullable<int>().get_value_or_throw<int>(  ), int);
    BOOST_CHECK_THROW( Nullable<std::string>().get_value_or_throw<MyDummyException>( ), MyDummyException);
    BOOST_CHECK_THROW( Nullable<int>().get_value_or_throw<std::string>( ), std::string);
    BOOST_CHECK_EQUAL( Nullable<int>(144).get_value_or_throw<bool>( ), 144);
    BOOST_CHECK_EQUAL( Nullable<std::string>("Prague").get_value_or_throw<std::string>( ), "Prague");
    BOOST_CHECK_EQUAL( Nullable<std::string>("CZ.NIC").get_value_or_throw<MyDummyException>( ), "CZ.NIC");
}

#if 0
void unable_to_compile()
{
    int a = 5;
    unsigned b = 7;
    Nullable< int > na = a;
    Nullable< unsigned > nb = b;
    Nullable< const char* > s; // error: aggregate ‘Nullable<const char*> s’ has incomplete type and cannot be defined
    na == b; // error: no match for ‘operator==’ in ‘na == b’
    b == na; // error: no match for ‘operator==’ in ‘b == na’
    nb == a; // error: no match for ‘operator==’ in ‘nb == a’
    a == nb; // error: no match for ‘operator==’ in ‘a == nb’
    na != b; // error: no match for ‘operator!=’ in ‘na != b’
    b != na; // error: no match for ‘operator!=’ in ‘b != na’
    nb != a; // error: no match for ‘operator!=’ in ‘nb != a’
    a != nb; // error: no match for ‘operator!=’ in ‘a != nb’
    na == nb; // error: no match for ‘operator==’ in ‘na == nb’
    nb == na; // error: no match for ‘operator==’ in ‘nb == na’
    na != nb; // error: no match for ‘operator!=’ in ‘na != nb’
    nb != na; // error: no match for ‘operator!=’ in ‘nb != na’
}
#endif

BOOST_AUTO_TEST_SUITE_END();//TestNullable
