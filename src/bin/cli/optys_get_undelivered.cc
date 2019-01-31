/*
 *  Copyright (C) 2014  CZ.NIC, z.s.p.o.
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

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>


#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_general_args.hh"

#include "util/map_at.hh"
#include "src/util/subprocess.hh"
#include "util/printable.hh"
#include "src/util/optys/handle_optys_mail_args.hh"
#include "src/util/optys/download_client.hh"

#include "src/bin/cli/read_config_file.hh"

using namespace Database;

namespace Admin {

void notify_letters_optys_get_undelivered_impl(
        const std::string& optys_config_file,
        bool all_local_files_only,
        bool dump_config_to_log)
{
    //optys config
    std::map<std::string, std::string> set_cfg = read_config_file<HandleOptysUndeliveredArgs>(optys_config_file,
            dump_config_to_log);
    std::string local_download_dir = map_at(set_cfg, "local_download_dir");

    std::set<std::string> file_names;

    if(!all_local_files_only)
    {
        file_names = OptysDownloadClient(map_at(set_cfg,"host"), map_at(set_cfg,"port"), map_at(set_cfg,"user")
            , local_download_dir
            , map_at(set_cfg,"remote_data_dir")).download();
    }
    else
    {
        file_names = get_all_csv_file_names(local_download_dir);
    }

    std::cout << "data file names:";
    for(std::set<std::string>::const_iterator ci = file_names.begin(); ci != file_names.end(); ++ci) std::cout << " " << (*ci);
    std::cout << std::endl;

    process_undelivered_messages_data(local_download_dir, file_names);
}
} // namespace Admin;

