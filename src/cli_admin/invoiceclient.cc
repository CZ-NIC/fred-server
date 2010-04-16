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
#include "commonclient.h"
#include "invoiceclient.h"
#include "register/invoice_manager.h"
#include "../register/model_zone.h"
namespace Admin {


const struct options *
InvoiceClient::getOpts()
{
    return m_opts;
}

void
InvoiceClient::runMethod()
{
    if (m_conf.hasOpt(INVOICE_LIST_NAME)) {
        list();
    } else if (m_conf.hasOpt(INVOICE_ARCHIVE_NAME)) {
        archive();
    } else if (m_conf.hasOpt(INVOICE_LIST_FILTERS_NAME)) {
        list_filters();
    } else if (m_conf.hasOpt(INVOICE_SHOW_OPTS_NAME)) {
        show_opts();
    } else if (m_conf.hasOpt(INVOICE_CREDIT_NAME)) {
        credit();
    } else if (m_conf.hasOpt(INVOICE_FACTORING_NAME)) {
        factoring();
    } else if (m_conf.hasOpt(INVOICE_ADD_PREFIX_NAME)) {
        add_invoice_prefix();
    } else if (m_conf.hasOpt(INVOICE_CREATE_INVOICE_NAME)) {
        create_invoice();
    }
}

void
InvoiceClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("Invoice", getOpts(), getOptsCount());
}

void InvoiceClient::filter_reload_invoices(Register::Invoicing::Manager *invMan, Register::Invoicing::List *invList)
{     
    Database::Filters::Invoice *invFilter;
    invFilter = new Database::Filters::InvoiceImpl();

    if (m_conf.hasOpt(ID_NAME))
        invFilter->addId().setValue(
                Database::ID(m_conf.get<unsigned int>(ID_NAME)));

    if (m_conf.hasOpt(ZONE_ID_NAME))
        invFilter->addZoneId().setValue(
                Database::ID(m_conf.get<unsigned int>(ZONE_ID_NAME)));
    if (m_conf.hasOpt(ZONE_FQDN_NAME)) {
        invFilter->addZone().addFqdn().setValue(
                m_conf.get<std::string>(ZONE_FQDN_NAME));
    }
    if (m_conf.hasOpt(INVOICE_TYPE_NAME))
        invFilter->addType().setValue(
                m_conf.get<unsigned int>(INVOICE_TYPE_NAME));
    if (m_conf.hasOpt(INVOICE_NUMBER_NAME))
        invFilter->addNumber().setValue(
                m_conf.get<std::string>(INVOICE_NUMBER_NAME));

    apply_CRDATE(invFilter);
    apply_DATE(invFilter, INVOICE_TAXDATE_NAME, Tax);

    if (m_conf.hasOpt(REGISTRAR_ID_NAME))
        invFilter->addRegistrar().addId().setValue(
                Database::ID(m_conf.get<unsigned int>(REGISTRAR_ID_NAME)));
    if (m_conf.hasOpt(REGISTRAR_HANDLE_NAME))
        invFilter->addRegistrar().addHandle().setValue(
                m_conf.get<std::string>(REGISTRAR_HANDLE_NAME));
    if (m_conf.hasOpt(INVOICE_FILE_PDF_NAME)) {
        if (m_conf.get<unsigned int>(INVOICE_FILE_PDF_NAME) == 0) {
            invFilter->addFilePDF().setNULL();
        } else {
            invFilter->addFilePDF().setValue(Database::ID(
                        m_conf.get<unsigned int>(INVOICE_FILE_PDF_NAME)));
        }
    }
    if (m_conf.hasOpt(INVOICE_FILE_XML_NAME)) {
        if (m_conf.get<unsigned int>(INVOICE_FILE_XML_NAME) == 0) {
            invFilter->addFileXML().setNULL();
        } else {
            invFilter->addFileXML().setValue(Database::ID(
                        m_conf.get<unsigned int>(INVOICE_FILE_XML_NAME)));
        }
    }
    if (m_conf.hasOpt(INVOICE_FILE_NAME_NAME))
        invFilter->addFile().addName().setValue(
                m_conf.get<std::string>(INVOICE_FILE_NAME_NAME));


   Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(invFilter);

    invList->setLimit(m_conf.get<unsigned int>(LIMIT_NAME));
    invList->reload(*unionFilter);

    unionFilter->clear();
    delete unionFilter;   
}

