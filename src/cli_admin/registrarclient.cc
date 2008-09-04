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
#include "registrarclient.h"

namespace Admin {

RegistrarClient::RegistrarClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Registrar related options");
    m_options->add_options()
        add_opt(REGISTRAR_LIST_NAME)
        ADD_OPT_TYPE(REGISTRAR_ZONE_ADD_NAME, "Add new zone", std::string)
        ADD_OPT_TYPE(REGISTRAR_REGISTRAR_ADD_NAME, "Add new registrar (make a copy of REG-FRED_A)", std::string)
        ADD_OPT(REGISTRAR_REGISTRAR_ADD_ZONE_NAME, "Add access rights right for registrar to zone")
        ADD_OPT(REGISTRAR_ZONE_ADD_HELP_NAME, "")
        ADD_OPT(REGISTRAR_REGISTRAR_ADD_HELP_NAME, "")
        ADD_OPT(REGISTRAR_REGISTRAR_ADD_ZONE_HELP_NAME, "");

    m_optionsInvis = new boost::program_options::options_description(
            "Registrar related invisible options");
    m_optionsInvis->add_options()
        ADD_OPT_TYPE(ZONE_HANDLE_NAME, "Zone handle", std::string)
        ADD_OPT_TYPE(REGISTRAR_HANDLE_NAME, "Registrar handle", std::string);
}
RegistrarClient::RegistrarClient(
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

RegistrarClient::~RegistrarClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
RegistrarClient::init(
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
RegistrarClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
RegistrarClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}
#define reg(i)  regMan->getList()->get(i)

void
RegistrarClient::list()
{
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));

    Database::Filters::Registrar *regFilter;
    regFilter = new Database::Filters::RegistrarImpl();

    if (m_varMap.count(ID_NAME))
        regFilter->addId().setValue(
                Database::ID(m_varMap[ID_NAME].as<unsigned int>()));
    if (m_varMap.count(HANDLE_NAME))
        regFilter->addHandle().setValue(
                m_varMap[HANDLE_NAME].as<std::string>());
    if (m_varMap.count(NAME_NAME))
        regFilter->addOrganization().setValue(
                m_varMap[ORGANIZATION_NAME].as<std::string>());
    if (m_varMap.count(CITY_NAME))
        regFilter->addCity().setValue(
                m_varMap[CITY_NAME].as<std::string>());
    if (m_varMap.count(EMAIL_NAME))
        regFilter->addEmail().setValue(
                m_varMap[EMAIL_NAME].as<std::string>());
    if (m_varMap.count(COUNTRY_NAME))
        regFilter->addCountry().setValue(
                m_varMap[COUNTRY_NAME].as<std::string>());

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(regFilter);
    regMan->getList()->setLimit(m_varMap[LIMIT_NAME].as<unsigned int>());

    regMan->getList()->reload(*unionFilter, m_dbman);

    std::cout << "<object>\n";
    for (unsigned int i = 0; i < regMan->getList()->getCount(); i++) {
        std::cout
            << "\t<registrar>\n"
            << "\t\t<id>" << regMan->getList()->get(i)->getId() << "</id>\n"
            << "\t\t<handle>" << reg(i)->getHandle() << "</handle>\n"
            << "\t\t<name>" << reg(i)->getName() << "</name>\n"
            << "\t\t<url>" << reg(i)->getURL() << "</url>\n"
            << "\t\t<organization>" << reg(i)->getOrganization() << "</organization>\n"
            << "\t\t<street1>" << reg(i)->getStreet1() << "</street1>\n"
            << "\t\t<street2>" << reg(i)->getStreet2() << "</street2>\n"
            << "\t\t<street3>" << reg(i)->getStreet3() << "</street3>\n"
            << "\t\t<city>" << reg(i)->getCity() << "</city>\n"
            << "\t\t<province>" << reg(i)->getProvince() << "</province>\n"
            << "\t\t<postal_code>" << reg(i)->getPostalCode() << "</postal_code>\n"
            << "\t\t<country>" << reg(i)->getCountry() << "</country>\n"
            << "\t\t<telephone>" << reg(i)->getTelephone() << "</telephone>\n"
            << "\t\t<fax>" << reg(i)->getFax() << "</fax>\n"
            << "\t\t<email>" << reg(i)->getEmail() << "</email>\n"
            << "\t\t<system>" << reg(i)->getSystem() << "</system>\n"
            << "\t\t<credit>" << reg(i)->getCredit() << "</credit>\n";
        for (unsigned int j = 0; j < reg(i)->getACLSize(); j++) {
            std::cout
                << "\t\t<ACL>"
                << "\t\t\t<cert_md5>" << reg(i)->getACL(j)->getCertificateMD5() << "</cert_md5>\n"
                << "\t\t\t<pass>" << reg(i)->getACL(j)->getPassword() << "</pass>\n"
                << "\t\t</ACL>\n";
        }
        std::cout
            << std::endl;
    }
    std::cout << "</object>" << std::endl;

    unionFilter->clear();
    delete unionFilter;
}

int
RegistrarClient::zone_add()
{
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::string fqdn = m_varMap[REGISTRAR_ZONE_ADD_NAME].as<std::string>();
    try {
        zoneMan->addZone(fqdn);
    } catch (Register::ALREADY_EXISTS) {
        std::cerr << "Zone '" << fqdn << "' already exists in configuratin" << std::endl;
    }
    return 0;
}
int
RegistrarClient::registrar_add()
{
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    regMan->addRegistrar(m_varMap[REGISTRAR_REGISTRAR_ADD_NAME].as<std::string>());
    return 0;
}
int
RegistrarClient::registrar_add_zone()
{
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    std::string zone = m_varMap[ZONE_HANDLE_NAME].as<std::string>();
    std::string registrar = m_varMap[REGISTRAR_HANDLE_NAME].as<std::string>();
    regMan->addRegistrarZone(registrar, zone);
    return 0;
}

void
RegistrarClient::zone_add_help()
{
    std::cout <<
        "** Add new zone **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_ZONE_ADD_NAME << "=<zone_fqdn>\n"
        << std::endl;
}

void 
RegistrarClient::registrar_add_help()
{
    std::cout <<
        "** Add new registrar **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_REGISTRAR_ADD_NAME << "=<registrar_handle>\n"
        << std::endl;
}
void 
RegistrarClient::registrar_add_zone_help()
{
    std::cout <<
        "** Add registrar to zone **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_REGISTRAR_ADD_ZONE_NAME << " \\\n"
        "    --" << ZONE_HANDLE_NAME << "=<zone_fqdn> \\\n"
        "    --" << REGISTRAR_HANDLE_NAME << "=<registrar_handle>\n"
        << std::endl;
}

} // namespace Admin;

