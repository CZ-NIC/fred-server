/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/domain/create_domain.hh"

#include "libfred/object/object_state.hh"
#include "libfred/object/object_type.hh"

#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/command_selection_args.hh"
#include "src/util/cfg/handle_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_logging_args.hh"

#include "src/util/subprocess.hh"

#include <boost/assign/list_of.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

HandlerGrpVector help_gv = boost::assign::list_of(
        HandleGrpArgsPtr(
                new HandleHelpGrpArg("\nUsage: test_delete_objects_marked_as_delete_candidate <switches>\n")));

HandlerGrpVector config_gv = boost::assign::list_of(
        HandleGrpArgsPtr(new HandleConfigFileGrpArgs(CONFIG_FILE))) ;
HandlerGrpVector logging_gv = boost::assign::list_of(
        HandleGrpArgsPtr(new HandleLoggingArgsGrp));
HandlerGrpVector database_gv = boost::assign::list_of(
        HandleGrpArgsPtr(new HandleDatabaseArgsGrp));

const HandlerPtrGrid global_hpg = gv_list
        (help_gv)
        (config_gv)
        (logging_gv)
        (database_gv);

void create_domains(
            const std::string& registrar,
            const std::string& registrant,
            const std::string& prefix,
            int cnt_min,
            int cnt_max,
            const std::string& suffix)
{
    for (int cnt = cnt_min; cnt < cnt_max; ++cnt)
    {
        LibFred::OperationContextCreator ctx;
        std::ostringstream domain_fqdn;
        domain_fqdn << prefix << std::setw(3) << std::setfill('0') << cnt << suffix;
        const std::string fqdn = domain_fqdn.str();
        const auto create_object_result = LibFred::CreateDomain(
                fqdn,
                registrar,
                registrant).exec(ctx).create_object_result;
        const Database::Result dbres = ctx.get_conn().exec_params(
                "INSERT INTO object_state (object_id,state_id,valid_from,ohid_from) "
                "SELECT $1::BIGINT,id,NOW(),$2::BIGINT FROM enum_object_states WHERE name=$3::TEXT "
                "RETURNING 0",
                Database::query_param_list(create_object_result.object_id)
                                          (create_object_result.history_id)
                                          (Conversion::Enums::to_db_handle(LibFred::Object_State::delete_candidate)));
        if (dbres.size() <= 0)
        {
            throw std::runtime_error("no state inserted");
        }
        if (1 < dbres.size())
        {
            throw std::runtime_error("too many states inserted");
        }
        ctx.commit_transaction();
    }
}

