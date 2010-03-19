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


#define BANK_SHOW_OPTS_NAME             "bank_show_opts"
#define BANK_SHOW_OPTS_NAME_DESC        "show all banking options"
#define BANK_STATEMENT_LIST_NAME        "bank_statement_list"
#define BANK_STATEMENT_LIST_NAME_DESC   "list of statements (filterable)"
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

#define BANK_STATEMENT_LIST_HELP_NAME       "bank_statement_list_help"
#define BANK_STATEMENT_LIST_HELP_NAME_DESC  "bank statement list help"

#define BANK_CREDIT_NAME                "bank_credit"
#define BANK_CREDIT_NAME_DESC           "bank credit"

#define BANK_CREATE_CREDIT_INVOICE_NAME     "cr_credit_invoice"
#define BANK_CREATE_CREDIT_INVOICE_NAME_DESC "create also credit invoice if appliable"

#define BANK_ADD_ACCOUNT_NAME           "bank_add_account"
#define BANK_ADD_ACCOUNT_NAME_DESC      "add bank account"
#define BANK_ZONE_ID_NAME               "zone_id"
#define BANK_ZONE_ID_NAME_DESC          "zone id"
#define BANK_ZONE_NAME_NAME             "zone_fqdn"
#define BANK_ZONE_NAME_NAME_DESC        "zone fully qualified domain name"
#define BANK_ACCOUNT_NAME_NAME          "account_name"
#define BANK_ACCOUNT_NAME_NAME_DESC     "account name"

#define BANK_XML_FILE_STATEMENT_NAME           "bank_statement_file"
#define BANK_XML_FILE_STATEMENT_NAME_DESC      "path to original statement file"
#define BANK_XML_FILE_STATEMENT_MIME_NAME           "bank_statement_file_mimetype"
#define BANK_XML_FILE_STATEMENT_MIME_NAME_DESC      "mime type of original statement file"

#define BANK_MOVE_STATEMENT_NAME             "add_payment_to_statement"
#define BANK_MOVE_STATEMENT_NAME_DESC        "add payment to statement"

#define BANK_SET_PAYMENT_TYPE_NAME           "bank_set_payment_type"
#define BANK_SET_PAYMENT_TYPE_NAME_DESC      "change type of payment"

#define BANK_PAYMENT_LIST_NAME               "bank_payment_list"
#define BANK_PAYMENT_LIST_NAME_DESC          "list of payments (filterable)"

#define BANK_STATEMENT_ID_NAME               "bank_statement_id"
#define BANK_STATEMENT_ID_NAME_DESC          "statement id"
#define BANK_PAYMENT_ID_NAME                 "bank_payment_id"
#define BANK_PAYMENT_ID_NAME_DESC            "payment id"
#define BANK_PAYMENT_TYPE_NAME               "bank_payment_type"
#define BANK_PAYMENT_TYPE_NAME_DESC          "payment type"
#define BANK_FORCE_NAME                      "force"
#define BANK_FORCE_NAME_DESC                 "force"



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
    bool parse_line(const char *line, std::vector<std::string> &vec);

    static const struct options m_opts[];
public:
    BankClient()
    { }
    BankClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
    }
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
