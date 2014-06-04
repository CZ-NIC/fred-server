/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  @file
 *  domain browser backend config
 */

#ifndef HANDLE_DOMAINBROWSER_ARGS_H_
#define HANDLE_DOMAINBROWSER_ARGS_H_

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"

namespace po = boost::program_options;

/**
 * Domain browser backend config
 */
class HandleDomainBrowserArgs : public HandleArgs
{
public:
    unsigned int domain_list_limit;
    unsigned int nsset_list_limit;
    unsigned int keyset_list_limit;
    unsigned int contact_list_limit;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Domain browser server options")));

        cfg_opts->add_options()
            ("domain_browser.list_domains_limit",
                po::value<unsigned int>()->default_value(5000),"domain list chunk size")
            ("domain_browser.list_nssets_limit",
                po::value<unsigned int>()->default_value(5000),"nsset list chunk size")
            ("domain_browser.list_keysets_limit",
                po::value<unsigned int>()->default_value(5000),"keyset list chunk size")
            ("domain_browser.list_contacts_limit",
                po::value<unsigned int>()->default_value(5000),"contact list chunk size")
            ;

        return cfg_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        domain_list_limit = vm["domain_browser.list_domains_limit"].as<unsigned int>();
        nsset_list_limit = vm["domain_browser.list_nssets_limit"].as<unsigned int>();
        keyset_list_limit = vm["domain_browser.list_keysets_limit"].as<unsigned int>();
        contact_list_limit = vm["domain_browser.list_contacts_limit"].as<unsigned int>();
    }//handle
};

#endif //HANDLE_DOMAINBROWSER_ARGS_H_
