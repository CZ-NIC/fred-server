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

#include <math.h>
#include "commonclient.h"
#include "invoiceclient.h"
#include "fredlib/invoicing/invoice.h"
#include "../fredlib/model_zone.h"
#include "types/money.h"
namespace Admin {



void
InvoiceClient::runMethod()
{
    if (invoice_list) {
        list();
    } else if (invoice_archive) {
        archive();
    } else if (invoice_list_filters) {
        list_filters();
    } else if (invoice_show_opts) {
        //show_opts();
    } else if (invoice_credit) {
        credit();
    } else if (invoice_factoring) {
        factoring();
    } else if (invoice_add_prefix) {
        add_invoice_prefix();
    } else if (invoice_create) {
        create_invoice();
    }
}


void InvoiceClient::filter_reload_invoices(Fred::Invoicing::Manager *invMan, Fred::Invoicing::List *invList)
{     
    Database::Filters::Invoice *invFilter;
    invFilter = new Database::Filters::InvoiceImpl();

    if (params.invoice_id.is_value_set())
        invFilter->addId().setValue(
                Database::ID(params.invoice_id.get_value()));
    if (params.zone_id.is_value_set())
        invFilter->addZoneId().setValue(
                Database::ID(params.zone_id.get_value()));
    if (params.zone_fqdn.is_value_set())
        invFilter->addZone().addFqdn().setValue(
                params.zone_fqdn.get_value());
    if (params.type.is_value_set())
        invFilter->addType().setValue(
                params.type.get_value());
    if (params.number.is_value_set())
        invFilter->addNumber().setValue(
                params.number.get_value());
    if (params.crdate.is_value_set())
            invFilter->addCreateTime().setValue(
                    *parseDateTime(params.crdate.get_value()));
    if (params.taxdate.is_value_set())
            invFilter->addTaxDate().setValue(
                    *parseDate(params.taxdate.get_value()));
    if (params.registrar_id.is_value_set())
        invFilter->addRegistrar().addId().setValue(
                Database::ID(params.registrar_id.get_value()));
    if (params.registrar_handle.is_value_set())
        invFilter->addRegistrar().addHandle().setValue(
                params.registrar_handle.get_value());
    if (params.invoice_file_pdf.is_value_set()) {
        if (params.invoice_file_pdf.get_value() == 0) {
            invFilter->addFilePDF().setNULL();
        } else {
            invFilter->addFilePDF().setValue(Database::ID(
                    params.invoice_file_pdf.get_value()));
        }
    }
    if (params.invoice_file_xml.is_value_set()) {
        if (params.invoice_file_xml.get_value() == 0) {
            invFilter->addFileXML().setNULL();
        } else {
            invFilter->addFileXML().setValue(Database::ID(
                    params.invoice_file_xml.get_value()));
        }
    }
    if (params.invoice_file_name.is_value_set())
        invFilter->addFile().addName().setValue(
                params.invoice_file_name.get_value());


   Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(invFilter);

    invList->setLimit(params.limit.get_value());
    invList->reload(*unionFilter);

    unionFilter->clear();
    delete unionFilter;   //TODO fix
}

void
InvoiceClient::list()
{
    std::auto_ptr<Fred::Document::Manager> docMan(
            Fred::Document::Manager::create(
                docgen_path.get_value(),
                docgen_template_path.get_value(),
                fileclient_path.get_value(),
                m_nsAddr)
            );

    CorbaClient cc(0, NULL, m_nsAddr, nameservice_context);
    MailerManager mailMan(cc.getNS());

    std::auto_ptr<Fred::Invoicing::Manager> invMan(
            Fred::Invoicing::Manager::create(
                docMan.get(),
                &mailMan));

    std::auto_ptr<Fred::Invoicing::List> invList(
            invMan->createList());

    filter_reload_invoices(invMan.get(), invList.get());
    invList->exportXML(std::cout);
}


void
InvoiceClient::list_filters()
{
    std::auto_ptr<Fred::Document::Manager> docMan(
            Fred::Document::Manager::create(
                docgen_path.get_value(),
                docgen_template_path.get_value(),
                fileclient_path.get_value(),
                m_nsAddr)
            );

    CorbaClient cc(0, NULL, m_nsAddr, nameservice_context);
    MailerManager mailMan(cc.getNS());

    std::auto_ptr<Fred::Invoicing::Manager> invMan(
            Fred::Invoicing::Manager::create(
                docMan.get(),
                &mailMan));

    std::auto_ptr<Fred::Invoicing::List> invList(
            invMan->createList());

    filter_reload_invoices(invMan.get(), invList.get());

    std::cout << "<objects>\n";
    for (unsigned int i = 0; i < invList->getSize(); i++) {
        Fred::Invoicing::Invoice *inv = invList->get(i);
        std::cout
            << "\t<invoice>\n"
            << "\t\t<id>" << inv->getId() << "</id>\n"
            << "\t\t<zone>\n"
            << "\t\t\t<id>" << inv->getZoneId() << "</id>\n"
            // << "\t\t\t<name>" << inv->getZoneName() << "</name>\n"
            << "\t\t</zone>\n"
            << "\t\t<cr_date>" << inv->getCrDate() << "</cr_date>\n"
            << "\t\t<tax_date>" << inv->getTaxDate() << "</tax_date>\n"
            << "\t\t<account_period>" << inv->getAccountPeriod() << "</account_period>\n"
            << "\t\t<type>" << Fred::Invoicing::Type2Str(inv->getType()) << "</type>\n"
            << "\t\t<number>" << inv->getPrefix() << "</number>\n"
            << "\t\t<registrar_id>" << inv->getRegistrarId() << "</registrar_id>\n"
            << "\t\t<credit>" << inv->getCredit() << "</credit>\n"
            << "\t\t<price>" << inv->getPrice() << "</price>\n"
            // << "\t\t<vat_rate>" << inv->getVatRate() << "</vat_rate>\n"
            << "\t\t<total>" << inv->getTotal() << "</total>\n"
            << "\t\t<total_vat>" << inv->getTotalVat() << "</total_vat>\n"
            << "\t\t<var_symbol>" << inv->getVarSymbol() << "</var_symbol>\n"
            << "\t\t<file_pdf_id>" << inv->getFileId() << "</file_pdf_id>\n"
            << "\t\t<file_xml_id>" << inv->getFileXmlId() << "</file_xml_id>\n";
        for (unsigned int j = 0; j < inv->getSourceCount(); j++) {
            Fred::Invoicing::PaymentSource *source =
                (Fred::Invoicing::PaymentSource *)inv->getSource(j);
            std::cout
                << "\t\t<payment_source>\n"
                << "\t\t\t<id>" << source->getId() << "</id>\n"
                << "\t\t\t<number>" << source->getNumber() << "</number>\n"
                << "\t\t\t<credit>" << source->getCredit() << "</credit>\n"
                << "\t\t\t<total_price>" << source->getTotalPrice() << "</total_price>\n"
                << "\t\t\t<total_vat>" << source->getTotalVat() << "</total_vat>\n"
                << "\t\t\t<total_price_with_vat>" << source->getTotalPriceWithVat() << "</total_price_with_vat>\n"
                << "\t\t\t<cr_time>" << source->getCrTime() << "</cr_time>\n"
                << "\t\t</payment_source>\n";
        }
        for (unsigned int j = 0; j < inv->getActionCount(); j++) {
            Fred::Invoicing::PaymentAction *act =
                (Fred::Invoicing::PaymentAction *)inv->getAction(j);
            std::cout
                << "\t\t<payment_action>\n"
                << "\t\t\t<id>" << act->getObjectId() << "</id>\n"
                << "\t\t\t<name>" << act->getObjectName() << "</name>\n"
                << "\t\t\t<action_time>" << act->getActionTime() << "</action_time>\n"
                << "\t\t\t<ex_date>" << act->getExDate() << "</ex_date>\n"
                << "\t\t\t<action>" << act->getActionStr()
                // << Fred::Invoicing::PaymentActionType2Str(act->getAction())
                << "</action>\n"
                << "\t\t\t<units_count>" << act->getUnitsCount() << "</units_count>\n"
                << "\t\t\t<price_per_unit>" << act->getPricePerUnit() << "</price_per_unit>\n"
                << "\t\t</payment_action>\n";
        }
        for (unsigned int j = 0; j < inv->getPaymentCount(); j++) {
            Fred::Invoicing::Payment *pay =
                (Fred::Invoicing::Payment *)inv->getPaymentByIdx(j);
            std::cout
                << "\t\t<payment>\n"
                << "\t\t\t<price>" << pay->getPrice() << "</price>\n"
                << "\t\t\t<vat_rate>" << pay->getVatRate() << "</vat_rate>\n"
                << "\t\t\t<vat>" << pay->getVat() << "</vat>\n"
                << "\t\t\t<price_with_vat>" << pay->getPriceWithVat() << "</price_with_vat>\n"
                << "\t\t</payment>\n";
        }
        Fred::Invoicing::Subject *sup =
            (Fred::Invoicing::Subject *)inv->getSupplier();
        std::cout
            << "\t\t<supplier>\n"
            << "\t\t\t<id>" << sup->getId() << "</id>\n"
            << "\t\t\t<handle>" << sup->getHandle() << "</handle>\n"
            << "\t\t\t<name>" << sup->getName() << "</name>\n"
            << "\t\t\t<full_name>" << sup->getFullname() << "</full_name>\n"
            << "\t\t\t<street>" << sup->getStreet() << "</street>\n"
            << "\t\t\t<city>" << sup->getCity() << "</city>\n"
            << "\t\t\t<zip>" << sup->getZip() << "</zip>\n"
            << "\t\t\t<country>" << sup->getCountry() << "</country>\n"
            << "\t\t\t<ico>" << sup->getICO() << "</ico>\n"
            << "\t\t\t<vat_num>" << sup->getVatNumber() << "</vat_num>\n"
            << "\t\t\t<apply_vat>" << sup->getVatApply() << "</apply_vat>\n"
            << "\t\t\t<registration>" << sup->getRegistration() << "</registration>\n"
            << "\t\t\t<reclamation>" << sup->getReclamation() << "</reclamation>\n"
            << "\t\t\t<email>" << sup->getEmail() << "</email>\n"
            << "\t\t\t<url>" << sup->getURL() << "</url>\n"
            << "\t\t\t<phone>" << sup->getPhone() << "</phone>\n"
            << "\t\t\t<fax>" << sup->getFax() << "</fax>\n"
            << "\t\t</supplier>\n";
        Fred::Invoicing::Subject *cli =
            (Fred::Invoicing::Subject *)inv->getClient();
        std::cout
            << "\t\t<client>\n"
            << "\t\t\t<id>" << cli->getId() << "</id>\n"
            << "\t\t\t<handle>" << cli->getHandle() << "</handle>\n"
            << "\t\t\t<name>" << cli->getName() << "</name>\n"
            << "\t\t\t<full_name>" << cli->getFullname() << "</full_name>\n"
            << "\t\t\t<street>" << cli->getStreet() << "</street>\n"
            << "\t\t\t<city>" << cli->getCity() << "</city>\n"
            << "\t\t\t<zip>" << cli->getZip() << "</zip>\n"
            << "\t\t\t<country>" << cli->getCountry() << "</country>\n"
            << "\t\t\t<ico>" << cli->getICO() << "</ico>\n"
            << "\t\t\t<vat_num>" << cli->getVatNumber() << "</vat_num>\n"
            << "\t\t\t<apply_vat>" << cli->getVatApply() << "</apply_vat>\n"
            << "\t\t\t<registration>" << cli->getRegistration() << "</registration>\n"
            << "\t\t\t<reclamation>" << cli->getReclamation() << "</reclamation>\n"
            << "\t\t\t<email>" << cli->getEmail() << "</email>\n"
            << "\t\t\t<url>" << cli->getURL() << "</url>\n"
            << "\t\t\t<phone>" << cli->getPhone() << "</phone>\n"
            << "\t\t\t<fax>" << cli->getFax() << "</fax>\n"
            << "\t\t</client>\n";
        std::cout
            << "\t</invoice>\n";
    }
    std::cout << "</objects>" << std::endl;

}
void
InvoiceClient::archive()
{
    std::auto_ptr<Fred::Document::Manager> docMan(
            Fred::Document::Manager::create(
                docgen_path.get_value(),
                docgen_template_path.get_value(),
                fileclient_path.get_value(),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, nameservice_context);
    MailerManager mailMan(cc.getNS());
    std::auto_ptr<Fred::Invoicing::Manager> invMan(
            Fred::Invoicing::Manager::create(
                docMan.get(),
                &mailMan)
            );
    invMan->archiveInvoices(!invoice_dont_send);
    return;
}

long round_price(double x)
{
    return (x > 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
}

void
InvoiceClient::credit()
{
    std::auto_ptr<Fred::Invoicing::Manager>
        invMan(Fred::Invoicing::Manager::create());

    std::string taxDate;
    Money price;
    int zoneId, regId;
    bool regFilled = false;
    bool priceFilled = false;
    bool zoneFilled = false;


    if (credit_params.zone_id.is_value_set()) {
        zoneId = credit_params.zone_id.get_value();
        zoneFilled = true;
    }

    if (credit_params.registrar_id.is_value_set()) {
        regId = credit_params.registrar_id.get_value();
        regFilled = true;
    }

    if (credit_params.price.is_value_set()) {
        price = credit_params.price.get_value();
        priceFilled = true;
    }

    if (credit_params.taxdate.is_value_set()) {
        taxDate = credit_params.taxdate.get_value();
    } else {
        Database::Date now(Database::NOW);
        taxDate = now.to_string();
    }


    // TODO error messages are not precise for version2.3
    if (!regFilled) {
        std::cerr << "Registrar is not set, use ``--registrar_id'' to set it" << std::endl;
        return;
    }
    if (!priceFilled) {
        std::cerr << "Price is not set, use ``--price'' to set it" << std::endl;
        return;
    }
    if (!zoneFilled) {
        std::cerr << "Zone id is not set, use --zone_id to set it " << std::endl;
        return;
    }

    invMan->createDepositInvoice(taxDate, zoneId, regId, price);

}


void
InvoiceClient::factoring()
{
    std::auto_ptr<Fred::Invoicing::Manager>
        invMan(Fred::Invoicing::Manager::create());

    bool zoneFilled = false;
    bool regFilled = false;
    Database::ID zoneId;
    std::string zoneName;
    Database::Date toDate;
    Database::Date taxDate;
    Database::ID registrarId;
    std::string registrarName;

    if (factoring_params.zone_fqdn.is_value_set()) {
        zoneName = factoring_params.zone_fqdn.get_value();
        zoneFilled = true;
    }

    if (factoring_params.registrar_handle.is_value_set()) {
        registrarName = factoring_params.registrar_handle.get_value();
        regFilled = true;
    }
    Database::Date now(Database::NOW);
    Database::Date first_this(now.get().year(), now.get().month(), 1);
    Database::Date last_prev(first_this - Database::Days(1));

    if (factoring_params.todate.is_value_set()) {
        toDate = Database::Date(createDateTime(factoring_params.todate.get_value()));

        if(toDate.is_special()) {
            std::cerr << "Invalid date given for ``todate'' " << std::endl;
            toDate = first_this;
        }
    } else {
        toDate = first_this;
    }
    
    if (factoring_params.taxdate.is_value_set()) {
        taxDate = Database::Date(factoring_params.taxdate.get_value());

        if(taxDate.is_special()) {
            std::cerr << "Invalid date given for ``taxdate'' " << std::endl;
            taxDate = last_prev;
        }
    } else {
        taxDate = last_prev;
    }
    
    if (!zoneFilled) {
        std::cerr << "Zone is not set, use ``zone_fqdn'' to set it" << std::endl;
        return;
    }

    std::string toDate_str(toDate.to_string());
    std::string taxDate_str(taxDate.to_string());

    LOGGER(PACKAGE).debug ( boost::format("InvoiceClient::factoring"
            " zoneName %1%  registrarName %2% taxDate_str %3%  toDate_str %4%")
    % zoneName % registrarName % taxDate_str % toDate_str );

    boost::gregorian::from_simple_string(taxDate_str);

    if (!regFilled) {
        invMan->createAccountInvoices( zoneName
                , boost::gregorian::from_simple_string(taxDate_str)
                , boost::gregorian::from_simple_string(toDate_str));
    } else {
        invMan->createAccountInvoice( registrarName, zoneName
                , boost::gregorian::from_simple_string(taxDate_str)
                , boost::gregorian::from_simple_string(toDate_str));
    }

}

void
InvoiceClient::add_invoice_prefix()
{
    std::auto_ptr<Fred::Invoicing::Manager>
        invMan(Fred::Invoicing::Manager::create());

    unsigned int type = prefix_params.type.get_value();
    if (type > 1) {
        std::cerr << "Type can be either 0 or 1." << std::endl;
        return;
    }
    unsigned int year;
    if (prefix_params.year.is_value_set()) {
        year = prefix_params.year.get_value();
    } else {
        Database::Date now(Database::NOW);
        year = now.get().year();
    }
    unsigned long long prefix = prefix_params.prefix.get_value();
    if (prefix_params.zone_id.is_value_set()) {
        unsigned int zoneId = prefix_params.zone_id.get_value();
        invMan->insertInvoicePrefix(zoneId, type, year, prefix);
    } else if (prefix_params.zone_fqdn.is_value_set()) {
        std::string zoneName = prefix_params.zone_fqdn.get_value();
        invMan->insertInvoicePrefix(zoneName, type, year, prefix);
    }
}

void
InvoiceClient::create_invoice()
{
    // init file manager
    CorbaClient corba_client(0, 0, m_nsAddr, nameservice_context);
    FileManagerClient fm_client(corba_client.getNS());
    Fred::File::ManagerPtr file_manager(Fred::File::Manager::create(&fm_client));

    // bank manager
    Fred::Banking::ManagerPtr bank_manager(Fred::Banking::Manager::create(file_manager.get()));

    Database::ID paymentId = create_params.payment_id.get_value();
    if (create_params.registrar_handle.is_value_set()) {
        bank_manager->pairPaymentWithRegistrar(paymentId,
                create_params.registrar_handle.get_value());
    } else {
        std::cerr << "You have to specify  ``--registrar_handle''" << std::endl;
    }
}

} // namespace Admin;
