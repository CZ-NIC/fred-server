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

#include "commonclient.h"
#include "invoiceclient.h"

namespace Admin {

InvoiceClient::InvoiceClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Invoice related options");
    m_options->add_options()
        add_opt(INVOICE_LIST_NAME)
        // add_opt(INVOICE_LIST_PLAIN_NAME)
        add_opt(INVOICE_ARCHIVE_NAME)
        add_opt(INVOICE_LIST_HELP_NAME);
        // add_opt(INVOICE_LIST_PLAIN_HELP_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Invoice related invisible options");
    m_optionsInvis->add_options()
        ADD_OPT_TYPE(ID_NAME, "id id", unsigned int)
        ADD_OPT_TYPE(REGISTRANT_ID_NAME, "registrar id number", unsigned int)
        ADD_OPT_TYPE(ZONE_ID_NAME, "zone id number", unsigned int)
        ADD_OPT_TYPE(INVOICE_TYPE_NAME, "invoice type", unsigned int)
        ADD_OPT_TYPE(INVOICE_VAR_SYMBOL_NAME, "invoice var symbol", std::string)
        ADD_OPT_TYPE(INVOICE_NUMBER_NAME, "invoice number", std::string)
        ADD_OPT_TYPE(INVOICE_CRDATE_FROM_NAME, "invoice create date from", std::string)
        ADD_OPT_TYPE(INVOICE_CRDATE_TO_NAME, "invoice create date to", std::string)
        ADD_OPT_TYPE(INVOICE_TAXDATE_FROM_NAME, "invoice tax date from", std::string)
        ADD_OPT_TYPE(INVOICE_TAXDATE_TO_NAME, "invoice tax date to", std::string)
        ADD_OPT_TYPE(INVOICE_ARCHIVED_NAME, "archived flag", unsigned int)
        ADD_OPT_TYPE(INVOICE_OBJECT_ID_NAME, "object id", Register::TID)
        ADD_OPT_TYPE(INVOICE_OBJECT_NAME_NAME, "object name", std::string)
        ADD_OPT_TYPE(INVOICE_ADV_NUMBER_NAME, "advance number", std::string)
        ADD_OPT(INVOICE_DONT_SEND_NAME, "dont send mails with invoices during archivation")
        add_opt_type(INVOICE_FILE_ID_NAME, unsigned int)
        add_opt_type(INVOICE_FILE_NAME_NAME, std::string);
}
InvoiceClient::InvoiceClient(
        std::string connstring,
        std::string nsAddr,
        boost::program_options::variables_map varMap):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_varMap = varMap;
    m_options = NULL;
    m_optionsInvis = NULL;
}

InvoiceClient::~InvoiceClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
InvoiceClient::init(
        std::string connstring,
        std::string nsAddr,
        boost::program_options::variables_map varMap)
{
    m_connstring = connstring;
    m_nsAddr = nsAddr;
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_varMap = varMap;
}

