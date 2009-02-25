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
#include "registrarclient.h"

namespace Admin {

const struct options *
RegistrarClient::getOpts()
{
    return m_opts;
}

void
RegistrarClient::runMethod()
{
    if (m_conf.hasOpt(REGISTRAR_ZONE_ADD_NAME)) {
        zone_add();
    } else if (m_conf.hasOpt(REGISTRAR_REGISTRAR_ADD_NAME)) {
        registrar_add();
    } else if (m_conf.hasOpt(REGISTRAR_REGISTRAR_ADD_ZONE_NAME)) {
        registrar_add_zone();
    } else if (m_conf.hasOpt(REGISTRAR_LIST_NAME)) {
        list();
    } else if (m_conf.hasOpt(REGISTRAR_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
RegistrarClient::show_opts() 
{
    callHelp(m_conf, no_help);
    print_options("Registrar", getOpts(), getOptsCount());
}

#define reg(i)  regMan->getList()->get(i)

void
RegistrarClient::list()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));

    Database::Filters::Registrar *regFilter;
    regFilter = new Database::Filters::RegistrarImpl(true);

    if (m_conf.hasOpt(ID_NAME))
        regFilter->addId().setValue(
                Database::ID(m_conf.get<unsigned int>(ID_NAME)));
    if (m_conf.hasOpt(HANDLE_NAME))
        regFilter->addHandle().setValue(
                m_conf.get<std::string>(HANDLE_NAME));
    if (m_conf.hasOpt(NAME_NAME))
        regFilter->addOrganization().setValue(
                m_conf.get<std::string>(ORGANIZATION_NAME));
    if (m_conf.hasOpt(CITY_NAME))
        regFilter->addCity().setValue(
                m_conf.get<std::string>(CITY_NAME));
    if (m_conf.hasOpt(EMAIL_NAME))
        regFilter->addEmail().setValue(
                m_conf.get<std::string>(EMAIL_NAME));
    if (m_conf.hasOpt(COUNTRY_NAME))
        regFilter->addCountry().setValue(
                m_conf.get<std::string>(COUNTRY_NAME));

    // apply_ID(regFilter);
    // apply_HANDLE(regFilter);
    // apply_NAME(regFilter);
    // apply_CITY(regFilter);
    // apply_EMAIL(regFilter);
    // apply_COUNTRY(regFilter);

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(regFilter);
    //regMan->getList()->setLimit(m_conf.get<unsigned int>(LIMIT_NAME));

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

void
RegistrarClient::zone_add()
{
    callHelp(m_conf, zone_add_help);
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::string fqdn = m_conf.get<std::string>(REGISTRAR_ZONE_FQDN_NAME);
    try {
        zoneMan->addZone(fqdn);
    } catch (Register::ALREADY_EXISTS) {
        std::cerr << "Zone '" << fqdn << "' already exists in configuratin" << std::endl;
    }
    return;
}
void
RegistrarClient::registrar_add()
{
    callHelp(m_conf, registrar_add_help);
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    regMan->addRegistrar(m_conf.get<std::string>(REGISTRAR_HANDLE_NAME));
    return;
}
void
RegistrarClient::registrar_add_zone()
{
    callHelp(m_conf, registrar_add_zone_help);
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    std::string zone = m_conf.get<std::string>(REGISTRAR_ZONE_FQDN_NAME);
    std::string registrar = m_conf.get<std::string>(REGISTRAR_HANDLE_NAME);
    regMan->addRegistrarZone(registrar, zone);
    return;
}

void
RegistrarClient::zone_add_help()
{
    std::cout <<
        "** Add new zone **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_ZONE_ADD_NAME
               << " --" << REGISTRAR_ZONE_FQDN_NAME << "=<zone_fqdn>\n"
        << std::endl;
}

void
RegistrarClient::registrar_add_help()
{
    std::cout <<
        "** Add new registrar **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_REGISTRAR_ADD_NAME
               << " --" << REGISTRAR_HANDLE_NAME << "=<registrar_handle>\n"
        << std::endl;
}
void
RegistrarClient::registrar_add_zone_help()
{
    std::cout <<
        "** Add registrar to zone **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_REGISTRAR_ADD_ZONE_NAME << " \\\n"
        "    --" << REGISTRAR_ZONE_FQDN_NAME << "=<zone_fqdn> \\\n"
        "    --" << REGISTRAR_HANDLE_NAME << "=<registrar_handle>\n"
        << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_REGISTRAR, name, name##_DESC, type, callable, visible}

const struct options
RegistrarClient::m_opts[] = {
    ADDOPT(REGISTRAR_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_ZONE_ADD_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_REGISTRAR_ADD_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_REGISTRAR_ADD_ZONE_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    add_ID,
    add_HANDLE,
    add_NAME,
    add_CITY,
    add_EMAIL,
    add_COUNTRY,
    ADDOPT(REGISTRAR_ZONE_FQDN_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_HANDLE_NAME, TYPE_STRING, false, false),
};

#undef ADDOPT

int 
RegistrarClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

