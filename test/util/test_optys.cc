/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <fstream>
#include <ios>
#include <iterator>
#include <iomanip>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "config.h"
#include "libfred/opcontext.hh"
#include "src/bin/cli/optys_get_undelivered.hh"
#include "src/bin/cli/read_config_file.hh"
#include "util/util.hh"
#include "util/map_at.hh"
#include "src/util/subprocess.hh"
#include "util/printable.hh"
#include "src/util/optys/download_client.hh"
#include "src/util/optys/handle_optys_mail_args.hh"
#include "test/setup/fixtures.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"
#include <boost/test/unit_test.hpp>
#include <utility>

BOOST_AUTO_TEST_SUITE(TestOptys)

const std::string server_name = "test-optys";


/**
 * test csv_parser
 */
BOOST_AUTO_TEST_CASE(test_csv_file_name)
{
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
        "test0.csv\n./\ntest1.csv\ntest2.csv"),"|") ==
        "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
        "test0.csv\n./\ntest1.csv\ntest2.csv\n"),"|") ==
        "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\ntest0.csv\n./\ntest1.csv\ntest2.csv"),"|") ==
            "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "test0.csv\n./\ntest1.csv\n\ntest2.csv"),"|") ==
            "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\n./\ntest0.csv\ntest1.csv\ntest2.csv"),"|") ==
            "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\n./\ntest0.csv\ntest1.csv\ntest2.csv\n"),"|") ==
            "test0.csv|test1.csv|test2.csv");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\n./\n"),"|") ==
            "");
    BOOST_CHECK(Util::format_container(downloaded_csv_data_filenames_parser(
            "\n./"),"|") ==
            "");
}

/**
 * fake some 'sent' letters into message_archive and csv data of undelivered letters
 */
struct undelivered_fixture : virtual Test::instantiate_db_template
{
    std::set<unsigned long long> msg_id_set;
    std::string msg_id_str;
    std::string test_csv_data;
    std::string local_download_dir;

