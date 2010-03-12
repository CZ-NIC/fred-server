/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include "log/logger.h"
#include "register/register.h"
// #include "corba/admin/admin_impl.h"
// #include "corba/mailer_manager.h"

#include "simple.h"
#include "commonclient.h"
#include "keysetclient.h"
#include "nssetclient.h"
#include "domainclient.h"
#include "contactclient.h"
#include "invoiceclient.h"
#include "bankclient.h"
#include "pollclient.h"
#include "registrarclient.h"
#include "notifyclient.h"
#include "objectclient.h"
#include "infobuffclient.h"
#include "fileclient.h"
#include "mailclient.h"
#include "publicreqclient.h"
#include "enumparamclient.h"

using namespace boost::posix_time;

int
findRequestExecutor(Config::Conf &conf, METHODS &methods)
{
    METHODS_IT it = methods.begin();
    for (; it != methods.end(); ++it) {
        if (conf.hasOpt(it->first)) {
            return it->second;
        }
    }
    return 0;
}

#define ADDOPT_NOTYPE(name, desc)   (name, desc)
#define ADDOPT_STRING(name, desc)   (name, boost::program_options::value<std::string>(), desc)
#define ADDOPT_INT(name, desc)   (name, boost::program_options::value<int>(), desc)
#define ADDOPT_UINT(name, desc)   (name, boost::program_options::value<unsigned int>(), desc)
#define ADDOPT_ULONGLONG(name, desc)    (name, boost::program_options::value<unsigned long long>(), desc)
#define ADDOPT_DOUBLE(name, desc)   (name, boost::program_options::value<double>(), desc)
#define ADDOPT_BOOL(name, desc)  (name, boost::program_options::value<bool>(), desc)

void
appendOptions(
        boost::program_options::options_description &all,
        boost::program_options::options_description &visible,
        METHODS &methods,
        const options *opts,
        int optsCount)
{
    for (int i = 0; i < optsCount; i++) {
        switch (opts[i].type) {
            case TYPE_NOTYPE:
                all.add_options()
                    ADDOPT_NOTYPE(opts[i].name, opts[i].description);
                if (opts[i].visible) {
                    visible.add_options()
                        ADDOPT_NOTYPE(opts[i].name, opts[i].description);
                }
                break;
            case TYPE_STRING:
                all.add_options()
                    ADDOPT_STRING(opts[i].name, opts[i].description);
                if (opts[i].visible) {
                    visible.add_options()
                        ADDOPT_STRING(opts[i].name, opts[i].description);
                }
                break;
            case TYPE_INT:
                all.add_options()
                    ADDOPT_INT(opts[i].name, opts[i].description);
                if (opts[i].visible) {
                    visible.add_options()
                        ADDOPT_INT(opts[i].name, opts[i].description);
                }
                break;
            case TYPE_UINT:
                all.add_options()
                    ADDOPT_UINT(opts[i].name, opts[i].description);
                if (opts[i].visible) {
                    visible.add_options()
                        ADDOPT_UINT(opts[i].name, opts[i].description);
                }
                break;
            case TYPE_ULONGLONG:
                all.add_options()
                    ADDOPT_ULONGLONG(opts[i].name, opts[i].description);
                if (opts[i].visible) {
                    visible.add_options()
                        ADDOPT_ULONGLONG(opts[i].name, opts[i].description);
                }
            case TYPE_BOOL:
                all.add_options()
                    ADDOPT_BOOL(opts[i].name, opts[i].description);
                if (opts[i].visible) {
                    visible.add_options()
                        ADDOPT_BOOL(opts[i].name, opts[i].description);
                }
                break;
            case TYPE_DOUBLE:
                all.add_options()
                    ADDOPT_DOUBLE(opts[i].name, opts[i].description);
                if (opts[i].visible) {
                    visible.add_options()
                        ADDOPT_DOUBLE(opts[i].name, opts[i].description);
                }
                break;
            default:
                std::cerr << "Unknown type" << std::endl;
                exit(1);
        }
        if (opts[i].callable) {
            methods.insert(std::make_pair(opts[i].name, opts[i].client));
        }
    }
}

#undef ADDOPT_NOTYPE
#undef ADDOPT_STRING
#undef ADDOPT_INT
#undef ADDOPT_UINT
#undef ADDOPT_BOOL

