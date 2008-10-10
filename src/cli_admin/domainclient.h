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

#ifndef _DOMAINCLIENT_H_
#define _DOMAINCLIENT_H_

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"

#define DOMAIN_SHOW_OPTS_NAME           "domain_show_opts"
#define DOMAIN_SHOW_OPTS_NAME_DESC      "show all domain command line options"
#define DOMAIN_CREATE_NAME              "domain_create"
#define DOMAIN_CREATE_NAME_DESC         "create domain"
#define DOMAIN_UPDATE_NAME              "domain_update"
#define DOMAIN_UPDATE_NAME_DESC         "update domain"
#define DOMAIN_LIST_NAME                "domain_list"
#define DOMAIN_LIST_NAME_DESC           "list of all domains (via filters)"
#define DOMAIN_LIST_PLAIN_NAME          "domain_list_plain"
#define DOMAIN_LIST_PLAIN_NAME_DESC     "list of all domains"
#define DOMAIN_INFO_NAME                "domain_info"
#define DOMAIN_INFO_NAME_DESC           "info on domain"

#define DOMAIN_REGISTRANT_NAME          "registrant"
#define DOMAIN_REGISTRANT_NAME_DESC     "registrant description"
#define DOMAIN_NSSET_NAME               "nsset"
#define DOMAIN_NSSET_NAME_DESC          "nsset description"
#define DOMAIN_KEYSET_NAME              "keyset"
#define DOMAIN_KEYSET_NAME_DESC         "keyset description"
#define DOMAIN_PERIOD_NAME              "period"
#define DOMAIN_PERIOD_NAME_DESC         "period description"

#define DOMAIN_CREATE_HELP_NAME         "domain_create_help"
#define DOMAIN_CREATE_HELP_NAME_DESC    "help on domain create"
#define DOMAIN_UPDATE_HELP_NAME         "domain_update_help"
#define DOMAIN_UPDATE_HELP_NAME_DESC    "help on domain update"
#define DOMAIN_LIST_HELP_NAME           "domain_list_help"
#define DOMAIN_LIST_HELP_NAME_DESC      "help on domain list"

#define DOMAIN_EXP_DATE_FROM_NAME       "exp_date_from"
#define DOMAIN_EXP_DATE_FROM_NAME_DESC  "expiration date from (date format: \"YYYY-MM-DD\")"
#define DOMAIN_EXP_DATE_TO_NAME         "exp_date_to"
#define DOMAIN_EXP_DATE_TO_NAME_DESC    "expiration date to (date format: \"YYYY-MM-DD\")"
#define DOMAIN_OUT_DATE_FROM_NAME       "out_zone_date_from"
#define DOMAIN_OUT_DATE_FROM_NAME_DESC  "out zone date from (date format: \"YYYY-MM-DD\")"
#define DOMAIN_OUT_DATE_TO_NAME         "out_zone_date_to"
#define DOMAIN_OUT_DATE_TO_NAME_DESC    "out zone date to (date format: \"YYYY-MM-DD\")"
#define DOMAIN_CANC_DATE_FROM_NAME      "cancel_date_from"
#define DOMAIN_CANC_DATE_FROM_NAME_DESC "cancel date from (date format: \"YYYY-MM-DD\")"
#define DOMAIN_CANC_DATE_TO_NAME        "cancel_date_to"
#define DOMAIN_CANC_DATE_TO_NAME_DESC   "cancel date to (date format: \"YYYY-MM-DD\")"

namespace Admin {

class DomainClient {
private:
    std::string m_connstring;
    std::string m_nsAddr;
    CORBA::Long m_clientId;
    DB m_db;
    Database::Manager *m_dbman;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
public:
    DomainClient();
    DomainClient(std::string connstring,
            std::string nsAddr);
    ~DomainClient();
    void init(std::string connstring,
            std::string nsAddr,
            Config::Conf &conf);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;
    void show_opts() const;

    void domain_list();

    int domain_list_plain();

    int domain_create();

    int domain_update();

    int domain_info();

    void domain_list_help();
    void domain_update_help();
    void domain_create_help();
    void list_help();
};

} // namespace Admin;

#endif // _DOMAINCLIENT_H_
