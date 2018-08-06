
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>

#include "src/util/random.hh"
#include "src/libfred/requests/session_cache.hh"
#include "test/setup/tests_common.hh"

using namespace ::LibFred::Logger;


void add_sequence(SessionCache &sc, unsigned count)
{
    for (unsigned i=1; i<=count; i++) {
        std::shared_ptr<ModelSession> session(new ModelSession);
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
            std::shared_ptr<ModelSession> s = sc.get(i);
        } catch(CACHE_MISS) {
            THREAD_BOOST_ERROR("Cache miss occured when not allowed");
        }
    }
}

bool verify_item(SessionCache &sc, Database::ID id)
{
    try {
        std::shared_ptr<ModelSession> s = sc.get(id);
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
            std::shared_ptr<ModelSession> s = sc.get(i);
        } catch(CACHE_MISS) {
            exception = true;
        }

        if(!exception) {
            THREAD_BOOST_ERROR("Item present but it should not be");
        }
    }
}

template<class T>
class threaded_sequence {
public:
    explicit threaded_sequence(T init = 1) : seq(init) { };

    operator T() {
        boost::mutex::scoped_lock lock(m);
        return seq;
    }

    T operator ++ () {
        boost::mutex::scoped_lock lock(m);
        ++seq;
        return seq;
    }
private:
    T seq;
    boost::mutex m;
};

typedef threaded_sequence<unsigned> sequence;
//typedef unsigned threaded_sequence;

template<class T>
class ThreadWorkerFunctor {
public:
ThreadWorkerFunctor(unsigned n,
                boost::barrier &sb,
               std::size_t thread_group_divisor,
               T &sc,
               unsigned maxid,
               seconds t) :
       sb_ptr(sb),
       divisor(thread_group_divisor),
       number(n),
       scache(sc),
       max_id(maxid),
       ttl(t) { };

    void operator()() {
        if(number % divisor)//if synchronized thread
        {
            sb_ptr.wait();
        } else {//non-synchronized thread
            //std::cout << "NOwaiting: " << number_ << std::endl;
        }

        worker(typename T::cache_type());
    }

    virtual void worker(cache_general_type_tag) = 0;
    virtual void worker(cache_ttl_type_tag) = 0;
    virtual void worker(cache_oldrec_type_tag) = 0;

private:
    boost::barrier &sb_ptr;
    int divisor;

protected:
    int number;
    T &scache;
    unsigned max_id;
    seconds ttl;
};

typedef ThreadWorkerFunctor<SessionCache> ThreadWorker;

/** This is threaded add test for version with TTL
 *   - the cache might not accept the item but once
 *   it accepts it, it MUST keep it until TTL expires,
 */
class WorkerSimple : public ThreadWorker {
public:
    WorkerSimple(unsigned n,
                    boost::barrier &sb,
                   std::size_t thread_group_divisor,
                   SessionCache &sc,
                   unsigned maxid,
                   seconds t) :
                       ThreadWorker(n, sb, thread_group_divisor, sc, maxid, t)
    { }

    virtual void worker(cache_general_type_tag)
    {
        if (number%3) {
            std::shared_ptr<ModelSession> n(new ModelSession());

            unsigned id = Random::integer(1, max_id);
            n->setId(id);
            n->setUserId(id);
            scache.add(id, n);
        } else {
            unsigned id = Random::integer(1, max_id);

            scache.remove(id);
        }
    }
    virtual void worker(cache_ttl_type_tag) {
        worker(cache_general_type_tag());
    }
    virtual void worker(cache_oldrec_type_tag) {
        worker(cache_general_type_tag());
    }
};

/** This is threaded add test for version without TTL
 *  - this verison always accepts new item because
 *  it can always successfully perform garbage collection
 *  to make space for the new data

void worker_add(SessionCache &sc, unsigned max_id, seconds ttl)
{
    std::shared_ptr<ModelSession> n(new ModelSession());

    unsigned id = Random::integer(1, max_id);
    n->setId(id)
    sc.add(id, n);

    BOOST_CHECK(verify_item(sc, id));
}
*/


class WorkerComplete : public ThreadWorker {
public:
    WorkerComplete(unsigned n,
        boost::barrier &sb,
        std::size_t thread_group_divisor,
        SessionCache &sc,
        unsigned maxid,
        seconds t,
        sequence &tsq,
        sequence &refused,
        boost::mutex &pa)
         : ThreadWorker(n, sb, thread_group_divisor, sc, maxid, t),
           id_sequence(tsq),
           refused_count(refused),
           protect_added(pa)
    { }

    virtual void worker(cache_general_type_tag) {
        THREAD_BOOST_ERROR(" This should NEVER be run.");
    }

