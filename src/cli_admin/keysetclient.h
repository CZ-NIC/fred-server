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
#include "fredlib/registry.h"
#include "baseclient.h"

#include "keyset_params.h"

#define KEYSET_SHOW_OPTS_NAME       "keyset_show_opts"
#define KEYSET_SHOW_OPTS_NAME_DESC  "show all keyset command line options"
#define KEYSET_LIST_NAME            "keyset_list"
#define KEYSET_LIST_NAME_DESC       "list of all keysets (via filters)"
#define KEYSET_LIST_PLAIN_NAME      "keyset_list_plain"
#define KEYSET_LIST_PLAIN_NAME_DESC "list of all keysets (via ccReg_i)"
#define KEYSET_CHECK_NAME           "keyset_check"
#define KEYSET_CHECK_NAME_DESC      "check keyset state"
#define KEYSET_INFO_NAME            "keyset_info"
#define KEYSET_INFO_NAME_DESC       "keyset info (via epp_impl)"
#define KEYSET_INFO2_NAME           "keyset_info2"
#define KEYSET_INFO2_NAME_DESC      "keyset info (via ccReg_i::info method)"

#define KEYSET_LIST_HELP_NAME           "keyset_list_help"
#define KEYSET_LIST_HELP_NAME_DESC      "help for keyset list"
#define KEYSET_INFO_HELP_NAME           "keyset_info_help"
#define KEYSET_INFO_HELP_NAME_DESC      "help for keyset info"
#define KEYSET_CHECK_HELP_NAME          "keyset_check_help"
#define KEYSET_CHECK_HELP_NAME_DESC     "help for keyset checking"

namespace Admin {

class KeysetClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;
    std::string nameservice_context;
    bool keyset_list;
    optional_string keyset_check;
    bool keyset_list_plain;
    optional_string keyset_info;
    optional_string keyset_info2;
    bool keyset_show_opts;
    KeysetListArgs m_list_args;

    static const struct options m_opts[];
public:
    KeysetClient()
    : keyset_list(false)
    , keyset_list_plain(false)
    , keyset_show_opts(false)
    { }
    KeysetClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const std::string& _nameservice_context,
            bool _keyset_list,
            const optional_string& _keyset_check,
            bool _keyset_list_plain,
            const optional_string& _keyset_info,
            const optional_string& _keyset_info2,
            bool _keyset_show_opts,
            const KeysetListArgs &_list_args)
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , keyset_list(_keyset_list)
    , keyset_check(_keyset_check)
    , keyset_info2(_keyset_info2)
    , keyset_show_opts(_keyset_show_opts)
    , m_list_args(_list_args)
    {
        m_db = connect_DB(connstring
                , std::runtime_error("KeysetClient db connection failed"));
    }

    static const struct options *getOpts();
    static int getOptsCount();

    void runMethod();

    void show_opts();

    void list();
    void list_plain();
    void check();
    void info();
    void info2();

    void list_help();
    void info_help();
    void check_help();
}; // class KeysetClient

} // namespace Admin;

#endif // _KEYSETCLIENT_H_
