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


#include <fstream>
#include <queue>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>


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

class RandomDataGenerator //data generator is actually pseudo-random
{
    //typedef for a random number generator
    //boost::mt19937, boost::ecuyer1988, boost::minstd_rand
    typedef boost::mt19937 base_generator_t;

    //uniform random number distribution of values in given range
    typedef boost::uniform_int<> int_distribution_t;
    typedef boost::uniform_real<> real_distribution_t;

    unsigned seed_;
    base_generator_t rng;
    int_distribution_t gen_letter52;//both case letters
    int_distribution_t gen_numletter10;//any number
    int_distribution_t gen_num1_5;//number 1-5
    int_distribution_t gen_num1_6;//number 1-6
    int_distribution_t gen_int;//signed integer
    real_distribution_t gen_real;//signed double
    int_distribution_t gen_time;//from 1.1.1990 to the end of unix time (19.01.2038 04:14:07 (CET))
    boost::variate_generator<base_generator_t, int_distribution_t > letter52;
    boost::variate_generator<base_generator_t, int_distribution_t > numletter10;
    boost::variate_generator<base_generator_t, int_distribution_t > num1_5;
    boost::variate_generator<base_generator_t, int_distribution_t > num1_6;
    boost::variate_generator<base_generator_t, int_distribution_t > gint;
    boost::variate_generator<base_generator_t, real_distribution_t > greal;
    boost::variate_generator<base_generator_t, int_distribution_t > gtime;

public:
    //ctor
    RandomDataGenerator(unsigned seed = 0)
        : seed_(seed ? seed : msseed())
          , rng(seed_)

          //ranges definitions
        , gen_letter52(0,51)
        , gen_numletter10(0,9)
        , gen_num1_5(1,5)
        , gen_num1_6(1,6)
        , gen_int(std::numeric_limits<int>::min(), std::numeric_limits<int>::max())
        , gen_real(std::numeric_limits<double>::min(), std::numeric_limits<double>::max())
        , gen_time(631148400, 2147483647)


          //generator instances
        , letter52(rng, gen_letter52)
        , numletter10(rng, gen_numletter10)
        , num1_5(rng, gen_num1_5)
        , num1_6(rng, gen_num1_6)
        , gint(rng, gen_int)
        , greal(rng, gen_real)
        , gtime(rng, gen_time)
    {
        std::cout << "RandomDataGenerator using seed: " << seed_ << std::endl;
    }

    //used seed getter
    unsigned get_seed() const
    {
        return seed_;
    }

    //generate some letter A-Z a-z
    char xletter()
    {
        unsigned rnumber = letter52();
        char ret = rnumber < 26
                ? static_cast<char>(rnumber + 65) //A-Z
                : static_cast<char>(rnumber + 71) ; //a-z
        return ret;
    }

    //generate some letter 0-9
    char xnumletter()
    {
        return static_cast<char>(numletter10() + 48);
    }


    //generate some string of given length
    std::string xstring(std::size_t length)
    {
        std::string ret;
        ret.reserve(length);//allocation
        for(std::size_t i = 0; i < length; ++i)
            ret.push_back(xletter());
        return ret;
    }

    //generate some string of given length
    std::string xnumstring(std::size_t length)
    {
        std::string ret;
        ret.reserve(length);//allocation
        for(std::size_t i = 0; i < length; ++i)
            ret.push_back(xnumletter());
        return ret;
    }

    //generate signed integer 1-5
    int xnum1_5()
    {
        return num1_5();
    }

    //generate signed integer 1-6
    int xnum1_6()
    {
        return num1_6();
    }

    //generate some signed integer
    int xint()
    {
        return gint();
    }

    //generate some unsigned integer
    unsigned xuint()
    {
        return static_cast<unsigned>(- std::numeric_limits<int>::min()) + gint();
    }

   unsigned long msseed()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_usec + tv.tv_sec;
    }
   //generate some signed real number
   double xreal()
   {
       return greal();
   }
   //generate some unix time from 1.1.1990 to the end of unix time (19.01.2038 04:14:07 (CET))
   time_t xtime()
   {
       return gtime();
   }
   //generate some posix time from 1.1.1990 to the end of unix time (19.01.2038 04:14:07 (CET))
   boost::posix_time::ptime xptime()
   {
       return boost::posix_time::from_time_t(gtime());
   }

   //generate some gregorian date from 1.1.1990 to the end of unix time (19.01.2038)
   boost::gregorian::date xdate()
   {
       return xptime().date();
   }

};


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
    std::size_t const thread_number = 300;// 300;//number of threads in test
    std::size_t const thread_group_divisor = 10;
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
                    << " decription: " << thread_result.desc
                    << std::endl;
        }
    }//for i

}


