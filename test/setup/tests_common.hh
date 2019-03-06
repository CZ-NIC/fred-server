/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#ifndef TESTS_COMMON_HH_A264CF6CAEE049C09B30361B0EB1F176
#define TESTS_COMMON_HH_A264CF6CAEE049C09B30361B0EB1F176

#include <boost/thread/mutex.hpp>
#include <boost/test/unit_test.hpp>

extern boost::mutex boost_test_mutex;

#define THREAD_BOOST_CHECK( P ) do {                        \
    boost::mutex::scoped_lock lock_guard(boost_test_mutex); \
    BOOST_CHECK( P );                                       \
} while (0)


#define THREAD_BOOST_CHECK_MESSAGE( P, M ) do {             \
    boost::mutex::scoped_lock lock_guard(boost_test_mutex); \
    BOOST_CHECK_MESSAGE( P, M );                            \
} while (0)


#define THREAD_BOOST_ERROR( M ) do {                        \
    boost::mutex::scoped_lock lock_guard(boost_test_mutex); \
    BOOST_ERROR( M );                                       \
} while (0)


#define THREAD_BOOST_TEST_MESSAGE( M ) do {                        \
    boost::mutex::scoped_lock lock_guard(boost_test_mutex);        \
    BOOST_TEST_MESSAGE( M );                                       \
} while (0)

#endif
