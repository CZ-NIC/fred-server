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
#ifndef TEST_COMMON_THREADED_HH_1E18B1560B43967007BA2BEA7C019EBA//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define TEST_COMMON_THREADED_HH_1E18B1560B43967007BA2BEA7C019EBA

#include "test/setup/tests_common.hh"
#include "src/util/concurrent_queue.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"

#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>

/*
 * General worker for threaded test:
 *   run() method must be defined
 *  template parameters:
 *    R - what the worker (run() method) returns
 *    P - parametres for the worker
 *
 *  P, result queue reference and other
 *  important objects are declared protected in the class
 */
template <typename R, typename P>
class ThreadedTestWorker
{
public:
    using ThreadedTestResultQueue = concurrent_queue<R>;
    using ParamsType = P;
    using ResultType = R;

    ThreadedTestWorker(
            unsigned number,
            boost::barrier* sb,
            std::size_t thread_group_divisor,
            ThreadedTestResultQueue* result_queue_ptr,
            P p)
        : number_(number),
          sb_(sb),
          tgd_(thread_group_divisor),
          results(result_queue_ptr),
          params(p)
     {}

     void operator()()
     {
        try
        {
            if (number_ % tgd_)//if synchronized thread
            {
                if (sb_ != nullptr)
                {
                    sb_->wait();//wait for other synced threads
                }
            }

            const R res = run(params);

            if (results != nullptr)
            {
                results->guarded_access().push(res);
            }

        }
        catch (const std::exception & e)
        {
            THREAD_BOOST_ERROR(std::string("Exception caught in worker: ") + e.what());
        }
        catch (...)
        {
            THREAD_BOOST_TEST_MESSAGE(std::string("Unknown exception in operator(), thread number: ") +
                                      boost::lexical_cast<std::string>(number_));
            return;
        }
     }
     virtual R run(const P &p) = 0;
protected:
    unsigned number_;
    boost::barrier *sb_;
    std::size_t tgd_;
    ThreadedTestResultQueue* results;
    P params;
};


template <typename W>
unsigned threadedTest(
        const typename W::ParamsType &params,
        void (*checker_func)(const typename W::ResultType &p))
{
    using ResultQueue = typename W::ThreadedTestResultQueue;

    const HandleThreadGroupArgs* const thread_args_ptr =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleThreadGroupArgs>();

    const std::size_t number_of_threads = thread_args_ptr->number_of_threads;
    const std::size_t thread_group_divisor = thread_args_ptr->thread_group_divisor;
    // int(number_of_threads - (number_of_threads % thread_group_divisor ? 1 : 0)
    // - number_of_threads / thread_group_divisor) is number of synced threads

    ResultQueue result_queue;
    // TODO
    // ThreadedTestResultQueue result_queue;

    //vector of thread functors
    std::vector<W> tw_vector;
    tw_vector.reserve(number_of_threads);

    unsigned barriers_number = number_of_threads - ((number_of_threads % thread_group_divisor) == 0 ? 0 : 1) -
                                               (number_of_threads / thread_group_divisor);

    if (barriers_number < 1)
    {
        barriers_number = 1;
    }

    BOOST_TEST_MESSAGE("thread barriers:: " + boost::lexical_cast<std::string>(barriers_number));

    //synchronization barriers instance
    boost::barrier sb(barriers_number);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < number_of_threads; ++i)
    {
        tw_vector.push_back(W(
                i,
                &sb,
                thread_group_divisor, &result_queue, params));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE("threads end result_queue.size(): " +
                       boost::lexical_cast<std::string>(result_queue.unguarded_access().size()));

    while (!result_queue.unguarded_access().empty())
    {
        const auto thread_result = result_queue.unguarded_access().pop();
        checker_func(thread_result);
    }

    return number_of_threads;
}

#endif//TEST_COMMON_THREADED_HH_1E18B1560B43967007BA2BEA7C019EBA
