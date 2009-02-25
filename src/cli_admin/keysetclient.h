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

#ifndef _KEYSETCLIENT_H_
#define _KEYSETCLIENT_H_

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "register/register.h"
#include "baseclient.h"

#define KEYSET_SHOW_OPTS_NAME       "keyset_show_opts"
#define KEYSET_SHOW_OPTS_NAME_DESC  "show all keyset command line options"
#define KEYSET_LIST_NAME            "keyset_list"
#define KEYSET_LIST_NAME_DESC       "list of all keysets (via filters)"
#define KEYSET_LIST_PLAIN_NAME      "keyset_list_plain"
#define KEYSET_LIST_PLAIN_NAME_DESC "list of all keysets (via ccReg_i)"
#define KEYSET_CHECK_NAME           "keyset_check"
#define KEYSET_CHECK_NAME_DESC      "check keyset state"
#define KEYSET_SEND_AUTH_INFO_NAME      "keyset_send_auth_info"
#define KEYSET_SEND_AUTH_INFO_NAME_DESC "send authorization info"
#define KEYSET_TRANSFER_NAME        "keyset_transfer"
#define KEYSET_TRANSFER_NAME_DESC   "transfer keyset"
#define KEYSET_UPDATE_NAME          "keyset_update"
#define KEYSET_UPDATE_NAME_DESC     "update keyset"
#define KEYSET_DELETE_NAME          "keyset_delete"
#define KEYSET_DELETE_NAME_DESC     "delete keyset"
#define KEYSET_CREATE_NAME          "keyset_create"
#define KEYSET_CREATE_NAME_DESC     "create keyset"
#define KEYSET_CREATE2_NAME         "keyset_create2"
#define KEYSET_INFO_NAME            "keyset_info"
#define KEYSET_INFO_NAME_DESC       "keyset info (via epp_impl)"
#define KEYSET_INFO2_NAME           "keyset_info2"
#define KEYSET_INFO2_NAME_DESC      "keyset info (via ccReg_i::info method)"

#define KEYSET_DSRECORDS_NAME       "dsrecords"
#define KEYSET_DSRECORDS_NAME_DESC  "list of dsrecords (used with --keyset_create commnad)"
#define KEYSET_DSREC_ADD_NAME       "dsrec_add"
#define KEYSET_DSREC_ADD_NAME_DESC  "list of dsrecords to add"
#define KEYSET_DSREC_REM_NAME       "dsrec_rem"
#define KEYSET_DSREC_REM_NAME_DESC  "list of dsrecords to remove"

#define KEYSET_DNSKEY_NAME          "dnskeys"
#define KEYSET_DNSKEY_NAME_DESC     "list of dnskey records"
#define KEYSET_DNSKEY_ADD_NAME      "dnskey_add"
#define KEYSET_DNSKEY_ADD_NAME_DESC "list of dnskeys to add"
#define KEYSET_DNSKEY_REM_NAME      "dnskey_rem"
#define KEYSET_DNSKEY_REM_NAME_DESC "list of dnskeys to remove"

#define KEYSET_LIST_HELP_NAME           "keyset_list_help"
#define KEYSET_LIST_HELP_NAME_DESC      "help for keyset list"
#define KEYSET_UPDATE_HELP_NAME         "keyset_update_help"
#define KEYSET_UPDATE_HELP_NAME_DESC    "help for keyset updating"
#define KEYSET_CREATE_HELP_NAME         "keyset_create_help"
#define KEYSET_CREATE_HELP_NAME_DESC    "help for keyset creating"
#define KEYSET_DELETE_HELP_NAME         "keyset_delete_help"
#define KEYSET_DELETE_HELP_NAME_DESC    "help for keyset deleting"
#define KEYSET_INFO_HELP_NAME           "keyset_info_help"
#define KEYSET_INFO_HELP_NAME_DESC      "help for keyset info"
#define KEYSET_CHECK_HELP_NAME          "keyset_check_help"
#define KEYSET_CHECK_HELP_NAME_DESC     "help for keyset checking"

namespace Admin {

class KeysetClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    KeysetClient()
    { }
    KeysetClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
    }
    ~KeysetClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();

    void runMethod();

    void show_opts();

    void list();
    void list_plain();
    void check();
    void send_auth_info();
    void transfer();
    void update();
    void del();
    void create();
    void info();
    void info2();

    void list_help();
    void create_help();
    void update_help();
    void delete_help();
    void info_help();
    void check_help();
}; // class KeysetClient

} // namespace Admin;

#endif // _KEYSETCLIENT_H_