// fred-admin --object_delete_candidates --object_delete_types=domain --object_delete_parts=2 --object_delete_spread_during_time=60
void delete_delete_candidates(
        const std::string& fred_admin,
        const std::set<LibFred::Object_Type::Enum>& object_types,
        int parts,
        int duration_sec)
{
    Cmd::Executable cmd(fred_admin);
    cmd("--object_delete_candidates");
    if (!object_types.empty())
    {
        std::ostringstream types;
        for (const auto& object_type : object_types)
        {
            if (!types.str().empty())
            {
                types << ",";
            }
            types << Conversion::Enums::to_db_handle(object_type);
        }
        cmd("--object_delete_types=" + types.str());
    }
    {
        std::ostringstream arg;
        arg << parts;
        cmd("--object_delete_parts=" + arg.str());
    }
    {
        std::ostringstream arg;
        arg << duration_sec;
        cmd("--object_delete_spread_during_time=" + arg.str());
    }
    const auto result = cmd.run();
    if (!result.stdout.empty())
    {
        std::cout << result.stdout << std::endl;
    }
    if (!result.stderr.empty())
    {
        std::cerr << result.stderr << std::endl;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        CfgArgGroups::init<HandleHelpGrpArg>(global_hpg)->handle(argc, argv);
        const auto t = std::chrono::system_clock::now().time_since_epoch();
        const auto t_sec = std::chrono::duration_cast<std::chrono::seconds>(t).count();
        std::string registrar;
        std::string registrant;
        {
            LibFred::OperationContextCreator ctx;
            const Database::Result dbres = ctx.get_conn().exec(
                    "SELECT UPPER(handle) FROM registrar WHERE NOT system LIMIT 1");
            if (dbres.size() != 1)
            {
                throw std::runtime_error("No registrar available");
            }
            registrar = static_cast<std::string>(dbres[0][0]);
            std::cout << "registrar:  " << registrar << std::endl;
            std::ostringstream contact_handle;
            contact_handle << "KONTAKT-" << t_sec;
            registrant = contact_handle.str();
            std::cout << "registrant: " << registrant << std::endl;
            LibFred::CreateContact create_contact(registrant, registrar);
            create_contact.set_name("František Mazaný");
            LibFred::Contact::PlaceAddress place;
            place.street1 = "Kandidátská 1";
            place.city = "Deletov";
            place.postalcode = "98765";
            place.country = "CZ";
            create_contact.set_place(place);
            create_contact.set_disclosename(true);
            create_contact.set_discloseorganization(true);
            create_contact.set_discloseaddress(true);
            create_contact.exec(ctx);
            ctx.commit_transaction();
        }

        constexpr int number_of_domains = 128;
        constexpr int number_of_threads = 16;
        constexpr int domains_per_thread = number_of_domains / number_of_threads;
        std::vector<std::thread> threads;
        threads.reserve(0 < number_of_threads ? number_of_threads - 1 : 0);
        std::ostringstream out;
        out << "delete-candidate-" << t_sec << "-";
        const std::string domain_prefix = out.str();
        const std::string domain_suffix = ".cz";
        for (int idx = 1; idx < number_of_threads; ++idx)
        {
            threads.emplace_back(
                    create_domains,
                    std::cref(registrar),
                    std::cref(registrant),
                    std::cref(domain_prefix),
                    idx * domains_per_thread,
                    (idx + 1) * domains_per_thread,
                    std::cref(domain_suffix));
        }
        create_domains(
                registrar,
                registrant,
                domain_prefix,
                0,
                domains_per_thread,
                domain_suffix);
        for (std::size_t idx = 0; idx < threads.size(); ++idx)
        {
            threads[idx].join();
        }

        threads.clear();
        constexpr int number_of_deleters = 3;
        constexpr int parts = number_of_deleters;
        constexpr int duration_in_seconds = 5;
        threads.reserve(number_of_deleters);
        const std::string fred_admin = "root/sbin/fred-admin";
        const std::set<LibFred::Object_Type::Enum> object_types = { LibFred::Object_Type::domain };
        std::cout << "start deletion" << std::endl;
        for (int idx = 0; idx < number_of_deleters; ++idx)
        {
            threads.emplace_back(
                    delete_delete_candidates,
                    std::cref(fred_admin),
                    std::cref(object_types),
                    parts,
                    duration_in_seconds);
        }
        delete_delete_candidates(
                fred_admin,
                object_types,
                1,
                duration_in_seconds + 1);
        for (std::size_t idx = 0; idx < threads.size(); ++idx)
        {
            threads[idx].join();
        }
        return EXIT_SUCCESS;
    }
    catch (const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch (const LibFred::CreateContact::Exception& e)
    {
        if (e.is_set_invalid_contact_handle())
        {
            std::cerr << "LibFred::CreateContact failure: invalid contact handle" << std::endl;
        }
        if (e.is_set_unknown_ssntype())
        {
            std::cerr << "LibFred::CreateContact failure: unknown ssntype" << std::endl;
        }
        if (e.is_set_unknown_registrar_handle())
        {
            std::cerr << "LibFred::CreateContact failure: unknown registrar handle" << std::endl;
        }
        if (e.is_set_unknown_country())
        {
            std::cerr << "LibFred::CreateContact failure: unknown country" << std::endl;
        }
        if (e.is_set_forbidden_company_name_setting())
        {
            std::cerr << "LibFred::CreateContact failure: forbidden company name setting" << std::endl;
        }
        std::cerr << "LibFred::CreateContact failure: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }
}
