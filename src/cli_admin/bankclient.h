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


#define BANK_SHOW_OPTS_NAME             "bank_show_opts"
#define BANK_SHOW_OPTS_NAME_DESC        "show all banking options"
#define BANK_STATEMENT_LIST_NAME        "bank_statement_list"
#define BANK_STATEMENT_LIST_NAME_DESC   "list of payments (filterable)"
#define BANK_ONLINE_LIST_NAME           "bank_online_list"
#define BANK_ONLINE_LIST_NAME_DESC      "list of online payments (filterable)"
#define BANK_DATE_NAME                  "bank_date"
#define BANK_DATE_NAME_DESC             "bank create date"
#define BANK_ID_NAME                    "bank_id"
#define BANK_ID_NAME_DESC               "bank_id_desc"
#define BANK_ACCOUNT_ID_NAME            "bank_account_id"
#define BANK_ACCOUNT_ID_NAME_DESC       "bank_account_id description"
#define BANK_OLD_BALANCE_DATE_NAME      "bank_old_balance_date"
#define BANK_OLD_BALANCE_DATE_NAME_DESC "bank_old_balance_date description"
#define BANK_ACCOUNT_NUMBER_NAME        "bank_account_number"
#define BANK_ACCOUNT_NUMBER_NAME_DESC   "bank_account_number description"
#define BANK_BANK_CODE_NAME             "bank_code"
#define BANK_BANK_CODE_NAME_DESC        "bank_code description"
#define BANK_CONST_SYMBOL_NAME          "bank_const_symbol"
#define BANK_CONST_SYMBOL_NAME_DESC     "bank const symbol desctription"
#define BANK_SORT_NAME                  "bank_sort"
#define BANK_SORT_NAME_DESC             "sort statement list"
#define BANK_SORT_DESC_NAME             "sort_desc"
#define BANK_SORT_DESC_NAME_DESC        "sort descending (default is ascending)"
#define BANK_INVOICE_ID_NAME            "bank_invoice_id"
#define BANK_INVOICE_ID_NAME_DESC       "invoice id"

#define BANK_IMPORT_XML_NAME            "bank_import_xml"
#define BANK_IMPORT_XML_NAME_DESC       "xml file with bank statement(s)"

#define BANK_IMPORT_XML_HELP_NAME       "bank_import_xml_help"
#define BANK_IMPORT_XML_HELP_NAME_DESC  "bank_import_xml help"

#define BANK_XML_FILE_NAME              "bank_xml"
#define BANK_XML_FILE_NAME_DESC         "xml file name"

#define BANK_ONLINE_NAME                "bank_online"
#define BANK_ONLINE_NAME_DESC           "if imported bank statement is online"

#define BANK_STATEMENT_LIST_HELP_NAME       "bank_statement_list_help"
#define BANK_STATEMENT_LIST_HELP_NAME_DESC  "bank statement list help"
#define BANK_ONLINE_LIST_HELP_NAME          "bank_online_list_help"
#define BANK_ONLINE_LIST_HELP_NAME_DESC     "bank_online_list_help"

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "baseclient.h"


namespace Admin {

class BankClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
    bool parse_line(const char *line, std::vector<std::string> &vec);
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

    void statement_list();
    void online_list();

    void import_xml();
    void import_xml_help();

    void online_list_help();
    void statement_list_help();
};

} // namespace Admin;

#endif //_BANKCLIENT_H_