void
InvoiceClient::list()
{
    callHelp(m_conf, list_help);

    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );

    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    MailerManager mailMan(cc.getNS());

    std::auto_ptr<Register::Invoicing::Manager> invMan(
            Register::Invoicing::Manager::create(
                docMan.get(),
                &mailMan));

    std::auto_ptr<Register::Invoicing::List> invList(
            invMan->createList());

    filter_reload_invoices(invMan.get(), invList.get());
    invList->exportXML(std::cout);

}


void
InvoiceClient::list_filters()
{
    callHelp(m_conf, list_help);

    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );

    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    MailerManager mailMan(cc.getNS());

    std::auto_ptr<Register::Invoicing::Manager> invMan(
            Register::Invoicing::Manager::create(
                docMan.get(),
                &mailMan));

    std::auto_ptr<Register::Invoicing::List> invList(
            invMan->createList());

    filter_reload_invoices(invMan.get(), invList.get());

    std::cout << "<objects>\n";
    for (unsigned int i = 0; i < invList->getSize(); i++) {
        Register::Invoicing::Invoice *inv = invList->get(i);
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
            << "\t\t<type>" << Register::Invoicing::Type2Str(inv->getType()) << "</type>\n"
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
            Register::Invoicing::PaymentSource *source = 
                (Register::Invoicing::PaymentSource *)inv->getSource(j);
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
            Register::Invoicing::PaymentAction *act =
                (Register::Invoicing::PaymentAction *)inv->getAction(j);
            std::cout
                << "\t\t<payment_action>\n"
                << "\t\t\t<id>" << act->getObjectId() << "</id>\n"
                << "\t\t\t<name>" << act->getObjectName() << "</name>\n"
                << "\t\t\t<action_time>" << act->getActionTime() << "</action_time>\n"
                << "\t\t\t<ex_date>" << act->getExDate() << "</ex_date>\n"
                << "\t\t\t<action>" << act->getActionStr()
                // << Register::Invoicing::PaymentActionType2Str(act->getAction())
                << "</action>\n"
                << "\t\t\t<units_count>" << act->getUnitsCount() << "</units_count>\n"
                << "\t\t\t<price_per_unit>" << act->getPricePerUnit() << "</price_per_unit>\n"
                << "\t\t</payment_action>\n";
        }
        for (unsigned int j = 0; j < inv->getPaymentCount(); j++) {
            Register::Invoicing::Payment *pay = 
                (Register::Invoicing::Payment *)inv->getPayment(j);
            std::cout
                << "\t\t<payment>\n"
                << "\t\t\t<price>" << pay->getPrice() << "</price>\n"
                << "\t\t\t<vat_rate>" << pay->getVatRate() << "</vat_rate>\n"
                << "\t\t\t<vat>" << pay->getVat() << "</vat>\n"
                << "\t\t\t<price_with_vat>" << pay->getPriceWithVat() << "</price_with_vat>\n"
                << "\t\t</payment>\n";
        }
        Register::Invoicing::Subject *sup =
            (Register::Invoicing::Subject *)inv->getSupplier();
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
        Register::Invoicing::Subject *cli =
            (Register::Invoicing::Subject *)inv->getClient();
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
    callHelp(m_conf, archive_help);
    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    MailerManager mailMan(cc.getNS());
    std::auto_ptr<Register::Invoicing::Manager> invMan(
            Register::Invoicing::Manager::create(
                docMan.get(),
                &mailMan)
            );
    invMan->archiveInvoices(!m_conf.hasOpt(INVOICE_DONT_SEND_NAME));
    return;
}

