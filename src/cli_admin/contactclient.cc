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
#include "contactclient.h"
#include "register/register.h"

namespace Admin {

const struct options *
ContactClient::getOpts()
{
    return m_opts;
}

void
ContactClient::runMethod()
{
    if (m_conf.hasOpt(CONTACT_INFO_NAME)) {
        info();
    } else if (m_conf.hasOpt(CONTACT_LIST_NAME)) {
        list();
    } else if (m_conf.hasOpt(CONTACT_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
ContactClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("Contact", getOpts(), getOptsCount());
}

void
ContactClient::info()
{
    callHelp(m_conf, no_help);
    CLIENT_LOGIN;

    std::string name = m_conf.get<std::string>(CONTACT_INFO_NAME);
    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + name + "</fqdn>";
    cltrid = "info_contact";

    ccReg::Contact *c = new ccReg::Contact;
    
    epp->ContactInfo(name.c_str(), c, clientId, cltrid.c_str(), xml.c_str());

    std::cout << c->Name << std::endl;

    CLIENT_LOGOUT;

    return;
}

void
ContactClient::list()
{
    callHelp(m_conf, list_help);
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(&m_db, true));
    std::auto_ptr<Register::Contact::List> conList(
            conMan->createList());

    Database::Filters::Contact *conFilter;
    conFilter = new Database::Filters::ContactHistoryImpl();

    apply_ID(conFilter);
    apply_HANDLE(conFilter);
    apply_NAME(conFilter);
    apply_ORGANIZATION(conFilter);
    apply_CITY(conFilter);
    apply_EMAIL(conFilter);
    apply_NOTIFY_EMAIL(conFilter);
    apply_VAT(conFilter);
    apply_SSN(conFilter);
    apply_REGISTRAR_ID(conFilter);
    apply_REGISTRAR_HANDLE(conFilter);
    apply_REGISTRAR_NAME(conFilter);
    apply_CRDATE(conFilter);
    apply_DELDATE(conFilter);
    apply_UPDATE(conFilter);
    apply_TRANSDATE(conFilter);

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(conFilter);
    apply_LIMIT(conList);

    conList->reload(*unionFilter);

    std::cout << "<objects>\n";
    for (unsigned int i = 0; i < conList->getCount(); i++) {
        Register::Contact::Contact *contact = conList->getContact(i);
        std::cout
            << "\t<contact>\n"
            << "\t\t<id>" << contact->getId() << "</id>\n"
            << "\t\t<handle>" << contact->getHandle() << "</handle>\n"
            << "\t\t<name>" << contact->getName() << "</name>\n"
            << "\t\t<street1>" << contact->getStreet1() << "</street1>\n"
            << "\t\t<street2>" << contact->getStreet2() << "</street2>\n"
            << "\t\t<street3>" << contact->getStreet3() << "</street3>\n"
            << "\t\t<province>" << contact->getProvince() << "</province>\n"
            << "\t\t<postal_code>" << contact->getPostalCode() << "</postal_code>\n"
            << "\t\t<city>" << contact->getCity() << "</city>\n"
            << "\t\t<province>" << contact->getProvince() << "</province>\n"
            << "\t\t<country>" << contact->getCountry() << "</country>\n"
            << "\t\t<telephone>" << contact->getTelephone() << "</telephone>\n"
            << "\t\t<fax>" << contact->getFax() << "</fax>\n"
            << "\t\t<email>" << contact->getEmail() << "</email>\n"
            << "\t\t<notify_email>" << contact->getNotifyEmail() << "</notify_email>\n"
            << "\t\t<ssn>" << contact->getSSN() << "</ssn>\n"
            << "\t\t<ssn_type>" << contact->getSSNType() << "</ssn_type>\n"
            << "\t\t<ssn_type_id>" << contact->getSSNTypeId() << "</ssn_type_id>\n"
            << "\t\t<vat>" << contact->getVAT() << "</vat>\n"
            << "\t\t<disclose_name>" << contact->getDiscloseName() << "</disclose_name>\n"
            << "\t\t<disclose_organization>" << contact->getDiscloseOrganization() << "</disclose_organization>\n"
            << "\t\t<disclose_addr>" << contact->getDiscloseAddr() << "</disclose_addr>\n"
            << "\t\t<disclose_email>" << contact->getDiscloseEmail() << "</disclose_email>\n"
            << "\t\t<disclose_telephone>" << contact->getDiscloseTelephone() << "</disclose_telephone>\n"
            << "\t\t<disclose_fax>" << contact->getDiscloseFax() << "</disclose_fax>\n"
            << "\t\t<disclose_vat>" << contact->getDiscloseVat() << "</disclose_vat>\n"
            << "\t\t<disclose_ident>" << contact->getDiscloseIdent() << "</disclose_ident>\n"
            << "\t\t<disclose_notify_email>" << contact->getDiscloseNotifyEmail() << "</disclose_notify_email>\n";
        if (m_conf.hasOpt(FULL_LIST_NAME)) {
            std::cout
                << "\t\t<create_date>" << contact->getCreateDate() << "</create_date>\n"
                << "\t\t<transfer_date>" << contact->getTransferDate() << "</transfer_date>\n"
                << "\t\t<update_date>" << contact->getUpdateDate() << "</update_date>\n"
                << "\t\t<delete_date>" << contact->getDeleteDate() << "</delete_date>\n"
                << "\t\t<registrar>\n"
                << "\t\t\t<id>" << contact->getRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << contact->getRegistrarHandle() << "</handle>\n"
                << "\t\t</registrar>\n"
                << "\t\t<create_registrar>\n"
                << "\t\t\t<id>" << contact->getCreateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << contact->getCreateRegistrarHandle() << "</handle>\n"
                << "\t\t</create_registrar>\n"
                << "\t\t<update_registrar>\n"
                << "\t\t\t<id>" << contact->getUpdateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << contact->getUpdateRegistrarHandle() << "</handle>\n"
                << "\t\t</update_registrar>\n"
                << "\t\t<auth_password>" << contact->getAuthPw() << "</auth_password>\n"
                << "\t\t<ROID>" << contact->getROID() << "</ROID>\n";
            for (unsigned int j = 0; j < contact->getStatusCount(); j++) {
                Register::Status *status = (Register::Status *)contact->getStatusByIdx(j);
                std::cout
                    << "\t\t<status>\n"
                    << "\t\t\t<id>" << status->getStatusId() << "</id>\n"
                    << "\t\t\t<from>" << status->getFrom() << "</from>\n"
                    << "\t\t\t<to>" << status->getTo() << "</to>\n"
                    << "\t\t</status>\n";
            }
        }
        std::cout
            << "\t</contact>\n";
    }
    std::cout << "</object>" << std::endl;
    unionFilter->clear();
    delete unionFilter;
}

void
ContactClient::list_help()
{
    std::cout
        << "** Contact list **\n\n"
        << "  $ " << g_prog_name << " --" << CONTACT_LIST_NAME << " \\\n"
        << "    [--" << ID_NAME << "=<id_nubmer>] \\\n"
        << "    [--" << HANDLE_NAME << "=<handle>] \\\n"
        << "    [--" << NAME_NAME << "=<name>] \\\n"
        << "    [--" << ORGANIZATION_NAME << "=<organization>] \\\n"
        << "    [--" << CITY_NAME << "=<city>] \\\n"
        << "    [--" << EMAIL_NAME << "=<email>] \\\n"
        << "    [--" << NOTIFY_EMAIL_NAME << "=<email>] \\\n"
        << "    [--" << VAT_NAME << "=<vat>] \\\n"
        << "    [--" << SSN_NAME << "=<ssn>] \\\n"
        << "    [--" << REGISTRAR_ID_NAME << "=<registrar_id_number>] \\\n"
        << "    [--" << REGISTRAR_HANDLE_NAME << "=<registrar_handle>] \\\n"
        << "    [--" << REGISTRAR_NAME_NAME << "=<registrar_name>] \\\n"
        << "    [--" << CRDATE_NAME << "=<create_date>] \\\n"
        << "    [--" << DELDATE_NAME << "=<delete_date>] \\\n"
        << "    [--" << UPDATE_NAME << "=<update_date>] \\\n"
        << "    [--" << TRANSDATE_NAME << "=<transfer_date>] \\\n"
        << "    [--" << FULL_LIST_NAME << "]\n"
        << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_CONTACT, name, name##_DESC, type, callable, visible}

const struct options
ContactClient::m_opts[] = {
    ADDOPT(CONTACT_INFO_NAME, TYPE_STRING, true, true),
    ADDOPT(CONTACT_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(CONTACT_LIST_PLAIN_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(CONTACT_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    add_ID,
    add_HANDLE,
    add_NAME,
    add_ORGANIZATION,
    add_CITY,
    add_EMAIL,
    add_NOTIFY_EMAIL,
    add_VAT,
    add_SSN,
    add_REGISTRAR_ID,
    add_REGISTRAR_HANDLE,
    add_REGISTRAR_NAME,
    add_CRDATE,
    add_UPDATE,
    add_TRANSDATE,
    add_DELDATE,
};

#undef ADDOPT

int 
ContactClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;
