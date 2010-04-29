/*
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE Test model
//not using UTF defined main
#define BOOST_TEST_NO_MAIN


#include "random_data_generator.h"
#include "faked_args.h"
#include "concurrent_queue.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>
#include <queue>
#include <sys/time.h>
#include <time.h>


// Sun CC doesn't handle boost::iterator_adaptor yet
#if !defined(__SUNPRO_CC) || (__SUNPRO_CC > 0x530)
#include <boost/generator_iterator.hpp>
#endif

#ifdef BOOST_NO_STDC_NAMESPACE
namespace std
{
  using ::time;
}
#endif


//compose args processing
class CmdLineArgHandlers
{
    //nonowning container of handlers
    typedef std::vector<HandleArgs*> HandlerVector;
    HandlerVector handler;
public:
    HandleGeneralArgs general_args;
    HandleDatabaseArgs database_args;
    HandleThreadGroupArgs thread_group_args;

    CmdLineArgHandlers()
    {
        //order of arguments processing
        handler.push_back(&general_args);
        handler.push_back(&database_args);
        handler.push_back(&thread_group_args);

        //gater options_descriptions for help print
        for(HandlerVector::iterator i = handler.begin(); i != handler.end(); ++i )
            general_args.po_description.push_back((*i)->get_options_description());
    }

    FakedArgs handle( int argc, char* argv[])
    {
        FakedArgs fa;

        //initial fa
        fa.prealocate_for_argc(argc);
        for (int i = 0; i < argc ; ++i)
            fa.add_argv(argv[i]);

        for(HandlerVector::iterator i = handler.begin(); i != handler.end(); ++i )
        {
            FakedArgs fa_out;
            (*i)->handle( fa.get_argc(), fa.get_argv(), fa_out);
            //chaining output to input
            fa.clear();
            fa.prealocate_for_argc(fa_out.get_argc());
            for(int i = 0; i < fa_out.get_argc(); ++i)
            {
                fa.add_argv( fa_out.get_argv()[i] );
            }
        }
        return fa;
    }

}cmdlinehandlers;

#include "test-model.h"

    /*
     * to log queries:
     *  in /etc/postgresql/8.4/main/postgresql.conf set:
     *
     *  log_min_duration_statement = 0
     *  log_duration = off
     *  log_statement = 'none'
     *
     *  lower shared_buffers ~2MB
     *  set max_connections > 300 ~400
     *
     * postgres restart
     *
     * */



//synchronization using barriers
struct sync_barriers
{
    boost::barrier insert_barrier;
    boost::barrier reload_barrier;
    boost::barrier update_barrier;
    sync_barriers(std::size_t thread_number)
        : insert_barrier(thread_number)
        , reload_barrier(thread_number)
        , update_barrier(thread_number)
    {}
};

struct Result
{
    unsigned number;//thread number
    unsigned ret;//return code
    std::string desc;//some closer description
    Result()
    : number(0)
      , ret(std::numeric_limits<unsigned>::max())
      , desc("empty result")
      {}
};

typedef concurrent_queue<Result> ResultQueue;

//thread functor
class ModelBankPaymentThreadWorker
{
public:

    ModelBankPaymentThreadWorker(unsigned number,unsigned sleep_time, sync_barriers* sb_ptr, std::size_t thread_group_divisor, ResultQueue* result_queue_ptr = 0, unsigned seed = 0)
            : number_(number)
            , sleep_time_(sleep_time)
            , sb_ptr_(sb_ptr)
            , rdg_(seed)
            , tgd_(thread_group_divisor)
            , rsq_ptr (result_queue_ptr)
    {}

