
#include <boost/test/unit_test.hpp>
#include "requests/session_cache.h"


using namespace Fred::Logger;

void add_sequence(SessionCache &sc, unsigned count) 
{
    for (unsigned i=1; i<=count; i++) {
        ModelSession session;
        session.setUserName("name");
        session.setUserId(i);

        sc.add(i, session);

    }
}

void remove_items_sequence(SessionCache &sc, unsigned count)
{
    for (unsigned i=1; i<= count; i++) {
        sc.remove(i);
    }
}


void verify_items_sequence(SessionCache &sc, unsigned count)
{
    for (unsigned i=1; i<= count; i++) {
        try {
            ModelSession s = sc.get(i);
        } catch(CACHE_MISS) {
            BOOST_FAIL("Cache miss occured when not allowed");
        } 
    }
}

void verify_item(SessionCache &sc, Database::ID id)
{
    try {
        ModelSession s = sc.get(id);
    } catch(CACHE_MISS) {
        BOOST_FAIL("Item not present in cache");
    }
}

void verify_items_miss_sequence(SessionCache &sc, unsigned count)
{
    for (unsigned i=1; i<= count; i++) {
        bool exception = false;
        try {
            ModelSession s = sc.get(i);
        } catch(CACHE_MISS) {
            exception = true;
        } 
        
        if(!exception) {
            BOOST_FAIL("Item present but it should not be");
        }
    }
}


BOOST_AUTO_TEST_SUITE(TestCache)

BOOST_AUTO_TEST_CASE( test_add )
{ 
    SessionCache sc(100, 3600);

    add_sequence(sc, 100);

    verify_items_sequence(sc, 100);
    // it should still be there
    verify_items_sequence(sc, 100);
}

BOOST_AUTO_TEST_CASE( test_remove )
{
    SessionCache sc(10, 300);

    add_sequence(sc, 8);    

    // this should pass 
    sc.remove(123);
    sc.remove(11);
    sc.remove(2348023);
        
    // no item was removed so far 
    verify_items_sequence(sc, 8);

    remove_items_sequence(sc, 8);

    // make sure items are really not there
    verify_items_miss_sequence(sc, 8);
}

BOOST_AUTO_TEST_CASE( test_garbage_all )
{
    SessionCache sc(1000, 3);

    add_sequence(sc, 1001);
    verify_items_sequence(sc, 1001);
    // now there's 1 item over capacity, we're just 
    // before garbage collection attempt 

    sleep(4); 

    // this should trigger garbage collection
    ModelSession session;
    session.setUserName("garbage trigger");
    session.setUserId(1002);
    sc.add(1002, session);

    // all items should be garbaged now 
    verify_items_miss_sequence(sc, 1001);
    // this one should be still present
    verify_item(sc, 1002);

}

BOOST_AUTO_TEST_CASE( test_garbage_some )
{
    SessionCache sc(1000, 3);

    add_sequence(sc, 1001);
    verify_items_sequence(sc, 1001);

    // now there's 1 item over capacity, we're just 
    // before garbage collection attempt 

    sleep(4); 

    verify_item(sc, 225);
    verify_item(sc, 768);

    // this should trigger garbage collection
    ModelSession session;
    session.setUserName("garbage trigger");
    session.setUserId(1002);
    sc.add(1002, session);

    // these should be still present
    verify_item(sc, 225);
    verify_item(sc, 768);
    verify_item(sc, 1002);

    // all items should be garbaged now 
    verify_items_miss_sequence(sc, 224);

}


/*
BOOST_AUTO_TEST_CASE( test_garbage2 )
BOOST_AUTO_TEST_CASE( threaded_test )
BOOST_AUTO_TEST_CASE( advanced_garbage )
*/
    

BOOST_AUTO_TEST_SUITE_END();
