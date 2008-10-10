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

#ifndef _BANKCLIENT_H_
#define _BANKCLIENT_H_


#define BANK_SHOW_OPTS_NAME         "bank_show_opts"
#define BANK_SHOW_OPTS_NAME_DESC    "show all banking options"
#define BANK_ONLINE_LIST_NAME       "bank_online_list"
#define BANK_ONLINE_LIST_NAME_DESC  "list of online payments"
#define BANK_STATEMENT_LIST_NAME    "bank_statement_list"
#define BANK_STATEMENT_LIST_NAME_DESC "list of bank statements"

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"


namespace Admin {

class BankClient {
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
    BankClient();
    BankClient(std::string connstring,
            std::string nsAddr);
    ~BankClient();
    void init(std::string connstring,
            std::string nsAddr,
            Config::Conf &conf);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;
    void show_opts() const;

    int online_list();
    int statement_list();
};

} // namespace Admin;

#endif //_BANKCLIENT_H_