void
InvoiceClient::credit()
{
    callHelp(m_conf, credit_help);
    std::auto_ptr<Register::Invoicing::Manager>
        invMan(Register::Invoicing::Manager::create());

    // std::auto_ptr<Register::Invoicing::Invoice>
        // invoice(invMan->createDepositInvoice());
    std::string taxDate;
    long price;
    int zoneId, regId;
    bool regFilled = false;
    bool priceFilled = false;
    bool zoneFilled = false;

    /* not supported
    if (m_conf.hasOpt(INVOICE_ZONE_NAME_NAME)) {
        zone = m_conf.get<std::string>(INVOICE_ZONE_NAME_NAME);
    }
    */
    if (m_conf.hasOpt(INVOICE_ZONE_ID_NAME)) {
        zoneId = m_conf.get<unsigned int>(INVOICE_ZONE_ID_NAME);
        zoneFilled = true;
    }

    if (m_conf.hasOpt(INVOICE_REGISTRAR_ID_NAME)) {
        regId = m_conf.get<unsigned int>(
                    INVOICE_REGISTRAR_ID_NAME);
        regFilled = true;
    }
    /*
    if (m_conf.hasOpt(INVOICE_REGISTRAR_HANDLE_NAME)) {
                registrar = m_conf.get<std::string>(INVOICE_REGISTRAR_HANDLE_NAME);
        regFilled = true;
    }
    */
    if (m_conf.hasOpt(INVOICE_PRICE_NAME)) {
        // invoice->setPrice(Database::Money(
        price = 100 * m_conf.get<double>(INVOICE_PRICE_NAME);
        priceFilled = true;
    }

    if (m_conf.hasOpt(INVOICE_TAXDATE_NAME)) {
        // invoice->setTaxDate(Database::Date(
        taxDate = m_conf.get<std::string>(INVOICE_TAXDATE_NAME);
    } else {
        Database::Date now(Database::NOW);
        taxDate = now.to_string();
    }

    /* not supported
    if (m_conf.hasOpt(CRDATE_NAME)) {
        invoice->setCrDate(Database::DateTime(
                    m_conf.get<std::string>(CRDATE_NAME)));
    }
    */

    // TODO error messages are not precise for version2.3
    if (!regFilled) {
        //std::cerr << "Registrar is not set, use ``--registrar_id'' or "
//            << "``--registrar_name'' to set it" << std::endl;

        std::cerr << "Registrar is not set, use ``--" << INVOICE_REGISTRAR_ID_NAME << "'' to set it" << std::endl;
        return;
    }
    if (!priceFilled) {
        std::cerr << "Price is not set, use ``--" << INVOICE_PRICE_NAME << "'' to set it" << std::endl;
        return;
    }
    if (!zoneFilled) {
        std::cerr << "Zone id is not set, use --" << INVOICE_ZONE_ID_NAME << " to set it " << std::endl;
        return;
    }

    (void) invMan->createDepositInvoice(taxDate, zoneId, regId, price); 

    /*
    if (invoice->save() == false) {
        std::cerr << "Error has occured:" << std::endl;
        std::vector<std::string> errors = invoice->getErrors();
        for (int i = 0; i < (int)errors.size(); i++) {
            std::cerr << errors[i] << std::endl;
        }
    }
    */
}


/* 
 * this new crap shouldn't be used
void
InvoiceClient::factoring(Register::Invoicing::Manager *man,
        Database::ID zoneId, std::string zoneName,
        Database::ID regId, std::string regName,
        Database::Date toDate, Database::Date taxDate)
{
    std::auto_ptr<Register::Invoicing::Invoice>
        invoice(man->createAccountInvoice());
    if (zoneId != Database::ID()) {
        invoice->setZoneId(zoneId);
    }
    if (!zoneName.empty()) {
        ModelZone *zone = new ModelZone();
        zone->setFqdn(zoneName);
        invoice->setZoneId(zone->getId());
    }
    if (regId != Database::ID()) {
        invoice->setRegistrarId(regId);
    }
    if (!regName.empty()) {
        ModelRegistrar *registrar = new ModelRegistrar();
        registrar->setHandle(regName);
        invoice->setRegistrarId(registrar->getId());
    }
    invoice->setToDate(toDate);
    invoice->setTaxDate(taxDate);
    if (!invoice->save()) {
        std::vector<std::string> errors = invoice->getErrors();
        if (errors.size() == 0) {
            return;
        }
        std::cerr << "Error has occured:" << std::endl;
        for (int i = 0; i < (int)errors.size(); i++) {
            std::cerr << errors[i] << std::endl;
        }
    }
}
*/