    void operator()()
    {
        try
        {

            ModelBankPayment mbp1, mbp2;

            if(number_%tgd_)//if synchronized thread
            {
                std::cout << "waiting: " << number_ << std::endl;
                if(sb_ptr_)
                    sb_ptr_->insert_barrier.wait();//wait for other synced threads
            }
            else
            {//non-synchronized thread
                std::cout << "NOwaiting: " << number_ << std::endl;
            }

            //std::cout << "start: " << number_ << std::endl;
            mbp_insert_data insert_data;//mostly randomly generated data
            insert_data.statement_id=0;//fk bank_statement (id) - none
            insert_data.account_id=rdg_.xnum1_6(); //fk bank_account (id) - num 1-6
            insert_data.invoice_id=0; //fk invoice (id) - none
            insert_data.account_number=rdg_.xnumstring(17);//17 numletters
            insert_data.bank_code=rdg_.xnumstring(4);//4 numletters
            insert_data.operation_code=rdg_.xnum1_5(); // num 1-5
            insert_data.transfer_type=rdg_.xnum1_5(); // num 1-5
            insert_data.payment_status=rdg_.xnum1_6();// num 1-6
            insert_data.konstsym=rdg_.xnumstring(10);// 10 numletters
            insert_data.varsymb=rdg_.xnumstring(10);// 10 numletters
            insert_data.specsymb=rdg_.xnumstring(10);// 10 numletters
            insert_data.price=rdg_.xint();//int
            insert_data.account_evid=rdg_.xnumstring(20);//20 numletters
            insert_data.account_date=rdg_.xdate(); //some date
            insert_data.account_memo=rdg_.xstring(64); //64 chars
            insert_data.account_name=rdg_.xstring(64); //64 chars
            insert_data.crtime=rdg_.xptime();//timestamp

            Result res;
            res.ret = 0;
            res.number = number_;

            if((res.ret = mbp_insert_test(mbp1, insert_data))!=0)
            {
                res.desc = std::string("error in: mbp_insert_test using seed: ")
                    + boost::lexical_cast<std::string>(rdg_.get_seed());
                if(rsq_ptr) rsq_ptr->push(res);
                return;
            }

            if(number_%tgd_)
                if(sb_ptr_)
                    sb_ptr_->reload_barrier.wait();//wait for other synced threads
            if((res.ret = mbp_reload_test(mbp1, mbp2))!=0)
            {
                res.desc = std::string("error in: mbp_reload_test using seed: ")
                    + boost::lexical_cast<std::string>(rdg_.get_seed());
                if(rsq_ptr) rsq_ptr->push(res);
                return;
            }

            if(number_%tgd_)
                if(sb_ptr_)
                    sb_ptr_->update_barrier.wait();//wait for other synced threads
            if((res.ret = mbp_update_test(mbp1, mbp2))!=0)
            {
                res.desc = std::string("error in: mbp_update_test using seed: ")
                    + boost::lexical_cast<std::string>(rdg_.get_seed());
                if(rsq_ptr) rsq_ptr->push(res);
                return;
            }
            res.desc = std::string("ok");
            if(rsq_ptr) rsq_ptr->push(res);
           // std::cout << "end: " << number_ << std::endl;
        }
        catch(...)
        {
            std::cout << "exception in operator() thread number: " << number_ << std::endl;
        }
    }

private:
    //need only defaultly constructible members here
    unsigned    number_;//thred identification
    unsigned    sleep_time_;//[s]
    sync_barriers* sb_ptr_;
    RandomDataGenerator rdg_;
    std::size_t tgd_;//thread group divisor
    ResultQueue* rsq_ptr; //result queue non-owning pointer
};

BOOST_AUTO_TEST_CASE( test_model_files )
{
    BOOST_REQUIRE_EQUAL(model_insert_test() , 0);
    BOOST_REQUIRE_EQUAL(model_reload_test() , 0);
    BOOST_REQUIRE_EQUAL(model_update_test() , 0);
    BOOST_REQUIRE_EXCEPTION( model_nodatareload_test()
            , std::exception , check_std_exception_nodatafound);
    BOOST_REQUIRE_EQUAL(model_nodataupdate_test() , 0);
}



BOOST_AUTO_TEST_CASE( test_model_bank_payments_threaded )
{
    std::size_t const thread_number = cmdlinehandlers.thread_group_args.thread_number;
    std::size_t const thread_group_divisor = cmdlinehandlers.thread_group_args.thread_group_divisor;
    // int(thread_number - (thread_number % thread_group_divisor ? 1 : 0)
    // - thread_number / thread_group_divisor) is number of synced threads

    ResultQueue result_queue;

    //vector of thread functors
    std::vector<ModelBankPaymentThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    std::cout << "thread barriers:: "
            <<  (thread_number - (thread_number % thread_group_divisor ? 1 : 0)
                    - thread_number/thread_group_divisor)
            << std::endl;

    //synchronization barriers instance
    sync_barriers sb(thread_number - (thread_number % thread_group_divisor ? 1 : 0)
            - thread_number/thread_group_divisor);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(ModelBankPaymentThreadWorker(i,3,&sb, thread_group_divisor, &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    std::cout << "threads end result_queue.size(): " << result_queue.size() << std::endl;

    for(unsigned i = 0; i < thread_number; ++i)
    {
        Result thread_result;
        result_queue.try_pop(thread_result);

        BOOST_REQUIRE_EQUAL(thread_result.ret , 0);

        if(thread_result.ret)
        {
            std::cout << thread_result.desc
                    << " thread number: " << thread_result.number
                    << " return code: " << thread_result.ret
                    << " description: " << thread_result.desc
                    << std::endl;
        }
    }//for i
}

int main( int argc, char* argv[] )
{
    //processing of additional program options
    //producing faked args with unrecognized ones
    FakedArgs fa;
    try
    {
        fa = cmdlinehandlers.handle(argc, argv);
    }
    catch(const ReturnFromMain&)
    {
        return 0;
    }

//fn init_unit_test_suite added in 1.35.0
#if ( BOOST_VERSION > 103401 )

    // prototype for user's unit test init function
#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
    extern bool init_unit_test();

    boost::unit_test::init_unit_test_func init_func = &init_unit_test;
#else
    extern ::boost::unit_test::test_suite* init_unit_test_suite( int argc, char* argv[] );

    boost::unit_test::init_unit_test_func init_func = &init_unit_test_suite;
#endif

    return ::boost::unit_test::unit_test_main( init_func, fa.get_argc(), fa.get_argv() );//using fake args
#else //1.34.1 and older
    return ::boost::unit_test::unit_test_main(  fa.get_argc(), fa.get_argv() );//using fake args
#endif //1.35.0 and newer

}

