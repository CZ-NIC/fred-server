/*  
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 * 
 * This file is part of FRED.
 * 
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 * 
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file config_handler.h
 *  common option handlers
 */


#ifndef CONFIG_HANDLER_H_
#define CONFIG_HANDLER_H_

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <typeinfo>

#include <boost/assign/list_of.hpp>
#include <boost/program_options.hpp>
#include <boost/utility.hpp>
#include <boost/format.hpp>

#include "register/db_settings.h"
#include "faked_args.h"
#include "handle_args.h"
#include "handle_general_args.h"



//owning container of handlers
typedef boost::shared_ptr<HandleArgs> HandleArgsPtr;
typedef std::vector<HandleArgsPtr > HandlerPtrVector;
typedef std::map<std::string, HandleArgsPtr > HandlerPtrMap;

//compose args processing
/* possible usage:
HandlerPtrVector ghpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
(HandleArgsPtr(new HandleThreadGroupArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs));

in UTF main
 fa = CfgArgs::instance(ghpv)->handle(argc, argv);
*/

/**
 * \class CfgArgs
 * \brief handlers container and processor
 */
class CfgArgs : boost::noncopyable
{
    HandlerPtrVector hpv_;
    HandlerPtrMap hpm_;
    static std::auto_ptr<CfgArgs> instance_ptr;
public:
    template <class T> HandleArgsPtr get_handler_by_type()
    {
        HandlerPtrMap::const_iterator it;
        it = hpm_.find( typeid(T).name() );
        if(it != hpm_.end())//if found
            return it->second;
        //not found
        return HandleArgsPtr(static_cast<HandleArgs*>(0));
    }

    template <class T> T* get_handler_ptr_by_type()
    {
        HandlerPtrMap::const_iterator it;
        it = hpm_.find( typeid(T).name() );
        if(it != hpm_.end())//if found
            return dynamic_cast<T*>(it->second.get());
        //not found
        throw std::runtime_error("error: handler not found");
    }
    friend class std::auto_ptr<CfgArgs>;
protected:
    ~CfgArgs(){}
private:
    CfgArgs(const HandlerPtrVector& hpv)
        : hpv_(hpv) //vector init
    {
        //map init
        for(HandlerPtrVector::const_iterator i = hpv.begin()
                ; i != hpv.end(); ++i )
            hpm_[typeid( *((*i).get()) ).name()] = *i;
    }

public:
    static CfgArgs * instance(const HandlerPtrVector& hpv);
    static CfgArgs * instance();

    FakedArgs fa;
    FakedArgs handle( int argc, char* argv[])
    {
        //initial fa
        fa.prealocate_for_argc(argc);
        for (int i = 0; i < argc ; ++i)
            fa.add_argv(argv[i]);

        for(HandlerPtrVector::const_iterator i = hpv_.begin()
                ; i != hpv_.end(); ++i )
        {
            FakedArgs fa_out;
            (*i)->handle( fa.get_argc(), fa.get_argv(), fa_out);
            fa=fa_out;//last output to next input
        }//for HandlerPtrVector
        return fa;
    }//handle
};//class CfgArgs

//static instance init
std::auto_ptr<CfgArgs> CfgArgs::instance_ptr(0);

//setter
CfgArgs* CfgArgs::instance(const HandlerPtrVector& hpv)
{
    std::auto_ptr<CfgArgs>
    tmp_instance(new CfgArgs(hpv));

    //gather options_descriptions for help print if present
    HandleArgsPtr ha =
            tmp_instance->get_handler_by_type<HandleGeneralArgs>();
    HandleGeneralArgs* hga = 0;//nonowning temp child
    if(ha.get() != 0)
        hga = dynamic_cast<HandleGeneralArgs*>(ha.get());
    if(hga != 0)
    {
        for(HandlerPtrVector::const_iterator i = hpv.begin()
                ; i != hpv.end(); ++i )
            hga->po_description.push_back((*i)->get_options_description());
    }
    instance_ptr = tmp_instance;
    return instance_ptr.get();
}

//getter
CfgArgs* CfgArgs::instance()
{
    CfgArgs* ret = instance_ptr.get();
    if (ret == 0) throw std::runtime_error("error: CfgArgs instance not set");
    return ret;
}

/**
 * \class HandleDatabaseArgs
 * \brief database options handler
 */
class HandleDatabaseArgs : public HandleArgs
{
public:

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> db_opts(
                new boost::program_options::options_description(
                        std::string("Database connection configuration")));
        db_opts->add_options()
                ("database.name", boost::program_options
                            ::value<std::string>()->default_value(std::string("fred"))
                        , "database name")
                ("database.user", boost::program_options
                            ::value<std::string>()->default_value(std::string("fred"))
                        , "database user name")
                ("database.password", boost::program_options
                            ::value<std::string>(), "database password")
                ("database.host", boost::program_options
                            ::value<std::string>()->default_value(std::string("localhost"))
                        , "database hostname")
                ("database.port", boost::program_options
                            ::value<unsigned int>(), "database port number")
                ("database.timeout", boost::program_options
                            ::value<unsigned int>(), "database timeout");

        return db_opts;
    }//get_options_description

    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

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

    }//handle
};

/**
 * \class HandleThreadGroupArgs
 * \brief thread group options handler
 */
class HandleThreadGroupArgs : public HandleArgs
{
public:
    std::size_t thread_number ;// 300;//number of threads in test
    std::size_t thread_group_divisor;// = 10;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> thread_opts(
                new boost::program_options::options_description(
                        std::string("Thread group configuration")));
        thread_opts->add_options()
                ("thread_number", boost::program_options
                            ::value<unsigned int>()->default_value(300)
                             , "number of threads in group")
                ("thread_group_divisor", boost::program_options
                            ::value<unsigned int>()->default_value(10)
                             , "designates fraction of non-synchronized threads");
        return thread_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        thread_number = (vm.count("thread_number") == 0
                ? 300 : vm["thread_number"].as<unsigned>());
        std::cout << "thread_number: " << thread_number
                << " vm[\"thread_number\"].as<unsigned>(): "
                << vm["thread_number"].as<unsigned>() << std::endl;

        thread_group_divisor = (vm.count("thread_group_divisor") == 0
                ? 10 : vm["thread_group_divisor"].as<unsigned>());
        std::cout << "thread_group_divisor: " << thread_group_divisor<< "" << std::endl;
    }//handle
};

/**
 * \class HandleCorbaNameServiceArgs
 * \brief corba nameservice options handler
 */
class HandleCorbaNameServiceArgs : public HandleArgs
{
public:
    std::string nameservice_host ;
    unsigned nameservice_port;
    std::string nameservice_context;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("CORBA NameService configuration")));
        opts_descs->add_options()
                ("nameservice.host", boost::program_options
                            ::value<std::string>()->default_value(std::string("localhost"))
                        , "nameservice host name")
                ("nameservice.port", boost::program_options
                            ::value<unsigned int>()->default_value(2809)
                             , "nameservice port number")
                ("nameservice.context", boost::program_options
                         ::value<std::string>()->default_value(std::string("fred"))
                     , "freds context in name service");

        return opts_descs;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        nameservice_host = (vm.count("nameservice.host") == 0
                ? std::string("localhost") : vm["nameservice.host"].as<std::string>());
        nameservice_port = (vm.count("nameservice.port") == 0
                ? 2809 : vm["nameservice.port"].as<unsigned>());
        nameservice_context = (vm.count("nameservice.context") == 0
                ? std::string("fred") : vm["nameservice.context"].as<std::string>());
    }//handle
};

#endif //CONFIG_HANDLER_H_