void
InvoiceClient::factoring()
{
    callHelp(m_conf, factoring_help);
    std::auto_ptr<Register::Invoicing::Manager>
        invMan(Register::Invoicing::Manager::create());
    std::auto_ptr<Register::Registrar::Manager>
        regMan(Register::Registrar::Manager::create(&m_db));

    bool zoneFilled = false;
    bool regFilled = false;
    Database::ID zoneId;
    std::string zoneName;
    Database::Date toDate;
    Database::Date taxDate;
    Database::ID registrarId;
    std::string registrarName;

    if (m_conf.hasOpt(INVOICE_ZONE_NAME_NAME)) {
        zoneName = m_conf.get<std::string>(INVOICE_ZONE_NAME_NAME);
        zoneFilled = true;
    }
    /* this is not supported currently
    if (m_conf.hasOpt(INVOICE_ZONE_ID_NAME)) {
        zoneId = Database::ID(
                    m_conf.get<unsigned int>(INVOICE_ZONE_ID_NAME));
        zoneFilled = true;
    }
    if (m_conf.hasOpt(INVOICE_REGISTRAR_ID_NAME)) {
        registrarId = Database::ID(
                    m_conf.get<unsigned int>(INVOICE_REGISTRAR_ID_NAME));
        regFilled = true;
    }
    */
    if (m_conf.hasOpt(INVOICE_REGISTRAR_HANDLE_NAME)) {
        registrarName = m_conf.get<std::string>(INVOICE_REGISTRAR_HANDLE_NAME);
        regFilled = true;
    }
    Database::Date now(Database::NOW);
    Database::Date first_this(now.get().year(), now.get().month(), 1);
    Database::Date last_prev(first_this - Database::Days(1));

    if (m_conf.hasOpt(INVOICE_TODATE_NAME)) {
        toDate = Database::Date(createDateTime(m_conf.get<std::string>(INVOICE_TODATE_NAME)));

        if(toDate.is_special()) {
            std::cerr << "Invalid date given for ``" <<  INVOICE_TODATE_NAME << "'' " << std::endl;
            toDate = last_prev;
        }
    } else {
        toDate = last_prev;
    }
    
    if (m_conf.hasOpt(INVOICE_TAXDATE_NAME)) {
        taxDate = Database::Date(m_conf.get<std::string>(INVOICE_TAXDATE_NAME));

        if(taxDate.is_special()) {
            std::cerr << "Invalid date given for ``" <<  INVOICE_TAXDATE_NAME << "'' " << std::endl;
            taxDate = first_this;
        }
    } else {
        taxDate = first_this;
    }
    
    if (!zoneFilled) {
        std::cerr << "Zone is not set, use ``"<< INVOICE_ZONE_ID_NAME << "'' or "
            << "``" << INVOICE_ZONE_NAME_NAME << "'' to set it" << std::endl;
        return;
    }

    std::string toDate_str(toDate.to_string());
    std::string taxDate_str(taxDate.to_string());

    if (!regFilled) {
        invMan->factoring_all( Manager::getConnectionString().c_str(), zoneName.c_str(), taxDate_str.c_str(), toDate_str.c_str());
    } else {
        invMan->factoring( Manager::getConnectionString().c_str(), registrarName.c_str(), zoneName.c_str(), taxDate_str.c_str(), toDate_str.c_str());
    }

    /*
    if (!regFilled) {
        Database::Filters::Registrar *regFilter;
        regFilter = new Database::Filters::RegistrarImpl(true);
        Database::Filters::Union *unionFilter;
        unionFilter = new Database::Filters::Union();
        unionFilter->addFilter(regFilter);
        Register::Registrar::RegistrarList::AutoPtr list( regMan->createList());
        list->reload(*unionFilter);
        int i = 0;
        Register::Registrar::Registrar *reg;
        while (1) {
            try {
                reg = list->get(i);
            } catch (...) {
                return;
            }
            factoring(invMan.get(), zoneId, zoneName, reg->getId(),
                    reg->getHandle(), toDate, taxDate);
            i++;
        }
    } else {
        factoring(invMan.get(), zoneId, zoneName, registrarId, registrarName,
                toDate, taxDate);
    }
    */
}

void
InvoiceClient::add_invoice_prefix()
{
    callHelp(m_conf, add_invoice_prefix_help);
    std::auto_ptr<Register::Invoicing::Manager>
        invMan(Register::Invoicing::Manager::create());

    unsigned int type = m_conf.get<unsigned int>(INVOICE_PREFIX_TYPE_NAME);
    if (type > 1) {
        std::cerr << "Type can be either 0 or 1." << std::endl;
        return;
    }
    unsigned int year;
    if (m_conf.hasOpt(INVOICE_PREFIX_YEAR_NAME)) {
        year = m_conf.get<unsigned int>(INVOICE_PREFIX_YEAR_NAME);
    } else {
        Database::Date now(Database::NOW);
        year = now.get().year();
    }
    unsigned long long prefix = m_conf.get<unsigned long long>(INVOICE_PREFIX_PREFIX_NAME);
    if (m_conf.hasOpt(INVOICE_ZONE_ID_NAME)) {
        unsigned int zoneId = m_conf.get<unsigned int>(INVOICE_ZONE_ID_NAME);
        invMan->insertInvoicePrefix(zoneId, type, year, prefix);
    } else if (m_conf.hasOpt(INVOICE_ZONE_NAME_NAME)) {
        std::string zoneName = m_conf.get<std::string>(INVOICE_ZONE_NAME_NAME);
        invMan->insertInvoicePrefix(zoneName, type, year, prefix);
    }
}