    undelivered_fixture()
    {
        ::LibFred::OperationContextCreator ctx;
        Database::Result msg_id_res = ctx.get_conn().exec(
        "INSERT INTO message_archive (status_id, comm_type_id, message_type_id, service_handle) "
        " VALUES "

        " ((SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " , (SELECT id FROM comm_type WHERE type = 'letter') "
        " , (SELECT id FROM message_type WHERE type = 'mojeid_pin3'), 'OPTYS'), "

        " ((SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
        " , (SELECT id FROM comm_type WHERE type = 'letter') "
        " , (SELECT id FROM message_type WHERE type = 'mojeid_pin3'), 'OPTYS'), "

        " ((SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " , (SELECT id FROM comm_type WHERE type = 'letter') "
        " , (SELECT id FROM message_type WHERE type = 'mojeid_pin3'), 'OPTYS'), "

        " ((SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
        " , (SELECT id FROM comm_type WHERE type = 'letter') "
        " , (SELECT id FROM message_type WHERE type = 'mojeid_pin3'), 'OPTYS'), "

        " ((SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " , (SELECT id FROM comm_type WHERE type = 'letter') "
        " , (SELECT id FROM message_type WHERE type = 'mojeid_pin3'), 'OPTYS'), "

        " ((SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
        " , (SELECT id FROM comm_type WHERE type = 'letter') "
        " , (SELECT id FROM message_type WHERE type = 'mojeid_pin3'), 'OPTYS'), "

        " ((SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " , (SELECT id FROM comm_type WHERE type = 'letter') "
        " , (SELECT id FROM message_type WHERE type = 'mojeid_pin3'), 'OPTYS'), "

        " ((SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " , (SELECT id FROM comm_type WHERE type = 'letter') "
        " , (SELECT id FROM message_type WHERE type = 'mojeid_pin3'), 'OPTYS'), "

        " ((SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " , (SELECT id FROM comm_type WHERE type = 'letter') "
        " , (SELECT id FROM message_type WHERE type = 'mojeid_pin3'), 'OPTYS') "

        " RETURNING id "
        );

        for(unsigned long long i = 0 ; i < msg_id_res.size(); ++i)
        {
            msg_id_set.insert(static_cast<unsigned long long>(msg_id_res[i]["id"]));
            msg_id_str += " " + static_cast<std::string>(msg_id_res[i]["id"]);
        }

        ctx.commit_transaction();
        BOOST_TEST_MESSAGE(msg_id_str);
        BOOST_REQUIRE(msg_id_set.size() == 9);

        //msg id 0 - 6 undelivered, 7 and 8 delivered
        test_csv_data = (boost::format(
            "%1%" "\x3B" "\x4E" "\x61" "\x20" "\x75" "\x76" "\x65" "\x64" "\x65" "\x6E" "\xE9" "\x20" "\x61" "\x64" "\x72" "\x65"
                "\x73" "\x65" "\x20" "\x6E" "\x65" "\x7A" "\x6E" "\xE1" "\x6D" "\xFD" "\x0D" "\x0A"
            "%2%" "\x3B" "\x4E" "\x65" "\x76" "\x79" "\x7A" "\x76" "\x65" "\x64" "\x6E" "\x75" "\x74" "\x6F" "\x0D" "\x0A"
            "%3%" "\x3B" "\x4F" "\x64" "\x73" "\x74" "\xEC" "\x68" "\x6F" "\x76" "\x61" "\x6C" "\x20" "\x73" "\x65" "\x0D" "\x0A"
            "%4%" "\x3B" "\x5A" "\x65" "\x6D" "\xF8" "\x65" "\x6C" "\x0D" "\x0A"
            "%5%" "\x3B" "\x41" "\x64" "\x72" "\x65" "\x73" "\x61" "\x20" "\x6E" "\x65" "\x64" "\x6F" "\x73" "\x74" "\x61" "\x74"
                "\x65" "\xE8" "\x6E" "\xE1" "\x0D" "\x0A"
            "%6%" "\x3B" "\x4E" "\x65" "\x70" "\xF8" "\x69" "\x6A" "\x61" "\x74" "\x6F" "\x20" "\x20" "\x0D" "\x0A"
            "%7%" "\x3B" "\x4A" "\x69" "\x6E" "\xFD" "\x20" "\x64" "\xF9" "\x76" "\x6F" "\x64" "\x0D"
            )
            % Util::get_nth(msg_id_set,0)
            % Util::get_nth(msg_id_set,1)
            % Util::get_nth(msg_id_set,2)
            % Util::get_nth(msg_id_set,3)
            % Util::get_nth(msg_id_set,4)
            % Util::get_nth(msg_id_set,5)
            % Util::get_nth(msg_id_set,6)
            ).str();

        //optys config
        std::map<std::string, std::string> set_cfg = Admin::read_config_file<HandleOptysUndeliveredArgs>(std::string(OPTYS_CONFIG),
                CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump);
        local_download_dir = map_at(set_cfg, "local_download_dir");
        BOOST_TEST_MESSAGE(local_download_dir);

        boost::filesystem::path local_download_dir_path(local_download_dir.c_str());
        if(!boost::filesystem::exists(local_download_dir))
        {
            SubProcessOutput output = ShellCmd("mkdir -p " + local_download_dir, 3600).execute();
            BOOST_REQUIRE_MESSAGE(output.stderr.empty() && output.is_exited() && (output.get_exit_status() == EXIT_SUCCESS),
                std::string("local download directory creation failed: ")+output.stderr);
        }
        BOOST_REQUIRE(boost::filesystem::is_directory(local_download_dir));

        //download directory cleanup
        SubProcessOutput output = ShellCmd("rm -f " + local_download_dir+"/*.csv", 3600).execute();
        BOOST_REQUIRE_MESSAGE(output.stderr.empty() && output.is_exited() && (output.get_exit_status() == EXIT_SUCCESS),
            std::string("download directory cleanup failed: ")+output.stderr);

        //write test csv data file
        std::ofstream csv_file;
        csv_file.open((local_download_dir+"/CZ.NIC_20140717.csv").c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
        BOOST_REQUIRE(csv_file.is_open());
        csv_file.write(test_csv_data.c_str(), test_csv_data.size());
    }

    ~undelivered_fixture(){}
};

/**
 * test processing undelivered letters, good path
 */
BOOST_FIXTURE_TEST_CASE(test_undelivered_proc, undelivered_fixture)
{
    BOOST_CHECK_NO_THROW(
    Admin::notify_letters_optys_get_undelivered_impl(
        std::string(OPTYS_CONFIG),
        true,//all_local_files_only
        CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump
    ));


    //check undelivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 7);

    //check delivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,7))
        (Util::get_nth(msg_id_set,8))
    ).size() == 2);
}

