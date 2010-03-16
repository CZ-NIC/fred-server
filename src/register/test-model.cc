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
    int_distribution_t gen_int;//signed integer
    real_distribution_t gen_real;//signed double
    boost::variate_generator<base_generator_t, int_distribution_t > letter52;
    boost::variate_generator<base_generator_t, int_distribution_t > gint;
    boost::variate_generator<base_generator_t, real_distribution_t > greal;

public:
    //ctor
    RandomDataGenerator(unsigned seed = 0)
        : seed_(seed ? seed : msseed())
          , rng(seed_)

          //ranges definitions
        , gen_letter52(0,51)
        , gen_int(std::numeric_limits<int>::min(), std::numeric_limits<int>::max())
        , gen_real(std::numeric_limits<double>::min(), std::numeric_limits<double>::max())

          //generator instances
        , letter52(rng, gen_letter52)
        , gint(rng, gen_int)
        , greal(rng, gen_real)
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

    //generate some string of given length
    std::string xstring(std::size_t length)
    {
        std::string ret;
        ret.reserve(length);//allocation
        for(std::size_t i = 0; i < length; ++i)
            ret.push_back(xletter());
        return ret;
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
        return tv.tv_usec;
    }

   double xreal()
   {
       return greal();
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
        std::cout << "waiting: " << number_ << " xstring: " << rdg.xstring(10)
                << "int limit test: "  << " xint: " << rdg.xint()
                << " xuint: " << rdg.xuint() << " xreal: " << rdg.xreal()
                << std::endl;
        sb_ptr_->insert_barrier.wait();//wait for other threads
        std::cout << "start: " << number_ << std::endl;


        boost::posix_time::seconds workTime(sleep_time_);// Pretend to do something useful...

        //boost::this_thread::sleep(workTime);
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
    std::size_t const thread_number = 10;//number of threads in test

    //vector of thread functors
    std::vector<ModelBankPaymentThreadWorker> tw_vector;
    tw_vector.reserve(thread_number);

    //synchronization barriers instance
    sync_barriers sb(thread_number);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < thread_number; ++i)
    {
        tw_vector.push_back(ModelBankPaymentThreadWorker(i,3,&sb));
        threads.create_thread(tw_vector.at(i));
    }

    threads.join_all();

    BOOST_CHECK( 1 + 1 == 2);
}


struct faked_args //faked args structure
{
    typedef std::vector<char> char_vector_t;//type for argv buffer
    typedef std::vector<char_vector_t> argv_buffers_t;//buffers vector type
    typedef std::vector<char*> argv_t;//pointers vector type

    argv_buffers_t argv_buffers;//owning vector of buffers
    argv_t argv;//new argv - nonowning
    int argc;//new number of args
};//struct faked_args

//removing our config from boost test cmdline
faked_args po_config( int argc, char* argv[] )
{
    faked_args fa;
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
    std::vector<std::string> to_pass_further
        = po::collect_unrecognized(parsed.options, po::include_positional);
    po::notify(vm);

     //faked args
     fa.argc = to_pass_further.size() + 1;//new number of args + first

     //vector prealocation
     fa.argv_buffers.reserve(fa.argc);
     fa.argv.reserve(fa.argc);

     //program name copy
     fa.argv_buffers.push_back(faked_args::char_vector_t());//added buffer
     std::size_t strsize = strlen(argv[0]);

     //preallocation of buffer for first ending with 0
     fa.argv_buffers[0].reserve(strsize+1);

     //actual program name copy
     for(unsigned i = 0; i < strsize;  ++i )
     {
         fa.argv_buffers[0].push_back(argv[0][i]);
     }//for i

     fa.argv_buffers[0].push_back(0);//zero terminated string
     fa.argv.push_back(&fa.argv_buffers[0][0]);//added char*

     //copying new arg vector with char pointers to data in
     //to_pass_further strings except first
     for(int i = 0; i < fa.argc-1; ++i)
     {
         std::size_t string_len = to_pass_further[i].length();//string len
         fa.argv_buffers.push_back(faked_args::char_vector_t());//added vector
         fa.argv.push_back(0);//added char* 0
         fa.argv_buffers[i+1].reserve(string_len+1);//preallocation of buffer ending with 0

         for(std::string::const_iterator si = to_pass_further[i].begin()
                 ; si != to_pass_further[i].end();  ++si )
         {
             fa.argv_buffers[i+1].push_back(*si);
         }//for si

         fa.argv_buffers[i+1].push_back(0);//zero terminated string
         fa.argv[i+1] = &fa.argv_buffers[i+1][0];
     }//for i

     if (vm.count("help"))
     {
         std::cout << dbconfig << std::endl;
         throw std::runtime_error("exiting after help");
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

     return fa;
}


int main( int argc, char* argv[] )
{
    //processing of additional program options
    //producing faked args with unrecognized ones
    faked_args fa = po_config( argc, argv);

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

    return ::boost::unit_test::unit_test_main( init_func, fa.argc, &fa.argv[0] );//using fake args
#else
    return ::boost::unit_test::unit_test_main(  fa.argc, &fa.argv[0] );//using fake args
#endif //if 1_38

}

