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

#include "simple.h"
#include "bankclient.h"
#include "commonclient.h"
#include "register/bank.h"
#include "register/register.h"

#include <iostream>
#include <fstream>
#include <string>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/confix.hpp>
#include <boost/spirit/utility/lists.hpp>
#include <boost/spirit/utility/escape_char.hpp>

namespace Admin {

BankClient::BankClient()
{
    m_options = new boost::program_options::options_description(
            "Bank related options");
    m_options->add_options()
        addOpt(BANK_STATEMENT_LIST_NAME)
        addOpt(BANK_ONLINE_LIST_NAME)
        addOpt(BANK_SHOW_OPTS_NAME)
        addOpt(BANK_IMPORT_XML_NAME)
        addOpt(BANK_IMPORT_XML_HELP_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Bank related sub options");
    m_optionsInvis->add_options()
        addOptStr(BANK_DATE_NAME)
        addOptDID(BANK_ID_NAME)
        addOptDID(BANK_ACCOUNT_ID_NAME)
        addOptStr(BANK_OLD_BALANCE_DATE_NAME)
        addOptStr(BANK_ACCOUNT_NUMBER_NAME)
        addOptStr(BANK_BANK_CODE_NAME)
        addOptStr(BANK_CONST_SYMBOL_NAME)
        addOptStr(BANK_SORT_NAME)
        addOpt(BANK_SORT_DESC_NAME)
        addOptDID(BANK_INVOICE_ID_NAME)
        addOptStr(BANK_XML_FILE_NAME)
        addOpt(BANK_ONLINE_NAME)
        addOptStr(OUTPUT_NAME);
}
BankClient::BankClient(
        std::string connstring,
        std::string nsAddr) : BaseClient(connstring, nsAddr)
{
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}

BankClient::~BankClient()
{
    delete m_options;
    delete m_optionsInvis;
}

void
BankClient::init(
        std::string connstring,
        std::string nsAddr,
        Config::Conf &conf)
{
    BaseClient::init(connstring, nsAddr);
    m_db.OpenDatabase(connstring.c_str());
    m_conf = conf;
}

boost::program_options::options_description *
BankClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
BankClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
BankClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
}

void
BankClient::online_list()
{
    std::ofstream output;
    if (m_conf.hasOpt(OUTPUT_NAME)) {
        output.open(m_conf.get<std::string>(OUTPUT_NAME).c_str(), std::ios::out);
    } else {
        output.open("/dev/stdout", std::ios::out);
    }
    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create(m_dbman));
    std::auto_ptr<Register::Banking::OnlineList> bankList(
            bankMan->createOnlineList());

    Database::Filters::OnlineStatement *statementFilter =
        new Database::Filters::OnlineStatementImpl();
    Database::Filters::Union *unionFilter = 
        new Database::Filters::Union();

    apply_ID(statementFilter);
    apply_DATETIME(statementFilter, BANK_DATE_NAME, Create);
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

    unionFilter->addFilter(statementFilter);
    bankList->reload(*unionFilter);

    bankList->exportXML(output);
}

void
BankClient::statement_list()
{
    std::ofstream output;
    if (m_conf.hasOpt(OUTPUT_NAME)) {
        output.open(m_conf.get<std::string>(OUTPUT_NAME).c_str(), std::ios::out);
    } else {
        output.open("/dev/stdout", std::ios::out);
    }
    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create(m_dbman));
    std::auto_ptr<Register::Banking::List> bankList(
            bankMan->createList());

    Database::Filters::Statement *statementFilter =
        new Database::Filters::StatementImpl();
    Database::Filters::Union *unionFilter = 
        new Database::Filters::Union();

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

    unionFilter->addFilter(statementFilter);
    bankList->reload(*unionFilter);

    bankList->exportXML(output);
}

void
BankClient::import_xml()
{
    std::string fileName;
    bool fromFile = false;
    bool isOnline = false;
    if (m_conf.hasOpt(BANK_XML_FILE_NAME)) {
        fromFile = true;
        fileName = m_conf.get<std::string>(BANK_XML_FILE_NAME);
    }
    if (m_conf.hasOpt(BANK_ONLINE_NAME)) {
        isOnline = true;
    }

    std::ifstream input;
    if (fromFile) {
        input.open(fileName.c_str(), std::ios::in);
    } else {
        input.open("/dev/stdin", std::ios::in);
    }
    std::auto_ptr<Register::Banking::Manager>
        bankMan(Register::Banking::Manager::create(m_dbman));
    bool retval;
    if (isOnline) {
        retval = bankMan->importOnlineStatementXml(input);
    } else {
        retval = bankMan->importStatementXml(input);
    }
    if (!retval) {
        std::cout << "Error occured!" << std::endl;
    }
}

void
BankClient::online_list_help()
{
    std::cout <<
        "** Online statemetn list **\n\n"
        "  $ " << g_prog_name << " --" << BANK_ONLINE_LIST_NAME << " \\\n"
        "    [--" << ID_NAME << "=<statement_id>] \\\n"
        "    [--" << BANK_DATE_NAME << "=<date>] \\\n"
        "    [--" << BANK_ACCOUNT_NUMBER_NAME << "=<account_number>] \\\n"
        "    [--" << BANK_BANK_CODE_NAME << "=<bank_code>] \\\n"
        "    [--" << BANK_CONST_SYMBOL_NAME << "=<const_symbol>] \\\n"
        "    [--" << BANK_INVOICE_ID_NAME << "=<invoice_id>]\n"
        << std::endl;
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
        "    [--" << BANK_ONLINE_NAME << "] \n" 
        << std::endl;
    std::cout << "If no xml file name is provided, program reads from stdin."
        << std::endl
        << "``--" << BANK_ONLINE_NAME << "'' specified if statement is normal or online."
        << std::endl;
}


} // namespace Admin;

