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
 *  optys undelivered mail notification download client header
 */

#ifndef DOWNLOAD_CLIENT_H_406c9d815fff40afa7345c85ff4238f9
#define DOWNLOAD_CLIENT_H_406c9d815fff40afa7345c85ff4238f9

#include <vector>
#include <set>
#include <string>
#include <utility>
#include <stdexcept>


#include "src/fredlib/messages/messages_impl.h"
#include "src/fredlib/file.h"
#include "util/db/nullable.h"

/**
 * Download data about letters from Optys.
 * Download csv files from Optys server save them in our download directory.
 * Return undelivered letter ids.
 */
class OptysDownloadClient
{
    const std::string host_;
    const std::string port_;
    const std::string user_;
    const std::string local_download_dir_;
    const std::string remote_data_dir_;

public:
    OptysDownloadClient(const std::string& host, const std::string& port, const std::string& user, const std::string& local_download_dir, const std::string& remote_data_dir);
    std::set<unsigned long long> download(); //return undelivered message archive id set or throw exception
};

std::set<std::string> downloaded_csv_data_filenames_parser(const std::string& formated_rsync_stdout);


#endif