class FakedArgs //faked args
{
    typedef std::vector<char> char_vector_t;//type for argv buffer
    typedef std::vector<char_vector_t> argv_buffers_t;//buffers vector type
    typedef std::vector<char*> argv_t;//pointers vector type

    argv_buffers_t argv_buffers;//owning vector of buffers
    argv_t argv;//new argv - nonowning

public:
    void clear()
    {
        argv_buffers.clear();
        argv.clear();
    }
    //optional memory prealocation for expected argc
    //actual argc is not affected
    void prealocate_for_argc(int expected_argc)
    {
        //vector prealocation
        argv_buffers.reserve(expected_argc);
        argv.reserve(expected_argc);
    }//prealocate_for_argc

    int get_argc() const //argc getter
    {
        return static_cast<int>(argv_buffers.size());
    }

    char** get_argv() //argv getter
    {
        return &argv[0];
    }

    void add_argv(char* asciiz)//add zero terminated C-style string of chars
    {
        add_argv(std::string(asciiz));
    }

    void add_argv(std::string str)//add std::string
    {
        std::cout << "add_argv str : " << str <<  std::endl;
        argv_buffers.push_back(FakedArgs::char_vector_t());//added buffer
        std::size_t strsize = str.length();
        //argv size
        std::size_t argv_size = argv_buffers.size();
        std::size_t argv_idx = argv_size - 1;
        //preallocation of buffer for first ending with 0
        argv_buffers[argv_idx].reserve(strsize+1);

        //actual string copy
        for(std::string::const_iterator si = str.begin()
                ; si != str.end();  ++si )
        {
            argv_buffers[argv_idx].push_back(*si);
        }//for si
        argv_buffers[argv_idx].push_back(0);//zero terminated string
        argv.push_back(&argv_buffers[argv_idx][0]);//added char*
        std::cout << "add_argv str : " << str <<  std::endl;
    }

};//class FakedArgs

void parse_config_file_to_faked_args(std::string fname, FakedArgs& fa )
{//options without values are ignored
    std::ifstream cfg_file(fname.c_str());
    if (cfg_file.fail())
      throw std::runtime_error("config file '" + fname + "' not found");

    std::string line, opt_prefix;
    while (std::getline(cfg_file, line))
    {
      boost::algorithm::trim(line);// strip whitespace
      if (!line.empty() && line[0] != '#')// ignore empty line and comments
      {
        if (line[0] == '[' && line[line.size() - 1] == ']')
        {// this is option prefix
          opt_prefix = line;
          boost::algorithm::erase_first(opt_prefix, "[");
          boost::algorithm::erase_last(opt_prefix,  "]");
        }//if [opt_prefix]
        else
        {// this is normal option
          std::string::size_type sep = line.find("=");
          if (sep != std::string::npos)
          {// get name and value couple without any whitespace
            std::string name  = boost::algorithm::trim_copy(line.substr(0, sep));
            std::string value = boost::algorithm::trim_copy(line.substr(sep + 1, line.size() - 1));
            if (!value.empty())
            {// push appropriate commnad-line string
                fa.add_argv("--" + opt_prefix + "." + name + "=" + value);
            }//if value not empty
          }// if '=' found
        }//else - option
      }//if not empty line
    }//while getline
}//parse_config_file_to_faked_args

