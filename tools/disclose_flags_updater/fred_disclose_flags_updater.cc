#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>

#include "tools/disclose_flags_updater/options.hh"
#include "tools/disclose_flags_updater/disclose_value.hh"
#include "tools/disclose_flags_updater/disclose_settings.hh"
#include "tools/disclose_flags_updater/contact_search_query.hh"
#include "tools/disclose_flags_updater/worker.hh"
#include "tools/disclose_flags_updater/thread_safe_output.hh"
#include "src/libfred/db_settings.hh"


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
            ("thread-count", po::value<uint16_t>(&opts.thread_count)->default_value(1), "number of worker threads")
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
                      << "\t- thread-count: " << opts.thread_count << std::endl
                      << "\t- dry-run: " << opts.dry_run << std::endl
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

        Database::Manager::init(new Database::ConnectionFactory(opts.db_connect));

        LibFred::OperationContextCreator ctx;
        Database::Result contact_result = ctx.get_conn().exec(contact_search_sql);

        TaskCollection update_tasks;
        update_tasks.reserve(contact_result.size());
        for (uint64_t i = 0; i < contact_result.size(); ++i)
        {
            update_tasks.emplace_back(
                static_cast<uint64_t>(contact_result[i][0]),
                static_cast<bool>(contact_result[i][1])
            );
        }

        if (opts.verbose)
        {
            std::cout << "total-contacts: " << update_tasks.size() << std::endl;
        }

        std::vector<Worker> workers;

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
            std::stringstream chunk_sizes;
            for (auto& w : workers)
            {
                chunk_sizes << w.total_count << " ";
            }
            std::cout << "workers: " << workers.size() << "(" << chunk_sizes.str() << ")" << std::endl;
        }

        std::cout << std::endl;

        std::thread progress_thread([&workers](){
            safe_cout("[progress thread] started");
            auto start_time = std::chrono::steady_clock::now();
            std::string eta_time_human = "n/a";

            bool is_finished;
            do
            {
                is_finished = true;
                for (auto i = 0ul; i < workers.size(); ++i)
                {
                    const auto& w = workers[i];
                    if (!w.exited)
                    {
                        is_finished = false;
                    }

                    auto ith_time = std::chrono::steady_clock::now();
                    auto eta_time = ((ith_time - start_time) / (w.done_count + 1)) * (w.total_count - w.done_count + 1);

                    auto eta_time_rest = eta_time;
                    auto eta_h = std::chrono::duration_cast<std::chrono::hours>(eta_time_rest);
                    eta_time_rest -= eta_h;
                    auto eta_m = std::chrono::duration_cast<std::chrono::minutes>(eta_time_rest);
                    eta_time_rest -= eta_m;
                    auto eta_s = std::chrono::duration_cast<std::chrono::seconds>(eta_time_rest);
                    std::ostringstream eta_format;

                    eta_format << std::setfill('0')
                               << std::setw(2) << eta_h.count() << "h"
                               << std::setw(2) << eta_m.count() << "m"
                               << std::setw(2) << eta_s.count() << "s";

                    std::ostringstream progress_format;
                    progress_format << i << ": " << std::setfill('0') << std::setw(3)
                                    << std::round(100 * (static_cast<float>(w.done_count) / w.total_count)) << "%"
                                    << std::setw(0)
                                    << " (" << w.done_count << "/" << w.total_count << ")"
                                    << " eta: " << eta_format.str() << "  \r";
                    safe_cout(progress_format.str());
                }
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
        for (auto& w : workers)
        {
            t_workers.push_back(std::thread(std::ref(w)));
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
                w.ctx->commit_transaction();
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