/**
 * test ignore empty lines, good path
 */
BOOST_FIXTURE_TEST_CASE(test_ignore_empty_line, undelivered_fixture)
{
    //insert empty lines
    SubProcessOutput output = ShellCmd(
        "sed -i 's/Nevyzvednuto/Nevyzvednuto\\n\\n/' " + local_download_dir + "/CZ.NIC_20140717.csv"
        " && echo >> " + local_download_dir + "/CZ.NIC_20140717.csv"
        " && echo >> " + local_download_dir + "/CZ.NIC_20140717.csv"
        " && mv " + local_download_dir + "/CZ.NIC_20140717.csv " + local_download_dir + "/CZ.NIC_20140717.csv_"
        " && echo > " + local_download_dir + "/CZ.NIC_20140717.csv"
        " && echo >> " + local_download_dir + "/CZ.NIC_20140717.csv"
        " && cat " + local_download_dir + "/CZ.NIC_20140717.csv_ >> " + local_download_dir + "/CZ.NIC_20140717.csv"
        " && rm -f " + local_download_dir + "/CZ.NIC_20140717.csv_"
        , 3600).execute();
    BOOST_REQUIRE_MESSAGE(output.stderr.empty() && output.is_exited() && (output.get_exit_status() == EXIT_SUCCESS),
        std::string("failed to insert empty lines: ")+output.stderr);

    BOOST_CHECK_NO_THROW(
    Admin::notify_letters_optys_get_undelivered_impl(
        std::string(OPTYS_CONFIG),
        true,//all_local_files_only
        CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump
    ));

    //check undelivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 7);

    //check delivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,7))
        (Util::get_nth(msg_id_set,8))
    ).size() == 2);
}

/**
 * test config file not found
 */
BOOST_AUTO_TEST_CASE(test_bad_config_file)
{
    BOOST_CHECK_THROW(
    Admin::notify_letters_optys_get_undelivered_impl(
        std::string(OPTYS_CONFIG)+"_bad",
        true,//all_local_files_only
        CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump
        ), std::runtime_error);
}

/**
 * test local download dir not found
 */
BOOST_FIXTURE_TEST_CASE(test_local_download_dir_not_found, undelivered_fixture)
{
    //remove download directory
    SubProcessOutput output = ShellCmd("rm -rf " + local_download_dir, 3600).execute();
    BOOST_REQUIRE_MESSAGE(output.stderr.empty() && output.is_exited() && (output.get_exit_status() == EXIT_SUCCESS),
        std::string("removal of download directory failed: ")+output.stderr);

    BOOST_CHECK_THROW(
    Admin::notify_letters_optys_get_undelivered_impl(
        std::string(OPTYS_CONFIG),
        true,//all_local_files_only
        CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump
        ), std::runtime_error);
}

/**
 * test overcoming unable to read file
 */
BOOST_FIXTURE_TEST_CASE(test_unable_to_read_file, undelivered_fixture)
{
    //remove read permission
    SubProcessOutput output = ShellCmd("chmod u-r " + local_download_dir + "/*.csv", 3600).execute();
    BOOST_REQUIRE_MESSAGE(output.stderr.empty() && output.is_exited() && (output.get_exit_status() == EXIT_SUCCESS),
        std::string("removal of read permission failed: ")+output.stderr);

    BOOST_CHECK_THROW(
    Admin::notify_letters_optys_get_undelivered_impl(
        std::string(OPTYS_CONFIG),
        true,//all_local_files_only
        CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump
    ), std::runtime_error);

    //check undelivered not set
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 4);

    //check undelivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 3);

    //check delivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,7))
        (Util::get_nth(msg_id_set,8))
    ).size() == 2);

}

