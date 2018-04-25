#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <chrono>
#include <boost/program_options.hpp>

#include "tools/disclose_flags_updater/options.hh"
#include "tools/disclose_flags_updater/disclose_value.hh"
#include "tools/disclose_flags_updater/disclose_settings.hh"
#include "tools/disclose_flags_updater/contact_search_query.hh"
#include "src/libfred/db_settings.hh"
#include "src/libfred/registrable_object/contact/update_contact.hh"


int main(int argc, char *argv[])
{
    using namespace Tools::DiscloseFlagsUpdater;

    try
    {
        DiscloseSettings discloses;
        GeneralOptions opts;

        namespace po = boost::program_options;
        po::options_description args("Options");
        args.add_options()
            ("verbose", po::bool_switch(&opts.verbose)->default_value(false), "verbose output")
            ("progress", po::bool_switch(&opts.progress_display)->default_value(false), "display progress bar")
            ("help", "produce usage message")
            ("dry-run", po::bool_switch(&opts.dry_run)->default_value(false), "only show what will be done")
            ("db-connect", po::value<std::string>(&opts.db_connect)->required(), "database connection string")
            ("set-name",
             po::value<DiscloseValue>(&discloses.name)->default_value(DiscloseValue::not_set, "not-set"),
             "disclose value for name")
            ("set-org",
             po::value<DiscloseValue>(&discloses.org)->default_value(DiscloseValue::not_set, "not-set"),
             "disclose value for organization")
            ("set-addr",
             po::value<DiscloseAddressValue>(&discloses.addr)->default_value(DiscloseAddressValue::not_set, "not-set"),
             "disclose value for all postall addresses")
            ("set-voice",
             po::value<DiscloseValue>(&discloses.voice)->default_value(DiscloseValue::not_set, "not-set"),
             "disclose value for telephone (voice)")
            ("set-fax",
             po::value<DiscloseValue>(&discloses.fax)->default_value(DiscloseValue::not_set, "not-set"),
             "disclose value for fax")
            ("set-email",
             po::value<DiscloseValue>(&discloses.email)->default_value(DiscloseValue::not_set, "not-set"),
             "disclose value for e-mail address")
            ("set-vat",
             po::value<DiscloseValue>(&discloses.vat)->default_value(DiscloseValue::not_set, "not-set"),
             "disclose value for VAT value")
            ("set-ident",
             po::value<DiscloseValue>(&discloses.ident)->default_value(DiscloseValue::not_set, "not-set"),
             "disclose value for contact identification type and value")
            ("set-notifyemail",
             po::value<DiscloseValue>(&discloses.notify_email)->default_value(DiscloseValue::not_set, "not-set"),
             "disclose value for contact identification type and value")
            ("by-registrar",
             po::value<std::string>(&opts.by_registrar)->required(),
             "all changes to dislose flags will be done by this registrar");


        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, args), vm);

        if (vm.count("help"))
        {
            std::cout << args << std::endl;
            return 0;
        }

        po::notify(vm);

        if (opts.verbose)
        {
            std::cout << "Settings:" << std::endl
                      << "\t- db: " << opts.db_connect << std::endl
                      << "\t- by-registrar: " << opts.by_registrar << std::endl
                      << "\t- disclose-settings: " << discloses << std::endl;
        }

        if (discloses.is_empty())
        {
            std::cout << std::endl << "Nothing to do. Exiting..." << std::endl;
            return 0;
        }

        auto contact_search_sql = make_query_search_contact_needs_update(discloses);

        if (opts.verbose)
        {
            std::cout << "\t- search-sql: " << contact_search_sql << std::endl;
        }
        std::cout << std::endl;

        Database::Manager::init(new Database::ConnectionFactory(opts.db_connect));

        LibFred::OperationContextCreator ctx;
        Database::Result contact_list = ctx.get_conn().exec(contact_search_sql);

        auto total_count = contact_list.size();

        auto start_time = std::chrono::steady_clock::now();
        std::string eta_time_human = "n/a";
        uint64_t progress_display_step = 50;

        for (unsigned int i = 0; i < contact_list.size(); ++i)
        {
            auto contact_id = static_cast<uint64_t>(contact_list[i][0]);
            auto contact_hidden_address_allowed = static_cast<bool>(contact_list[i][1]);

            auto update_op = LibFred::UpdateContactById(contact_id, opts.by_registrar);

            if (discloses.name != DiscloseValue::not_set)
            {
                update_op.set_disclosename(to_db_value(discloses.name));
            }
            if (discloses.org != DiscloseValue::not_set)
            {
                update_op.set_discloseorganization(to_db_value(discloses.org));
            }
            if (discloses.voice != DiscloseValue::not_set)
            {
                update_op.set_disclosetelephone(to_db_value(discloses.voice));
            }
            if (discloses.fax != DiscloseValue::not_set)
            {
                update_op.set_disclosefax(to_db_value(discloses.fax));
            }
            if (discloses.email != DiscloseValue::not_set)
            {
                update_op.set_discloseemail(to_db_value(discloses.email));
            }
            if (discloses.vat != DiscloseValue::not_set)
            {
                update_op.set_disclosevat(to_db_value(discloses.vat));
            }
            if (discloses.ident != DiscloseValue::not_set)
            {
                update_op.set_discloseident(to_db_value(discloses.ident));
            }
            if (discloses.notify_email != DiscloseValue::not_set)
            {
                update_op.set_disclosenotifyemail(to_db_value(discloses.notify_email));
            }
            if (discloses.addr != DiscloseAddressValue::not_set)
            {
                auto discloseaddress_value = true;
                if (discloses.addr == DiscloseAddressValue::hide_verified)
                {
                    if (contact_hidden_address_allowed)
                    {
                        discloseaddress_value = false;
                    }
                    else
                    {
                        discloseaddress_value = true;
                    }
                }
                else
                {
                    discloseaddress_value = to_db_value(discloses.addr);
                }
                update_op.set_discloseaddress(discloseaddress_value);
            }

            update_op.exec(ctx);

            if (opts.verbose)
            {
                std::cout << "id: " << contact_id << "  "
                          << "hidden-address-allowed: " << (contact_hidden_address_allowed ? "yes" : "no") << "\n";

                std::cout << update_op.to_string() << "\n";
                std::cout << std::endl;
            }
            else
            {
                if (opts.progress_display && (i % progress_display_step == 0 || i == 0))
                {
                    auto ith_time = std::chrono::steady_clock::now();
                    auto eta_time = ((ith_time - start_time) / (i + 1)) * (total_count - i + 1);

                    auto eta_time_rest = eta_time;
                    auto eta_h = std::chrono::duration_cast<std::chrono::hours>(eta_time_rest);
                    eta_time_rest -= eta_h;
                    auto eta_m = std::chrono::duration_cast<std::chrono::minutes>(eta_time_rest);
                    eta_time_rest -= eta_m;
                    auto eta_s = std::chrono::duration_cast<std::chrono::seconds>(eta_time_rest);
                    std::stringstream eta_format;

                    eta_format << std::setfill('0')
                               << std::setw(2) << eta_h.count() << "h"
                               << std::setw(2) << eta_m.count() << "m"
                               << std::setw(2) << eta_s.count() << "s";
                    eta_time_human = eta_format.str();


                    std::cout << std::setfill('0') << std::setw(3)
                              << std::round(100 * (static_cast<float>(i + 1) / contact_list.size())) << "%"
                              << std::setw(0)
                              << " (" << i + 1 << "/" << total_count << ")"
                              << " eta: " << eta_time_human
                              << "\r";
                    std::cout.flush();
                }
            }
        }
        std::cout << std::endl;
        if (!opts.dry_run)
        {
            ctx.commit_transaction();
            std::cout << "Total " << contact_list.size() << " contact(s) UPDATED." << std::endl;
        }
        else
        {
            std::cout << "Total " << contact_list.size() << " contact(s) SHOULD be updated." << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
