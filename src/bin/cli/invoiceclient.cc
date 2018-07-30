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
#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/invoiceclient.hh"
#include "src/libfred/credit.hh"
#include "src/libfred/invoicing/invoice.hh"
#include "src/libfred/model_zone.hh"
#include "src/util/types/money.hh"
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
    } else if (invoice_billing) {
        billing();
    } else if (invoice_add_prefix) {
        add_invoice_prefix();
    } else if (invoice_create) {
        create_invoice();
    }
}


void InvoiceClient::filter_reload_invoices(LibFred::Invoicing::Manager *invMan, LibFred::Invoicing::List *invList)
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
    std::unique_ptr<LibFred::Document::Manager> docMan(
            LibFred::Document::Manager::create(
                docgen_path.get_value(),
                docgen_template_path.get_value(),
                fileclient_path.get_value(),
                m_nsAddr)
            );

    CorbaClient cc(0, NULL, m_nsAddr, nameservice_context);
    MailerManager mailMan(cc.getNS());

    std::unique_ptr<LibFred::Invoicing::Manager> invMan(
            LibFred::Invoicing::Manager::create(
                docMan.get(),
                &mailMan));

    std::unique_ptr<LibFred::Invoicing::List> invList(
            invMan->createList());

    filter_reload_invoices(invMan.get(), invList.get());
    invList->exportXML(std::cout);
}


