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
        addOpt(BANK_OLD_ONLINE_LIST_NAME)
        addOpt(BANK_OLD_STATEMENT_LIST_NAME)
        addOpt(BANK_SHOW_OPTS_NAME)
        addOpt(BANK_LIST_NAME)
        addOpt(BANK_ONLINE_LIST_NAME);

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
        addOptDID(BANK_INVOICE_ID_NAME);
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

int
BankClient::old_online_list()
{
    std::ofstream stdout("/dev/stdout", std::ios::out);
    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create(m_dbman));
    std::auto_ptr<Register::Banking::OnlineList> bankList(
            bankMan->createOnlineList());

    Database::Filters::OnlineStatement *statementFilter =
        new Database::Filters::OnlineStatementImpl();
    Database::Filters::Union *unionFilter = 
        new Database::Filters::Union();

    unionFilter->addFilter(statementFilter);
    bankList->reload(*unionFilter);
    bankList->exportXML(stdout);
    return 0;
}

int
BankClient::old_statement_list()
{
    std::ofstream stdout("/dev/stdout", std::ios::out);
    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create(m_dbman));
    std::auto_ptr<Register::Banking::List> bankList(
            bankMan->createList());

    Database::Filters::Statement *statementFilter =
        new Database::Filters::StatementImpl();
    Database::Filters::Union *unionFilter = 
        new Database::Filters::Union();

    unionFilter->addFilter(statementFilter);
    bankList->reload(*unionFilter);
    bankList->exportXML(stdout);
    return 0;
}

void
BankClient::list()
{
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

    if (m_conf.hasOpt(BANK_SORT_NAME)) {
        bool direction = true;
        if (m_conf.hasOpt(BANK_SORT_DESC_NAME)) {
            direction = false;
        }
        bool sort = true;
        Register::Banking::MemberType type;
        std::string ssort = m_conf.get<std::string>(BANK_SORT_NAME);
        if (ssort.compare("account_number") == 0) {
            type = Register::Banking::MT_ACCOUNT_NUMBER;
        } else if (ssort.compare("bank_code") == 0) {
            type = Register::Banking::MT_BANK_CODE;
        } else if (ssort.compare("price") == 0) {
            type = Register::Banking::MT_PRICE;
        } else if (ssort.compare("invoice_id") == 0) {
            type = Register::Banking::MT_INVOICE_ID;
        } else {
            sort = false;
        }
        if (sort) {
            bankList->sort(type, direction);
        }
    }

    std::cout << "<object>\n";
    for (int i = 0; i < (int)bankList->getCount(); i++) {
        Register::Banking::Statement *stat = bankList->get(i);
        std::cout
            << "\t<payment>\n"
            << "\t\t<id>" << stat->getId() << "</id>\n"
            << "\t\t<account_id>" << stat->getAccountId() << "</account_id>\n"
            << "\t\t<date>" << stat->getDate() << "</date>\n"
            << "\t\t<old_date>" << stat->getOldDate() << "</old_date>\n"
            << "\t\t<balance>" << stat->getBalance().format() << "</balance>\n"
            << "\t\t<balance_old>" << stat->getOldBalance().format() << "</balance_old>\n"
            << "\t\t<credit>" << stat->getCredit().format() << "</credit>\n"
            << "\t\t<debet>" << stat->getDebet().format() << "</debet>\n";
        for (int j = 0; j < (int)stat->getStatementItemCount(); j++) {
            Register::Banking::StatementItem *statItem =
                (Register::Banking::StatementItem *)stat->getStatementItemByIdx(j);
            std::cout
                << "\t\t<statement_item>\n"
                << "\t\t\t<id>" << statItem->getId() << "</id>\n"
                << "\t\t\t<account_number>" << statItem->getAccountNumber() << "</account_number>\n"
                << "\t\t\t<bank_code>" << statItem->getBankCode() << "</bank_code>\n"
                << "\t\t\t<code>" << statItem->getCode() << "</code>\n"
                << "\t\t\t<const_symb>" << statItem->getConstSymbol() << "</const_symb>\n"
                << "\t\t\t<var_symb>" << statItem->getVarSymbol() << "</var_symb>\n"
                << "\t\t\t<spec_symb>" << statItem->getSpecSymbol() << "</spec_symb>\n"
                << "\t\t\t<price>" << statItem->getPrice().format() << "</price>\n"
                << "\t\t\t<evid_number>" << statItem->getEvidenceNumber() << "</evid_number>\n"
                << "\t\t\t<account_date>" << statItem->getDate() << "</account_date>\n"
                << "\t\t\t<account_memo>" << statItem->getMemo() << "</account_memo>\n"
                << "\t\t\t<invoice_id>" << statItem->getInvoiceId() << "</invoice_id>\n"
                << "\t\t</statement_item>\n";
        }
        std::cout
            << "\t</payment>\n";
    }
    std::cout << "</object>" << std::endl;
}

