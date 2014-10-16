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
 *  optys undelivered mail notification download client impl
 */

#include <string>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <sys/stat.h>
#include <errno.h>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "util/subprocess.h"
#include "util/printable.h"

#include "util/csv_parser.h"

#include "download_client.h"


    OptysDownloadClient::OptysDownloadClient(const std::string& host, int port, const std::string& user, const std::string& password, const std::string& download_dir)
    : host_(host)
    , port_(port)
    , user_(user)
    , password_(password)
    , download_dir_(download_dir)
    {
        if(!boost::filesystem::exists(download_dir))
        {
            throw std::runtime_error(std::string("download_dir: " + download_dir +" not found"));
        }
    }

    std::set<unsigned long long> OptysDownloadClient::download()
    {
        std::set<unsigned long long> message_id_set;

        return message_id_set;
    }

