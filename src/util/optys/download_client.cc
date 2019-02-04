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
#include <set>
#include <functional>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <sys/stat.h>
#include <errno.h>
#include <memory>
#include <boost/format.hpp>
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "libfred/opcontext.hh"

#include "src/util/subprocess.hh"
#include "util/printable.hh"
#include "util/util.hh"

#include "src/util/csv_parser.hh"

#include "config.h"

#ifdef HAVE_LOGGER
#include "util/log/logger.hh"
#endif

#include "src/util/optys/download_client.hh"


    OptysDownloadClient::OptysDownloadClient(const std::string& host, const std::string& port, const std::string& user, const std::string& local_download_dir, const std::string& remote_data_dir)
    : host_(host)
    , port_(port)
    , user_(user)
    , local_download_dir_(local_download_dir)
    , remote_data_dir_(remote_data_dir)
    {
        if(!boost::filesystem::exists(local_download_dir_) || !boost::filesystem::is_directory(local_download_dir_))
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

    std::set<std::string> get_all_csv_file_names(const std::string& local_download_dir)
    {
        std::set<std::string> file_names_csv;
        if(boost::filesystem::exists(local_download_dir) && boost::filesystem::is_directory(local_download_dir))
        {
            for(boost::filesystem::directory_iterator di (local_download_dir); di != boost::filesystem::directory_iterator(); ++di)
            {
                if(!boost::filesystem::is_directory(boost::filesystem::path(*di))
                    && boost::filesystem::is_regular_file(boost::filesystem::path(*di))
                    && boost::iends_with(boost::filesystem::path(*di).string(), std::string(".csv")))
                {
                    file_names_csv.insert((*di).path().filename().string());
                }
            }
        }
        else
        {
            throw std::runtime_error(std::string("local_download_dir: " + local_download_dir +" not found"));
        }
        return file_names_csv;
    }

    void process_undelivered_messages_data(const std::string& local_download_dir, const std::set<std::string>& downloaded_data_filenames)
    {
        std::string err_msg;
        //process csv files
        for(std::set<std::string>::const_iterator ci = downloaded_data_filenames.begin(); ci != downloaded_data_filenames.end(); ++ci)
        {
            std::set<unsigned long long> file_message_id_set;
            //open csv file
            std::ifstream csv_file_stream;
            std::string local_csv_file_name = local_download_dir + "/" + (*ci);
            LibFred::OperationContextCreator ctx;
            try
            {
                csv_file_stream.open (local_csv_file_name.c_str(), std::ios::in);
                if(!csv_file_stream.is_open())
                {
                    throw std::runtime_error("unable to open file");
                }

                //read csv file
                std::string csv_file_content;//buffer
                csv_file_stream.seekg (0, std::ios::end);
                long long csv_file_length = csv_file_stream.tellg();
                csv_file_stream.seekg (0, std::ios::beg);//reset

                //allocate csv file buffer
                csv_file_content.resize(csv_file_length);

                //read csv file
                csv_file_stream.read(&csv_file_content[0], csv_file_content.size());

                //remove UTF-8 BOM
                if (boost::starts_with(csv_file_content, "\xEF\xBB\xBF"))
                {
                    csv_file_content.erase(0,3);
                }

                //parse csv
                std::vector<std::vector<std::string> > csv_data = Util::CsvParser(csv_file_content).parse();

                //std::cerr << Util::format_csv_data(csv_data) << std::endl;

                for(std::vector<std::vector<std::string> >::const_iterator idci = csv_data.begin(); idci != csv_data.end(); ++idci)
                {
                    //ignore empty row, empty row contains empty field
                    if((idci->size() > 0) && (idci->at(0).size() > 0))
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
                        file_message_id_set.insert(id);
                    }
                }

                //set undelivered state
                if(!file_message_id_set.empty())
                {
                    LibFred::OperationContextCreator file_ctx;
                    Database::query_param_list params;//query params
                    std::string message_state_undelivered_query(
                    "UPDATE message_archive SET status_id = (SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
                    " WHERE status_id IN (SELECT id FROM enum_send_status WHERE status_name = 'sent' OR status_name = 'undelivered') "
                    " AND service_handle = 'OPTYS' AND (");
                    Util::HeadSeparator id_or_separator("id = $", " OR id = $");
                    for(std::set<unsigned long long>::const_iterator id_ci = file_message_id_set.begin();
                        id_ci != file_message_id_set.end(); ++id_ci)
                    {
                        message_state_undelivered_query += id_or_separator.get();
                        message_state_undelivered_query += params.add(*id_ci);
                        message_state_undelivered_query += "::bigint";
                    }
                    message_state_undelivered_query += ") RETURNING id";

                    Database::Result msg_id_res = file_ctx.get_conn().exec_params(message_state_undelivered_query, params);

                    std::set<unsigned long long> updated_msg_id_set;
                    for(unsigned long long i = 0; i < msg_id_res.size(); ++i)
                    {
                        updated_msg_id_set.insert(static_cast<unsigned long long>(msg_id_res[i]["id"]));
                    }

                    if(updated_msg_id_set == file_message_id_set)
                    {
                        file_ctx.commit_transaction();
                    }
                    else
                    {
                        std::set<unsigned long long> failed_msg_id_set;//not updated message_archive.id set from current file
                        std::set_difference(file_message_id_set.begin(), file_message_id_set.end(),
                            updated_msg_id_set.begin(), updated_msg_id_set.end(),
                            std::inserter(failed_msg_id_set, failed_msg_id_set.end()));

                        std::string err_msg("failed to set 'undelivered' state on letter id:");

                        for(std::set<unsigned long long>::const_iterator id_ci = failed_msg_id_set.begin();
                            id_ci != failed_msg_id_set.end(); ++id_ci)
                        {
                            err_msg += " ";
                            err_msg += boost::lexical_cast<std::string>(*id_ci);
                        }

                        throw std::runtime_error(err_msg);
                    }
                }
                else
                {
                    throw std::runtime_error("no undelivered letter id found");
                }
            }
            catch(const std::exception& ex)
            {
                err_msg += (boost::format(" file: %1% err: %2%") % local_csv_file_name % ex.what()).str();
            }
        }
        if(!err_msg.empty()) throw std::runtime_error(err_msg);
    }

    std::set<std::string> OptysDownloadClient::download()
    {
        const std::string ssh_account = user_ + "@" + host_;
        Cmd::Executable download_command("rsync");
        download_command("--include=*.csv")("--exclude=*")("-e")("ssh -p " + port_)("-ai")
            ("--out-format=%n")("--inplace")(ssh_account + ":" + remote_data_dir_ + "/")
            (local_download_dir_);

        std::set<std::string> downloaded_data_filenames;
        std::set<std::string> current_downloaded_data_filenames;
        int rsync_retry_count = 0;
        enum { COMMAND_LIFETIME_MAX = 3600 };// 3600s = 1hour
        do
        {
            LOGGER.debug("download_command: rsync");
            const SubProcessOutput output = download_command.run_with_path(COMMAND_LIFETIME_MAX);
            if (!output.stderr.empty() || !output.succeeded())
            {
                throw std::runtime_error("download command: rsync failed: " + output.stderr);
            }
            current_downloaded_data_filenames.clear();
            current_downloaded_data_filenames = downloaded_csv_data_filenames_parser(output.stdout);
            downloaded_data_filenames.insert(current_downloaded_data_filenames.begin(), current_downloaded_data_filenames.end());
            sleep(1);//wait for changes of remote data
            ++rsync_retry_count;
            enum { RETRY_COUNT_MAX = 30 };
            if (RETRY_COUNT_MAX < rsync_retry_count) {
                throw std::runtime_error("remote data still changing");
            }
        }
        while(current_downloaded_data_filenames.size() != 0);

        for(std::set<std::string>::const_iterator ci = downloaded_data_filenames.begin(); ci != downloaded_data_filenames.end(); ++ci)
        {
            //remove downloaded file from optys server
            const std::string remote_command = "rm -f " + remote_data_dir_ +  "/" + (*ci);
            const std::string local_command = "ssh \"" + ssh_account + "\" \"" + remote_command + "\"";

            LOGGER.debug("remove_downloaded_file_command: " + local_command);

            const SubProcessOutput output = Cmd::Executable("ssh")(ssh_account)(remote_command)
                                                .run_with_path(COMMAND_LIFETIME_MAX);
            if (!output.stderr.empty() || !output.succeeded())
            {
                throw std::runtime_error("remove command: " + local_command + " failed: " + output.stderr);
            }
        }

        return downloaded_data_filenames;
    }