void
InvoiceClient::list_filters()
{
    std::unique_ptr<LibFred::Document::Manager> docMan(
            LibFred::Document::Manager::create(
                docgen_path.get_value(),
                docgen_template_path.get_value(),
                fileclient_path.get_value(),
                m_nsAddr)
            );

    CorbaClient cc(0, NULL, m_nsAddr, nameservice_context);
    MailerManager mailMan(cc.getNS());

    std::unique_ptr<LibFred::Invoicing::Manager> invMan(
            LibFred::Invoicing::Manager::create(
                docMan.get(),
                &mailMan));

    std::unique_ptr<LibFred::Invoicing::List> invList(
            invMan->createList());

    filter_reload_invoices(invMan.get(), invList.get());

    std::cout << "<objects>\n";
    for (unsigned int i = 0; i < invList->getSize(); i++) {
        LibFred::Invoicing::Invoice *inv = invList->get(i);
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
            << "\t\t<type>" << LibFred::Invoicing::Type2Str(inv->getType()) << "</type>\n"
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
            LibFred::Invoicing::PaymentSource *source =
                (LibFred::Invoicing::PaymentSource *)inv->getSource(j);
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
            LibFred::Invoicing::PaymentAction *act =
                (LibFred::Invoicing::PaymentAction *)inv->getAction(j);
            std::cout
                << "\t\t<payment_action>\n"
                << "\t\t\t<id>" << act->getObjectId() << "</id>\n"
                << "\t\t\t<name>" << act->getObjectName() << "</name>\n"
                << "\t\t\t<action_time>" << act->getActionTime() << "</action_time>\n"
                << "\t\t\t<ex_date>" << act->getExDate() << "</ex_date>\n"
                << "\t\t\t<action>" << act->getActionStr()
                // << LibFred::Invoicing::PaymentActionType2Str(act->getAction())
                << "</action>\n"
                << "\t\t\t<units_count>" << act->getUnitsCount() << "</units_count>\n"
                << "\t\t\t<price_per_unit>" << act->getPricePerUnit() << "</price_per_unit>\n"
                << "\t\t</payment_action>\n";
        }
        for (unsigned int j = 0; j < inv->getPaymentCount(); j++) {
            LibFred::Invoicing::Payment *pay =
                (LibFred::Invoicing::Payment *)inv->getPaymentByIdx(j);
            std::cout
                << "\t\t<payment>\n"
                << "\t\t\t<price>" << pay->getPrice() << "</price>\n"
                << "\t\t\t<vat_rate>" << pay->getVatRate() << "</vat_rate>\n"
                << "\t\t\t<vat>" << pay->getVat() << "</vat>\n"
                << "\t\t\t<price_with_vat>" << pay->getPriceWithVat() << "</price_with_vat>\n"
                << "\t\t</payment>\n";
        }
        LibFred::Invoicing::Subject *sup =
            (LibFred::Invoicing::Subject *)inv->getSupplier();
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
        LibFred::Invoicing::Subject *cli =
            (LibFred::Invoicing::Subject *)inv->getClient();
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

} // "
void
InvoiceClient::archive()
{
    std::unique_ptr<LibFred::Document::Manager> docMan(
            LibFred::Document::Manager::create(
                docgen_path.get_value(),
                docgen_template_path.get_value(),
                fileclient_path.get_value(),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, nameservice_context);
    MailerManager mailMan(cc.getNS());
    std::unique_ptr<LibFred::Invoicing::Manager> invMan(
            LibFred::Invoicing::Manager::create(
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
    std::unique_ptr<LibFred::Invoicing::Manager>
        invMan(LibFred::Invoicing::Manager::create());

    boost::posix_time::ptime local_current_timestamp
        = boost::posix_time::microsec_clock::local_time();

    unsigned long long zoneId = 0;
    if (credit_params.zone_id.is_value_set())
    {
        zoneId = credit_params.zone_id.get_value();
    }
    else
    {
        throw std::runtime_error("Zone id is not set, use --zone_id to set it ");
    }

    unsigned long long regId = 0;
    if (credit_params.registrar_id.is_value_set())
    {
        regId = credit_params.registrar_id.get_value();
    }
    else
    {
        throw std::runtime_error("Registrar is not set, use ``--registrar_id'' to set it");
    }

    Money price;
    if (credit_params.price.is_value_set())
    {
        price = credit_params.price.get_value();
    }
    else
    {
        throw std::runtime_error("Price is not set, use ``--price'' to set it");
    }

    boost::gregorian::date taxDate;
    if (credit_params.taxdate.is_value_set())
    {
        taxDate = boost::gregorian::from_string(credit_params.taxdate.get_value());
    }
    else
    {
        taxDate = local_current_timestamp.date();
    }

    Money out_credit;
    unsigned long long invoice_id
        = invMan->createDepositInvoice(taxDate, zoneId, regId, price, local_current_timestamp, out_credit);

    LibFred::Credit::add_credit_to_invoice( regId,  zoneId, out_credit, invoice_id);

}


void
InvoiceClient::billing()
{
    //zone
    std::string zone_name;
    if (billing_params.zone_fqdn.is_value_set())
    {
        zone_name = billing_params.zone_fqdn.get_value();
    }
    else
    {
        throw std::runtime_error("billing: zone_fqdn not set");
    }

    //registrar
    std::string registrar_name;
    if (billing_params.registrar_handle.is_value_set())
    {
        registrar_name = billing_params.registrar_handle.get_value();
    }

    //fromdate
    boost::gregorian::date fromdate;
    if (billing_params.fromdate.is_value_set())
    {
        fromdate = boost::gregorian::from_simple_string(billing_params.fromdate.get_value());

        if(fromdate.is_special())
        {
            std::cerr << "Invalid date given for ``fromdate'' " << std::endl;
            throw std::runtime_error("billing: invalid fromdate set");
        }
    }//not set fromdate is default

    //todate
    boost::gregorian::date todate;
    if (billing_params.todate.is_value_set())
    {
        todate = boost::gregorian::from_simple_string(billing_params.todate.get_value());

        if(todate.is_special())
        {
            std::cerr << "Invalid date given for ``todate'' " << std::endl;
            throw std::runtime_error("billing: invalid todate set");
        }
    }
    else
    {
        boost::gregorian::date tmp_today
            = boost::posix_time::second_clock::local_time().date();
        todate = boost::gregorian::date(tmp_today.year()
            , tmp_today.month(), 1);//first day of current month
    }

    //taxdate
    boost::gregorian::date taxdate;
    if (billing_params.taxdate.is_value_set())
    {
        taxdate = boost::gregorian::from_simple_string(billing_params.taxdate.get_value());

        if(taxdate.is_special())
        {
            std::cerr << "Invalid date given for ``taxdate'' " << std::endl;
            throw std::runtime_error("billing: invalid taxdate set");
        }
    }
    else
    {
        taxdate = todate - boost::gregorian::days(1);//last day of last month
    }

    //invoicedate
    boost::posix_time::ptime invoicedate;
    if (billing_params.invoicedate.is_value_set())
    {
        invoicedate = boost::posix_time::time_from_string(billing_params.invoicedate.get_value());

        if(invoicedate.is_special())
        {
            std::cerr << "Invalid date given for ``invoicedate'' " << std::endl;
            throw std::runtime_error("billing: invalid invoicedate set");
        }
    }
    else
    {
        invoicedate = boost::posix_time::microsec_clock::local_time();
    }

    LOGGER(PACKAGE).debug ( boost::format("InvoiceClient::billing"
            " zonename %1%  registrarname %2% taxdate %3%  todate (not included) %4% invoicedate %5%")
    % zone_name % registrar_name
    % boost::gregorian::to_simple_string(taxdate)
    % boost::gregorian::to_simple_string(todate)
    % boost::posix_time::to_simple_string(invoicedate));

    //conversion from command line params into implementation params
    todate -= boost::gregorian::days(1); //today from date after range into last day of range

    std::unique_ptr<LibFred::Invoicing::Manager>
        invMan(LibFred::Invoicing::Manager::create());

    if (!billing_params.registrar_handle.is_value_set()) {
        invMan->createAccountInvoices( zone_name
                , taxdate, fromdate, todate, invoicedate);
    }
    else
    {
        invMan->createAccountInvoice( registrar_name, zone_name
                , taxdate, fromdate, todate, invoicedate);
    }
}

void
InvoiceClient::add_invoice_prefix()
{
    std::unique_ptr<LibFred::Invoicing::Manager>
        invMan(LibFred::Invoicing::Manager::create());

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
    LibFred::File::ManagerPtr file_manager(LibFred::File::Manager::create(&fm_client));

    // bank manager
    LibFred::Banking::ManagerPtr bank_manager(LibFred::Banking::Manager::create(file_manager.get()));

    Database::ID paymentId = create_params.payment_id.get_value();
    if (create_params.registrar_handle.is_value_set()) {
        try {
            bank_manager->pairPaymentWithRegistrar(paymentId,
                    create_params.registrar_handle.get_value());
        }
        catch (const std::runtime_error& e)
        {
            std::cerr << e.what() << std::endl;
        }
    } else {
        std::cerr << "You have to specify  ``--registrar_handle''" << std::endl;
    }
}

void create_invoice_prefixes(CreateInvoicePrefixesArgs params)
{
    std::unique_ptr<LibFred::Invoicing::Manager>
        invMan(LibFred::Invoicing::Manager::create());
    invMan->createInvoicePrefixes(params.for_current_year);
}

void add_invoice_number_prefix(AddInvoiceNumberPrefixArgs params)
{
    std::unique_ptr<LibFred::Invoicing::Manager>
        invMan(LibFred::Invoicing::Manager::create());
    invMan->addInvoiceNumberPrefix(
            params.prefix
            , params.zone_fqdn
            , params.invoice_type_name
            );
}

} // namespace Admin;