void
BankClient::online_list()
{
    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create(m_dbman));
    std::auto_ptr<Register::Banking::OnlineList> onlineBankList(
            bankMan->createOnlineList());

    Database::Filters::OnlineStatement *onlineStatFilter = 
        new Database::Filters::OnlineStatementImpl();
    Database::Filters::Union *unionFilter =
        new Database::Filters::Union();

    apply_ID(onlineStatFilter);
    apply_DATETIME(onlineStatFilter, BANK_DATE_NAME, Create);
    get_Str(onlineStatFilter, AccountNumber, BANK_ACCOUNT_NUMBER_NAME);
    get_Str(onlineStatFilter, BankCode, BANK_BANK_CODE_NAME);
    get_Str(onlineStatFilter, ConstSymbol, BANK_CONST_SYMBOL_NAME);
    if (m_conf.hasOpt(BANK_INVOICE_ID_NAME)) {
        if (m_conf.get<unsigned int>(BANK_INVOICE_ID_NAME) == 0) {
            onlineStatFilter->addInvoiceId().setNULL();
        } else {
            get_DID(onlineStatFilter, InvoiceId, BANK_INVOICE_ID_NAME);
        }
    }

    unionFilter->addFilter(onlineStatFilter);
    onlineBankList->reload(*unionFilter);

    if (m_conf.hasOpt(BANK_SORT_NAME)) {
        bool direction = true;
        if (m_conf.hasOpt(BANK_SORT_DESC_NAME)) {
            direction = false;
        }
        bool sort = true;
        Register::Banking::MemberType type;
        std::string ssort = m_conf.get<std::string>(BANK_SORT_NAME);
        if (ssort.compare("account_number") == 0) {
            type = Register::Banking::MT_ACCOUNT_NUMBER;
        } else if (ssort.compare("bank_code") == 0) {
            type = Register::Banking::MT_BANK_CODE;
        } else if (ssort.compare("price") == 0) {
            type = Register::Banking::MT_PRICE;
        } else if (ssort.compare("invoice_id") == 0) {
            type = Register::Banking::MT_INVOICE_ID;
        } else {
            sort = false;
        }
        if (sort) {
            onlineBankList->sort(type, direction);
        }
    }

    std::cout << "<object>\n";
    for (int i = 0; i < (int)onlineBankList->getCount(); i++) {
        Register::Banking::OnlineStatement *stat = onlineBankList->get(i);
        std::cout
            << "\t<online_payment>\n"
            << "\t\t<id>" << stat->getId() << "</id>\n"
            << "\t\t<account_id>" << stat->getAccountId() << "</account_id>\n"
            << "\t\t<price>" << stat->getPrice() << "</price>\n"
            << "\t\t<date>" << stat->getCrDate() << "</date>\n"
            << "\t\t<account_number>" << stat->getAccountNumber() << "</account_number>\n"
            << "\t\t<bank_code>" << stat->getBankCode() << "</bank_code>\n"
            << "\t\t<const_symb>" << stat->getConstSymbol() << "</const_symb>\n"
            << "\t\t<var_symb>" << stat->getVarSymbol() << "</var_symb>\n"
            << "\t\t<memo>" << stat->getMemo() << "</memo>\n"
            << "\t\t<name>" << stat->getAccountName() << "</name>\n"
            << "\t\t<ident>" << stat->getIdent() << "</ident>\n"
            << "\t\t<invoice_id>" << stat->getInvoiceId() << "</invoice_id>\n"
            << "\t</online_payment>\n";
    }
    std::cout << "</object>" << std::endl;
}

} // namespace Admin;

