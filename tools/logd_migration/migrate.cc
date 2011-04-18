/*
 * migrate.cc
 *
 *  Created on: Jan 23, 2009
 *      Author: jvicenik
 * 
 * is_monitoring flag
 *
 */


#include <string>
#include <signal.h>

#include "migrate.h"



//#include "conf/manager.h"

#include "old_utils/log.h"


#include "util.h"
#include "src/fredlib/requests/request_manager.h"

#include "m_epp_parser.h"

using namespace Database;

static const int COMMIT_INTERVAL = 1000;

static const char * EPP_SCHEMA = "/home/jvicenik/devel/fred_svn/fred/build/root/share/fred-mod-eppd/schemas/all.xsd";
static const std::string CONNECTION_STRING = "host=localhost port=22345 dbname=fred user=fred password=password connect_timeout=2";

/** length of date at the begining of an input line, input format is: date|xml */
static const int INPUT_DATE_LENGTH = 26;
static const int INPUT_ID_LENGTH   = 22;
static const int INPUT_SERVICE_LENGTH = 2;
static const int INPUT_MON_LENGTH = 1;
static const std::string LOG_FILENAME = "log_migration_log.txt";

pool_subst *mem_pool;

/** EPP service_id code; according to service_id table 
 */
const int LC_EPP = 3;

void logger(boost::format &fmt)
{
#ifdef HAVE_LOGGER
    LOGGER("fred-server").error(fmt);
#endif
}

void logger(std::string &str)
{
#ifdef HAVE_LOGGER
    LOGGER("fred-server").error(str);
#endif
}

void logger(const char *str)
{
#ifdef HAVE_LOGGER
    LOGGER("fred-server").error(str);
#endif
}

void signal_handler(int signum);

void signal_handler(int signum)
{
    std::cout << "Received signal " << signum << ", exiting. " << std::endl;
    exit(0);
}

int main()
{
    int trans_count;
    epp_red_command_type cmd_type;
    epp_command_data *cdata; /* command data structure */
    parser_status pstat;
    std::string line, rawline;
    void *schema;
    clock_t time1, time2, time3, t_parser=0, t_logcomm=0, t_backend=0;

    try {
        Database::Manager::init(new Database::ConnectionFactory(CONNECTION_STRING));

        Fred::Logger::ManagerImpl serv;

        cdata = NULL;

        // TODO -  check if the file EPP_SCHEMA exists (or better, create some config file for this thing with all the variables which are used
        schema = epp_parser_init(EPP_SCHEMA);

        signal(15, signal_handler);
        signal(11, signal_handler);
        signal(6, signal_handler);

        // setup loggin via LOGGER
        Logging::Manager::instance_ref().get(PACKAGE).addHandler(Logging::Log::LT_FILE, std::string(LOG_FILENAME));
        Logging::Manager::instance_ref().get(PACKAGE).setLevel(Logging::Log::LL_TRACE);
        LOGGER(PACKAGE).info("Logging initialized for migration");

        // TODO is this safe?
        Connection serv_conn = serv.get_connection();
        Transaction serv_transaction(serv_conn);

        //here we're changing the approach:
        //    accept action table id , | separator and xml like before
        //    use only the method insert_properties
        //

        mem_pool = new pool_subst();

        trans_count = 0;
        while (std::getline(std::cin, line)) {
            //size_t i;
            char *end = NULL;
            pool_manager man(mem_pool);

            if (line.empty()) continue;

            if (line[INPUT_ID_LENGTH] != '|') {
                logger("Error in input line: ID at the beginning doesn't have proper length");

                std::cout << "Error in input line: ID at the beginning doesn't have proper length: " << line << std::endl;
                continue;
            }

            std::string id_str = line.substr(0, INPUT_ID_LENGTH);
            line = line.substr(INPUT_ID_LENGTH + 1);
            TID request_id = strtoull(id_str.c_str(), &end, 10);

            if(line[INPUT_DATE_LENGTH] != '|') {
                logger("Error in input line: Date at the beginning doesn't have proper length");
                std::cout << "Error in input line: Date at the beginning doesn't have proper length" << std::endl;
                continue;
            }

            std::string date_str = line.substr(0, INPUT_DATE_LENGTH);
            line = line.substr(INPUT_DATE_LENGTH + 1);

            std::string monitoring_str = line.substr(0, INPUT_MON_LENGTH);
            line = line.substr(INPUT_MON_LENGTH + 1);

            bool mon_flag = (monitoring_str.at(0) == 't');
            // OK, both values found

            time1 = clock();
            pstat = epp_parse_command(line.c_str(), line.length(), &cdata, &cmd_type);

            time2 = clock();
            t_parser += time2 - time1;
            // count << (int)pstat << endl;

            // test if the failure is serious enough to close connection
            if (pstat > PARSER_HELLO) {
                switch (pstat) {
                    case PARSER_NOT_XML:
                        logger("Request is not XML");
                        continue;
                    case PARSER_NOT_COMMAND:
                        logger("Request is neither a command nor hello");
                        continue;
                    case PARSER_ESCHEMA:
                        logger("Schema's parser error - check correctness of schema");
                        continue;
                    case PARSER_EINTERNAL:
                        logger("Internal parser error occured when processing request");
                        continue;
                    default:
                        logger("Unknown error occured during parsing stage");
                        continue;
                }
            }
            time2 = clock();

            // we don't care about the request_type_id - it's already in the request table
            epp_action_type request_type_id = UnknownAction;
            std::auto_ptr<Fred::Logger::RequestProperties> props = log_epp_command(cdata, cmd_type, -1, &request_type_id);

            time3 = clock();
            t_logcomm += time3 - time2;
            epp_parser_request_cleanup(cdata);

            serv.insert_props_pub(date_str, LC_EPP, mon_flag, request_id, *props);
            trans_count++;

            if((trans_count % COMMIT_INTERVAL) == 0) {
                serv_transaction.commit();
            }

            t_backend += clock() - time3;
        }

        serv_transaction.commit();

        printf(" --------- REPORT: \n"
            " Parser:        %12i \n"
            " log_epp_command: %12i \n"
            " Fred::Logger::ManagerImpl:        %12i \n",
            (int)(t_parser/ 1000),
            (int)(t_logcomm / 1000),
            (int)(t_backend / 1000));

        free(schema);
        // if the transaction wasn't commited in the previous command
    }
    catch (std::exception &_e) {
        std::cerr << "error occured (" << _e.what() << ")" << std::endl;
    }
    catch (...) {
        std::cerr << "error occured" << std::endl;
    }
}

