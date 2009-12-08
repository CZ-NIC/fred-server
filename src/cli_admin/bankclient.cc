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

#include "simple.h"
#include "bankclient.h"
#include "commonclient.h"
#include "register/bank_manager.h"
#include "register/register.h"

#include <iostream>
#include <fstream>
#include <string>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/confix.hpp>
#include <boost/spirit/utility/lists.hpp>
#include <boost/spirit/utility/escape_char.hpp>

namespace Admin {

const struct options *
BankClient::getOpts()
{
    return m_opts;
}

void
BankClient::runMethod()
{
    if (m_conf.hasOpt(BANK_SHOW_OPTS_NAME)) {
        show_opts();
    } else if (m_conf.hasOpt(BANK_STATEMENT_LIST_NAME)) {
        statement_list();
    } else if (m_conf.hasOpt(BANK_IMPORT_XML_NAME)) {
        import_xml();
    } else if (m_conf.hasOpt(BANK_ADD_ACCOUNT_NAME)) {
        add_bank_account();
    } else if (m_conf.hasOpt(BANK_MOVE_STATEMENT_NAME)) {
        move_statement();
    }
}

void
BankClient::show_opts() 
{
    callHelp(m_conf, no_help);
    print_options("Bank", getOpts(), getOptsCount());
}

void
BankClient::statement_list()
{
    callHelp(m_conf, statement_list_help);
    std::ofstream output;
    if (m_conf.hasOpt(OUTPUT_NAME)) {
        output.open(m_conf.get<std::string>(OUTPUT_NAME).c_str(), std::ios::out);
    } else {
        output.open("/dev/stdout", std::ios::out);
    }
    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create());
    std::auto_ptr<Register::Banking::HeadList> bankList(
            bankMan->createList());

    Database::Filters::StatementHead *statementFilter =
        new Database::Filters::StatementHeadImpl();
    Database::Filters::Union unionFilter;

    apply_ID(statementFilter);
    apply_DATE(statementFilter, BANK_DATE_NAME, Create);
    apply_DATE(statementFilter, BANK_OLD_BALANCE_DATE_NAME, BalanceOld);
    get_DID(statementFilter, AccountId, BANK_ACCOUNT_ID_NAME);
    get_Str(statementFilter, AccountNumber, BANK_ACCOUNT_NUMBER_NAME);
    get_Str(statementFilter, BankCode, BANK_BANK_CODE_NAME);
    get_Str(statementFilter, ConstSymbol, BANK_CONST_SYMBOL_NAME);
    if (m_conf.hasOpt(BANK_INVOICE_ID_NAME)) {
        if (m_conf.get<unsigned int>(BANK_INVOICE_ID_NAME) == 0) {
            statementFilter->addInvoiceId().setNULL();
        } else {
            get_DID(statementFilter, InvoiceId, BANK_INVOICE_ID_NAME);
        }
    }

    unionFilter.addFilter(statementFilter);
    bankList->reload(unionFilter);

    bankList->exportXML(output);
}

void
BankClient::import_xml()
{
    callHelp(m_conf, import_xml_help);
    std::string fileName;
    bool fromFile = false;
    bool createCredit = false;
    if (m_conf.hasOpt(BANK_XML_FILE_NAME)) {
        fromFile = true;
        fileName = m_conf.get<std::string>(BANK_XML_FILE_NAME);
    }
    if (m_conf.hasOpt(BANK_CREATE_CREDIT_INVOICE_NAME)) {
        createCredit = true;
    }
    unsigned long long xml_id = 0;
    if (m_conf.hasOpt(BANK_XML_FILE_ID_NAME)) {
        xml_id = m_conf.get<unsigned long long>(BANK_XML_FILE_ID_NAME);
    }

    std::ifstream input;
    if (fromFile) {
        input.open(fileName.c_str(), std::ios::in);
    } else {
        input.open("/dev/stdin", std::ios::in);
    }
    std::auto_ptr<Register::Banking::Manager>
        bankMan(Register::Banking::Manager::create());
    bool retval;
    retval = bankMan->importStatementXml(input, xml_id, createCredit);
    if (!retval) {
        std::cout << "Error occured!" << std::endl;
    }
}

void
BankClient::add_bank_account()
{
    callHelp(m_conf, add_bank_account_help);
    std::string account_number = m_conf.get<std::string>(BANK_ACCOUNT_NUMBER_NAME);
    std::string account_name;
    if (m_conf.hasOpt(BANK_ACCOUNT_NAME_NAME)) {
        account_name = m_conf.get<std::string>(BANK_ACCOUNT_NAME_NAME);
    }
    std::string bank_code = m_conf.get<std::string>(BANK_BANK_CODE_NAME);
    std::auto_ptr<Register::Banking::Manager>
        bankMan(Register::Banking::Manager::create());
    bool retval = true;
    if (m_conf.hasOpt(BANK_ZONE_ID_NAME)) {
        Database::ID zoneId = m_conf.get<unsigned int>(BANK_ZONE_ID_NAME);
        retval = bankMan->insertBankAccount(zoneId, account_number, account_name, bank_code);
    } else if (m_conf.hasOpt(BANK_ZONE_NAME_NAME)) {
        std::string zoneName = m_conf.get<std::string>(BANK_ZONE_NAME_NAME);
        retval = bankMan->insertBankAccount(zoneName, account_number, account_name, bank_code);
    }
    if (!retval) {
        std::cout << "Error occured!" << std::endl;
    }
}

