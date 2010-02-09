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
    } else if (m_conf.hasOpt(REGISTRAR_ZONE_NS_ADD_NAME)) {
        zone_ns_add();
    } else if (m_conf.hasOpt(REGISTRAR_REGISTRAR_ACL_ADD_NAME)) {
        registrar_acl_add();
    } else if (m_conf.hasOpt(REGISTRAR_PRICE_ADD_NAME)) {
        price_add();
    }
}

void
RegistrarClient::show_opts() 
{
    callHelp(m_conf, no_help);
    print_options("Registrar", getOpts(), getOptsCount());
}

void
RegistrarClient::list()
{
    callHelp(m_conf, no_help);
    Register::Registrar::Manager::AutoPtr regMan
             = Register::Registrar::Manager::create(&m_db);

    Register::Registrar
        ::RegistrarList::AutoPtr reg_list(regMan->createList());

    std::auto_ptr<Database::Filters::Registrar>
        regFilter ( new Database::Filters::RegistrarImpl(true));

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
        regFilter->addCountryCode().setValue(
                m_conf.get<std::string>(COUNTRY_NAME));

    Database::Filters::UnionPtr unionFilter
        = Database::Filters::CreateClearedUnionPtr();
    unionFilter->addFilter(regFilter.release());

    reg_list->reload(*unionFilter.get());

    std::cout << "<object>\n";
    for (unsigned int i = 0; i < reg_list->getSize(); i++) {
        std::cout
            << "\t<registrar>\n"
            << "\t\t<id>" << reg_list->get(i)->getId() << "</id>\n"
            << "\t\t<handle>" << reg_list->get(i)->getHandle() << "</handle>\n"
            << "\t\t<name>" << reg_list->get(i)->getName() << "</name>\n"
            << "\t\t<url>" << reg_list->get(i)->getURL() << "</url>\n"
            << "\t\t<organization>" << reg_list->get(i)->getOrganization() << "</organization>\n"
            << "\t\t<street1>" << reg_list->get(i)->getStreet1() << "</street1>\n"
            << "\t\t<street2>" << reg_list->get(i)->getStreet2() << "</street2>\n"
            << "\t\t<street3>" << reg_list->get(i)->getStreet3() << "</street3>\n"
            << "\t\t<city>" << reg_list->get(i)->getCity() << "</city>\n"
            << "\t\t<province>" << reg_list->get(i)->getProvince() << "</province>\n"
            << "\t\t<postal_code>" << reg_list->get(i)->getPostalCode() << "</postal_code>\n"
            << "\t\t<country>" << reg_list->get(i)->getCountry() << "</country>\n"
            << "\t\t<telephone>" << reg_list->get(i)->getTelephone() << "</telephone>\n"
            << "\t\t<fax>" << reg_list->get(i)->getFax() << "</fax>\n"
            << "\t\t<email>" << reg_list->get(i)->getEmail() << "</email>\n"
            << "\t\t<system>" << reg_list->get(i)->getSystem() << "</system>\n"
            << "\t\t<credit>" << reg_list->get(i)->getCredit() << "</credit>\n";
        for (unsigned int j = 0; j < reg_list->get(i)->getACLSize(); j++) {
            std::cout
                << "\t\t<ACL>\n"
                << "\t\t\t<cert_md5>" << reg_list->get(i)->getACL(j)->getCertificateMD5() << "</cert_md5>\n"
                << "\t\t\t<pass>" << reg_list->get(i)->getACL(j)->getPassword() << "</pass>\n"
                << "\t\t</ACL>\n";
        }
        std::cout
            << "\t</registrar>\n";
    }
    std::cout << "</object>" << std::endl;
}

#define SET_IF_PRESENT(var, type, name)                     \
    if (m_conf.hasOpt(name)) {                              \
        var = m_conf.get<type>(name);                       \
    }
