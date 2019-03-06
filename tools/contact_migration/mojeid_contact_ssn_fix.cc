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
#include "libfred/db_settings.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "src/util/csv_parser.hh"

#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cerr << "Failure: expected 4 arguments\n"
                  << "Usage: " << argv[0] << " PQ_CONN_INFO LOGD_REQUEST_ID REGISTRAR_HANDLE CSV_INPUT_FILE" << std::endl;
        return EXIT_FAILURE;
    }
    try
    {
        const std::string conn_info(argv[1]);
        const unsigned long long logd_request_id = boost::lexical_cast<unsigned long long>(argv[2]);
        const std::string registrar_handle(argv[3]);
        const std::string file_name(argv[4]);

        Database::emplace_default_manager<Database::StandaloneManager>(conn_info);

        std::ifstream input_file_stream;
        input_file_stream.open(file_name.c_str(), std::ios::in);
        if (!input_file_stream.is_open())
        {
            throw std::runtime_error("unable to open input file");
        }
        std::string file_content;
        input_file_stream.seekg(0, std::ios::end);
        unsigned long long file_size = input_file_stream.tellg();
        input_file_stream.seekg(0, std::ios::beg);
        file_content.resize(file_size);
        input_file_stream.read(&file_content[0], file_content.size());

        typedef std::vector<std::vector<std::string> > Table;
        Table data = Util::CsvParser(file_content, ',').parse();

        LibFred::OperationContextCreator ctx;
        for (Table::const_iterator it = data.begin(); it != data.end(); ++it)
        {
            const unsigned long long contact_id = boost::lexical_cast<unsigned long long>(it->at(0));
            const std::string username = it->at(1);
            const std::string ssn_birth_date = it->at(2);

            std::cout << "id=" << contact_id << " username='" << username << "'" << std::endl;
            Database::Result to_fix = ctx.get_conn().exec_params(
               "SELECT"
                   " oreg.id, lower(oreg.name) AS name,"
                   " est.type as ssn_type, c.ssn,"
                   " os_v.id is not null AS validated,"
                   " os_m.id is not null AS is_mojeid"
                 " FROM object_registry oreg"
                   " JOIN contact c ON c.id = oreg.id"
                   " LEFT JOIN enum_ssntype est ON est.id = c.ssntype"
                   " LEFT JOIN object_state os_v ON os_v.object_id = c.id AND os_v.valid_to is null"
                       " AND os_v.state_id = (SELECT id FROM enum_object_states WHERE name = 'validatedContact')"
                   " LEFT JOIN object_state os_m ON os_m.object_id = c.id AND os_m.valid_to is null"
                       " AND os_m.state_id = (SELECT id FROM enum_object_states WHERE name = 'mojeidContact')"
                 " WHERE c.id = $1::bigint",
                 Database::query_param_list(contact_id)
            );

            if (to_fix.size() != 1)
            {
                std::cerr << "Warning: unable to get data for"
                          << " id=" << contact_id
                          << " username='" << username << "'" << std::endl;
                continue;
            }

            if (static_cast<std::string>(to_fix[0]["validated"]) == "t")
            {
                std::cerr << "Warning: already validated; skipping"
                          << " id=" << contact_id
                          << " username='" << username << "'" << std::endl;
                continue;
            }

            if (static_cast<std::string>(to_fix[0]["is_mojeid"]) == "f")
            {
                std::cerr << "Warning: not mojeid contact; skipping"
                          << " id=" << contact_id
                          << " username='" << username << "'" << std::endl;
                continue;
            }

            if (static_cast<std::string>(to_fix[0]["name"]) != username)
            {
                std::cerr << "OOPS: id from input data doesn't refer to same username!"
                          << " id=" << contact_id << ";"
                          << " aborting, check your input!" << std::endl;
                throw std::runtime_error("inconsistent input");
            }

            if (static_cast<std::string>(to_fix[0]["ssn_type"]) == "ICO")
            {
                std::cout << "Updating '" << static_cast<std::string>(to_fix[0]["name"]) << "'" << std::endl;

                LibFred::UpdateContactById cu(contact_id, registrar_handle);
                cu.set_logd_request_id(logd_request_id);
                cu.set_personal_id(LibFred::PersonalIdUnion::get_BIRTHDAY(ssn_birth_date));
                cu.exec(ctx);
            }
        }
        ctx.commit_transaction();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_SUCCESS;
    }
}