/**
 * test overcoming bad csv data, id not a number, skiping the file
 */
BOOST_FIXTURE_TEST_CASE(test_bad_csv_data_file, undelivered_fixture)
{
    //not a number id in csv data
    SubProcessOutput output = ShellCmd("echo \"badiddata;test\" > " + local_download_dir + "/CZ.NIC_20140717.csv", 3600).execute();
    BOOST_REQUIRE_MESSAGE(output.stderr.empty() && output.is_exited() && (output.get_exit_status() == EXIT_SUCCESS),
        std::string("making not a number id in csv data failed: ")+output.stderr);

    BOOST_CHECK_THROW(
    Admin::notify_letters_optys_get_undelivered_impl(
        std::string(OPTYS_CONFIG),
        true,//all_local_files_only
        CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump
    ), std::runtime_error);

    //check undelivered not set
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 4);

    //check undelivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 3);

    //check delivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,7))
        (Util::get_nth(msg_id_set,8))
    ).size() == 2);
}

/**
 * test overcoming wrong id in csv data
 */
BOOST_FIXTURE_TEST_CASE(test_wrong_id_in_csv_data, undelivered_fixture)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ctx.get_conn().exec("UPDATE message_archive SET service_handle = 'MANUAL' WHERE service_handle = 'OPTYS'");
        ctx.commit_transaction();
    }

    BOOST_CHECK_THROW(
    Admin::notify_letters_optys_get_undelivered_impl(
        std::string(OPTYS_CONFIG),
        true,//all_local_files_only
        CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump
    ), std::runtime_error);

    {
        ::LibFred::OperationContextCreator ctx;
        ctx.get_conn().exec("UPDATE message_archive SET service_handle = 'OPTYS' WHERE service_handle = 'MANUAL'");
        ctx.commit_transaction();
    }

    //check undelivered not set
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 4);

    //check undelivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 3);

    //check delivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,7))
        (Util::get_nth(msg_id_set,8))
    ).size() == 2);

}

/**
 * test overcoming empty csv file
 */
BOOST_FIXTURE_TEST_CASE(test_empty_csv_file, undelivered_fixture)
{
    //make csv file empty
    SubProcessOutput output = ShellCmd("echo > " + local_download_dir + "/CZ.NIC_20140717.csv", 3600).execute();
    BOOST_REQUIRE_MESSAGE(output.stderr.empty() && output.is_exited() && (output.get_exit_status() == EXIT_SUCCESS),
        std::string("making csv file empty failed: ")+output.stderr);

    BOOST_CHECK_THROW(
    Admin::notify_letters_optys_get_undelivered_impl(
        std::string(OPTYS_CONFIG),
        true,//all_local_files_only
        CfgArgs::instance()->get_handler_ptr_by_type<HandleLoggingArgs>()->log_config_dump
    ), std::runtime_error);

    //check undelivered not set
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 4);

    //check undelivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'undelivered') "
        " AND (id = $1::bigint OR id = $2::bigint OR id = $3::bigint OR id = $4::bigint OR id = $5::bigint OR id = $6::bigint OR id = $7::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,0))
        (Util::get_nth(msg_id_set,1))
        (Util::get_nth(msg_id_set,2))
        (Util::get_nth(msg_id_set,3))
        (Util::get_nth(msg_id_set,4))
        (Util::get_nth(msg_id_set,5))
        (Util::get_nth(msg_id_set,6))
    ).size() == 3);

    //check delivered
    BOOST_CHECK(::LibFred::OperationContextCreator().get_conn().exec_params(
        "SELECT id FROM message_archive WHERE service_handle = 'OPTYS' "
        " AND status_id = (SELECT id FROM enum_send_status WHERE status_name = 'sent') "
        " AND (id = $1::bigint OR id = $2::bigint)"
        ,Database::query_param_list
        (Util::get_nth(msg_id_set,7))
        (Util::get_nth(msg_id_set,8))
    ).size() == 2);

}


BOOST_AUTO_TEST_SUITE_END();
