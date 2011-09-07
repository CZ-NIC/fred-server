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




    /*
     * to log queries:
     *  in /etc/postgresql/8.4/main/postgresql.conf set:
     *
     *  log_min_duration_statement = 0
     *  log_duration = off
     *  log_statement = 'none'
     *
     *  for test:
     *  lower shared_buffers ~2MB
     *  set max_connections > 300 ~400
     *
     * postgres restart
     * in server.conf
     * [database]
     * timeout = 20
     *
     * */




#include "test-model.h"

BOOST_AUTO_TEST_SUITE(ModelTest)

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

    ModelBankPaymentThreadWorker(unsigned number,unsigned sleep_time
            , sync_barriers* sb_ptr, std::size_t thread_group_divisor
            , ResultQueue* result_queue_ptr = 0, unsigned seed = 0)
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
                //std::cout << "waiting: " << number_ << std::endl;
                if(sb_ptr_)
                    sb_ptr_->insert_barrier.wait();//wait for other synced threads
            }
            else
            {//non-synchronized thread
                //std::cout << "NOwaiting: " << number_ << std::endl;
            }

            //std::cout << "start: " << number_ << std::endl;
            mbp_insert_data insert_data;//mostly randomly generated data
            insert_data.statement_id=0;//fk bank_statement (id) - none
            insert_data.account_id=rdg_.xnum1_6(); //fk bank_account (id) - num 1-6
            insert_data.account_number=rdg_.xnumstring(17);//17 numletters
            insert_data.bank_code=rdg_.xnumstring(4);//4 numletters
            insert_data.operation_code=rdg_.xnum1_5(); // num 1-5
            insert_data.transfer_type=rdg_.xnum1_5(); // num 1-5
            insert_data.payment_status=rdg_.xnum1_6();// num 1-6
            insert_data.konstsym=rdg_.xnumstring(10);// 10 numletters
            insert_data.varsymb=rdg_.xnumstring(10);// 10 numletters
            insert_data.specsymb=rdg_.xnumstring(10);// 10 numletters
            insert_data.price=Money(rdg_.xnumstring(8)+"."+rdg_.xnumstring(2));//8.2 numletters
            insert_data.account_evid=rdg_.xnumstring(20);//20 numletters
            insert_data.account_date=rdg_.xdate(); //some date
            insert_data.account_memo=rdg_.xstring(63); //63 chars
            insert_data.account_name=rdg_.xstring(63); //63 chars
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
            BOOST_TEST_MESSAGE( "exception in operator() thread number: " << number_ );
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
    BOOST_CHECK_EQUAL(model_insert_test() , 0);
    BOOST_CHECK_EQUAL(model_reload_test() , 0);
    BOOST_CHECK_EQUAL(model_update_test() , 0);
    BOOST_CHECK_EXCEPTION( model_nodatareload_test()
            , std::exception , check_std_exception_nodatafound);
    BOOST_CHECK_EQUAL(model_nodataupdate_test() , 0);
}



BOOST_AUTO_TEST_CASE( test_model_bank_payments_threaded )
{
    HandleThreadGroupArgs* thread_args_ptr=CfgArgs::instance()->
                   get_handler_ptr_by_type<HandleThreadGroupArgs>();

    std::size_t const thread_number = thread_args_ptr->thread_number;
    std::size_t const thread_group_divisor = thread_args_ptr->thread_group_divisor;
    // int(thread_number - (thread_number % thread_group_divisor ? 1 : 0)
    // - thread_number / thread_group_divisor) is number of synced threads

    ResultQueue result_queue;

    //vector of thread functors
    std::vector<ModelBankPaymentThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    BOOST_TEST_MESSAGE( "thread barriers:: "
            <<  (thread_number - (thread_number % thread_group_divisor ? 1 : 0)
                    - thread_number/thread_group_divisor));

    //synchronization barriers instance
    sync_barriers sb(thread_number - (thread_number % thread_group_divisor ? 1 : 0)
            - thread_number/thread_group_divisor);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(ModelBankPaymentThreadWorker(i,3,&sb
                , thread_group_divisor, &result_queue));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_TEST_MESSAGE("threads end result_queue.size(): " << result_queue.size());

    for(unsigned i = 0; i < thread_number; ++i)
    {
        Result thread_result;
        result_queue.try_pop(thread_result);

        BOOST_REQUIRE_EQUAL(thread_result.ret , 0);

        if(thread_result.ret)
        {
            BOOST_TEST_MESSAGE( thread_result.desc
                    << " thread number: " << thread_result.number
                    << " return code: " << thread_result.ret
                    << " description: " << thread_result.desc
                    );
        }
    }//for i
}
BOOST_AUTO_TEST_SUITE_END();


