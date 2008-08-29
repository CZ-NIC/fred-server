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

#include "register/register.h"
#include "common.h"
#include "contact.h"

namespace Admin {

#define LOGIN_CONTACT \
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

#define LOGOUT_CONTACT \
    epp->ClientLogout(clientId,"system_delete_logout","<system_delete_logout/>");

Contact::Contact():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Contact related options");
    m_options->add_options()
        ADD_OPT_TYPE(CONTACT_INFO_NAME, "keyset info (via epp_impl", std::string)
        ADD_OPT_TYPE(CONTACT_INFO2_NAME, "keyset info (via ccReg_i)", std::string)
        ADD_OPT(CONTACT_LIST_NAME, "list of all contacts (via filters)")
        ADD_OPT(CONTACT_LIST_PLAIN_NAME, "list of all contacts (via epp_impl)");

    m_optionsInvis = new boost::program_options::options_description(
            "Contact related invisible options");
    m_optionsInvis->add_options();
}

Contact::Contact(
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

Contact::~Contact()
{
}

void
Contact::init(
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
Contact::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
Contact::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
Contact::info2()
{
    return 0;
}

int
Contact::info()
{
    LOGIN_CONTACT;

    std::string name = m_varMap[CONTACT_INFO_NAME].as<std::string>();
    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + name + "</fqdn>";
    cltrid = "info_contact";

    ccReg::Contact *c = new ccReg::Contact;
    
    epp->ContactInfo(name.c_str(), c, clientId, cltrid.c_str(), xml.c_str());

    std::cout << c->Name << std::endl;

    LOGOUT_CONTACT;
    return 0;
}

int
Contact::list()
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

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(conFilter);

    conList->setLimit(m_varMap[LIMIT_NAME].as<unsigned int>());
    conList->reload(*unionFilter, m_dbman);
    std::cout << "<objects>" << std::endl;
    for (unsigned int i = 0; i < conList->getCount(); i++) {
        std::cout
            << "\t<contact>\n"
            << "\t\t<id>" << conList->getContact(i)->getId() << "</id>\n"
            << "\t\t<handle>" << conList->getContact(i)->getHandle() << "</handle>\n"
            << "\t\t<name>" << conList->getContact(i)->getName() << "</name>\n"
            << "\t\t<street1>" << conList->getContact(i)->getStreet1() << "</street1>\n"
            << "\t\t<street2>" << conList->getContact(i)->getStreet2() << "</street2>\n"
            << "\t\t<street3>" << conList->getContact(i)->getStreet3() << "</street3>\n"
            << "\t\t<city>" << conList->getContact(i)->getCity() << "</city>\n"
            << "\t\t<postal_code>" << conList->getContact(i)->getPostalCode() << "</postal_code>\n"
            << "\t\t<province>" << conList->getContact(i)->getProvince() << "</province>\n"
            << "\t\t<country>" << conList->getContact(i)->getCountry() << "</country>\n"
            << "\t\t<telephone>" << conList->getContact(i)->getTelephone() << "</telephone>\n"
            << "\t\t<fax>" << conList->getContact(i)->getFax() << "</fax>\n"
            << "\t\t<email>" << conList->getContact(i)->getEmail() << "</email>\n"
            << "\t\t<notify_email>" << conList->getContact(i)->getNotifyEmail() << "</notify_email>\n"
            << "\t</contact>\n";
    }
    std::cout << "</object>" << std::endl;
    unionFilter->clear();
    return 0;
}

} // namespace Admin;