int
main(int argc, char **argv)
{
    try {
    boost::program_options::options_description generalOpts("General options");
    generalOpts.add_options()
        addOptStrDef(CLI_LANGUAGE_NAME, "CS")
        addOpt(CLI_HELP_DATES_NAME);

    boost::program_options::options_description generalOptsInvis("General invisible options");
    generalOptsInvis.add_options()
        addOpt(CLI_MOO_NAME);

    boost::program_options::options_description configurationOpts("Configuration options");
    configurationOpts.add_options()
        addOptStr(DB_NAME_NAME)
        addOptStr(DB_USER_NAME)
        addOptStr(DB_PASS_NAME)
        addOptStr(DB_HOST_NAME)
        addOptUInt(DB_PORT_NAME)
        addOptStr(NS_HOST_NAME)
        addOptUInt(NS_PORT_NAME)
        addOptUInt(LOG_TYPE_NAME)
        addOptUInt(LOG_LEVEL_NAME)
        addOptStr(LOG_FILE_NAME)
        addOptUInt(LOG_SYSLOG_NAME)
        addOptStr(REG_FILECLIENT_PATH_NAME)
        addOptBool(REG_RESTRICTED_HANDLES_NAME)
        addOptStr(REG_DOCGEN_PATH_NAME)
        addOptStr(REG_DOCGEN_TEMPLATE_PATH_NAME)
        addOptStr(LOGIN_REGISTRAR_NAME);

    boost::program_options::options_description fileOpts("");
    fileOpts.add(configurationOpts);
    fileOpts.add_options()
        ("*", boost::program_options::value<std::string>(),
         "other-options");

    boost::program_options::options_description commonOpts("Common options");
    commonOpts.add_options()
        (LIMIT_NAME, boost::program_options::value<unsigned int>()->default_value(50),
         "limit for output")
        (FULL_LIST_NAME,
         FULL_LIST_NAME_DESC);

    boost::program_options::variables_map varMap;

    // all valid options - including invisible
    boost::program_options::options_description allOpts("All allowed options");
    allOpts.add(generalOpts).
        add(generalOptsInvis).
        add(configurationOpts).
        add(commonOpts);

    // only visible options
    boost::program_options::options_description programOpts("Program options");
    METHODS methods;

#define APPENDOPTIONS(which)    appendOptions(allOpts, programOpts, methods, \
        Admin::which::getOpts(), Admin::which::getOptsCount())
    APPENDOPTIONS(DomainClient);
    APPENDOPTIONS(KeysetClient);
    APPENDOPTIONS(ContactClient);
    APPENDOPTIONS(InvoiceClient);
    APPENDOPTIONS(BankClient);
    APPENDOPTIONS(PollClient);
    APPENDOPTIONS(RegistrarClient);
    APPENDOPTIONS(NotifyClient);
    APPENDOPTIONS(ObjectClient);
    APPENDOPTIONS(InfoBuffClient);
    APPENDOPTIONS(NssetClient);
    APPENDOPTIONS(FileClient);
    APPENDOPTIONS(MailClient);
    APPENDOPTIONS(PublicRequestClient);
    APPENDOPTIONS(EnumParamClient);
#undef APPENDOPTIONS

    Config::Manager confMan = Config::ConfigManager::instance_ref();
    try {
        confMan.init(argc, argv);
        confMan.setCmdLineOptions(allOpts);
        confMan.setCfgFileOptions(fileOpts, CONFIG_FILE);
        confMan.parse();
    } catch (Config::Manager::ConfigParseError &err) {
        std::cerr << "Config parser error: " << err.what() << std::endl;
        exit(1);
    }

    if (confMan.isVersion()) {
        print_version();
        exit(0);
    }
    Config::Conf conf = confMan.get();

    Logging::Log::Level log_level;
    Logging::Log::Type log_type;

    if (conf.hasOpt(LOG_LEVEL_NAME)) {
         log_level = static_cast<Logging::Log::Level>(
                conf.get<unsigned int>(LOG_LEVEL_NAME));
    } else {
        log_level = static_cast<Logging::Log::Level>(8);
    }

    if (conf.hasOpt(LOG_TYPE_NAME)) {
        log_type = static_cast<Logging::Log::Type>(
                conf.get<unsigned int>(LOG_TYPE_NAME));
    } else {
        log_type = static_cast<Logging::Log::Type>(Logging::Log::LT_FILE);
    }

    boost::any param;
    if (log_type == Logging::Log::LT_FILE) {
        param = conf.get<std::string>(LOG_FILE_NAME);
    }
    if (log_type == Logging::Log::LT_SYSLOG) {
        param = conf.get<unsigned>(LOG_SYSLOG_NAME);
    }

    Logging::Manager::instance_ref().get(PACKAGE).addHandler(log_type, param);
    Logging::Manager::instance_ref().get(PACKAGE).setLevel(log_level);

    std::stringstream connstring;
    std::stringstream nsAddr;

    nsAddr << conf.get<std::string>(NS_HOST_NAME);
    if (conf.hasOpt(NS_PORT_NAME))
        nsAddr << ":" << conf.get<unsigned int>(NS_PORT_NAME);

    connstring << "dbname=" << conf.get<std::string>(DB_NAME_NAME)
               << " user=" << conf.get<std::string>(DB_USER_NAME);
    if (conf.hasOpt(DB_HOST_NAME))
        connstring << " host=" << conf.get<std::string>(DB_HOST_NAME);
    if (conf.hasOpt(DB_PORT_NAME))
        connstring << " port=" << conf.get<unsigned int>(DB_PORT_NAME);
    if (conf.hasOpt(DB_PASS_NAME))
        connstring << " password=" << conf.get<std::string>(DB_PASS_NAME);

    if (conf.hasUnknown()) {
        std::vector<std::string> unknown(conf.getUnknown());
        std::cout << "Unknown option(s):" << std::endl;
        for (int i = 0; i < (int)unknown.size(); i++) {
            std::cout << unknown[i] << std::endl;
        }
        exit(0);
    }

    Database::Manager::init(new Database::ConnectionFactory(connstring.str()));

    if (conf.hasOpt(CLI_MOO_NAME)) {
        print_moo();
        exit(0);
    } else if (conf.hasOpt(CLI_HELP_DATES_NAME)) {
        help_dates();
        exit(0);
    }

#define INIT_AND_RUN(what) { \
        Admin::what pom(connstring.str(), nsAddr.str(), conf); \
        pom.runMethod(); \
    }
    switch (findRequestExecutor(conf, methods)) {
        case CLIENT_DOMAIN:
            INIT_AND_RUN(DomainClient);
            break;
        case CLIENT_KEYSET:
            INIT_AND_RUN(KeysetClient);
            break;
        case CLIENT_CONTACT:
            INIT_AND_RUN(ContactClient);
            break;
        case CLIENT_INVOICE:
            INIT_AND_RUN(InvoiceClient);
            break;
        case CLIENT_BANK:
            INIT_AND_RUN(BankClient);
            break;
        case CLIENT_POLL:
            INIT_AND_RUN(PollClient);
            break;
        case CLIENT_REGISTRAR:
            INIT_AND_RUN(RegistrarClient);
            break;
        case CLIENT_NOTIFY:
            INIT_AND_RUN(NotifyClient);
            break;
        case CLIENT_OBJECT:
            INIT_AND_RUN(ObjectClient);
            break;
        case CLIENT_INFOBUFF:
            INIT_AND_RUN(InfoBuffClient);
            break;
        case CLIENT_NSSET:
            INIT_AND_RUN(NssetClient);
            break;
        case CLIENT_FILE:
            INIT_AND_RUN(FileClient);
            break;
        case CLIENT_MAIL:
            INIT_AND_RUN(MailClient);
            break;
        case CLIENT_PUBLICREQUEST:
            INIT_AND_RUN(PublicRequestClient);
            break;
        case CLIENT_ENUMPARAM:
            INIT_AND_RUN(EnumParamClient);
            break;
        default:
            if (confMan.isHelp()) {
                std::cout << "Usage: " << argv[0]  << " [options]" << std::endl << std::endl;
                std::cout << generalOpts << std::endl;
                std::cout << configurationOpts << std::endl;
                std::cout << commonOpts << std::endl;
                std::cout << programOpts << std::endl;
            } else {
                std::cout << "Unknown client" << std::endl;
            }
            break;
    }
#undef INIT_AND_RUN

    }
    catch (ccReg::EPP::EppError &e) {
        std::stringstream message;
        message << "EppError code: " << e.errCode << ", message: "
            << e.errMsg << std::endl;
        for (int ii = 0; ii < (int)e.errorList.length(); ii++) {
            message << "Reason code: " << e.errorList[ii].code
                    << ", message: "
                    << e.errorList[ii].reason
                    << ", Position: " << e.errorList[ii].position;
        }

        LOGGER(PACKAGE).error(message.str());
        std::cerr << message.str();
    }
    catch (CORBA::Exception &e) {
        std::stringstream message("CORBA::Exception");

        LOGGER(PACKAGE).error(message.str());
        std::cerr << message.str() << std::endl;
        return 3;
    }
    catch (std::exception &e) {
        std::stringstream message("Error: ");
        message << e.what();

        LOGGER(PACKAGE).error(message.str());
        std::cerr << message.str() << std::endl;
        return 2;
    }
    catch (Register::SQL_ERROR) {
        std::cerr << "SQL ERROR" << std::endl;
        return 1;
    }

    return 0;
}

