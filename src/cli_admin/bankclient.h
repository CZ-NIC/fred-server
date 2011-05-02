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

#ifndef _BANKCLIENT_H_
#define _BANKCLIENT_H_

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "baseclient.h"

#include "bank_params.h"


namespace Admin {

class BankClient : public BaseClient {
private:
    ccReg::EPP_var m_epp;
    std::string nameservice_context;
    bool bank_show_opts;//BANK_SHOW_OPTS_NAME
    bool bank_payment_list;//BANK_PAYMENT_LIST_NAME
    optional_ulong bank_payment_type;//BANK_PAYMENT_TYPE_NAME
    bool bank_import_xml;//BANK_IMPORT_XML_NAME
    ImportXMLArgs import_xml_params;
    bool bank_add_account;//BANK_ADD_ACCOUNT_NAME
    AddAccountArgs add_account_params;

    bool parse_line(const char *line, std::vector<std::string> &vec);

    static const struct options m_opts[];
public:
    BankClient()
    : bank_show_opts(false)
    , bank_payment_list(false)
    , bank_import_xml(false)
    , bank_add_account(false)
    { }
    BankClient(
            const std::string &connstring
            , const std::string &nsAddr
            , const std::string& _nameservice_context
            , bool _bank_show_opts
            , bool _bank_payment_list
            , const optional_ulong& _bank_payment_type
            , bool _bank_import_xml
            , const ImportXMLArgs& _import_xml_params
            , bool _bank_add_account
            , const AddAccountArgs& _add_account_params
            )
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , bank_show_opts(_bank_show_opts)
    , bank_payment_list(_bank_payment_list)
    , bank_payment_type(_bank_payment_type)
    , bank_import_xml(_bank_import_xml)
    , import_xml_params(_import_xml_params)
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
    void payment_list();
    void import_xml();
    void import_xml_help();
    void add_bank_account();
    void move_statement();
    void set_payment_type();

    void statement_list_help();
    void payment_list_help();
    void add_bank_account_help();
    void move_statement_help();
    void set_payment_type_help();
}; // class BankClient

} // namespace Admin;

#endif //_BANKCLIENT_H_