boost::program_options::options_description *
InvoiceClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
InvoiceClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
InvoiceClient::list()
{
    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_varMap[DOCGEN_PATH_NAME].as<std::string>(),
                m_varMap[DOCGEN_TEMPLATE_PATH_NAME].as<std::string>(),
                m_varMap[FILECLIENT_PATH_NAME].as<std::string>(),
                m_nsAddr)
            );

    CorbaClient cc(0, NULL, m_nsAddr);
    MailerManager mailMan(cc.getNS());

    std::auto_ptr<Register::Invoicing::Manager> invMan(
            Register::Invoicing::Manager::create(
                &m_db,
                docMan.get(),
                &mailMan));

    std::auto_ptr<Register::Invoicing::List> invList(
            invMan->createList());

    Database::Filters::Invoice *invFilter;
    invFilter = new Database::Filters::InvoiceImpl();

    if (m_varMap.count(ID_NAME))
        invFilter->addId().setValue(
                Database::ID(m_varMap[ID_NAME].as<unsigned int>()));

    if (m_varMap.count(ZONE_ID_NAME))
        invFilter->addZoneId().setValue(
                Database::ID(m_varMap[ZONE_ID_NAME].as<unsigned int>()));
    if (m_varMap.count(INVOICE_TYPE_NAME))
        invFilter->addType().setValue(
                m_varMap[INVOICE_TYPE_NAME].as<int>());
    if (m_varMap.count(INVOICE_NUMBER_NAME))
        invFilter->addNumber().setValue(
                m_varMap[INVOICE_NUMBER_NAME].as<std::string>());

    if (m_varMap.count(CRDATE_FROM_NAME) || m_varMap.count(CRDATE_TO_NAME)) {
        Database::DateTime crDateFrom("1901-01-01 00:00:00");
        Database::DateTime crDateTo("2101-01-01 00:00:00");
        if (m_varMap.count(CRDATE_FROM_NAME))
            crDateFrom.from_string(
                    m_varMap[CRDATE_FROM_NAME].as<std::string>());
        if (m_varMap.count(CRDATE_TO_NAME))
            crDateTo.from_string(
                    m_varMap[CRDATE_TO_NAME].as<std::string>());
        invFilter->addCreateTime().setValue(
                Database::DateTimeInterval(crDateFrom, crDateTo));
    }
    if (m_varMap.count(INVOICE_TAXDATE_FROM_NAME) || m_varMap.count(INVOICE_TAXDATE_TO_NAME)) {
        Database::Date taxDateFrom("1901-01-01");
        Database::Date taxDateTo("2101-01-01");
        if (m_varMap.count(INVOICE_TAXDATE_FROM_NAME))
            taxDateFrom.from_string(
                    m_varMap[INVOICE_TAXDATE_FROM_NAME].as<std::string>());
        if (m_varMap.count(INVOICE_TAXDATE_TO_NAME))
            taxDateTo.from_string(
                    m_varMap[INVOICE_TAXDATE_TO_NAME].as<std::string>());
        invFilter->addTaxDate().setValue(
                Database::DateInterval(taxDateFrom, taxDateTo));
    }

    if (m_varMap.count(REGISTRAR_ID_NAME))
        invFilter->addRegistrar().addId().setValue(
                Database::ID(m_varMap[REGISTRAR_ID_NAME].as<unsigned int>()));
    if (m_varMap.count(REGISTRAR_HANDLE_NAME))
        invFilter->addRegistrar().addHandle().setValue(
                m_varMap[REGISTRAR_HANDLE_NAME].as<std::string>());
    if (m_varMap.count(INVOICE_FILE_ID_NAME))
        invFilter->addFile().addId().setValue(
                Database::ID(m_varMap[INVOICE_FILE_ID_NAME].as<unsigned int>()));
    if (m_varMap.count(INVOICE_FILE_NAME_NAME))
        invFilter->addFile().addName().setValue(
                m_varMap[INVOICE_FILE_NAME_NAME].as<std::string>());

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(invFilter);

    invList->setLimit(m_varMap[LIMIT_NAME].as<unsigned int>());
    invList->reload(*unionFilter, m_dbman);

    std::cout << "<objects>\n";
    for (unsigned int i = 0; i < invList->getCount(); i++) {
        Register::Invoicing::Invoice *inv = invList->get(i);
        std::cout
            << "\t<invoice>\n"
            << "\t\t<id>" << inv->getId() << "</id>\n"
            << "\t\t<zone>\n"
            << "\t\t\t<id>" << inv->getZone() << "</id>\n"
            << "\t\t\t<name>" << inv->getZoneName() << "</name>\n"
            << "\t\t</zone>\n"
            << "\t\t<cr_date>" << inv->getCrTime() << "</cr_date>\n"
            << "\t\t<tax_date>" << inv->getTaxDate() << "</tax_date>\n"
            << "\t\t<account_period>" << inv->getAccountPeriod() << "</account_period>\n"
            << "\t\t<type>" << Register::Invoicing::Type2Str(inv->getType()) << "</type>\n"
            << "\t\t<number>" << inv->getNumber() << "</number>\n"
            << "\t\t<registrar_id>" << inv->getRegistrar() << "</registrar_id>\n"
            << "\t\t<credit>" << inv->getCredit() << "</credit>\n"
            << "\t\t<price>" << inv->getPrice() << "</price>\n"
            << "\t\t<vat_rate>" << inv->getVatRate() << "</vat_rate>\n"
            << "\t\t<total>" << inv->getTotal() << "</total>\n"
            << "\t\t<total_vat>" << inv->getTotalVAT() << "</total_vat>\n"
            << "\t\t<var_symbol>" << inv->getVarSymbol() << "</var_symbol>\n"
            << "\t\t<file_pdf_id>" << inv->getFilePDF() << "</file_pdf_id>\n"
            << "\t\t<file_xml_id>" << inv->getFileXML() << "</file_xml_id>\n";
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
                << "\t\t\t<action>"
                << Register::Invoicing::PaymentActionType2Str(act->getAction())
                << "</action>\n"
                << "\t\t\t<units_count>" << act->getUnitsCount() << "</units_count>\n"
                << "\t\t\t<price_per_unit>" << act->getPricePerUnit() << "</price_per_unit>\n"
                << "\t\t</payment_action>\n";
        }
        for (unsigned int j = 0; j < inv->getPaymentCount(); j++) {
            Register::Invoicing::Payment *pay = 
                (Register::Invoicing::Payment *)inv->getPaymentByIdx(j);
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
            << "\t\t\t<phome>" << sup->getPhone() << "</phone>\n"
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
            << "\t\t\t<phome>" << cli->getPhone() << "</phone>\n"
            << "\t\t\t<fax>" << cli->getFax() << "</fax>\n"
            << "\t\t</client>\n";
        std::cout
            << std::endl;
    }
    std::cout << "</objects>" << std::endl;

    unionFilter->clear();
    delete unionFilter;
}

