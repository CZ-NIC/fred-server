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
        ADD_OPT(INVOICE_DONT_SEND_NAME, "dont send mails with invoices during archivation");
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
    std::ofstream stdout("/dev/stdout",std::ios::out);   

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
                &mailMan)
            );
    std::auto_ptr<Register::Invoicing::List> invList(invMan->createList());
    if (m_varMap.count(ID_NAME))
        invList->setIdFilter(m_varMap[ID_NAME].as<unsigned int>());
    if (m_varMap.count(REGISTRAR_ID_NAME))
        invList->setRegistrarFilter(m_varMap[REGISTRAR_ID_NAME].as<unsigned int>());
    if (m_varMap.count(ZONE_ID_NAME))
        invList->setZoneFilter(m_varMap[ZONE_ID_NAME].as<unsigned int>());
    if (m_varMap.count(INVOICE_TYPE_NAME))
        invList->setTypeFilter(m_varMap[INVOICE_TYPE_NAME].as<unsigned int>());
    if (m_varMap.count(INVOICE_VAR_SYMBOL_NAME))
        invList->setVarSymbolFilter(m_varMap[INVOICE_VAR_SYMBOL_NAME].as<std::string>());
    if (m_varMap.count(INVOICE_NUMBER_NAME))
        invList->setNumberFilter(m_varMap[INVOICE_NUMBER_NAME].as<std::string>());

    ptime crDateFrom(neg_infin);
    if (m_varMap.count(INVOICE_CRDATE_FROM_NAME))
        crDateFrom = time_from_string(m_varMap[INVOICE_CRDATE_FROM_NAME].as<std::string>());
    ptime crDateTo(pos_infin);
    if (m_varMap.count(INVOICE_CRDATE_TO_NAME))
        crDateTo = time_from_string(m_varMap[INVOICE_CRDATE_TO_NAME].as<std::string>());
    invList->setCrDateFilter(time_period(crDateFrom, crDateTo));

    ptime taxDateFrom(neg_infin);
    if (m_varMap.count(INVOICE_TAXDATE_FROM_NAME))
        taxDateFrom = time_from_string(m_varMap[INVOICE_TAXDATE_FROM_NAME].as<std::string>());
    ptime taxDateTo(pos_infin);
    if (m_varMap.count(INVOICE_TAXDATE_TO_NAME))
        taxDateTo = time_from_string(m_varMap[INVOICE_TAXDATE_TO_NAME].as<std::string>());
    invList->setTaxDateFilter(time_period(taxDateFrom, taxDateTo));
    
    if (m_varMap.count(INVOICE_ARCHIVED_NAME)) {
        Register::Invoicing::List::ArchiveFilter archFilt;
        switch (m_varMap[INVOICE_ARCHIVED_NAME].as<unsigned int>()) {
            case 0:
                archFilt = Register::Invoicing::List::AF_UNSET;
                break;
            case 1:
                archFilt = Register::Invoicing::List::AF_SET;
                break;
            default:
                archFilt = Register::Invoicing::List::AF_IGNORE;
                break;
        }
        invList->setArchivedFilter(archFilt);
    }

    if (m_varMap.count(INVOICE_OBJECT_ID_NAME))
        invList->setObjectIdFilter(m_varMap[INVOICE_OBJECT_ID_NAME].as<Register::TID>());
    if (m_varMap.count(INVOICE_OBJECT_NAME_NAME))
        invList->setObjectNameFilter(m_varMap[INVOICE_OBJECT_NAME_NAME].as<std::string>());
    if (m_varMap.count(INVOICE_ADV_NUMBER_NAME))
        invList->setAdvanceNumberFilter(m_varMap[INVOICE_ADV_NUMBER_NAME].as<std::string>());

    invList->reload();
    invList->exportXML(stdout);
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
