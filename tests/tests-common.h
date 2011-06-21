#ifndef _TESTS_COMMON_H_
#define _TESTS_COMMON_H_

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

#endif //_TESTS_COMMON_H_