void
RegistrarClient::zone_add()
{
    callHelp(m_conf, zone_add_help);
    Register::Zone::Manager::ZoneManagerPtr zoneMan
        = Register::Zone::Manager::create();
    std::string fqdn = m_conf.get<std::string>(REGISTRAR_ZONE_FQDN_NAME);
    int exPeriodMin = 12;
    int exPeriodMax = 120;
    int ttl = 18000;
    std::string hostmaster = "hostmaster@localhost";
    int updateRetr = 3600;
    int refresh = 16000;
    int expiry = 1209600;
    int minimum = 7200;
    std::string nsFqdn("localhost");

    SET_IF_PRESENT(exPeriodMin, int, REGISTRAR_EX_PERIOD_MIN_NAME);
    SET_IF_PRESENT(exPeriodMax, int, REGISTRAR_EX_PERIOD_MAX_NAME);
    SET_IF_PRESENT(ttl, int, REGISTRAR_TTL_NAME);
    SET_IF_PRESENT(hostmaster, std::string, REGISTRAR_HOSTMASTER_NAME);
    SET_IF_PRESENT(updateRetr, int, REGISTRAR_UPDATE_RETR_NAME);
    SET_IF_PRESENT(refresh, int, REGISTRAR_REFRESH_NAME);
    SET_IF_PRESENT(expiry, int, REGISTRAR_EXPIRY_NAME);
    SET_IF_PRESENT(minimum, int, REGISTRAR_MINIMUM_NAME);
    SET_IF_PRESENT(nsFqdn, std::string, REGISTRAR_NS_FQDN_NAME);
    try {
        zoneMan->addZone(fqdn, exPeriodMin, exPeriodMax, ttl, hostmaster,
                refresh, updateRetr, expiry, minimum, nsFqdn);
    } catch (Register::ALREADY_EXISTS) {
        std::cerr << "Zone '" << fqdn << "' already exists" << std::endl;
    }
    return;
}
#undef SET_IF_PRESENT

void
RegistrarClient::zone_ns_add()
{
    callHelp(m_conf, zone_ns_add_help);
    Register::Zone::Manager::ZoneManagerPtr zoneMan
        = Register::Zone::Manager::create();
    std::string zone = m_conf.get<std::string>(REGISTRAR_ZONE_FQDN_NAME);
    std::string fqdn = m_conf.get<std::string>(REGISTRAR_NS_FQDN_NAME);
    std::string addr;
    if (m_conf.hasOpt(REGISTRAR_ADDR_NAME)) {
        addr = m_conf.get<std::string>(REGISTRAR_ADDR_NAME);
    } else {
        addr = "";
    }
    try {
        zoneMan->addZoneNs(zone, fqdn, addr);
    } catch (...) {
        std::cerr << "An error has occured" << std::endl;
    }
}

#define SET_IF_PRESENT(setter, name)                        \
    if (m_conf.hasOpt(name)) {                              \
        registrar->setter(m_conf.get<std::string>(name));   \
    }
void
RegistrarClient::registrar_add()
{
    callHelp(m_conf, registrar_add_help);
    Register::Registrar::Manager::AutoPtr regMan
             = Register::Registrar::Manager::create(&m_db);
    Register::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();
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
    registrar->save();
}

#undef SET_IF_PRESENT

void
RegistrarClient::registrar_acl_add()
{
    callHelp(m_conf, registrar_acl_add_help);
    Register::Registrar::Manager::AutoPtr regMan
        = Register::Registrar::Manager::create(&m_db);
    std::string handle = m_conf.get<std::string>(REGISTRAR_ADD_HANDLE_NAME);
    std::string cert = m_conf.get<std::string>(REGISTRAR_CERT_NAME);
    std::string pass = m_conf.get<std::string>(REGISTRAR_PASSWORD_NAME);
    try {
        regMan->addRegistrarAcl(handle, cert, pass);
    } catch (...) {
        std::cerr << "An error has occured" << std::endl;
    }
}

void
RegistrarClient::registrar_add_zone()
{
    callHelp(m_conf, registrar_add_zone_help);
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
    Register::Registrar::addRegistrarZone(registrar, zone, fromDate, toDate);
    return;
}

