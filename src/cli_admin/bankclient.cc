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
#include "corba/file_manager_client.h"

#include <iostream>
#include <fstream>
#include <string>

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
        // statement_list();
    } else if (m_conf.hasOpt(BANK_PAYMENT_LIST_NAME)) {
        payment_list();
    } else if (m_conf.hasOpt(BANK_IMPORT_XML_NAME)) {
        import_xml();
    } else if (m_conf.hasOpt(BANK_ADD_ACCOUNT_NAME)) {
        add_bank_account();
    } else if (m_conf.hasOpt(BANK_MOVE_STATEMENT_NAME)) {
        move_statement();
    } else if (m_conf.hasOpt(BANK_SET_PAYMENT_TYPE_NAME)) {
        set_payment_type();
    }
}

void
BankClient::show_opts() 
{
    callHelp(m_conf, no_help);
    print_options("Bank", getOpts(), getOptsCount());
}

/*
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
*/

void
BankClient::payment_list()
{
    callHelp(m_conf, payment_list_help);

    /* init file manager */
    CorbaClient corba_client(0, 0, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    FileManagerClient fm_client(corba_client.getNS());
    Register::File::ManagerPtr file_manager(Register::File::Manager::create(&fm_client));

    /* bank manager */
    Register::Banking::ManagerPtr bank_manager(Register::Banking::Manager::create(file_manager.get()));
    Register::Banking::PaymentListPtr list(bank_manager->createPaymentList());

    Database::Filters::BankPayment *payment_filter = new Database::Filters::BankPaymentImpl();
    if (m_conf.hasOpt(BANK_PAYMENT_TYPE_NAME)) {
        int type = m_conf.get<int>(BANK_PAYMENT_TYPE_NAME);
        if (type >= 1 && type <= 6) {
            payment_filter->addType().setValue(type);
        }
        else {
            throw std::runtime_error("payment list: parameter error: bank "
                                     "payment type should be from interval <1, 6>");
        }
    }

    Database::Filters::Union filter;
    filter.addFilter(payment_filter);
    list->reload(filter);

    for (unsigned int i = 0; i < list->size(); ++i) {
        Register::Banking::Payment *payment = list->get(i);
        if (payment) {
            std::cout << payment->getId() << std::endl;
        }
    }
}

void
BankClient::import_xml()
{
    callHelp(m_conf, import_xml_help);

    bool from_file = false;
    std::string file_name;
    if (m_conf.hasOpt(BANK_XML_FILE_NAME)) {
        from_file = true;
        file_name = m_conf.get<std::string>(BANK_XML_FILE_NAME);
    }

    bool generate_invoice = false;
    if (m_conf.hasOpt(BANK_CREATE_CREDIT_INVOICE_NAME)) {
        generate_invoice = true;
    }

    /* path to original statement file */
    std::string statement_file;
    if (m_conf.hasOpt(BANK_XML_FILE_STATEMENT_NAME)) {
        statement_file = m_conf.get<std::string>(BANK_XML_FILE_STATEMENT_NAME);
    }

    /* mime type of original statement file */
    std::string statement_mime;
    if (m_conf.hasOpt(BANK_XML_FILE_STATEMENT_MIME_NAME)) {
        statement_mime = m_conf.get<std::string>(BANK_XML_FILE_STATEMENT_MIME_NAME);
    }

    if ((!statement_file.empty() && statement_mime.empty())
            || (statement_file.empty() && !statement_mime.empty())) {
        std::cerr << "Error: `" << BANK_XML_FILE_STATEMENT_MIME_NAME << "' "
                  << "and `" << BANK_XML_FILE_STATEMENT_NAME << "' parameters "
                  << "should be used together";
        return;
    }

    std::ifstream input;
    if (from_file) {
        input.open(file_name.c_str(), std::ios::in);
    } else {
        input.open("/dev/stdin", std::ios::in);
    }

    if (!input.is_open()) {
        throw std::runtime_error(str(boost::format(
                        "bank client: import statement xml: "
                        "file '%1% not found") % file_name));
    }

    /* init file manager */
    CorbaClient corba_client(0, 0, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    FileManagerClient fm_client(corba_client.getNS());
    Register::File::ManagerPtr file_manager(Register::File::Manager::create(&fm_client));

    /* bank manager */
    Register::Banking::ManagerPtr bank_manager(Register::Banking::Manager::create(file_manager.get()));
    bank_manager->importStatementXml(input, statement_file, statement_mime, generate_invoice);
}

void
BankClient::add_bank_account()
{
    callHelp(m_conf, add_bank_account_help);
    std::string account_number = m_conf.get<std::string>(BANK_ACCOUNT_NUMBER_NAME);
    std::string bank_code = m_conf.get<std::string>(BANK_BANK_CODE_NAME);

    std::string account_name;
    if (m_conf.hasOpt(BANK_ACCOUNT_NAME_NAME)) {
        account_name = m_conf.get<std::string>(BANK_ACCOUNT_NAME_NAME);
    }
    std::string zone_fqdn;
    if (m_conf.hasOpt(BANK_ZONE_NAME_NAME)) {
        zone_fqdn = m_conf.get<std::string>(BANK_ZONE_NAME_NAME);
    }

    /* init file manager */
    CorbaClient corba_client(0, 0, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    FileManagerClient fm_client(corba_client.getNS());
    Register::File::ManagerPtr file_manager(Register::File::Manager::create(&fm_client));

    /* bank manager */
    Register::Banking::ManagerPtr bank_manager(Register::Banking::Manager::create(file_manager.get()));
    bank_manager->addBankAccount(account_number, bank_code, zone_fqdn, account_name);
}

void
BankClient::move_statement()
{
    callHelp(m_conf, move_statement_help);
    Database::ID itemId = m_conf.get<unsigned long long>(BANK_PAYMENT_ID_NAME);
    Database::ID headId = m_conf.get<unsigned int>(BANK_STATEMENT_ID_NAME);
    bool force = false;
    if (m_conf.hasOpt(BANK_FORCE_NAME)) {
        force = true;
    }

    /* init file manager */
    CorbaClient corba_client(0, 0, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    FileManagerClient fm_client(corba_client.getNS());
    Register::File::ManagerPtr file_manager(Register::File::Manager::create(&fm_client));

    /* bank manager */
    Register::Banking::ManagerPtr bank_manager(Register::Banking::Manager::create(file_manager.get()));
    bank_manager->pairPaymentWithStatement(itemId, headId, force);
}

void
BankClient::set_payment_type()
{
    callHelp(m_conf, set_payment_type_help);

    Database::ID payment_id = m_conf.get<unsigned long long>(BANK_PAYMENT_ID_NAME);
    int type = m_conf.get<int>(BANK_PAYMENT_TYPE_NAME);

     /* init file manager */
    CorbaClient corba_client(0, 0, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    FileManagerClient fm_client(corba_client.getNS());
    Register::File::ManagerPtr file_manager(Register::File::Manager::create(&fm_client));

    /* bank manager */
    Register::Banking::ManagerPtr bank_manager(Register::Banking::Manager::create(file_manager.get()));
    bank_manager->setPaymentType(payment_id, type);
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
BankClient::payment_list_help()
{
    std::cout <<
        "** Payment list **\n\n"
        "  $ " << g_prog_name << " --" << BANK_PAYMENT_LIST_NAME << " \\\n"
        "    [--" << BANK_PAYMENT_TYPE_NAME << "=[1,2,3,4,5,6]\n"
        << std::endl;
}

void
BankClient::import_xml_help()
{
    std::cout <<
        "** Import xml **\n\n"
        "  $ " << g_prog_name << " --" << BANK_IMPORT_XML_NAME << " \\\n"
        "    [--" << BANK_XML_FILE_NAME << "=<xml_file_name>] \\\n"
        "    [--" << BANK_XML_FILE_STATEMENT_NAME << "=<path_to_statement_file>] \\\n"
        "    [--" << BANK_XML_FILE_STATEMENT_MIME_NAME << "=<mime_type_of_statement_file>] \\\n"
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
        "** Add payment to statement **\n\n"
        "  $ " << g_prog_name << " --" << BANK_MOVE_STATEMENT_NAME << " \\\n"
        "   --" << BANK_PAYMENT_ID_NAME << "=<payment_id> \\\n"
        "   --" << BANK_STATEMENT_ID_NAME << "=<statement_id> \\\n"
        "   [--" << BANK_FORCE_NAME << "]\n"
        << std::endl;
    std::cout << "If ``statement_id'' is zero, that set ``NULL'' as statement id"
        << std::endl;
}

void
BankClient::set_payment_type_help()
{
    std::cout <<
        "** Change type of given payment **\n\n"
        "  $ " << g_prog_name << " --" << BANK_SET_PAYMENT_TYPE_NAME << " \\\n"
        "   --" << BANK_PAYMENT_ID_NAME << "=<" << BANK_PAYMENT_ID_NAME_DESC << "> \\\n"
        "   --" << BANK_PAYMENT_TYPE_NAME << "=<" << BANK_PAYMENT_TYPE_NAME_DESC << "> \\\n"
        << std::endl;
    std::cout << "Where '" << BANK_PAYMENT_TYPE_NAME << "' is from interval <1, 6>" << std::endl;
}


#define ADDOPT(name, type, callable, visible) \
    {CLIENT_BANK, name, name##_DESC, type, callable, visible}

const struct options
BankClient::m_opts[] = {
    ADDOPT(BANK_STATEMENT_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_PAYMENT_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_IMPORT_XML_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_ADD_ACCOUNT_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_MOVE_STATEMENT_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(BANK_SET_PAYMENT_TYPE_NAME, TYPE_NOTYPE, true, true),
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
    ADDOPT(BANK_XML_FILE_STATEMENT_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_XML_FILE_STATEMENT_MIME_NAME, TYPE_STRING, false, false),
    ADDOPT(BANK_FORCE_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(BANK_STATEMENT_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(BANK_PAYMENT_ID_NAME, TYPE_ULONGLONG, false, false),
    ADDOPT(BANK_PAYMENT_TYPE_NAME, TYPE_INT, false, false)
};

#undef ADDOPT

int 
BankClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