    virtual void worker(cache_ttl_type_tag)
    {
        if(number%2) {
            std::shared_ptr<ModelSession> n(new ModelSession());

            unsigned id = ++id_sequence;
            THREAD_BOOST_TEST_MESSAGE( boost::format("Got new id: %1%") % id);

            n->setId(id);
            n->setUserId(id);
            scache.add(id, n);

            if (!verify_item(scache, id)) {
                // if the cache didn't save it, bail out
                ++refused_count;
                return;
            }
            // TODO that 1 second should be some const
            if( ttl-seconds(1) > seconds(0) ) {
                sleep ((ttl-seconds(1)).total_seconds());
            }
            THREAD_BOOST_CHECK_MESSAGE(verify_item(scache, id), "Check if record is still present just before timeout...");
        } else {
            std::shared_ptr<ModelSession> n(new ModelSession());

            unsigned id = ++id_sequence;

            THREAD_BOOST_TEST_MESSAGE( boost::format("Got new id: %1%") % id);

            n->setId(id);
            n->setUserId(id);
            scache.add(id, n);

            if (!verify_item(scache, id)) {
                ++refused_count;
                return;
            }
            // if the cache didn't save it, bail out

            // don't remove the records immediately - that would uncontrolably make
            // space for other records - if they can't fit, make it happen
            if(ttl > seconds(1)) {
                sleep(1);
                THREAD_BOOST_CHECK(verify_item(scache, id));
            }

            scache.remove(id);

            // now it should not be present - id was unique
            THREAD_BOOST_CHECK(!verify_item(scache, id));
        }
    }

    virtual void worker(cache_oldrec_type_tag)
    {
        if(number%2) {
            std::shared_ptr<ModelSession> n(new ModelSession());

            unsigned id = ++id_sequence;
            THREAD_BOOST_TEST_MESSAGE( boost::format("Got new id: %1%") % id);
            n->setId(id);
            n->setUserId(id);

            // without lock there'd be no guarantee that it's not deleted in the meantime
            boost::mutex::scoped_lock lock(protect_added);
            scache.add(id, n);

            if (!verify_item(scache, id)) {
                THREAD_BOOST_ERROR("Record not saved in cache.");
            }
        } else {
            std::shared_ptr<ModelSession> n(new ModelSession());

            unsigned id = ++id_sequence;
            THREAD_BOOST_TEST_MESSAGE( boost::format("Got new id: %1%") % id);
            n->setId(id);
            n->setUserId(id);

            boost::mutex::scoped_lock lock(protect_added);
            scache.add(id, n);

            if (!verify_item(scache, id)) {
                THREAD_BOOST_ERROR("Record not saved in cache.");
                return;
            }
            scache.remove(id);

            // now it should not be present - id was unique
            THREAD_BOOST_CHECK(!verify_item(scache, id));
        }
    }

protected:
    sequence &id_sequence;
    sequence &refused_count;
    boost::mutex &protect_added;
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
    std::shared_ptr<ModelSession> session(new ModelSession());
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
    std::shared_ptr<ModelSession> session(new ModelSession());
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

BOOST_AUTO_TEST_CASE(cache_threaded_test)
{
    const unsigned number_of_threads = 300;
    const unsigned thread_group_divisor = 30;
    boost::barrier sb(number_of_threads - (number_of_threads % thread_group_divisor ? 1 : 0) -
                      (number_of_threads / thread_group_divisor));

    const unsigned ttl = 5;
    SessionCache scache(5, ttl);
    const unsigned max_id = 10;

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < number_of_threads; ++i)
    {
        try
        {
            threads.create_thread(WorkerSimple(i,sb, thread_group_divisor, scache, max_id, seconds(ttl)));
        }
        catch (const std::exception &e)
        {
            THREAD_BOOST_ERROR(e.what());
        }
    }

    threads.join_all();
}

BOOST_AUTO_TEST_CASE(cache_ultimate_threaded_test)
{

    const unsigned number_of_threads = 300;
    const unsigned cache_capacity = 250;
    unsigned thread_group_divisor = 30;
    boost::barrier sb(number_of_threads - (number_of_threads % thread_group_divisor ? 1 : 0) -
                      (number_of_threads / thread_group_divisor));

    const unsigned ttl = 5;
    SessionCache scache(cache_capacity, ttl);
    const unsigned max_id = 10;

    sequence id_seq(1);
    sequence refused_count(0);
    boost::mutex prot_added;

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < number_of_threads; ++i)
    {
        try
        {
            threads.create_thread(WorkerComplete(
                    i,
                    sb,
                    thread_group_divisor,
                    scache,
                    max_id,
                    seconds(ttl),
                    id_seq,
                    refused_count,
                    prot_added));
        }
        catch (const std::exception &e)
        {
            THREAD_BOOST_ERROR(e.what());
        }
    }

    threads.join_all();
    BOOST_TEST_MESSAGE(boost::format("refused_count: %1%") % static_cast<unsigned>(refused_count));

    // there is always 1 element above the limit
    BOOST_CHECK(1 + refused_count + cache_capacity <= number_of_threads);
}

BOOST_AUTO_TEST_SUITE_END();
