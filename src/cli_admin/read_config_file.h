/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef READ_CONFIG_FILE_H_
#define READ_CONFIG_FILE_H_

#include <map>
#include <string>

#include <boost/assign/list_of.hpp>

#include "cfg/faked_args.h"
#include "cfg/handle_args.h"
//#include "cfg/config_handler_decl.h"
#include "cfg/handle_general_args.h"

namespace Admin {

/**
 * read extra config file into key-value config map
 */
template <class HANDLE_ARGS> std::map<std::string, std::string> readConfigFile(const std::string &conf_file)
{
        boost::shared_ptr<HANDLE_ARGS> handle_args_ptr(new HANDLE_ARGS);

        HandlerPtrVector hpv =
            boost::assign::list_of
                (HandleArgsPtr(new HandleConfigFileArgs(conf_file) ))
                (HandleArgsPtr(handle_args_ptr));

        // it always needs some argv vector, argc cannot be 0
        FakedArgs fa;
        fa.add_argv(std::string(""));

        //handle
        for(HandlerPtrVector::const_iterator i = hpv.begin()
                ; i != hpv.end(); ++i )
        {
            FakedArgs fa_out;
            (*i)->handle( fa.get_argc(), fa.get_argv(), fa_out);
            fa=fa_out;//last output to next input
        }//for HandlerPtrVector

        std::map<std::string, std::string> set_cfg = handle_args_ptr->get_map();

        return set_cfg;
}

} // namespace Admin;

#endif // READ_CONFIG_FILE_H_

