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

#ifndef BANKCLIENT_HH_662B03479AF14F298CC7D200FA4752A1
#define BANKCLIENT_HH_662B03479AF14F298CC7D200FA4752A1

#include <boost/program_options.hpp>
#include <iostream>

#include "src/bin/corba/admin/admin_impl.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/bin/cli/baseclient.hh"

#include "src/bin/cli/bank_params.hh"


namespace Admin {

class BankClient : public BaseClient {
private:
    ccReg::EPP_var m_epp;
    std::string nameservice_context;
    bool bank_show_opts;//BANK_SHOW_OPTS_NAME
    bool bank_add_account;//BANK_ADD_ACCOUNT_NAME
    AddAccountArgs add_account_params;

    bool parse_line(const char *line, std::vector<std::string> &vec);

    static const struct options m_opts[];
public:
    BankClient()
    : bank_show_opts(false)
    , bank_add_account(false)
    { }
    BankClient(
            const std::string &connstring
            , const std::string &nsAddr
            , const std::string& _nameservice_context
            , bool _bank_show_opts
            , bool _bank_add_account
            , const AddAccountArgs& _add_account_params
            )
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , bank_show_opts(_bank_show_opts)
    , bank_add_account(_bank_add_account)
    , add_account_params(_add_account_params)
    { }
    ~BankClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void statement_list();
    void add_bank_account();
    void move_statement();

    void statement_list_help();
    void add_bank_account_help();
    void move_statement_help();
}; // class BankClient

} // namespace Admin;

#endif
