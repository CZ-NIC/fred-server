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

#include <memory>
#include <iostream>

#define BOOST_TEST_MODULE Test model
#define BOOST_TEST_NO_MAIN

#include <string>
#include <boost/format.hpp>
#include "db_settings.h"
#include "log/logger.h"
#include "log/context.h"
#include <boost/test/included/unit_test.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>


#include "model_files.h"

// #define CONNECTION_STRING       "host=localhost dbname=fred user=fred port=6655"
#define CONNECTION_STRING       "host=localhost dbname=fred user=fred"

ModelFiles mf1, mf2;

unsigned model_insert_test()
{
    unsigned ret=0;
    //insert data
    mf1.setName("dummy_test_name");
    mf1.setPath("~/log");
    //mf1.setMimeType("application/octet-stream");//default
    mf1.setCrDate(Database::DateTime("2010-03-10 12:00:01"));
    mf1.setFilesize(50000);
    mf1.setFileTypeId(2);//invoice xml
    mf1.insert();

    Database::Connection conn = Database::Manager::acquire();
    std::string query = str(boost::format(
            "select id, name, path, mimetype, crdate, filesize, filetype "
            "from files WHERE id = %1%") % mf1.getId() );
    Database::Result res = conn.exec( query );
    if ((res.size() > 0) && (res[0].size() == 7))
    {    //check data inserted by model
        if(static_cast<unsigned long long>(res[0][0]) != mf1.getId()) ret+=1;
        if(mf1.getName().compare(res[0][1])) ret+=2;
        if(mf1.getPath().compare(res[0][2])) ret+=4;
        if(std::string("application/octet-stream").compare(res[0][3])) ret+=8;
        if(mf1.getCrDate().to_string()
            .compare(Database::DateTime(std::string(res[0][4])).to_string()))
                ret+=16;
        if(static_cast<int>(res[0][5]) != mf1.getFilesize()) ret+=32;
        if(static_cast<unsigned long long>(res[0][6]) != mf1.getFileTypeId()) ret+=64;

    }//if res size
    else ret+=128;

    if (ret != 0 ) std::cerr << "model_insert_test ret: "<< ret << std::endl;

    return ret;
}

unsigned model_reload_test()
{
    unsigned ret=0;
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);
    std::string query = str(boost::format("UPDATE files SET name = E'', path = E'', mimetype = E'',"
            " crdate = '2000-01-01 00:00:01', filesize = 80000, fileType = 1 WHERE id = %1%") % mf1.getId() );
    conn.exec( query );
    tx.commit();

    mf2.setId(mf1.getId());
    mf2.reload();

    //check data from UPDATE query after reload
    if(mf2.getId() != mf1.getId()) ret+=1;
    if(mf2.getName().compare("")) ret+=2;
    if(mf2.getPath().compare("")) ret+=4;
    if(mf2.getMimeType().compare("")) ret+=8;
    if(Database::DateTime(mf2.getCrDate()).to_string().compare(
        Database::DateTime("2000-01-01 00:00:01").to_string() ))    ret+=16;
    if(mf2.getFilesize() != 80000 ) ret+=32;
    if(mf2.getFileTypeId() != 1 ) ret+=64;

    if (ret != 0 ) std::cerr << "model_reload_test ret: "<< ret << std::endl;
    return ret;
}


unsigned model_update_test()
{
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
    unsigned ret=0;

    mf1.setFilesize(80000);
    mf1.update();

    mf1.reload();

    //compare mf1 and mf2, it should be same,  ret=0 is OK

    if(mf1.getId() != mf2.getId()) ret+=1;
    if(mf1.getName() != mf2.getName())
    {
        std::cerr << mf1.getName() << std::endl;
        std::cerr << mf2.getName() << std::endl;
        ret+=2;
    }

    if(mf1.getPath() != mf2.getPath())
    {
        std::cerr << mf1.getPath() << std::endl;
        std::cerr << mf2.getPath() << std::endl;

        ret+=4;
    }

    if(mf1.getMimeType() != mf2.getMimeType()) ret+=8;
    if(mf1.getCrDate() != mf2.getCrDate())
    {
        std::cerr << mf1.getCrDate() << std::endl;
        std::cerr << mf2.getCrDate() << std::endl;
        ret+=16;
    }

    if(mf1.getFilesize() != mf2.getFilesize()) ret+=32;
    if(mf1.getFileTypeId() != mf2.getFileTypeId())
    {
        std::cerr << mf1.getFileTypeId() << std::endl;
        std::cerr << mf2.getFileTypeId() << std::endl;

        ret+=64;
    }


    if(ret !=0 ) std::cerr << "model_update_test ret: "<< ret << std::endl;

    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);
    std::string query = str(boost::format("DELETE FROM files WHERE id = %1%") % mf1.getId() );
    conn.exec( query );
    tx.commit();

    return ret;

}

unsigned model_nodatareload_test()
{
    unsigned ret=0;
    mf2.reload();
    return ret;
}

unsigned model_nodataupdate_test()
{
    unsigned ret=0;
    mf2.setName("x");
    mf2.update();
    return ret;
}



bool check_std_exception_nodatafound(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("No data found")) != std::string::npos);
}

BOOST_AUTO_TEST_CASE( test_model )
{
    BOOST_REQUIRE_EQUAL(model_insert_test() , 0);
    BOOST_REQUIRE_EQUAL(model_reload_test() , 0);
    BOOST_REQUIRE_EQUAL(model_update_test() , 0);
    BOOST_REQUIRE_EXCEPTION( model_nodatareload_test()
            , std::exception , check_std_exception_nodatafound);
    BOOST_REQUIRE_EQUAL(model_nodataupdate_test() , 0);
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

    // prototype for user's unit test init function
#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
    extern bool init_unit_test();

    boost::unit_test::init_unit_test_func init_func = &init_unit_test;
#else
    extern ::boost::unit_test::test_suite* init_unit_test_suite( int argc, char* argv[] );

    boost::unit_test::init_unit_test_func init_func = &init_unit_test_suite;
#endif

    return ::boost::unit_test::unit_test_main( init_func, fa.argc, &fa.argv[0] );//using fake args
}

