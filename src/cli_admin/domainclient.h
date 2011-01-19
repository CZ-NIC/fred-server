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

#ifndef _DOMAINCLIENT_H_
#define _DOMAINCLIENT_H_

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "baseclient.h"

#define DOMAIN_SHOW_OPTS_NAME           "domain_show_opts"
#define DOMAIN_SHOW_OPTS_NAME_DESC      "show all domain command line options"
#define DOMAIN_LIST_NAME                "domain_list"
#define DOMAIN_LIST_NAME_DESC           "list of all domains (via filters)"
#define DOMAIN_LIST_PLAIN_NAME          "domain_list_plain"
#define DOMAIN_LIST_PLAIN_NAME_DESC     "list of all domains"
#define DOMAIN_INFO_NAME                "domain_info"
#define DOMAIN_INFO_NAME_DESC           "info on domain"

#define DOMAIN_LIST_HELP_NAME           "domain_list_help"
#define DOMAIN_LIST_HELP_NAME_DESC      "help on domain list"

#define DOMAIN_EXP_DATE_NAME            "exp_date"
#define DOMAIN_EXP_DATE_NAME_DESC       "expiration date (type ``./fred-admin --help_dates'' for further date&time information)"
#define DOMAIN_OUT_DATE_NAME            "out_zone_date"
#define DOMAIN_OUT_DATE_NAME_DESC       "out zone date (type ``./fred-admin --help_dates'' for further date&time information)"
#define DOMAIN_CANC_DATE_NAME           "cancel_date"
#define DOMAIN_CANC_DATE_NAME_DESC      "cancel date (type ``./fred-admin --help_dates'' for further date&time information)"

namespace Admin {

class DomainClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    DomainClient()
    { }
    DomainClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        DB* db= new DB;
        if (!db->OpenDatabase(connstring.c_str()))
         {
            throw std::runtime_error("DomainClient db connection failed");
         }
        m_db = DBDisconnectPtr(db);
    }

    ~DomainClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();

    void runMethod();

    void show_opts();
    void domain_list();
    void domain_list_plain();
    void domain_info();

    void domain_list_help();
    void list_help();
}; // class DomainClient

} // namespace Admin;

#endif // _DOMAINCLIENT_H_
