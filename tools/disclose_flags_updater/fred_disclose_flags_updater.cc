#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

#include "tools/disclose_flags_updater/options.hh"
#include "tools/disclose_flags_updater/disclose_value.hh"
#include "tools/disclose_flags_updater/disclose_settings.hh"
#include "tools/disclose_flags_updater/contact_search_query.hh"
#include "tools/disclose_flags_updater/worker.hh"
#include "tools/disclose_flags_updater/thread_safe_output.hh"
#include "src/libfred/db_settings.hh"
#include "src/libfred/registrar/info_registrar.hh"


int main(int argc, char *argv[])
{
    using namespace Tools::DiscloseFlagsUpdater;

    try
    {
        /* parser in boost/optional/optional_io.hpp seems broken */
        std::uint64_t logd_request_id = 0;
        DiscloseSettings discloses;
        GeneralOptions opts;

        namespace po = boost::program_options;
        po::options_description args("Options");
        args.add_options()
            ("verbose", po::bool_switch(&opts.verbose)->default_value(false), "verbose output")
            ("progress", po::bool_switch(&opts.progress_display)->default_value(false), "display progress bar")
            ("thread-count", po::value<std::int16_t>(&opts.thread_count)->default_value(1), "number of worker threads")
            ("logd-request-id", po::value<std::uint64_t>(&logd_request_id)->default_value(0, "--"),
             "logger request id for all contact updates")
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

        if (logd_request_id != 0)
        {
            opts.logd_request_id = logd_request_id;
        }

        if (opts.thread_count < 1)
        {
            std::cerr << "Error: --thread-count must be at least 1." << std::endl;
            return 1;
        }

        Database::Manager::init(new Database::ConnectionFactory(opts.db_connect));
        try
        {
            LibFred::OperationContextCreator ctx;
            LibFred::InfoRegistrarByHandle(opts.by_registrar).exec(ctx);
        }
        catch (...)
        {
            std::cerr << "Error: registrar not found" << std::endl;
            return 1;
        }

        if (opts.verbose)
        {
            std::cout << "Settings:" << std::endl
                      << "\t- db: " << opts.db_connect << std::endl
                      << "\t- by-registrar: " << opts.by_registrar << std::endl
                      << "\t- thread-count: " << opts.thread_count << std::endl
                      << "\t- dry-run: " << opts.dry_run << std::endl
                      << "\t- logd-request-id: " << opts.logd_request_id << std::endl
                      << "\t- disclose-settings: " << discloses << "\n" << std::endl;
        }

        if (discloses.is_empty())
        {
            std::cout << std::endl << "Nothing to do. Exiting..." << std::endl;
            return 0;
        }

        auto contact_search_sql = make_query_search_contact_needs_update(discloses);

        if (opts.verbose)
        {
            std::cout << "search-sql: " << contact_search_sql << std::endl;
        }

        LibFred::OperationContextCreator ctx;
        Database::Result contact_result = ctx.get_conn().exec(contact_search_sql);

        TaskCollection update_tasks;
        update_tasks.reserve(contact_result.size());
        for (std::uint64_t i = 0; i < contact_result.size(); ++i)
        {
            update_tasks.emplace_back(
                static_cast<std::uint64_t>(contact_result[i][0]),
                static_cast<bool>(contact_result[i][1])
            );
        }

        if (opts.verbose)
        {
            std::cout << "total-contacts: " << update_tasks.size() << std::endl;
        }

        std::vector<Worker> workers;
        workers.reserve(opts.thread_count);

        const auto chunk_size = update_tasks.size() / opts.thread_count;
        auto chunk_begin = update_tasks.begin();
        for (auto i = 0; i < opts.thread_count - 1; ++i)
        {
            auto chunk_end = chunk_begin + chunk_size;
            workers.emplace_back(opts, discloses, std::make_pair(chunk_begin, chunk_end));
            chunk_begin = chunk_end;
        }
        workers.emplace_back(opts, discloses, std::make_pair(chunk_begin, update_tasks.end()));

        if (opts.verbose)
        {
            std::ostringstream chunk_sizes;
            chunk_sizes << " ";
            for (auto& w : workers)
            {
                chunk_sizes << w.get_total_count() << " ";
            }
            std::cout << "workers: " << workers.size() << "  (" << chunk_sizes.str() << ")" << std::endl;
        }

        std::cout << std::endl;

        std::thread progress_thread([&workers](){
            safe_cout("[progress thread] started\n");
            const auto start_time = std::chrono::steady_clock::now();

            bool is_finished;
            do
            {
                is_finished = true;
                std::ostringstream progress_format;
                for (auto i = 0ul; i < workers.size(); ++i)
                {
                    const auto& w = workers[i];
                    if (!w.has_exited())
                    {
                        is_finished = false;
                    }

                    const auto ith_time = std::chrono::steady_clock::now();
                    const auto eta_time = ((ith_time - start_time) / (w.get_done_count() + 1))
                                        * (w.get_total_count() - w.get_done_count() + 1);

                    auto eta_time_rest = eta_time;
                    const auto eta_h = std::chrono::duration_cast<std::chrono::hours>(eta_time_rest);
                    eta_time_rest -= eta_h;
                    const auto eta_m = std::chrono::duration_cast<std::chrono::minutes>(eta_time_rest);
                    eta_time_rest -= eta_m;
                    const auto eta_s = std::chrono::duration_cast<std::chrono::seconds>(eta_time_rest);
                    std::ostringstream eta_format;

                    eta_format << std::setfill('0')
                               << std::setw(2) << eta_h.count() << "h"
                               << std::setw(2) << eta_m.count() << "m"
                               << std::setw(2) << eta_s.count() << "s";

                    progress_format << i << ": " << std::setfill('0') << std::setw(3)
                                    << std::round(100 * (static_cast<float>(w.get_done_count()) / w.get_total_count())) << "%"
                                    << std::setw(0)
                                    << " (" << w.get_done_count() << "/" << w.get_total_count() << ")"
                                    << " eta: " << eta_format.str() << "  ";
                }
                safe_cout(progress_format.str() + "\r");
                safe_cout_flush();
                if (!is_finished)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
            while (!is_finished);
            safe_cout("\n[progress thread] finished\n");
        });


        std::vector<std::thread> t_workers;
        t_workers.reserve(workers.size());
        for (auto& w : workers)
        {
            t_workers.emplace_back(std::ref(w));
        }

        safe_cout("workers: " + std::to_string(t_workers.size()) + " workers\n");

        for (auto& t : t_workers)
        {
            t.join();
        }
        progress_thread.join();

        std::cout << std::endl;
        if (!opts.dry_run)
        {
            for (auto& w : workers)
            {
                w.get_ctx()->commit_transaction();
            }
            std::cout << "Total " << contact_result.size() << " contact(s) UPDATED." << std::endl;
        }
        else
        {
            std::cout << "Total " << contact_result.size() << " contact(s) SHOULD be updated." << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
