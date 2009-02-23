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
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"

#include "simple.h"
#include "commonclient.h"
#include "keysetclient.h"
#include "nssetclient.h"
#include "domainclient.h"
#include "contactclient.h"
#include "invoiceclient.h"
#include "authinfoclient.h"
#include "bankclient.h"
#include "pollclient.h"
#include "registrarclient.h"
#include "notifyclient.h"
#include "objectclient.h"
#include "infobuffclient.h"
#include "fileclient.h"
#include "mailclient.h"
#include "publicreqclient.h"

using namespace boost::posix_time;

std::string
findRequestExecutor(Config::Conf &conf, METHODS &methods)
{
    METHODS_IT it = methods.begin();
    for (; it != methods.end(); ++it) {
        if (conf.hasOpt(it->first)) {
            return it->second;
        }
    }
    return "";
}

int
main(int argc, char **argv)
{
    Admin::KeysetClient keyset;
    Admin::DomainClient domain;
    Admin::ContactClient contact;
    Admin::InvoiceClient invoice;
    Admin::AuthInfoClient authinfo;
    Admin::BankClient bank;
    Admin::PollClient poll;
    Admin::RegistrarClient registrar;
    Admin::NotifyClient notify;
    Admin::ObjectClient object;
    Admin::InfoBuffClient infobuff;
    Admin::NssetClient nsset;
    Admin::FileClient file;
    Admin::MailClient mail;
    Admin::PublicRequestClient publicrequest;

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
    boost::program_options::options_description all("All allowed options");
    all.add(generalOpts).
        add(generalOptsInvis).
        add(configurationOpts).
        add(*domain.getVisibleOptions()).
        add(*domain.getInvisibleOptions()).
        add(*keyset.getVisibleOptions()).
        add(*keyset.getInvisibleOptions()).
        add(*contact.getVisibleOptions()).
        add(*contact.getInvisibleOptions()).
        add(*invoice.getVisibleOptions()).
        add(*invoice.getInvisibleOptions()).
        add(*authinfo.getVisibleOptions()).
        add(*authinfo.getInvisibleOptions()).
        add(*bank.getVisibleOptions()).
        add(*bank.getInvisibleOptions()).
        add(*poll.getVisibleOptions()).
        add(*poll.getInvisibleOptions()).
        add(*registrar.getVisibleOptions()).
        add(*registrar.getInvisibleOptions()).
        add(*notify.getVisibleOptions()).
        add(*notify.getInvisibleOptions()).
        add(*object.getVisibleOptions()).
        add(*object.getInvisibleOptions()).
        add(*infobuff.getVisibleOptions()).
        add(*infobuff.getInvisibleOptions()).
        add(*nsset.getVisibleOptions()).
        add(*nsset.getInvisibleOptions()).
        add(*file.getVisibleOptions()).
        add(*file.getInvisibleOptions()).
        add(*mail.getVisibleOptions()).
        add(*mail.getInvisibleOptions()).
        add(*publicrequest.getVisibleOptions()).
        add(*publicrequest.getInvisibleOptions()).
        add(commonOpts);

    // only visible options
    boost::program_options::options_description visible("Allowed options");
    visible.add(generalOpts).
        add(configurationOpts).
        add(*domain.getVisibleOptions()).
        add(*keyset.getVisibleOptions()).
        add(*contact.getVisibleOptions()).
        add(*invoice.getVisibleOptions()).
        add(*authinfo.getVisibleOptions()).
        add(*bank.getVisibleOptions()).
        add(*poll.getVisibleOptions()).
        add(*registrar.getVisibleOptions()).
        add(*notify.getVisibleOptions()).
        add(*object.getVisibleOptions()).
        add(*infobuff.getVisibleOptions()).
        add(*nsset.getVisibleOptions()).
        add(*file.getVisibleOptions()).
        add(*mail.getVisibleOptions()).
        add(*publicrequest.getVisibleOptions()).
        add(commonOpts);

    Config::Manager confMan = Config::ConfigManager::instance_ref();
    try {
        confMan.init(argc, argv);
        confMan.setCmdLineOptions(all);
        confMan.setCfgFileOptions(fileOpts, CONFIG_FILE);
        confMan.parse();
    } catch (Config::Manager::ConfigParseError &err) {
        std::cerr << "Config parser error: " << err.what() << std::endl;
        exit(1);
    }

    if (argc == 1 || (argc == 2 && confMan.isHelp())) {
        std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << visible << std::endl;
        exit(0);
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
    //conf.print(std::cout);

   
    Logging::Manager::instance_ref().get("tracer").addHandler(log_type, param);
    Logging::Manager::instance_ref().get("tracer").setLevel(log_level);
    Logging::Manager::instance_ref().get("db").addHandler(log_type, param);
    Logging::Manager::instance_ref().get("db").setLevel(log_level);
    Logging::Manager::instance_ref().get("register").addHandler(log_type, param);
    Logging::Manager::instance_ref().get("register").setLevel(log_level);
    Logging::Manager::instance_ref().get("corba").addHandler(log_type, param);
    Logging::Manager::instance_ref().get("corba").setLevel(log_level);
    Logging::Manager::instance_ref().get("mailer").addHandler(log_type, param);
    Logging::Manager::instance_ref().get("mailer").setLevel(log_level);
    Logging::Manager::instance_ref().get("old_log").addHandler(log_type, param);
    Logging::Manager::instance_ref().get("old_log").setLevel(log_level);
    Logging::Manager::instance_ref().get("fred-server").addHandler(log_type, param);
    Logging::Manager::instance_ref().get("fred-server").setLevel(log_level);
    

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

    METHODS methods_list;

    keyset.init(connstring.str(), nsAddr.str(), conf, methods_list);
    domain.init(connstring.str(), nsAddr.str(), conf, methods_list);
    contact.init(connstring.str(), nsAddr.str(), conf, methods_list);
    invoice.init(connstring.str(), nsAddr.str(), conf, methods_list);
    authinfo.init(connstring.str(), nsAddr.str(), conf, methods_list);
    bank.init(connstring.str(), nsAddr.str(), conf, methods_list);
    poll.init(connstring.str(), nsAddr.str(), conf, methods_list);
    registrar.init(connstring.str(), nsAddr.str(), conf, methods_list);
    notify.init(connstring.str(), nsAddr.str(), conf, methods_list);
    object.init(connstring.str(), nsAddr.str(), conf, methods_list);
    infobuff.init(connstring.str(), nsAddr.str(), conf, methods_list);
    nsset.init(connstring.str(), nsAddr.str(), conf, methods_list);
    file.init(connstring.str(), nsAddr.str(), conf, methods_list);
    mail.init(connstring.str(), nsAddr.str(), conf, methods_list);
    publicrequest.init(connstring.str(), nsAddr.str(), conf, methods_list);

    if (conf.hasUnknown()) {
        std::vector<std::string> unknown(conf.getUnknown());
        std::cout << "Unknown options:" << std::endl;
        for (int i = 0; i < (int)unknown.size(); i++) {
            std::cout << unknown[i] << std::endl;
        }
        return 0;
    }

    if (conf.hasOpt(CLI_MOO_NAME)) {
        print_moo();
        exit(0);
    } else if (conf.hasOpt(CLI_HELP_DATES_NAME)) {
        help_dates();
        exit(0);
    }

    std::string executor = findRequestExecutor(conf, methods_list);
    if (executor.compare(DOMAIN_CLIENT) == 0) {
        domain.runMethod();
    } else if (executor.compare(KEYSET_CLIENT) == 0) {
        keyset.runMethod();
    } else if (executor.compare(CONTACT_CLIENT) == 0) {
        contact.runMethod();
    } else if (executor.compare(INVOICE_CLIENT) == 0) {
        invoice.runMethod();
    } else if (executor.compare(AUTHINFO_CLIENT) == 0) {
        authinfo.runMethod();
    } else if (executor.compare(BANK_CLIENT) == 0) {
        bank.runMethod();
    } else if (executor.compare(POLL_CLIENT) == 0) {
        poll.runMethod();
    } else if (executor.compare(REGISTRAR_CLIENT) == 0) {
        registrar.runMethod();
    } else if (executor.compare(NOTIFY_CLIENT) == 0) {
        notify.runMethod();
    } else if (executor.compare(OBJECT_CLIENT) == 0) {
        object.runMethod();
    } else if (executor.compare(INFOBUFF_CLIENT) == 0) {
        infobuff.runMethod();
    } else if (executor.compare(NSSET_CLIENT) == 0) {
        nsset.runMethod();
    } else if (executor.compare(FILE_CLIENT) == 0) {
        file.runMethod();
    } else if (executor.compare(MAIL_CLIENT) == 0) {
        mail.runMethod();
    } else if (executor.compare(PUBLICREQUEST_CLIENT) == 0) {
        publicrequest.runMethod();
    } else {
        std::cout << "sakrapes, tohle neznam" << std::endl;
    }

    } catch (ccReg::EPP::EppError &e) {
        std::cerr << "EppError code: " << e.errCode << ", message: " 
            << e.errMsg << std::endl;
        for (int ii = 0; ii < (int)e.errorList.length(); ii++) {
            std::cerr
                << "Reason code: " << e.errorList[ii].code
                << ", message: " 
                << e.errorList[ii].reason
                << ", Position: " << e.errorList[ii].position << std::endl;
        }
    } catch (CORBA::Exception &e) {
        std::cerr << "CORBA error" << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;;
        std::exit(2);
    } catch (Register::SQL_ERROR) {
        std::cerr << "SQL ERROR" << std::endl;
        std::exit(1);
    }

    std::exit(0);
}

