
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>

#include "random.h"
#include "requests/session_cache.h"

using namespace Fred::Logger;


void add_sequence(SessionCache &sc, unsigned count) 
{
    for (unsigned i=1; i<=count; i++) {
        boost::shared_ptr<ModelSession> session(new ModelSession);
        session->setUserName("name");
        session->setUserId(i);

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
            boost::shared_ptr<ModelSession> s = sc.get(i);
        } catch(CACHE_MISS) {
            BOOST_FAIL("Cache miss occured when not allowed");
        } 
    }
}

bool verify_item(SessionCache &sc, Database::ID id)
{
    try {
        boost::shared_ptr<ModelSession> s = sc.get(id);
    } catch(CACHE_MISS) {
        return false;
    }
    return true;
}

void verify_items_miss_sequence(SessionCache &sc, unsigned count)
{
    for (unsigned i=1; i<= count; i++) {
        bool exception = false;
        try {
            boost::shared_ptr<ModelSession> s = sc.get(i);
        } catch(CACHE_MISS) {
            exception = true;
        }
        
        if(!exception) {
            BOOST_FAIL("Item present but it should not be");
        }
    }
}

class ThreadWorker {
public:
ThreadWorker(unsigned n,
                boost::barrier* sb,
               std::size_t thread_group_divisor,
               SessionCache &sc,
               unsigned maxid,
               seconds t) :
       number(n),
       sb_ptr(sb),
       divisor(thread_group_divisor),
       scache(sc),
       max_id(maxid),
       ttl(t) { };

    void operator()() {
        if(number % divisor)//if synchronized thread
        {
            //std::cout << "waiting: " << number_ << std::endl;
            if(sb_ptr)
            sb_ptr->wait();//wait for other synced threads
        }
        else
        {//non-synchronized thread
            //std::cout << "NOwaiting: " << number_ << std::endl;
        }

        worker();
    }

    virtual void worker()
    {
        BOOST_FAIL("This should NEVER be run.");
    }


private:
    int number;
    boost::barrier* sb_ptr;
    int divisor;

protected:
    SessionCache &scache;
    unsigned max_id;
    seconds ttl;
};



/** This is threaded add test for version with TTL
 *   - the cache might not accept the item but once
 *   it accepts it, it MUST keep it until TTL expires,
 */
class WorkerAdd : public ThreadWorker {
public:
    WorkerAdd(unsigned n,
                    boost::barrier* sb,
                   std::size_t thread_group_divisor,
                   SessionCache &sc,
                   unsigned maxid,
                   seconds t) :
                       ThreadWorker(n, sb, thread_group_divisor, sc, maxid, t)
    { }

    void worker()
    {
        boost::shared_ptr<ModelSession> n(new ModelSession());

        unsigned id = Random::integer(1, max_id);
        n->setId(id);
        n->setUserId(id);
        scache.add(id, n);

    }
};

/** This is threaded add test for version without TTL
 *  - this verison always accepts new item because
 *  it can always successfully perform garbage collection
 *  to make space for the new data

void worker_add(SessionCache &sc, unsigned max_id, seconds ttl)
{
    boost::shared_ptr<ModelSession> n(new ModelSession());

    unsigned id = Random::integer(1, max_id);
    n->setId(id)
    sc.add(id, n);

    BOOST_CHECK(verify_item(sc, id));
}
*/

class WorkerRemove : public ThreadWorker {
public:
    WorkerRemove(unsigned n,
        boost::barrier* sb,
        std::size_t thread_group_divisor,
        SessionCache &sc,
        unsigned maxid,
        seconds t) :
           ThreadWorker(n, sb, thread_group_divisor, sc, maxid, t)
    { }
    void worker()
    {
        unsigned id = Random::integer(1, max_id);

        scache.remove(id);
    }
};

typedef unsigned threaded_sequence;

class WorkerComplete : public ThreadWorker {
public:
    WorkerComplete(unsigned n,
        boost::barrier* sb,
        std::size_t thread_group_divisor,
        SessionCache &sc,
        unsigned maxid,
        seconds t,
        threaded_sequence &tsq,
        threaded_sequence &refused) :
           ThreadWorker(n, sb, thread_group_divisor, sc, maxid, t),
           id_sequence(tsq),
           refused_count(refused)
    { }

protected:
    threaded_sequence &id_sequence;
    threaded_sequence &refused_count;
};

class WorkerCompleteAdd : public WorkerComplete {
public:
    WorkerCompleteAdd(unsigned n,boost::barrier* sb,std::size_t thread_group_divisor,SessionCache &sc,
            unsigned maxid,seconds t,threaded_sequence &tsq,threaded_sequence &rc) :
           WorkerComplete(n, sb, thread_group_divisor, sc, maxid, t, tsq, rc)
    { }
    virtual void worker()
    {
        boost::shared_ptr<ModelSession> n(new ModelSession());

        unsigned id = id_sequence++;
        BOOST_TEST_MESSAGE( boost::format("Got new id: %1%") % id_sequence);
        n->setId(id);
        n->setUserId(id);
        scache.add(id, n);

        if (!verify_item(scache, id)) {
            refused_count++;
            return;
        }
        // if the cache didn't save it, bail out

        // TODO that 1 second should be some const
        if( ttl-seconds(1) > seconds(0) ) {
            sleep ((ttl-seconds(1)).total_seconds());
        }
        // now it should still be present
        BOOST_CHECK(verify_item(scache, id));
    }
};

