
#include <vector>

#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/lexical_cast.hpp>
#include "tests-common.h"
#include "concurrent_queue.h"
#include "cfg/handle_threadgroup_args.h"

/* 
 * General worker for threaded test:
 *   run() method must be defined
 *  template parameters:
 *    RESULT - what the worker (run() method) returns
 *    PARAMS - parametres for the worker
 *
 *  PARAMS, result queue reference and other
 *  important objects are declared protected in the class
 */
template <typename RESULT, typename PARAMS> class ThreadedTestWorker {

public:
    typedef concurrent_queue<RESULT> ThreadedTestResultQueue;
    typedef PARAMS PARAMS_TYPE;
    typedef RESULT RESULT_TYPE;


    ThreadedTestWorker(unsigned number
             , boost::barrier* sb
             , std::size_t thread_group_divisor
             , ThreadedTestResultQueue* result_queue_ptr
             , PARAMS p
                    )

             : number_(number)
             , sb_(sb)
             , tgd_(thread_group_divisor)
             , results(result_queue_ptr)
             , params(p)

     {}

     void operator()()
     {
        try
        {
            if(number_ % tgd_)//if synchronized thread
            {
                if(sb_ != NULL) {
                    sb_->wait();//wait for other synced threads
                }
            }

            RESULT res = run(params);

            if(results != NULL)  {
                results->push(res);
            }

        } catch(const std::exception & ex) {
            THREAD_BOOST_ERROR( std::string("Exception caught in worker: ") + ex.what() );
        }
        catch(...)
        {
            THREAD_BOOST_TEST_MESSAGE( std::string("Unknown exception in operator(), thread number: ") 
		+ boost::lexical_cast<std::string>(number_) );
            return;
        }
     }

     virtual RESULT run(const PARAMS &p) = 0;

protected:
    unsigned number_;

    boost::barrier *sb_;
    std::size_t tgd_;
    ThreadedTestResultQueue *results;
    PARAMS params;
};


template <typename WORKER>
    unsigned threadedTest (
            const typename WORKER::PARAMS_TYPE &params,
            void (*checker_func)(const typename WORKER::RESULT_TYPE &p)
            )
{
    typedef typename WORKER::ThreadedTestResultQueue RESULT_QUEUE;

    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                       get_handler_ptr_by_type<HandleThreadGroupArgs>();

    std::size_t const thread_number = thread_args_ptr->thread_number;
    std::size_t const thread_group_divisor = thread_args_ptr->thread_group_divisor;
    // int(thread_number - (thread_number % thread_group_divisor ? 1 : 0)
    // - thread_number / thread_group_divisor) is number of synced threads

    RESULT_QUEUE result_queue;
    // TODO
    // ThreadedTestResultQueue result_queue;

    //vector of thread functors
    std::vector<WORKER> tw_vector;
    tw_vector.reserve(thread_number);

    unsigned barriers_number =  thread_number
            - (thread_number % thread_group_divisor ? 1 : 0)
            - thread_number/thread_group_divisor;

    if(barriers_number < 1) {
        barriers_number = 1;
    }

    BOOST_TEST_MESSAGE( std::string("thread barriers:: ")
            + boost::lexical_cast<std::string>(barriers_number)
            );

    //synchronization barriers instance
    boost::barrier sb(barriers_number);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(WORKER(i,&sb
                , thread_group_divisor, &result_queue, params));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE( std::string("threads end result_queue.size(): ") 
	+ boost::lexical_cast<std::string> (result_queue.size()) );

    for(unsigned i = 0; i < thread_number; ++i)
    {
        typename WORKER::RESULT_TYPE thread_result;
        if(!result_queue.try_pop(thread_result)) {
            continue;
        }

        checker_func(thread_result);
    }//for i

    return thread_number;
}