void
RegistrarClient::price_add()
{
    callHelp(m_conf, price_add_help);
    Register::Zone::Manager::ZoneManagerPtr zoneMan
        = Register::Zone::Manager::create();
    Database::DateTime validFrom(Database::NOW_UTC);
    if (m_conf.hasOpt(REGISTRAR_VALID_FROM_NAME)) {
        validFrom.from_string(m_conf.get<std::string>(REGISTRAR_VALID_FROM_NAME));
    }
    Database::DateTime validTo;
    if (m_conf.hasOpt(REGISTRAR_VALID_TO_NAME)) {
        validTo.from_string(m_conf.get<std::string>(REGISTRAR_VALID_TO_NAME));
    }
    Database::Money price(m_conf.get<int>(REGISTRAR_PRICE_NAME) * 100);
    int period = 12;
    if (m_conf.hasOpt(REGISTRAR_PERIOD_NAME)) {
        period = m_conf.get<int>(REGISTRAR_PERIOD_NAME);
    }
    if (!(m_conf.hasOpt(REGISTRAR_ZONE_FQDN_NAME) ||
            m_conf.hasOpt(REGISTRAR_ZONE_ID_NAME))) {
        std::cerr << "You have to specity either ``--" << REGISTRAR_ZONE_FQDN_NAME
            << "'' or ``--" << REGISTRAR_ZONE_ID_NAME << "''." << std::endl;
        return;
    }
    if (m_conf.hasOpt(REGISTRAR_CREATE_OPERATION_NAME)) {
        if (m_conf.hasOpt(REGISTRAR_ZONE_FQDN_NAME)) {
            std::string zone = m_conf.get<std::string>(REGISTRAR_ZONE_FQDN_NAME);
            zoneMan->addPrice(zone, Register::Zone::CREATE, validFrom,
                    validTo, price, period);
        } else {
            unsigned int zoneId = m_conf.get<unsigned int>(REGISTRAR_ZONE_ID_NAME);
            zoneMan->addPrice(zoneId, Register::Zone::CREATE, validFrom,
                    validTo, price, period);
        }
    }
    if (m_conf.hasOpt(REGISTRAR_RENEW_OPERATION_NAME)) {
        if (m_conf.hasOpt(REGISTRAR_ZONE_FQDN_NAME)) {
            std::string zone = m_conf.get<std::string>(REGISTRAR_ZONE_FQDN_NAME);
            zoneMan->addPrice(zone, Register::Zone::RENEW, validFrom,
                    validTo, price, period);
        } else {
            unsigned int zoneId = m_conf.get<unsigned int>(REGISTRAR_ZONE_ID_NAME);
            zoneMan->addPrice(zoneId, Register::Zone::RENEW, validFrom,
                    validTo, price, period);
        }
    }
}

void
RegistrarClient::zone_add_help()
{
    std::cout <<
        "** Add new zone **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_ZONE_ADD_NAME << " \\\n"
        "    --" << REGISTRAR_ZONE_FQDN_NAME << "=<zone_fqdn> \\\n"
        "    [--" << REGISTRAR_EX_PERIOD_MIN_NAME << "=<ex_period_min>] \\\n"
        "    [--" << REGISTRAR_EX_PERIOD_MAX_NAME << "=<ex_period_max>] \\\n"
        "    [--" << REGISTRAR_TTL_NAME << "=<ttl>] \\\n"
        "    [--" << REGISTRAR_HOSTMASTER_NAME << "=<hostmaster>] \\\n"
        "    [--" << REGISTRAR_UPDATE_RETR_NAME << "=<update_retr>] \\\n"
        "    [--" << REGISTRAR_REFRESH_NAME << "=<refresh>] \\\n"
        "    [--" << REGISTRAR_EXPIRY_NAME << "=<expiry>] \\\n"
        "    [--" << REGISTRAR_MINIMUM_NAME << "=<minimum>] \\\n"
        "    [--" << REGISTRAR_NS_FQDN_NAME << "=<ns_fqdn>]\n"
        << std::endl;
}