void
BankClient::move_statement()
{
    callHelp(m_conf, move_statement_help);
    Database::ID itemId = m_conf.get<unsigned int>(BANK_ITEM_ID_NAME);
    Database::ID headId = m_conf.get<unsigned int>(BANK_HEAD_ID_NAME);
    bool force = false;
    if (m_conf.hasOpt(BANK_FORCE_NAME)) {
        force = true;
    }
    std::auto_ptr<Register::Banking::Manager>
        bankMan(Register::Banking::Manager::create());
    bankMan->moveItemToPayment(itemId, headId, force);
}

void
BankClient::statement_list_help()
{
    std::cout <<
        "** Statemetn list **\n\n"
        "  $ " << g_prog_name << " --" << BANK_STATEMENT_LIST_NAME << " \\\n"
        "    [--" << ID_NAME << "=<statement_id>] \\\n"
        "    [--" << BANK_DATE_NAME << "=<date>] \\\n"
        "    [--" << BANK_ACCOUNT_NUMBER_NAME << "=<account_number>] \\\n"
        "    [--" << BANK_BANK_CODE_NAME << "=<bank_code>] \\\n"
        "    [--" << BANK_CONST_SYMBOL_NAME << "=<const_symbol>] \\\n"
        "    [--" << BANK_INVOICE_ID_NAME << "=<invoice_id>]\n"
        << std::endl;
}

void
BankClient::import_xml_help()
{
    std::cout <<
        "** Import xml **\n\n"
        "  $ " << g_prog_name << " --" << BANK_IMPORT_XML_NAME << " \\\n"
        "    [--" << BANK_XML_FILE_NAME << "=<file_name>] \\\n"
        "    [--" << BANK_XML_FILE_ID_NAME << "=<xml_file_id>] \\\n"
        "    [--" << BANK_CREATE_CREDIT_INVOICE_NAME << "]\n"
        << std::endl;
    std::cout << "If no xml file name is provided, program reads from stdin."
        << std::endl;
}

void
BankClient::add_bank_account_help()
{
    std::cout << 
        "** Add new bank account **\n\n"
        "  $ " << g_prog_name << " --" << BANK_ADD_ACCOUNT_NAME << " \\\n"
        "    --" << BANK_ZONE_ID_NAME << "=<zone_id> | \\\n"
        "    --" << BANK_ZONE_NAME_NAME << "=<zone_fqdn> \\\n"
        "    --" << BANK_ACCOUNT_NUMBER_NAME << "=<account_number> \\\n"
        "    --" << BANK_BANK_CODE_NAME << "=<bank_code> \\\n"
        "    [--" << BANK_ACCOUNT_NAME_NAME << "=<account_name>]\n"
        << std::endl;
}

void
BankClient::move_statement_help()
{
    std::cout << 
        "** Add new bank account **\n\n"
        "  $ " << g_prog_name << " --" << BANK_MOVE_STATEMENT_NAME << " \\\n"
        "   --" << BANK_ITEM_ID_NAME << "=<item_id> \\\n"
        "   --" << BANK_HEAD_ID_NAME << "=<new_head_id> \\\n"
        "   [--" << BANK_FORCE_NAME << "]\n"
        << std::endl;
    std::cout << "If ``head_id'' is zero, that set ``NULL'' as head id"
        << std::endl;
}


#define ADDOPT(name, type, callable, visible) \
    {CLIENT_BANK, name, name##_DESC, type, callable, visible}

const struct options
BankClient::m_opts[] = {
    ADDOPT(BANK_STATEMENT_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_IMPORT_XML_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_ADD_ACCOUNT_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_MOVE_STATEMENT_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_DATE_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(BANK_ACCOUNT_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(BANK_OLD_BALANCE_DATE_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_ACCOUNT_NUMBER_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_BANK_CODE_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_CONST_SYMBOL_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_INVOICE_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(BANK_XML_FILE_NAME, TYPE_STRING, false, false),
    ADDOPT(OUTPUT_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_CREDIT_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(BANK_CREATE_CREDIT_INVOICE_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(BANK_ZONE_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(BANK_ZONE_NAME_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_ACCOUNT_NAME_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_XML_FILE_ID_NAME, TYPE_ULONGLONG, false, false),
    ADDOPT(BANK_FORCE_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(BANK_ITEM_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(BANK_HEAD_ID_NAME, TYPE_UINT, false, false),
};

#undef ADDOPT

int 
BankClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

