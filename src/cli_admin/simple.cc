/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
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
        addOptStrDef(CLI_LANGUAGE_NAME, "CS");

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
        addOptStr(REG_DOCGEN_TEMPLATE_PATH_NAME);

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

    if (argc == 1 || confMan.isHelp()) {
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

    keyset.init(connstring.str(), nsAddr.str(), conf);
    domain.init(connstring.str(), nsAddr.str(), conf);
    contact.init(connstring.str(), nsAddr.str(), conf);
    invoice.init(connstring.str(), nsAddr.str(), conf);
    authinfo.init(connstring.str(), nsAddr.str(), conf);
    bank.init(connstring.str(), nsAddr.str(), conf);
    poll.init(connstring.str(), nsAddr.str(), conf);
    registrar.init(connstring.str(), nsAddr.str(), conf);
    notify.init(connstring.str(), nsAddr.str(), conf);
    object.init(connstring.str(), nsAddr.str(), conf);
    infobuff.init(connstring.str(), nsAddr.str(), conf);
    nsset.init(connstring.str(), nsAddr.str(), conf);
    file.init(connstring.str(), nsAddr.str(), conf);
    mail.init(connstring.str(), nsAddr.str(), conf);
    publicrequest.init(connstring.str(), nsAddr.str(), conf);

    if (conf.hasUnknown()) {
        std::vector<std::string> unknown(conf.getUnknown());
        std::cout << "Unknown options:" << std::endl;
        for (int i = 0; i < (int)unknown.size(); i++) {
            std::cout << unknown[i] << std::endl;
        }
        return 0;
    }

    if (conf.hasOpt(CONTACT_INFO2_NAME)) {
        contact.info2();
    } else if (conf.hasOpt(CONTACT_INFO_NAME)) {
        contact.info();
    } else if (conf.hasOpt(CONTACT_LIST_NAME)) {
        contact.list();
    } else if (conf.hasOpt(CONTACT_LIST_HELP_NAME)) {
        contact.list_help();
    } else if (conf.hasOpt(CONTACT_SHOW_OPTS_NAME)) {
        contact.show_opts();
    }

    if (conf.hasOpt(KEYSET_LIST_NAME)) {
        keyset.list();
    } else if (conf.hasOpt(KEYSET_CHECK_NAME)) {
        keyset.check();
    } else if (conf.hasOpt(KEYSET_SEND_AUTH_INFO_NAME)) {
        keyset.send_auth_info();
    } else if (conf.hasOpt(KEYSET_TRANSFER_NAME)) {
        keyset.transfer();
    } else if (conf.hasOpt(KEYSET_LIST_PLAIN_NAME)) {
        keyset.list_plain();
    } else if (conf.hasOpt(KEYSET_UPDATE_NAME)) {
        keyset.update();
    } else if (conf.hasOpt(KEYSET_DELETE_NAME)) {
        keyset.del();
    } else if (conf.hasOpt(KEYSET_UPDATE_HELP_NAME)) {
        keyset.update_help();
    } else if (conf.hasOpt(KEYSET_CREATE_HELP_NAME)) {
        keyset.create_help();
    } else if (conf.hasOpt(KEYSET_DELETE_HELP_NAME)) {
        keyset.delete_help();
    } else if (conf.hasOpt(KEYSET_INFO_HELP_NAME)) {
        keyset.info_help();
    } else if (conf.hasOpt(KEYSET_CHECK_HELP_NAME)) {
        keyset.check_help();
    } else if (conf.hasOpt(KEYSET_LIST_HELP_NAME)) {
        keyset.list_help();
    } else if (conf.hasOpt(KEYSET_CREATE_NAME)) {
        keyset.create();
    } else if (conf.hasOpt(KEYSET_INFO2_NAME)) {
        keyset.info2();
    } else if (conf.hasOpt(KEYSET_INFO_NAME)) {
        keyset.info();
    } else if (conf.hasOpt(KEYSET_SHOW_OPTS_NAME)) {
        keyset.show_opts();
    }
    
    if (conf.hasOpt(DOMAIN_LIST_PLAIN_NAME)) {
        domain.domain_list_plain();
    } else if (conf.hasOpt(DOMAIN_CREATE_HELP_NAME)) {
        domain.domain_create_help();
    } else if (conf.hasOpt(DOMAIN_UPDATE_HELP_NAME)) {
        domain.domain_update_help();
    } else if (conf.hasOpt(DOMAIN_CREATE_NAME)) {
        domain.domain_create();
    } else if (conf.hasOpt(DOMAIN_UPDATE_NAME)) {
        domain.domain_update();
    } else if (conf.hasOpt(DOMAIN_INFO_NAME)) {
        domain.domain_info();
    } else if (conf.hasOpt(DOMAIN_LIST_HELP_NAME)) {
        domain.list_help();
    } else if (conf.hasOpt(DOMAIN_LIST_NAME)) {
        domain.domain_list();
    } else if (conf.hasOpt(DOMAIN_SHOW_OPTS_NAME)) {
        domain.show_opts();
    }
    
    if (conf.hasOpt(INVOICE_LIST_NAME)) {
        invoice.list();
    } else if (conf.hasOpt(INVOICE_ARCHIVE_NAME)) {
        invoice.archive();
    } else if (conf.hasOpt(INVOICE_LIST_HELP_NAME)) {
        invoice.list_help();
    } else if (conf.hasOpt(INVOICE_ARCHIVE_HELP_NAME)) {
        invoice.archive_help();
    } else if (conf.hasOpt(INVOICE_LIST_FILTERS_NAME)) {
        invoice.list_filters();
    } else if (conf.hasOpt(INVOICE_SHOW_OPTS_NAME)) {
        invoice.show_opts();
    }

    if (conf.hasOpt(AUTHINFO_PDF_NAME)) {
        authinfo.pdf();
    } else if (conf.hasOpt(AUTHINFO_PDF_HELP_NAME)) {
        authinfo.pdf_help();
    } else if (conf.hasOpt(AUTHINFO_SHOW_OPTS_NAME)) {
        authinfo.show_opts();
    }
    
    if (conf.hasOpt(BANK_ONLINE_LIST_NAME)) {
        bank.online_list();
    } else if (conf.hasOpt(BANK_STATEMENT_LIST_NAME)) {
        bank.statement_list();
    } else if (conf.hasOpt(BANK_SHOW_OPTS_NAME)) {
        bank.show_opts();
    }

    if (conf.hasOpt(POLL_LIST_ALL_NAME)) {
        poll.list_all();
    } else if (conf.hasOpt(POLL_LIST_NEXT_NAME)) {
        poll.list_next();
    } else if (conf.hasOpt(POLL_CREATE_STATE_CHANGES_NAME) ||
            conf.hasOpt(POLL_CREATE_STATE_CHANGES_2_NAME)) {
        poll.create_state_changes();
    } else if (conf.hasOpt(POLL_CREATE_LOW_CREDIT_NAME) ||
            conf.hasOpt(POLL_CREATE_LOW_CREDIT_2_NAME)) {
        poll.create_low_credit();
    } else if (conf.hasOpt(POLL_SET_SEEN_NAME)) {
        poll.set_seen();
    } else if (conf.hasOpt(POLL_SHOW_OPTS_NAME)) {
        poll.show_opts();
    }
    
    if (conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)) {
        registrar.zone_add();
    } else if (conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)) {
        registrar.registrar_add();
    } else if (conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)) {
        registrar.registrar_add_zone();
    } else if (conf.hasOpt(REGISTRAR_ZONE_ADD_HELP_NAME)) {
        registrar.zone_add_help();
    } else if (conf.hasOpt(REGISTRAR_REGISTRAR_ADD_HELP_NAME)) {
        registrar.registrar_add_help();
    } else if (conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_HELP_NAME)) {
        registrar.registrar_add_zone_help();
    } else if (conf.hasOpt(REGISTRAR_LIST_NAME)) {
        registrar.list();
    } else if (conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)) {
        registrar.show_opts();
    }
    
    if (conf.hasOpt(NOTIFY_STATE_CHANGES_NAME)) {
        notify.state_changes();
    } else if (conf.hasOpt(NOTIFY_LETTERS_CREATE_NAME)) {
        notify.letters_create();
    } else if (conf.hasOpt(NOTIFY_SHOW_OPTS_NAME)) {
        notify.show_opts();
    }

    if (conf.hasOpt(OBJECT_NEW_STATE_REQUEST_NAME)) {
        object.new_state_request();
    } else if (conf.hasOpt(OBJECT_LIST_NAME)) {
        object.list();
    } else if (conf.hasOpt(OBJECT_UPDATE_STATES_NAME)) {
        object.update_states();
    } else if (conf.hasOpt(OBJECT_DELETE_CANDIDATES_NAME)) {
        object.delete_candidates();
    } else if (conf.hasOpt(OBJECT_REGULAR_PROCEDURE_NAME)) {
        object.regular_procedure();
    } else if (conf.hasOpt(OBJECT_SHOW_OPTS_NAME)) {
        object.show_opts();
    }
    
    if (conf.hasOpt(NSSET_LIST_NAME)) {
        nsset.list();
    } else if (conf.hasOpt(NSSET_LIST_HELP_NAME)) {
        nsset.list_help();
    } else if (conf.hasOpt(NSSET_SHOW_OPTS_NAME)) {
        nsset.show_opts();
    }

    if (conf.hasOpt(FILE_LIST_NAME)) {
        file.list();
    } else if (conf.hasOpt(FILE_LIST_HELP_NAME)) {
        file.list_help();
    } else if (conf.hasOpt(FILE_SHOW_OPTS_NAME)) {
        file.show_opts();
    }

    if (conf.hasOpt(MAIL_LIST_NAME)) {
        mail.list();
    } else if (conf.hasOpt(MAIL_LIST_HELP_NAME)) {
        mail.list_help();
    } else if (conf.hasOpt(MAIL_SHOW_OPTS_NAME)) {
        mail.show_opts();
    }

    if (conf.hasOpt(PUBLICREQ_LIST_NAME)) {
        publicrequest.list();
    } else if (conf.hasOpt(PUBLICREQ_LIST_HELP_NAME)) {
        publicrequest.list_help();
    } else if (conf.hasOpt(PUBLICREQ_SHOW_OPTS_NAME)) {
        publicrequest.show_opts();
    }

    } catch (ccReg::EPP::EppError &e) {
        std::cerr << "EppError code: " << e.errCode << ", message: " 
            << e.errMsg << std::endl;
        for (int ii = 0; ii < (int)e.errorList.length(); ii++) {
            std::cerr << "Reason code: " << e.errorList[ii].code << ", message: " 
                << e.errorList[ii].reason << std::endl;
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