void
RegistrarClient::zone_ns_add_help()
{
    std::cout <<
        "** Add new nameserver to zone**\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_ZONE_NS_ADD_NAME << " \\\n"
        "    --" << REGISTRAR_ZONE_FQDN_NAME << "=<zone_fqdn> \\\n"
        "    --" << REGISTRAR_NS_FQDN_NAME << "=<ns_fqdn> \\\n"
        "    --" << REGISTRAR_ADDR_NAME << "=<addr>\n"
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
RegistrarClient::registrar_acl_add_help()
{
    std::cout << 
        "** Add new certificate add password to registrar **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_REGISTRAR_ACL_ADD_NAME << " \\\n"
        "    --" << REGISTRAR_ADD_HANDLE_NAME << "=<handle> \\\n"
        "    --" << REGISTRAR_CERT_NAME << "=<certificate> \\\n"
        "    --" << REGISTRAR_PASSWORD_NAME << "=<password>\n"
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

void
RegistrarClient::price_add_help()
{
    std::cout <<
        "** Add new price for the zone **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_PRICE_ADD_NAME << " \\\n"
        "    --" << REGISTRAR_CREATE_OPERATION_NAME << " | \\\n"
        "    --" << REGISTRAR_RENEW_OPERATION_NAME << " \\\n"
        "    --" << REGISTRAR_ZONE_FQDN_NAME << "=<zone_fqdn> | \\\n"
        "    --" << REGISTRAR_ZONE_ID_NAME << "=<zone_id> \\\n"
        "    [--" << REGISTRAR_VALID_FROM_NAME << "=<valid_from_timestamp>] \\\n"
        "    [--" << REGISTRAR_VALID_TO_NAME << "=<valid_to_timestamp>] \\\n"
        "    --" << REGISTRAR_PRICE_NAME << "=<price> \\\n"
        "    [--" << REGISTRAR_PERIOD_NAME << "=<period>]\n"
        << std::endl;
    std::cout <<
        "Default value for the ``valid from'' is NOW and NULL for ``valid_to''.\n"
        "Default pediod is 12 (it means twelve months).\n";
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_REGISTRAR, name, name##_DESC, type, callable, visible}

const struct options
RegistrarClient::m_opts[] = {
    ADDOPT(REGISTRAR_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_ZONE_ADD_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_ZONE_NS_ADD_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_REGISTRAR_ADD_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_REGISTRAR_ACL_ADD_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_REGISTRAR_ADD_ZONE_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_PRICE_ADD_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(REGISTRAR_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    add_ID,
    add_HANDLE,
    add_NAME,
    add_CITY,
    add_EMAIL,
    add_COUNTRY,
    ADDOPT(REGISTRAR_ZONE_FQDN_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_ZONE_ID_NAME, TYPE_UINT, false, false),
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
    ADDOPT(REGISTRAR_TO_DATE_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_EX_PERIOD_MAX_NAME, TYPE_INT, false, false),
    ADDOPT(REGISTRAR_EX_PERIOD_MIN_NAME, TYPE_INT, false, false),
    ADDOPT(REGISTRAR_TTL_NAME, TYPE_INT, false, false),
    ADDOPT(REGISTRAR_HOSTMASTER_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_UPDATE_RETR_NAME, TYPE_INT, false, false),
    ADDOPT(REGISTRAR_REFRESH_NAME, TYPE_INT, false, false),
    ADDOPT(REGISTRAR_EXPIRY_NAME, TYPE_INT, false, false),
    ADDOPT(REGISTRAR_MINIMUM_NAME, TYPE_INT, false, false),
    ADDOPT(REGISTRAR_NS_FQDN_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_ADDR_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_CREATE_OPERATION_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(REGISTRAR_RENEW_OPERATION_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(REGISTRAR_VALID_FROM_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_VALID_TO_NAME, TYPE_STRING, false, false),
    ADDOPT(REGISTRAR_PRICE_NAME, TYPE_INT, false, false),
    ADDOPT(REGISTRAR_PERIOD_NAME, TYPE_INT, false, false)
};

#undef ADDOPT

int 
RegistrarClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

