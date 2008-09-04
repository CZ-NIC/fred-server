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
#include "old_utils/log.h"
#include "old_utils/conf.h"
#include "register/register.h"
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"

#include "simpleclient.h"
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

    try {
    boost::program_options::options_description generalOpts("General options");
    generalOpts.add_options()
        ("help,h",
         "print this help and quit")
        ("version,V",
         "print version and license information and quit")
        ("conf",
         boost::program_options::value<std::string>()->default_value(CONFIG_FILE),
         "configuration file")
        (LANGUAGE_NAME,
         boost::program_options::value<std::string>()->default_value("CS"),
         "communication language");

    boost::program_options::options_description generalOptsInvis("General invisible options");
    generalOptsInvis.add_options()
        ("moo",
         "moo");

    boost::program_options::options_description configurationOpts("Configuration options");
    configurationOpts.add_options()
        (DBNAME_NAME, 
         boost::program_options::value<std::string>()->default_value("fred"),
         "database name")
        (DBUSER_NAME, 
         boost::program_options::value<std::string>()->default_value("fred"),
         "database user")
        (DBPASS_NAME, 
         boost::program_options::value<std::string>(),
         "database password")
        (DBHOST_NAME, 
         boost::program_options::value<std::string>()->default_value("localhost"),
         "database host address")
        (DBPORT_NAME, 
         boost::program_options::value<unsigned int>()->default_value(5432),
         "database port")
        (NSHOST_NAME, 
         boost::program_options::value<std::string>()->default_value("localhost"),
         "CORBA nameservice host")
        (NSPORT_NAME, 
         boost::program_options::value<unsigned int>()->default_value(2809),
         "CORBA nameservice port")
        (NSSERVICE_NAME,
         boost::program_options::value<std::string>()->default_value("localhost:2809"),
         "CORBA nameservice address")
        (LOG_LEVEL_NAME, 
         boost::program_options::value<unsigned int>()->default_value(ERROR_LOG),
         "minimal level of logging")
        (LOG_LOCAL_NAME, 
         boost::program_options::value<unsigned int>()->default_value(2),
         "syslog local facility number")
        (DOCGEN_PATH_NAME,
         boost::program_options::value<std::string>()->default_value("/usr/bin/fred-doc2pdf"),
         "path to fred2pdf document generator")
        (DOCGEN_TEMPLATE_PATH_NAME,
         boost::program_options::value<std::string>()->default_value("/usr/share/fred2pdf/templates"),
         "path to fred2pdf document generator templates")
        (FILECLIENT_PATH_NAME, 
         boost::program_options::value<std::string>()->default_value("/usr/bin/filemanager_client"),
         "path to corba client file manager")
        (RESTRICTED_HANDLES_NAME,
         boost::program_options::value<unsigned int>()->default_value(1),
         "restricted format for handles")
        (DEBUG_NAME,
         //boost::program_options::value<unsigned int>(),
         "debug");

    boost::program_options::options_description fileDesc("");
    fileDesc.add(configurationOpts);
    fileDesc.add_options()
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
        add(commonOpts);


    boost::program_options::store(
            boost::program_options::parse_command_line(
                argc,
                argv,
                all
                ),
            varMap);

    std::ifstream configFile(varMap["conf"].as<std::string>().c_str());
    boost::program_options::store(
            boost::program_options::parse_config_file(
                configFile,
                fileDesc),
            varMap);

    if (varMap.count("help") || argc == 1) {
        std::cout << visible << std::endl;
        exit(0);
    }
    if (varMap.count("version")) {
        print_version();
        exit(0);
    }
    if (varMap.count("moo")) {
        print_moo();
        exit(0);
    }

    SysLogger::get().setLevel(
            varMap["log-level"].as<unsigned int>());
    SysLogger::get().setFacility(
            varMap["log-local"].as<unsigned int>());    

    Logging::Manager::instance_ref().get("tracer").addHandler(Logging::Log::LT_SYSLOG);
    Logging::Manager::instance_ref().get("tracer").setLevel(Logging::Log::LL_TRACE);
    Logging::Manager::instance_ref().get("db").addHandler(Logging::Log::LT_SYSLOG);
    Logging::Manager::instance_ref().get("db").setLevel(Logging::Log::LL_DEBUG);    

    std::stringstream connstring;
    std::stringstream nsAddr;

    nsAddr << varMap[NSSERVICE_NAME].as<std::string>();

    connstring << "dbname=" << varMap[DBNAME_NAME].as<std::string>() 
               << " user=" << varMap[DBUSER_NAME].as<std::string>();
    if (varMap.count(DBHOST_NAME))
        connstring << " host=" << varMap[DBHOST_NAME].as<std::string>();
    if (varMap.count(DBPORT_NAME))
        connstring << " port=" << varMap[DBPORT_NAME].as<unsigned int>();
    if (varMap.count(DBPASS_NAME))
        connstring << " password=" << varMap[DBPASS_NAME].as<std::string>();

    keyset.init(connstring.str(), nsAddr.str(), varMap);
    domain.init(connstring.str(), nsAddr.str(), varMap);
    contact.init(connstring.str(), nsAddr.str(), varMap);
    invoice.init(connstring.str(), nsAddr.str(), varMap);
    authinfo.init(connstring.str(), nsAddr.str(), varMap);
    bank.init(connstring.str(), nsAddr.str(), varMap);
    poll.init(connstring.str(), nsAddr.str(), varMap);
    registrar.init(connstring.str(), nsAddr.str(), varMap);
    notify.init(connstring.str(), nsAddr.str(), varMap);
    object.init(connstring.str(), nsAddr.str(), varMap);
    infobuff.init(connstring.str(), nsAddr.str(), varMap);
    nsset.init(connstring.str(), nsAddr.str(), varMap);
    file.init(connstring.str(), nsAddr.str(), varMap);
    mail.init(connstring.str(), nsAddr.str(), varMap);

    if (varMap.count("show-opts")) {
        std::cout << "config: " << varMap["conf"].as<std::string>() << std::endl;
        std::cout << "dbname: " << varMap[DBNAME_NAME].as<std::string>() << std::endl;
        std::cout << "dbuser: " << varMap[DBUSER_NAME].as<std::string>() << std::endl;
        std::cout << "dbpass: " << (varMap.count(DBPASS_NAME) ?
            varMap[DBPASS_NAME].as<std::string>() : "")
            << std::endl;
        std::cout << "dbhost: " << varMap[DBHOST_NAME].as<std::string>() << std::endl;
        std::cout << "dbport: " << varMap[DBPORT_NAME].as<unsigned int>() << std::endl;
        std::cout << "nameservice: " << varMap[NSSERVICE_NAME].as<std::string>() << std::endl;
        std::exit(0);
    }

    if (varMap.count(CONTACT_INFO2_NAME)) {
        contact.info2();
    } else if (varMap.count(CONTACT_INFO_NAME)) {
        contact.info();
    } else if (varMap.count(CONTACT_LIST_NAME)) {
        contact.list();
    } else if (varMap.count(CONTACT_LIST_HELP_NAME)) {
        contact.list_help();
    }

    if (varMap.count(KEYSET_LIST_NAME)) {
        keyset.list();
    } else if (varMap.count(KEYSET_CHECK_NAME)) {
        keyset.check();
    } else if (varMap.count(KEYSET_SEND_AUTH_INFO_NAME)) {
        keyset.send_auth_info();
    } else if (varMap.count(KEYSET_TRANSFER_NAME)) {
        keyset.transfer();
    } else if (varMap.count(KEYSET_LIST_PLAIN_NAME)) {
        keyset.list_plain();
    } else if (varMap.count(KEYSET_UPDATE_NAME)) {
        keyset.update();
    } else if (varMap.count(KEYSET_DELETE_NAME)) {
        keyset.del();
    } else if (varMap.count(KEYSET_UPDATE_HELP_NAME)) {
        keyset.update_help();
    } else if (varMap.count(KEYSET_CREATE_HELP_NAME)) {
        keyset.create_help();
    } else if (varMap.count(KEYSET_DELETE_HELP_NAME)) {
        keyset.delete_help();
    } else if (varMap.count(KEYSET_INFO_HELP_NAME)) {
        keyset.info_help();
    } else if (varMap.count(KEYSET_CHECK_HELP_NAME)) {
        keyset.check_help();
    } else if (varMap.count(KEYSET_LIST_HELP_NAME)) {
        keyset.list_help();
    } else if (varMap.count(KEYSET_CREATE_NAME)) {
        keyset.create();
    } else if (varMap.count(KEYSET_INFO2_NAME)) {
        keyset.info2();
    } else if (varMap.count(KEYSET_INFO_NAME)) {
        keyset.info();
    } else if (varMap.count(DOMAIN_LIST_NAME)) {
        domain.domain_list();
    }
    
    if (varMap.count(DOMAIN_LIST_PLAIN_NAME)) {
        domain.domain_list_plain();
    } else if (varMap.count(DOMAIN_CREATE_HELP_NAME)) {
        domain.domain_create_help();
    } else if (varMap.count(DOMAIN_UPDATE_HELP_NAME)) {
        domain.domain_update_help();
    } else if (varMap.count(DOMAIN_CREATE_NAME)) {
        domain.domain_create();
    } else if (varMap.count(DOMAIN_UPDATE_NAME)) {
        domain.domain_update();
    } else if (varMap.count(DOMAIN_INFO_NAME)) {
        domain.domain_info();
    } else if (varMap.count(DOMAIN_LIST_HELP_NAME)) {
        domain.list_help();
    }
    
    if (varMap.count(INVOICE_LIST_NAME)) {
        invoice.list();
    } else if (varMap.count(INVOICE_ARCHIVE_NAME)) {
        invoice.archive();
    } else if (varMap.count(INVOICE_LIST_HELP_NAME)) {
        invoice.list_help();
    } else if (varMap.count(INVOICE_ARCHIVE_HELP_NAME)) {
        invoice.archive_help();
    }

    if (varMap.count(AUTHINFO_PDF_NAME)) {
        authinfo.pdf();
    } else if (varMap.count(AUTHINFO_PDF_HELP_NAME)) {
        authinfo.pdf_help();
    }
    
    if (varMap.count(BANK_ONLINE_LIST_NAME)) {
        bank.online_list();
    } else if (varMap.count(BANK_STATEMENT_LIST_NAME)) {
        bank.statement_list();
    }

    if (varMap.count(POLL_LIST_ALL_NAME)) {
        poll.list_all();
    } else if (varMap.count(POLL_LIST_NEXT_NAME)) {
        poll.list_next();
    } else if (varMap.count(POLL_CREATE_STATE_CHANGES_NAME)) {
        poll.create_state_changes();
    } else if (varMap.count(POLL_CREATE_LOW_CREDIT_NAME)) {
        poll.create_low_credit();
    } else if (varMap.count(POLL_SET_SEEN_NAME)) {
        poll.set_seen();
    }
    
    if (varMap.count(REGISTRAR_ZONE_ADD_NAME)) {
        registrar.zone_add();
    } else if (varMap.count(REGISTRAR_REGISTRAR_ADD_NAME)) {
        registrar.registrar_add();
    } else if (varMap.count(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)) {
        registrar.registrar_add_zone();
    } else if (varMap.count(REGISTRAR_ZONE_ADD_HELP_NAME)) {
        registrar.zone_add_help();
    } else if (varMap.count(REGISTRAR_REGISTRAR_ADD_HELP_NAME)) {
        registrar.registrar_add_help();
    } else if (varMap.count(REGISTRAR_REGISTRAR_ADD_ZONE_HELP_NAME)) {
        registrar.registrar_add_zone_help();
    } else if (varMap.count(REGISTRAR_LIST_NAME)) {
        registrar.list();
    }
    
    if (varMap.count(NOTIFY_STATE_CHANGES_NAME)) {
        notify.state_changes();
    } else if (varMap.count(NOTIFY_LETTERS_CREATE_NAME)) {
        notify.letters_create();
    }

    if (varMap.count(OBJECT_NEW_STATE_REQUEST_NAME)) {
        object.new_state_request();
    } else if (varMap.count(OBJECT_LIST_NAME)) {
        object.list();
    } else if (varMap.count(OBJECT_UPDATE_STATES_NAME)) {
        object.update_states();
    } else if (varMap.count(OBJECT_DELETE_CANDIDATES_NAME)) {
        object.delete_candidates();
    }
    
    if (varMap.count(NSSET_LIST_NAME)) {
        nsset.list();
    } else if (varMap.count(NSSET_LIST_HELP_NAME)) {
        nsset.list_help();
    }

    if (varMap.count(FILE_LIST_NAME)) {
        file.list();
    } else if (varMap.count(FILE_LIST_HELP_NAME)) {
        file.list_help();
    }

    if (varMap.count(MAIL_LIST_NAME)) {
        mail.list();
    } else if (varMap.count(MAIL_LIST_HELP_NAME)) {
        file.list_help();
    }

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;;
        std::exit(2);
    } catch (Register::SQL_ERROR) {
        std::cerr << "SQL ERROR" << std::endl;
        std::exit(1);
    }

    std::exit(0);
}