void
InvoiceClient::create_invoice()
{
    callHelp(m_conf, create_invoice_help);

    /* init file manager */
    CorbaClient corba_client(0, 0, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    FileManagerClient fm_client(corba_client.getNS());
    Register::File::ManagerPtr file_manager(Register::File::Manager::create(&fm_client));

    /* bank manager */
    Register::Banking::ManagerPtr bank_manager(Register::Banking::Manager::create(file_manager.get()));

    Database::ID paymentId = m_conf.get<unsigned int>(INVOICE_PAYMENT_ID_NAME);
    if (m_conf.hasOpt(REGISTRAR_HANDLE_NAME)) {
        bank_manager->pairPaymentWithRegistrar(paymentId,
                m_conf.get<std::string>(REGISTRAR_HANDLE_NAME));
    } else {
        std::cerr << "You have to specify ``--"<< REGISTRAR_HANDLE_NAME << "''" << std::endl;

        // std::cerr << "You have to specify ``--" << REGISTRAR_ID_NAME
          //  << "'' or ``--" << REGISTRAR_HANDLE_NAME << "''" << std::endl;
    }
}

void
InvoiceClient::list_help()
{
    std::cout <<
        "** List invoices **\n\n"
        "  $ " << g_prog_name << " --" << INVOICE_LIST_NAME << " \\\n"
        "    [--" << ID_NAME << "=<id_number>] \\\n"
        "    [--" << ZONE_ID_NAME << "=<id_number>] \\\n"
        "    [--" << ZONE_FQDN_NAME << "=<zone_fqdn>] \\\n"
        "    [--" << INVOICE_TYPE_NAME << "=<invoice_type>] \\\n"
        "    [--" << INVOICE_NUMBER_NAME << "=<invoice_number>] \\\n"
        "    [--" << CRDATE_NAME << "=<invoice_create_date>] \\\n"
        "    [--" << INVOICE_TAXDATE_NAME << "=<invoice_taxdate>] \\\n"
        "    [--" << REGISTRAR_ID_NAME <<    "=<registrar_id>] \\\n"
        "    [--" << REGISTRAR_HANDLE_NAME <<    "=<registrar_handle>] \\\n"
        "    [--" << INVOICE_FILE_NAME_NAME << "=<file_name>] \n"
        "    [--" << INVOICE_FILE_XML_NAME << "=<xml file_name>] \n"
        "    [--" << INVOICE_FILE_PDF_NAME << "=<pdf file_name>] \n"
        << std::endl;
}

void
InvoiceClient::archive_help()
{
    std::cout <<
        "** Invoice archive **\n\n"
        "  $ " << g_prog_name << " --" << INVOICE_ARCHIVE_NAME << " \\\n"
        "    [--" << INVOICE_DONT_SEND_NAME << "] \\\n"
        << std::endl;
}

void
InvoiceClient::credit_help()
{
    // TODO features not supported in 2.3 are commented out
    std::cout <<
        "** Invoice credit **\n\n"
        "  $ " << g_prog_name << " --" << INVOICE_CREDIT_NAME << " \\\n"
        "    --" << INVOICE_ZONE_ID_NAME << "=<zone_id> | \\\n"
        // "    --" << INVOICE_ZONE_NAME_NAME << "=<zone_fqdn> \\\n"
        "    --" << INVOICE_REGISTRAR_ID_NAME << "=<registrar_id> | \\\n"
        // "    --" << INVOICE_REGISTRAR_HANDLE_NAME << "=<registrar_handle> \\\n"
        "    --" << INVOICE_PRICE_NAME << "=<price> \\\n"
        // "    [--" << CRDATE_NAME << "=<create_time_stamp>] \\\n"
        "    [--" << INVOICE_TAXDATE_NAME << "=<tax_date>]\n"
        << std::endl;
    // std::cout << "Default value for ``--" << CRDATE_NAME << "'' is current timestamp "
     //   << "and today for ``--" << INVOICE_TAXDATE_NAME << "''."
     //   << std::endl;
//    std::cout << "Default value for ``--" << INVOICE_TAXDATE_NAME << "'' is today's timestamp. For ``--" << INVOICE_TODATE_NAME << "'' it's last day of previous month.  " << std::endl;
    std::cout << "Default value for ``--" << INVOICE_TAXDATE_NAME << "'' is today's timestamp. " << std::endl;
}

void
InvoiceClient::factoring_help()
{
    std::cout <<
        "** Invoice factoring **\n\n"
        "  $ " << g_prog_name << " --" << INVOICE_FACTORING_NAME << " \\\n"
        // not supported "    --" << INVOICE_ZONE_ID_NAME << "=<zone_id> | \\\n"
        "    --" << INVOICE_ZONE_NAME_NAME << "=<zone_fqdn> \\\n"
        // not supported "    [--" << INVOICE_REGISTRAR_ID_NAME << "=<registrar_id> | \\\n"
        "    --" << INVOICE_REGISTRAR_HANDLE_NAME << "=<registrar_handle>] \\\n"
        "    [--" << INVOICE_TODATE_NAME << "=<to_date>] \\\n"
        "    [--" << INVOICE_TAXDATE_NAME << "=<tax_date>]\n"
        << std::endl;
    std::cout << "Default value for ``to date'' is last day of previous month "
        "and for ``tax date'' it is first day of this month."
        << std::endl;
}

void
InvoiceClient::add_invoice_prefix_help()
{
    std::cout <<
        "** Invoice add prefix **\n\n"
        "  $ " << g_prog_name << " --" << INVOICE_ADD_PREFIX_NAME << " \\\n"
        "    --" << INVOICE_ZONE_ID_NAME << "=<zone_id> | \\\n"
        "    --" << INVOICE_ZONE_NAME_NAME << "=<zone_fqdn> \\\n"
        "    --" << INVOICE_PREFIX_TYPE_NAME << "=<type> \\\n"
        "    [--" << INVOICE_PREFIX_YEAR_NAME << "=<year>] \\\n"
        "    --" << INVOICE_PREFIX_PREFIX_NAME << "=<prefix_number>\n"
        << std::endl <<
        "Type is either 0 (for the deposit invoice) or 1 (for account invoice).\n"
        "Default value for the year is the current year.\n"
        << std::endl;
}

void
InvoiceClient::create_invoice_help()
{
    std::cout <<
        "** Manually create invoice for payment **\n\n"
        "  $ " << g_prog_name << " --" << INVOICE_CREATE_INVOICE_NAME << " \\\n"
        "    --" << INVOICE_PAYMENT_ID_NAME << "=<payment_id> \\\n"
        "    --" << REGISTRAR_HANDLE_NAME << "=<registrar_handle>\n"
        << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_INVOICE, name, name##_DESC, type, callable, visible}

const struct options
InvoiceClient::m_opts[] = {
    ADDOPT(INVOICE_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(INVOICE_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(INVOICE_LIST_FILTERS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(INVOICE_ARCHIVE_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(INVOICE_CREDIT_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(INVOICE_FACTORING_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(INVOICE_ADD_PREFIX_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(INVOICE_CREATE_INVOICE_NAME, TYPE_NOTYPE, true, true),
    add_ID,
    add_REGISTRAR_ID,
    add_REGISTRAR_HANDLE,
    add_ZONE_ID,
    add_ZONE_FQDN, 
    add_CRDATE,
    ADDOPT(INVOICE_TYPE_NAME, TYPE_UINT, false, false),
    ADDOPT(INVOICE_VAR_SYMBOL_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_TAXDATE_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_ARCHIVED_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_OBJECT_ID_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_OBJECT_NAME_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_ADV_NUMBER_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_FILE_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(INVOICE_FILE_NAME_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_FILE_XML_NAME, TYPE_UINT, false, false),
    ADDOPT(INVOICE_FILE_PDF_NAME, TYPE_UINT, false, false),
    ADDOPT(INVOICE_ZONE_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(INVOICE_ZONE_NAME_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_REGISTRAR_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(INVOICE_REGISTRAR_HANDLE_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_PRICE_NAME, TYPE_DOUBLE, false, false),
    ADDOPT(INVOICE_TODATE_NAME, TYPE_STRING, false, false),
    ADDOPT(INVOICE_DONT_SEND_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(INVOICE_PREFIX_TYPE_NAME, TYPE_UINT, false, false),
    ADDOPT(INVOICE_PREFIX_YEAR_NAME, TYPE_UINT, false, false),
    ADDOPT(INVOICE_PREFIX_PREFIX_NAME, TYPE_ULONGLONG, false, false),
    ADDOPT(INVOICE_PAYMENT_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(INVOICE_NUMBER_NAME, TYPE_STRING, false, false),
};

#undef ADDOPT

int 
InvoiceClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;
