/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/util/password_storage.hh"

#include "src/util/password_storage/impl/pbkdf2.hh"
#include "src/util/password_storage/impl/plaintext.hh"

#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

std::ostream& operator<<(std::ostream &out, const struct ::timespec& dt)
{
    std::ostringstream s;
    s << dt.tv_sec << "." << std::setw(9) << std::setfill('0') << std::right << dt.tv_nsec;
    return out << s.str();
}

BOOST_AUTO_TEST_SUITE(Tests)
BOOST_AUTO_TEST_SUITE(Util)
BOOST_AUTO_TEST_SUITE(PasswordStorage)

const std::string server_name = "test-password-storage";

namespace {

class Stopwatch
{
public:
    void start()
    {
        ::clock_gettime(clock, &t0_);
    }
    struct ::timespec get_intermediate_time()const
    {
        struct ::timespec t1;
        ::clock_gettime(clock, &t1);
        struct ::timespec dt;
        if (t0_.tv_nsec <= t1.tv_nsec)
        {
            dt.tv_sec = t1.tv_sec - t0_.tv_sec;
            dt.tv_nsec = t1.tv_nsec - t0_.tv_nsec;
        }
        else
        {
            dt.tv_sec = t1.tv_sec - t0_.tv_sec - 1;
            dt.tv_nsec = 1000000000 + t1.tv_nsec - t0_.tv_nsec;
        }
        return dt;
    }
private:
    static const ::clockid_t clock = CLOCK_PROCESS_CPUTIME_ID;
    struct ::timespec t0_;
};

typedef ::PasswordStorage::Impl::AlgPbkdf2<::PasswordStorage::Impl::Pbkdf2::HashFunction::sha512, 0x01 << 16> TestedAlgorithm;

template <class A>
std::string get_alg_tag()
{
    return A::get_prefix();
}

template <class A>
auto encrypt_password_using(const std::string& password)
{
    Stopwatch stopwatch;
    stopwatch.start();
    const auto encrypted = A::encrypt_password(password);
    const auto dt = stopwatch.get_intermediate_time();
    BOOST_TEST_MESSAGE("encrypt_password using " << get_alg_tag<A>() << " took " << dt << "s of CPU time");
    return encrypted;
}

bool test_is_password_correct(
        const std::string& password,
        const ::PasswordStorage::PasswordData& encrypted)
{
    Stopwatch stopwatch;
    stopwatch.start();
    bool password_is_correct;
    try
    {
        ::PasswordStorage::check_password(password, encrypted);
        password_is_correct = true;
    }
    catch (const ::PasswordStorage::IncorrectPassword&)
    {
        password_is_correct = false;
    }
    const struct ::timespec dt = stopwatch.get_intermediate_time();
    BOOST_TEST_MESSAGE("check_password took " << dt << "s of CPU time");
    return password_is_correct;
}

} // namespace {anonymous}

BOOST_AUTO_TEST_CASE(test_function_is_password_correct)
{
    const std::string password = "nazdar";
    const auto encrypted = encrypt_password_using<TestedAlgorithm>(password);
    BOOST_CHECK(test_is_password_correct(password, encrypted));
    const auto incorrect_password = password + " ";
    BOOST_CHECK(!test_is_password_correct(incorrect_password, encrypted));
}

BOOST_AUTO_TEST_CASE(test_base64)
{
    const char* const src_data[5] = {"", "a", "ab", "abc", "abcd"};
    for (int idx = 0; idx < 5; ++idx)
    {
        const int data_len = std::strlen(src_data[idx]) + 1;
        const auto data = std::shared_ptr<unsigned char>(new unsigned char[data_len], [](auto p) { delete[] p; });
        std::memcpy(data.get(), src_data[idx], data_len);
        const auto src = ::PasswordStorage::BinaryData::from_raw_binary_data(data, data_len);
        const ::PasswordStorage::Base64EncodedData base64_data(src);
        const ::PasswordStorage::BinaryData result(base64_data);
        BOOST_CHECK_EQUAL(src.get_size_of_raw_binary_data(), data_len);
        BOOST_CHECK_EQUAL(result.get_size_of_raw_binary_data(), data_len);
        if ((src.get_size_of_raw_binary_data() == data_len) &&
            (result.get_size_of_raw_binary_data() == data_len))
        {
            BOOST_CHECK_EQUAL(std::memcmp(src.get_raw_binary_data().get(), result.get_raw_binary_data().get(), data_len), 0);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()//Tests/Util/PasswordStorage
BOOST_AUTO_TEST_SUITE_END()//Tests/Util
BOOST_AUTO_TEST_SUITE_END()//Tests
