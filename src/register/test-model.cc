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
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


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
    sync_barriers(std::size_t thread_number)
        : insert_barrier(thread_number)
    {}
};

//thread functor
class ModelBankPaymentThreadWorker
{
public:

    ModelBankPaymentThreadWorker(unsigned number,unsigned sleep_time, sync_barriers* sb_ptr, unsigned seed = 0)
            : number_(number)
            , sleep_time_(sleep_time)
            , sb_ptr_(sb_ptr)
            , rdg(seed)
    {}

    void operator()()
    {
        ModelBankPayment mbp1, mbp2;


        if(number_%2)//if odd number
        {
            std::cout << "waiting: " << number_ << std::endl;
            /*
                    << " xstring: " << rdg.xstring(10)
                    << " int limit test: "  << " xint: " << rdg.xint()
                    << " xuint: " << rdg.xuint() << " xreal: " << rdg.xreal()
                    << " xnumletter: " << rdg.xnumletter()
                    << " xtime: " << rdg.xtime()
                    << " xptime: " << rdg.xptime()
                    << " Date xptime: " << Database::Date(rdg.xptime().date())
                    << std::endl;
                    */

            sb_ptr_->insert_barrier.wait();//wait for other odd threads
        }
        else
        {//even threads don't wait
            std::cout << "NOwaiting: " << number_ << std::endl;
            /*
                        << " xstring: " << rdg.xstring(10)
                        << " int limit test: "  << " xint: " << rdg.xint()
                        << " xuint: " << rdg.xuint() << " xreal: " << rdg.xreal()
                        << " xnumletter: " << rdg.xnumletter() << std::endl;
                        */
        }

        std::cout << "start: " << number_ << std::endl;


        mbp_insert_data insert_data;

        insert_data.statement_id=0;//fk bank_statement (id) - none
        insert_data.account_id=rdg.xnum1_6(); //fk bank_account (id) - num 1-6
        insert_data.invoice_id=0; //fk invoice (id) - none
        insert_data.account_number=rdg.xnumstring(17);//17 numletters
        insert_data.bank_code=rdg.xnumstring(4);//4 numletters
        insert_data.operation_code=rdg.xnum1_5(); // num 1-5
        insert_data.transfer_type=rdg.xnum1_5(); // num 1-5
        insert_data.payment_status=rdg.xnum1_6();// num 1-6
        insert_data.konstsym=rdg.xnumstring(10);// 10 numletters
        insert_data.varsymb=rdg.xnumstring(10);// 10 numletters
        insert_data.specsymb=rdg.xnumstring(10);// 10 numletters
        insert_data.price=rdg.xint();//int
        insert_data.account_evid=rdg.xnumstring(20);//20 numletters
        insert_data.account_date=rdg.xdate(); //some date
        insert_data.account_memo=rdg.xstring(64); //64 chars
        insert_data.account_name=rdg.xstring(64); //64 chars
        insert_data.crtime=rdg.xptime();//timestamp


        BOOST_REQUIRE_EQUAL(
                mbp_insert_test(mbp1, insert_data)
                , 0);

        BOOST_REQUIRE_EQUAL(
                mbp_reload_test(mbp1, mbp2)
                , 0);

        std::cout << "end: " << number_ << std::endl;
    }

private:

    ModelBankPayment mbp;
    unsigned    number_;//thred identification
    unsigned    sleep_time_;//[s]
    sync_barriers* sb_ptr_;
    RandomDataGenerator rdg;
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
    std::size_t const thread_number = 300;//number of threads in test

    //vector of thread functors
    std::vector<ModelBankPaymentThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    //synchronization barriers instance
    sync_barriers sb(thread_number/2);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(ModelBankPaymentThreadWorker(i,3,&sb));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_CHECK( 1 + 1 == 2 );
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


//removing our config from boost test cmdline
//return value:
//  true - do not continue with tests and return after po_config
//  false - continue with tests
bool po_config( int argc, char* argv[] , FakedArgs& fa )
{
    namespace po = boost::program_options;
    po::options_description
        dbconfig (std::string("Database connection configuration"));
    dbconfig.add_options()
        ("help", "print help message")
        ("dbname", po::value<std::string>()->default_value(std::string("fred"))
                , "database name")
        ("dbuser", po::value<std::string>()->default_value(std::string("fred"))
                , "database user name")
        ("dbpassword", po::value<std::string>(), "database password")
        ("dbhost", po::value<std::string>()->default_value(std::string("localhost"))
                , "database hostname")
        ("dbport", po::value<unsigned int>(), "database port number")
        ("dbtimeout", po::value<unsigned int>(), "database timeout")
        ;
    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv).
                            options(dbconfig).allow_unregistered().run();
    po::store(parsed, vm);
    typedef std::vector<std::string> string_vector_t;
    string_vector_t to_pass_further
        = po::collect_unrecognized(parsed.options, po::include_positional);
    po::notify(vm);

    //faked args
    fa.clear();//to be sure that fa is empty

    //preallocate for new number of args + first programm name
    fa.prealocate_for_argc(to_pass_further.size() + 1);

    //program name copy
    fa.add_argv(argv[0]);

    //copying a new arg vector
    for(string_vector_t::const_iterator i = to_pass_further.begin()
            ; i != to_pass_further.end()
            ; ++i)
    {
        fa.add_argv(*i);//string
    }//for i

     if (vm.count("help"))
     {
         std::cout << dbconfig << std::endl;
         return true;//do not continue with tests and return
     }

     /* construct connection string */
     std::string dbhost = (vm.count("dbhost") == 0 ? ""
             : "host=" + vm["dbhost"].as<std::string>() + " ");
     std::string dbpass = (vm.count("dbpassword") == 0 ? ""
             : "password=" + vm["dbpassword"].as<std::string>() + " ");
     std::string dbname = (vm.count("dbname") == 0 ? ""
             : "dbname=" + vm["dbname"].as<std::string>() + " ");
     std::string dbuser = (vm.count("dbuser") == 0 ? ""
             : "user=" + vm["dbuser"].as<std::string>() + " ");
     std::string dbport = (vm.count("dbport") == 0 ? ""
             : "port=" + boost::lexical_cast<std::string>(vm["dbport"].as<unsigned>()) + " ");
     std::string dbtime = (vm.count("dbtimeout") == 0 ? ""
             : "connect_timeout=" + boost::lexical_cast<std::string>(vm["dbtimeout"].as<unsigned>()) + " ");

     std::string conn_info = str(boost::format("%1% %2% %3% %4% %5% %6%")
                                               % dbhost
                                               % dbport
                                               % dbname
                                               % dbuser
                                               % dbpass
                                               % dbtime);

     //std::cout << "database connection set to: " << conn_info << std::endl;
     LOGGER(PACKAGE).info(boost::format("database connection set to: `%1%'")
                                         % conn_info);

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