class WorkerCompleteRemove : public WorkerComplete {
public:
    WorkerCompleteRemove(unsigned n, boost::barrier* sb,std::size_t thread_group_divisor,SessionCache &sc,
            unsigned maxid,seconds t,threaded_sequence &tsq,threaded_sequence &rc) :
           WorkerComplete(n, sb, thread_group_divisor, sc, maxid, t, tsq, rc)
    { }
    virtual void worker()
    {
        boost::shared_ptr<ModelSession> n(new ModelSession());

        unsigned id = id_sequence++;
        BOOST_TEST_MESSAGE( boost::format("Got new id: %1%") % id_sequence);
        n->setId(id);
        n->setUserId(id);
        scache.add(id, n);

        if (!verify_item(scache, id)) {
            refused_count++;
            return;
        }
        // if the cache didn't save it, bail out

        // don't remove the records immediately - that would uncontrolably make
        // space for other records - if they can't fit, make it happen
        if(ttl > seconds(1)) {
            sleep(1);
            // it should still be there
            BOOST_CHECK(verify_item(scache, id));
        }

        scache.remove(id);

        // now it should not be present - id was unique
        BOOST_CHECK(!verify_item(scache, id));
    }
};



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
    boost::shared_ptr<ModelSession> session(new ModelSession());
    session->setUserName("garbage trigger");
    session->setUserId(1002);
    sc.add(1002, session);

    // all items should be garbaged now
    verify_items_miss_sequence(sc, 1001);
    // this one should be still present
    BOOST_CHECK(verify_item(sc, 1002));

}

BOOST_AUTO_TEST_CASE( test_garbage_some )
{
    SessionCache sc(1000, 3);

    add_sequence(sc, 1001);
    verify_items_sequence(sc, 1001);

    // now there's 1 item over capacity, we're just
    // before garbage collection attempt

    sleep(4);

    BOOST_CHECK(verify_item(sc, 225));
    BOOST_CHECK(verify_item(sc, 768));

    // this should trigger garbage collection
    boost::shared_ptr<ModelSession> session(new ModelSession());
    session->setUserName("garbage trigger");
    session->setUserId(1002);
    sc.add(1002, session);

    // these should be still present
    BOOST_CHECK(verify_item(sc, 225));
    BOOST_CHECK(verify_item(sc, 768));
    BOOST_CHECK(verify_item(sc, 1002));

    // all items should be garbaged now
    verify_items_miss_sequence(sc, 224);

}




BOOST_AUTO_TEST_CASE( cache_threaded_test )
{

    unsigned thread_number = 300;
    unsigned thread_group_divisor = 30;
    boost::barrier sb(thread_number - (thread_number % thread_group_divisor ? 1 : 0)
              - thread_number/thread_group_divisor);

    const unsigned ttl = 5;
    SessionCache scache(5, ttl);
    const unsigned max_id = 10;

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        try {
            if(!(i%3)) {
                threads.create_thread(WorkerRemove(i,&sb, thread_group_divisor, scache, max_id, seconds(ttl)));
            } else {
                threads.create_thread(WorkerAdd(i,&sb, thread_group_divisor, scache, max_id, seconds(ttl)));
            }
        } catch(std::exception &e) {
            BOOST_FAIL(e.what());
        }
    }

    threads.join_all();
}


BOOST_AUTO_TEST_CASE( cache_ultimate_threaded_test )
{

    const unsigned thread_number = 300;
    const unsigned cache_capacity = 250;
    unsigned thread_group_divisor = 30;
    boost::barrier sb(thread_number - (thread_number % thread_group_divisor ? 1 : 0)
              - thread_number/thread_group_divisor);

    const unsigned ttl = 5;
    SessionCache scache(cache_capacity, ttl);
    const unsigned max_id = 10;

    threaded_sequence id_seq(1);
    threaded_sequence refused_count(0);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        try {
            if(!(i%2)) {
                threads.create_thread(WorkerCompleteRemove(
                        i,&sb, thread_group_divisor, scache, max_id, seconds(ttl), id_seq, refused_count));
            } else {
                threads.create_thread(WorkerCompleteAdd(
                        i,&sb, thread_group_divisor, scache, max_id, seconds(ttl), id_seq, refused_count));
            }
        } catch(std::exception &e) {
            BOOST_FAIL(e.what());
        }
    }

    threads.join_all();
    BOOST_TEST_MESSAGE( boost::format("refused_count: %1%") % refused_count);

    // there is always 1 element above the limit
    BOOST_CHECK(refused_count + cache_capacity + 1 <= thread_number);
}


/*
BOOST_AUTO_TEST_CASE( test_garbage2 )

BOOST_AUTO_TEST_CASE( advanced_garbage )
*/
    

BOOST_AUTO_TEST_SUITE_END();
