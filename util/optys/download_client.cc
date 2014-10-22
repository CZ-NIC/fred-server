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
#include <functional>
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
#include <boost/algorithm/string.hpp>

#include "util/subprocess.h"
#include "util/printable.h"

#include "util/csv_parser.h"

#include "download_client.h"


    OptysDownloadClient::OptysDownloadClient(const std::string& host, const std::string& port, const std::string& user, const std::string& local_download_dir, const std::string& remote_data_dir)
    : host_(host)
    , port_(port)
    , user_(user)
    , local_download_dir_(local_download_dir)
    , remote_data_dir_(remote_data_dir)
    {
        if(!boost::filesystem::exists(local_download_dir_))
        {
            throw std::runtime_error(std::string("local_download_dir: " + local_download_dir_ +" not found"));
        }
    }

    std::set<std::string> downloaded_csv_data_filenames_parser(const std::string& formated_rsync_stdout)
    {
        std::set<std::string> downloaded_data_filenames1;
        boost::split(downloaded_data_filenames1,formated_rsync_stdout,boost::is_any_of("\n"));
        std::string datafile_suffix(".csv");
        std::set<std::string> downloaded_data_filenames2;
        for(std::set<std::string>::const_iterator ci = downloaded_data_filenames1.begin()
            ; ci != downloaded_data_filenames1.end(); ++ci)
        {
            if((*ci).length() > datafile_suffix.length()
                && boost::iends_with(*ci,datafile_suffix))
            {
                downloaded_data_filenames2.insert(*ci);
            }
        }
        return downloaded_data_filenames2;
    }

    std::set<unsigned long long> OptysDownloadClient::download()
    {
        std::set<unsigned long long> message_id_set;

        std::string download_command = std::string("rsync --include=\"*.csv\" --exclude=\"*\"")
            + " -e \"ssh -p "+ port_ + "\" -ai --out-format=\"%n\" --inplace "
            + user_ + "@" + host_ + ":" + remote_data_dir_ + " " + local_download_dir_;

        std::set<std::string> downloaded_data_filenames;
        std::set<std::string> current_downloaded_data_filenames;
        int rsync_retry_count = 0;
        do
        {
            SubProcessOutput output = ShellCmd(download_command.c_str(), 3600).execute();
            if (!output.stderr.empty() || !output.is_exited() || (output.get_exit_status() != EXIT_SUCCESS))
            {
                throw std::runtime_error(std::string("download command: " + download_command +" failed: ")+output.stderr);
            }
            current_downloaded_data_filenames.clear();
            current_downloaded_data_filenames = downloaded_csv_data_filenames_parser(output.stdout);
            downloaded_data_filenames.insert(current_downloaded_data_filenames.begin(), current_downloaded_data_filenames.end());
            sleep(1);//wait for changes of remote data
            ++rsync_retry_count;
            if(rsync_retry_count > 30) throw std::runtime_error("remote data still changing");
        }
        while(current_downloaded_data_filenames.size() != 0);

        //process csv files
        for(std::set<std::string>::const_iterator ci = downloaded_data_filenames.begin(); ci != downloaded_data_filenames.end(); ++ci)
        {
            //open csv file
            std::ifstream csv_file_stream;
            std::string local_csv_file_name = local_download_dir_ +"/" + (*ci);
            csv_file_stream.open (local_csv_file_name.c_str(), std::ios::in);
            if(!csv_file_stream.is_open()) throw std::runtime_error("csv_file_stream.open failed, "
                "unable to open file: "+ local_csv_file_name);

            //read csv file
            std::string csv_file_content;//buffer
            csv_file_stream.seekg (0, std::ios::end);
            long long csv_file_length = csv_file_stream.tellg();
            csv_file_stream.seekg (0, std::ios::beg);//reset

            //allocate csv file buffer
            csv_file_content.resize(csv_file_length);

            //read csv file
            csv_file_stream.read(&csv_file_content[0], csv_file_content.size());

            //parse csv
            std::vector<std::vector<std::string> > csv_data = Util::CsvParser(csv_file_content).parse();
            for(std::vector<std::vector<std::string> >::const_iterator idci = csv_data.begin(); idci != csv_data.end(); ++idci)
            {
                unsigned long long id = 0;
                try
                {
                    id = boost::lexical_cast<unsigned long long>(idci->at(0));
                }
                catch(...)
                {
                    throw std::runtime_error("invalid csv data");
                }
                message_id_set.insert(id);
            }

            //remove downloaded file from optys server
            std::string remove_downloaded_file_command = std::string("ssh ") + user_ + "@" + host_ + " \"rm -f " + remote_data_dir_ +  "/" + (*ci) + "\"";
            SubProcessOutput output = ShellCmd(remove_downloaded_file_command.c_str(), 3600).execute();
            if (!output.stderr.empty() || !output.is_exited() || (output.get_exit_status() != EXIT_SUCCESS))
            {
                throw std::runtime_error(std::string("remove command: " + remove_downloaded_file_command +" failed: ")+output.stderr);
            }
        }

        return message_id_set;
    }