int
InvoiceClient::archive()
{
    std::cout << "invoice archive" << std::endl;
    return 1;
    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_varMap["docgen-path"].as<std::string>(),
                m_varMap["docgen-template-path"].as<std::string>(),
                m_varMap["fileclient-path"].as<std::string>(),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr);
    MailerManager mailMan(cc.getNS());
    std::auto_ptr<Register::Invoicing::Manager> invMan(
            Register::Invoicing::Manager::create(
                &m_db,
                docMan.get(),
                &mailMan)
            );
    invMan->archiveInvoices(!m_varMap.count(INVOICE_DONT_SEND_NAME));
    return 0;
}

void
InvoiceClient::list_help()
{
    std::cout <<
        "** List invoices **\n\n"
        "  $ " << g_prog_name << " --" << INVOICE_LIST_NAME << " \\\n"
        "    [--" << ID_NAME << "=<id_number>] \\\n"
        "    [--" << REGISTRANT_ID_NAME << "=<id_number>] \\\n"
        "    [--" << ZONE_ID_NAME << "=<id_number>] \\\n"
        "    [--" << INVOICE_TYPE_NAME << "=<invoice_type>] \\\n"
        "    [--" << INVOICE_VAR_SYMBOL_NAME << "=<invoice_var_symbol>] \\\n"
        "    [--" << INVOICE_NUMBER_NAME << "=<invoice_number>] \\\n"
        "    [--" << INVOICE_CRDATE_FROM_NAME << "=<invoice_create_date_from>] \\\n"
        "    [--" << INVOICE_CRDATE_TO_NAME << "=<invoice_create_date_to>] \\\n"
        "    [--" << INVOICE_TAXDATE_FROM_NAME << "=<invoice_taxdate_from>] \\\n"
        "    [--" << INVOICE_TAXDATE_TO_NAME << "=<invoice_taxdate_to>] \\\n"
        "    [--" << INVOICE_ARCHIVED_NAME << "=<invoice_archive_flag(0=not archived,1=archived,other=ignore>] \\\n"
        "    [--" << INVOICE_OBJECT_ID_NAME << "=<invoice_object_id_number>] \\\n"
        "    [--" << INVOICE_OBJECT_NAME_NAME << "=<invoice_object_name>] \\\n"
        "    [--" << INVOICE_ADV_NUMBER_NAME << "=<invoice_advance_number>] \n"
        << std::endl;
}

void
InvoiceClient::archive_help()
{
    std::cout <<
        "** Invoice archive **\n\n"
        "  $ " << g_prog_name << " --" << INVOICE_ARCHIVE_NAME << " \\\n"
        "    --" << INVOICE_DONT_SEND_NAME << " \n"
        << std::endl;
}

} // namespace Admin;