//removing our config from boost test cmdline
//return value:
//  true - do not continue with tests and return after po_config
//  false - continue with tests
bool po_config( int argc, char* argv[] , FakedArgs& fa )
{
    namespace po = boost::program_options;

    po::options_description
            genconfig (std::string("General configuration"));
    genconfig.add_options()
        ("help", "print this help message");

    po::options_description
        dbconfig (std::string("Database connection configuration"));
    dbconfig.add_options()
        ("database.name", po::value<std::string>()->default_value(std::string("fred"))
                , "database name")
        ("database.user", po::value<std::string>()->default_value(std::string("fred"))
                , "database user name")
        ("database.password", po::value<std::string>(), "database password")
        ("database.host", po::value<std::string>()->default_value(std::string("localhost"))
                , "database hostname")
        ("database.port", po::value<unsigned int>(), "database port number")
        ("database.timeout", po::value<unsigned int>(), "database timeout");

    std::string default_config;

#ifdef CONFIG_FILE
        std::cout << "CONFIG_FILE: "<< CONFIG_FILE << std::endl;
        default_config = std::string(CONFIG_FILE);
#else
        default_config = std::string("");
#endif

    if(default_config.length() != 0)
    {
        genconfig.add_options()
                ("config,C", po::value<std::string>()->default_value(default_config)
                , "path to configuration file");
    }
    else
    {
        genconfig.add_options()
                ("config,C", po::value<std::string>(), "path to configuration file");
    }

    typedef std::vector<std::string> string_vector_t;
    string_vector_t to_pass_further;//args

    po::variables_map gen_vm;
    po::parsed_options gen_parsed = po::command_line_parser(argc, argv).
                            options(genconfig).allow_unregistered().run();
    po::store(gen_parsed, gen_vm);

    to_pass_further
        = po::collect_unrecognized(gen_parsed.options, po::include_positional);
    po::notify(gen_vm);

    //general config actions
    if (gen_vm.count("help"))
    {
        std::cout << "\n" << genconfig << "\n" << dbconfig << std::endl;
        return true;//do not continue with tests and return
    }

    FakedArgs dbfa;//faked args for db config
    dbfa.clear();//to be sure that fa is empty
    dbfa.prealocate_for_argc(to_pass_further.size() + 1);//new number of args + first program name
    dbfa.add_argv(argv[0]);//program name copy
    for(string_vector_t::const_iterator i = to_pass_further.begin()
            ; i != to_pass_further.end()
            ; ++i)
    {//copying a new arg vector
        dbfa.add_argv(*i);//string
    }//for i

    //read config file if configured and append content to dbfa
    if (gen_vm.count("config,C"))
    {
        std::string fname = gen_vm["config,C"].as<std::string>();
        if(fname.length())
            parse_config_file_to_faked_args(fname, dbfa );
    }

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(dbfa.get_argc(),dbfa.get_argv()).
                            options(dbconfig).allow_unregistered().run();
    po::store(parsed, vm);

    to_pass_further
        = po::collect_unrecognized(parsed.options, po::include_positional);
    po::notify(vm);

    //faked args for unittest framework returned by reference in params
    fa.clear();//to be sure that fa is empty
    fa.prealocate_for_argc(to_pass_further.size() + 1);//new number of args + first program name
    fa.add_argv(argv[0]);//program name copy
    for(string_vector_t::const_iterator i = to_pass_further.begin()
            ; i != to_pass_further.end()
            ; ++i)
    {//copying a new arg vector
        fa.add_argv(*i);//string
    }//for i

    /* construct connection string */
    std::string dbhost = (vm.count("database.host") == 0 ? ""
            : "host=" + vm["database.host"].as<std::string>() + " ");
    std::string dbpass = (vm.count("database.password") == 0 ? ""
            : "password=" + vm["database.password"].as<std::string>() + " ");
    std::string dbname = (vm.count("database.name") == 0 ? ""
            : "dbname=" + vm["database.name"].as<std::string>() + " ");
    std::string dbuser = (vm.count("database.user") == 0 ? ""
            : "user=" + vm["database.user"].as<std::string>() + " ");
    std::string dbport = (vm.count("database.port") == 0 ? ""
            : "port=" + boost::lexical_cast<std::string>(vm["database.port"].as<unsigned>()) + " ");
    std::string dbtime = (vm.count("database.timeout") == 0 ? ""
            : "connect_timeout=" + boost::lexical_cast<std::string>(vm["database.timeout"].as<unsigned>()) + " ");
    std::string conn_info = str(boost::format("%1% %2% %3% %4% %5% %6%")
                                              % dbhost
                                              % dbport
                                              % dbname
                                              % dbuser
                                              % dbpass
                                              % dbtime);
    std::cout << "database connection set to: " << conn_info << std::endl;

    Database::Manager::init(new Database::ConnectionFactory(conn_info));

    return false;
}


int main( int argc, char* argv[] )
{
    //processing of additional program options
    //producing faked args with unrecognized ones
    FakedArgs fa;
    if(po_config( argc, argv, fa)) return 0;

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

