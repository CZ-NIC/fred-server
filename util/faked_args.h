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
 *  @file faked_args.h
 *  manipulation with cmdline arguments
 */


#ifndef FAKED_ARGS_H_
#define FAKED_ARGS_H_

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include "db_settings.h"


/**
 * \class FakedArgs
 * \brief for manipulation with cmdline arguments
 */


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

///end process because of cmdline processing by calling return 0; in main()
class ReturnFromMain : public Exception {
public:
    ReturnFromMain(const std::string& _what) : Exception(_what) {
  }
};

class HandleArgs
{
public:
    virtual boost::shared_ptr<boost::program_options::options_description>
        get_options_description()=0;
    virtual void handle( int argc, char* argv[], FakedArgs &fa ) = 0;
};

class HandleGeneralArgs : public HandleArgs
{
    ///options descriptions reference used to print help for all options
    typedef std::vector<boost::shared_ptr<boost::program_options::options_description> > PoDescs;


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

public:
    PoDescs po_description;

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> gen_opts(
                new boost::program_options::options_description(
                        std::string("General configuration")));
        gen_opts->add_options()
                ("help", "print this help message");

        std::string default_config;

#ifdef CONFIG_FILE
        std::cout << "CONFIG_FILE: "<< CONFIG_FILE << std::endl;
        default_config = std::string(CONFIG_FILE);
#else
        default_config = std::string("");
#endif
        if(default_config.length() != 0)
        {
            gen_opts->add_options()
                    ("config,C", boost::program_options
                            ::value<std::string>()->default_value(default_config)
                    , "path to configuration file");
        }
        else
        {
            gen_opts->add_options()
                    ("config,C", boost::program_options
                            ::value<std::string>(), "path to configuration file");
        }

        return gen_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
       boost::shared_ptr<boost::program_options::options_description>
           gen_opts(get_options_description());

        typedef std::vector<std::string> string_vector_t;
        string_vector_t to_pass_further;//args

        boost::program_options::variables_map gen_vm;
        boost::program_options::parsed_options gen_parsed
            = boost::program_options::command_line_parser(argc, argv)
            .options(*gen_opts).allow_unregistered().run();
        boost::program_options::store(gen_parsed, gen_vm);

        to_pass_further
            = boost::program_options::collect_unrecognized(gen_parsed.options
                    , boost::program_options::include_positional);
        boost::program_options::notify(gen_vm);

        //general config actions
        if (gen_vm.count("help"))
        {
            std::cout << std::endl;
            for(PoDescs::iterator it = po_description.begin(); it != po_description.end(); ++it)
            {
                std::cout << **it << std::endl;
            }
            throw ReturnFromMain("help called");
        }

        //return other args
        fa.clear();//to be sure that fa is empty
        //new number of args + first program name
        fa.prealocate_for_argc(to_pass_further.size() + 1);
        fa.add_argv(argv[0]);//program name copy
        for(string_vector_t::const_iterator i = to_pass_further.begin()
                ; i != to_pass_further.end(); ++i)
        {//copying a new arg vector
            fa.add_argv(*i);//string
        }//for i

        //read config file if configured and append content to fa
        if (gen_vm.count("config,C"))
        {
            std::string fname = gen_vm["config,C"].as<std::string>();
            if(fname.length())
                parse_config_file_to_faked_args(fname, fa );
        }

    }//handle
};

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
        boost::shared_ptr<boost::program_options::options_description>
            db_opts(get_options_description());

        boost::program_options::variables_map vm;
        boost::program_options::parsed_options parsed
            = boost::program_options::command_line_parser(argc,argv)
                .options(*db_opts).allow_unregistered().run();
        boost::program_options::store(parsed, vm);

        typedef std::vector<std::string> string_vector_t;
        string_vector_t to_pass_further;//args

        to_pass_further
            = boost::program_options::collect_unrecognized(parsed.options
                    , boost::program_options::include_positional);
        boost::program_options::notify(vm);

        //faked args for unittest framework returned by reference in params
        fa.clear();//to be sure that fa is empty
        fa.prealocate_for_argc(to_pass_further.size() + 1);//new number of args + first program name
        fa.add_argv(argv[0]);//program name copy
        for(string_vector_t::const_iterator i = to_pass_further.begin()
                ; i != to_pass_further.end(); ++i)
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

    }//handle
};



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
                            ::value<unsigned int>()->default_value(300), "number of threads in group")
                ("thread_group_divisor", boost::program_options
                            ::value<unsigned int>()->default_value(10), "designates fraction of non-synchronized threads");
        return thread_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::shared_ptr<boost::program_options::options_description>
        thread_opts(get_options_description());

        boost::program_options::variables_map vm;
        boost::program_options::parsed_options parsed
            = boost::program_options::command_line_parser(argc,argv)
                .options(*thread_opts).allow_unregistered().run();
        boost::program_options::store(parsed, vm);

        typedef std::vector<std::string> string_vector_t;
        string_vector_t to_pass_further;//args

        to_pass_further
            = boost::program_options::collect_unrecognized(parsed.options
                    , boost::program_options::include_positional);
        boost::program_options::notify(vm);

        //faked args for unittest framework returned by reference in params
        fa.clear();//to be sure that fa is empty
        fa.prealocate_for_argc(to_pass_further.size() + 1);//new number of args + first program name
        fa.add_argv(argv[0]);//program name copy
        for(string_vector_t::const_iterator i = to_pass_further.begin()
                ; i != to_pass_further.end(); ++i)
        {//copying a new arg vector
            fa.add_argv(*i);//string
        }//for i

        thread_number = (vm.count("thread_number") == 0
                ? 300 : vm["thread_number"].as<unsigned>());
        thread_group_divisor = (vm.count("thread_group_divisor") == 0
                ? 10 : vm["thread_group_divisor"].as<unsigned>());
    }//handle
};



#endif //FAKED_ARGS_H_
