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
