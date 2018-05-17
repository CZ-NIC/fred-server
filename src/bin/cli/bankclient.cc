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


#include "src/bin/cli/bankclient.hh"
#include "src/bin/cli/commonclient.hh"
#include "src/libfred/banking/bank_manager.hh"
#include "src/libfred/registry.hh"
#include "src/bin/corba/file_manager_client.hh"

#include <iostream>
#include <fstream>
#include <string>

namespace Admin {


void
BankClient::runMethod()
{
    if (bank_show_opts) {
        //show_opts();
    } else if (bank_payment_list) {//BANK_PAYMENT_LIST_NAME
        payment_list();
    } else if (bank_import_xml) {//BANK_IMPORT_XML_NAME
        import_xml();
    } else if (bank_add_account) {//BANK_ADD_ACCOUNT_NAME
        add_bank_account();
    }
}

void
BankClient::payment_list()
{

    // init file manager
    CorbaClient corba_client(0, 0, m_nsAddr, nameservice_context);
    FileManagerClient fm_client(corba_client.getNS());
    LibFred::File::ManagerPtr file_manager(LibFred::File::Manager::create(&fm_client));

    // bank manager
    LibFred::Banking::ManagerPtr bank_manager(LibFred::Banking::Manager::create(file_manager.get()));
    LibFred::Banking::PaymentListPtr list(bank_manager->createPaymentList());

    Database::Filters::BankPayment *payment_filter = new Database::Filters::BankPaymentImpl();
    if (bank_payment_type.is_value_set()) {
        int type = bank_payment_type.get_value();
        if (type >= 1 && type <= 6) {
            payment_filter->addType().setValue(type);
        }
        else {
            throw std::runtime_error("payment list: parameter error: bank for( "
                                     "payment type should be from interval <1, 6>");
        }
    }

    Database::Filters::Union filter;
    filter.addFilter(payment_filter);
    list->reload(filter);

    for (unsigned int i = 0; i < list->size(); ++i) {
        LibFred::Banking::Payment *payment = list->get(i);
        if (payment) {
            if (bank_payment_list_details) {
                std::cout << std::setw(10) << std::left;
            }
            std::cout << payment->getId();
            if (bank_payment_list_details) {
                std::cout << to_iso_extended_string(payment->getCrTime())
                          << "  amount: "
                          << std::setw(10) << std::right << payment->getPrice();
                if (!payment->getAccountMemo().empty()) {
                    std::cout << "  account_memo: " << payment->getAccountMemo();
                }
                if (!payment->getAccountName().empty()) {
                    std::cout << "  account_name: " << payment->getAccountName();
                }
            }
            std::cout << std::endl;
        }
    }
}

void
BankClient::import_xml()
{
    bool from_file = false;
    std::string file_name;
    if (import_xml_params.bank_xml.is_value_set()) {//BANK_XML_FILE_NAME
        from_file = true;
        file_name = import_xml_params.bank_xml.get_value();
    }

    bool generate_invoice = false;
    if (import_xml_params.cr_credit_invoice) {//BANK_CREATE_CREDIT_INVOICE_NAME
        generate_invoice = true;
    }

    // path to original statement file
    std::string statement_file;
    if (import_xml_params.bank_statement_file.is_value_set()) {//BANK_XML_FILE_STATEMENT_NAME
        statement_file = import_xml_params.bank_statement_file.get_value();
    }

    // mime type of original statement file
    std::string statement_mime;
    if (import_xml_params.bank_statement_file_mimetype.is_value_set()) {//BANK_XML_FILE_STATEMENT_MIME_NAME
        statement_mime = import_xml_params.bank_statement_file_mimetype.get_value();
    }

    if ((!statement_file.empty() && statement_mime.empty())
            || (statement_file.empty() && !statement_mime.empty())) {
        std::cerr << "Error: `" << "bank_statement_file_mimetype" << "' "
                  << "and `" << "bank_statement_file" << "' parameters "
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

    // init file manager
    CorbaClient corba_client(0, 0, m_nsAddr, nameservice_context);
    FileManagerClient fm_client(corba_client.getNS());
    LibFred::File::ManagerPtr file_manager(LibFred::File::Manager::create(&fm_client));

    // bank manager
    LibFred::Banking::ManagerPtr bank_manager(LibFred::Banking::Manager::create(file_manager.get()));
    bank_manager->importStatementXml(input, statement_file, statement_mime, generate_invoice);
}

void
BankClient::add_bank_account()
{
    //callHelp(m_conf, add_bank_account_help);
    std::string account_number = add_account_params.bank_account_number;//BANK_ACCOUNT_NUMBER_NAME
    std::string bank_code = add_account_params.bank_code;//BANK_BANK_CODE_NAME

    std::string account_name;
    if (add_account_params.account_name.is_value_set()) {//BANK_ACCOUNT_NAME_NAME
        account_name = add_account_params.account_name.get_value();
    }
    std::string zone_fqdn;
    if (add_account_params.zone_fqdn.is_value_set()) {//BANK_ZONE_NAME_NAME
        zone_fqdn = add_account_params.zone_fqdn.get_value();
    }

    // init file manager
    CorbaClient corba_client(0, 0, m_nsAddr, nameservice_context);
    FileManagerClient fm_client(corba_client.getNS());
    LibFred::File::ManagerPtr file_manager(LibFred::File::Manager::create(&fm_client));

    // bank manager
    LibFred::Banking::ManagerPtr bank_manager(LibFred::Banking::Manager::create(file_manager.get()));
    bank_manager->addBankAccount(account_number, bank_code, zone_fqdn, account_name);
}

} // namespace Admin;

