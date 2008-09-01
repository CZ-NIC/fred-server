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
#include "contactclient.h"
#include "register/register.h"

namespace Admin {

#define LOGIN_CONTACTCLIENT \
CorbaClient cc(0, NULL, m_nsAddr.c_str()); \
CORBA::Object_var o = cc.getNS()->resolve("EPP"); \
ccReg::EPP_var epp; \
epp = ccReg::EPP::_narrow(o); \
CORBA::Long clientId = 0; \
ccReg::Response_var r; \
if (!m_db.ExecSelect( \
            "SELECT r.handle,ra.cert,ra.password " \
            "FROM registrar r, registraracl ra " \
            "WHERE r.id=ra.registrarid AND r.system='t' LIMIT 1 ") \
        ) \
    return -1; \
if (!m_db.GetSelectRows()) \
    return -1; \
std::string handle = m_db.GetFieldValue(0,0); \
std::string cert = m_db.GetFieldValue(0,1); \
std::string password = m_db.GetFieldValue(0,2); \
m_db.FreeSelect(); \
r = epp->ClientLogin(handle.c_str(),password.c_str(),"","system_delete_login","<system_delete_login/>", \
        clientId,cert.c_str(),ccReg::EN); \
if (r->code != 1000 || !clientId) { \
    std::cerr << "Cannot connect: " << r->code << std::endl; \
    return -1; \
}

#define LOGOUT_CONTACTCLIENT \
    epp->ClientLogout(clientId,"system_delete_logout","<system_delete_logout/>");

ContactClient::ContactClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Contact related options");
    m_options->add_options()
        ADD_OPT_TYPE(CONTACT_INFO_NAME, "contact info (via epp_impl", std::string)
        ADD_OPT_TYPE(CONTACT_INFO2_NAME, "contact info (via ccReg_i)", std::string)
        ADD_OPT(CONTACT_LIST_NAME, "list of all contacts (via filters)")
        ADD_OPT(CONTACT_LIST_PLAIN_NAME, "list of all contacts (via epp_impl)");

    m_optionsInvis = new boost::program_options::options_description(
            "Contact related invisible options");
    m_optionsInvis->add_options();
}

ContactClient::ContactClient(
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

ContactClient::~ContactClient()
{
}

void
ContactClient::init(
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
ContactClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
ContactClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
ContactClient::info2()
{
    return 0;
}

int
ContactClient::info()
{
    LOGIN_CONTACTCLIENT;

    std::string name = m_varMap[CONTACT_INFO_NAME].as<std::string>();
    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + name + "</fqdn>";
    cltrid = "info_contact";

    ccReg::Contact *c = new ccReg::Contact;
    
    epp->ContactInfo(name.c_str(), c, clientId, cltrid.c_str(), xml.c_str());

    std::cout << c->Name << std::endl;

    LOGOUT_CONTACTCLIENT;
    return 0;
}

int
ContactClient::list()
{
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(&m_db, true));
    std::auto_ptr<Register::Contact::List> conList(
            conMan->createList());

    Database::Filters::Contact *conFilter;
    conFilter = new Database::Filters::ContactHistoryImpl();

    if (m_varMap.count(ID_NAME))
        conFilter->addId().setValue(
                Database::ID(m_varMap[ID_NAME].as<unsigned int>()));
    if (m_varMap.count(HANDLE_NAME))
        conFilter->addHandle().setValue(
                m_varMap[HANDLE_NAME].as<std::string>());
    if (m_varMap.count(NAME_NAME))
        conFilter->addName().setValue(
                m_varMap[NAME_NAME].as<std::string>());
    if (m_varMap.count(ORGANIZATION_NAME))
        conFilter->addOrganization().setValue(
                m_varMap[ORGANIZATION_NAME].as<std::string>());
    if (m_varMap.count(CITY_NAME))
        conFilter->addCity().setValue(
                m_varMap[CITY_NAME].as<std::string>());
    if (m_varMap.count(EMAIL_NAME))
        conFilter->addEmail().setValue(
                m_varMap[EMAIL_NAME].as<std::string>());
    if (m_varMap.count(NOTIFY_EMAIL_NAME))
        conFilter->addNotifyEmail().setValue(
                m_varMap[NOTIFY_EMAIL_NAME].as<std::string>());
    if (m_varMap.count(VAT_NAME))
        conFilter->addVat().setValue(
                m_varMap[VAT_NAME].as<std::string>());
    if (m_varMap.count(SSN_NAME))
        conFilter->addSsn().setValue(
                m_varMap[SSN_NAME].as<std::string>());

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(conFilter);
    conList->setLimit(m_varMap[LIMIT_NAME].as<unsigned int>());

    conList->reload(*unionFilter, m_dbman);

    std::cout << "<objects>" << std::endl;
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
        if (m_varMap.count(FULL_LIST_NAME)) {
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
    return 0;
}

} // namespace Admin;
