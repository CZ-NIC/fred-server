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

#include <string>
#include <vector>

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

#define SET_IF_PRESENT(setter, name)                        \
    if (m_conf.hasOpt(name)) {                              \
        registrar->setter(m_conf.get<std::string>(name));   \
    }
void
RegistrarClient::registrar_add()
{
    callHelp(m_conf, registrar_add_help);
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    std::auto_ptr<Register::Registrar::Registrar> registrar(
            regMan->createRegistrar());
    registrar->setHandle(m_conf.get<std::string>(REGISTRAR_ADD_HANDLE_NAME));
    registrar->setCountry(m_conf.get<std::string>(REGISTRAR_COUNTRY_NAME));
    SET_IF_PRESENT(setIco, REGISTRAR_ICO_NAME);
    SET_IF_PRESENT(setDic, REGISTRAR_DIC_NAME);
    SET_IF_PRESENT(setVarSymb, REGISTRAR_VAR_SYMB_NAME);
    SET_IF_PRESENT(setName, REGISTRAR_ADD_NAME_NAME);
    SET_IF_PRESENT(setOrganization, REGISTRAR_ORGANIZATION_NAME);
    SET_IF_PRESENT(setStreet1, REGISTRAR_STREET1_NAME);
    SET_IF_PRESENT(setStreet2, REGISTRAR_STREET2_NAME);
    SET_IF_PRESENT(setStreet3, REGISTRAR_STREET3_NAME);
    SET_IF_PRESENT(setCity, REGISTRAR_CITY_NAME);
    SET_IF_PRESENT(setProvince, REGISTRAR_STATEORPROVINCE_NAME);
    SET_IF_PRESENT(setPostalCode, REGISTRAR_POSTALCODE_NAME);
    SET_IF_PRESENT(setTelephone, REGISTRAR_TELEPHONE_NAME);
    SET_IF_PRESENT(setFax, REGISTRAR_FAX_NAME);
    SET_IF_PRESENT(setEmail, REGISTRAR_EMAIL_NAME);
    SET_IF_PRESENT(setURL, REGISTRAR_URL_NAME);
    if (m_conf.hasOpt(REGISTRAR_SYSTEM_NAME)) {
        registrar->setSystem(true);
    } else {
        registrar->setSystem(false);
    }
    if (m_conf.hasOpt(REGISTRAR_NO_VAT_NAME)) {
        registrar->setVat(false);
    } else {
        registrar->setVat(true);
    }
    std::vector<std::string> certs = separate(
            m_conf.get<std::string>(REGISTRAR_CERT_NAME), ',');
    std::vector<std::string> passwords = separate(
            m_conf.get<std::string>(REGISTRAR_PASSWORD_NAME), ',');
    if (certs.size() != passwords.size()) {
        std::cerr << "passwords and certificates number is not same" << std::endl;
        return;
    }
    for (int i = 0; i < (int)certs.size(); i++) {
        Register::Registrar::ACL *acl = registrar->newACL();
        acl->setCertificateMD5(certs[i]);
        acl->setPassword(passwords[i]);
    }
    registrar->save();
}

#undef SET_IF_PRESENT

void
RegistrarClient::registrar_add_zone()
{
    callHelp(m_conf, registrar_add_zone_help);
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    std::string zone = m_conf.get<std::string>(REGISTRAR_ZONE_FQDN_NAME);
    std::string registrar = m_conf.get<std::string>(REGISTRAR_ADD_HANDLE_NAME);
    Database::Date fromDate;
    Database::Date toDate;
    if (m_conf.hasOpt(REGISTRAR_FROM_DATE_NAME)) {
        fromDate.from_string(m_conf.get<std::string>(REGISTRAR_FROM_DATE_NAME));
    }
    if (m_conf.hasOpt(REGISTRAR_TO_DATE_NAME)) {
        toDate.from_string(m_conf.get<std::string>(REGISTRAR_TO_DATE_NAME));
    }
    regMan->addRegistrarZone(registrar, zone, fromDate, toDate);
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
        "  $ " << g_prog_name << " --" << REGISTRAR_REGISTRAR_ADD_NAME << " \\\n"
        "    --" << REGISTRAR_ADD_HANDLE_NAME << "=<registrar_handle> \\\n"
        "    --" << REGISTRAR_COUNTRY_NAME << "=<country_code> \\\n"
        "    --" << REGISTRAR_CERT_NAME << "=<certificate>[,<cetificate_2>, ...] \\\n"
        "    --" << REGISTRAR_PASSWORD_NAME << "=<password>[,<password_2>, ...] \\\n"
        "    [--" << REGISTRAR_ICO_NAME << "=<ico>] \\\n"
        "    [--" << REGISTRAR_DIC_NAME << "=<dic>] \\\n"
        "    [--" << REGISTRAR_VAR_SYMB_NAME << "=<var_symbol>] \\\n"
        "    [--" << REGISTRAR_ADD_NAME_NAME << "=<name>] \\\n"
        "    [--" << REGISTRAR_ORGANIZATION_NAME << "=<organizatin>] \\\n"
        "    [--" << REGISTRAR_STREET1_NAME << "=<street1>] \\\n"
        "    [--" << REGISTRAR_STREET2_NAME << "=<street2>] \\\n"
        "    [--" << REGISTRAR_STREET3_NAME << "=<street3>] \\\n"
        "    [--" << REGISTRAR_CITY_NAME << "=<city>] \\\n"
        "    [--" << REGISTRAR_STATEORPROVINCE_NAME << "=<state_or_province>] \\\n"
        "    [--" << REGISTRAR_POSTALCODE_NAME << "=<postal_code>] \\\n"
        "    [--" << REGISTRAR_TELEPHONE_NAME << "=<telephone>] \\\n"
        "    [--" << REGISTRAR_FAX_NAME << "=<fax>] \\\n"
        "    [--" << REGISTRAR_EMAIL_NAME << "=<email>] \\\n"
        "    [--" << REGISTRAR_URL_NAME << "=<url>] \\\n"
        "    [--" << REGISTRAR_NO_VAT_NAME << "] \\\n"
        "    [--" << REGISTRAR_SYSTEM_NAME << "]\n"
        << std::endl;
}
void
RegistrarClient::registrar_add_zone_help()
{
    std::cout <<
        "** Add registrar to zone **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_REGISTRAR_ADD_ZONE_NAME << " \\\n"
        "    --" << REGISTRAR_ZONE_FQDN_NAME << "=<zone_fqdn> \\\n"
        "    --" << REGISTRAR_ADD_HANDLE_NAME << "=<registrar_handle> \\\n"
        "    [--" << REGISTRAR_FROM_DATE_NAME << "=<valid_from_date>] \\\n"
        "    [--" << REGISTRAR_TO_DATE_NAME << "=<valid_to_date>]\n"
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
    ADDOPT(REGISTRAR_ADD_HANDLE_NAME, TYPE_STRING, false, false),



    ADDOPT(REGISTRAR_ICO_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_DIC_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_VAR_SYMB_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_ADD_NAME_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_ORGANIZATION_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_STREET1_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_STREET2_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_STREET3_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_CITY_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_STATEORPROVINCE_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_POSTALCODE_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_COUNTRY_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_TELEPHONE_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_FAX_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_EMAIL_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_URL_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_CERT_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_PASSWORD_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_NO_VAT_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(REGISTRAR_SYSTEM_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(REGISTRAR_FROM_DATE_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_TO_DATE_NAME, TYPE_STRING, false, false)
};

#undef ADDOPT

int 
RegistrarClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

