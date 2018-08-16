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

#include "src/backend/admin/zone/zone.hh"
#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/registrarclient.hh"
#include "src/bin/corba/file_manager_client.hh"
#include "src/util/types/money.hh"

#include <boost/asio.hpp>
#include <string>
#include <vector>

namespace Admin {


void
RegistrarClient::runMethod()
{
    if (zone_add_) {//REGISTRAR_ZONE_ADD_NAME
        zone_add();
    } else if (registrar_add_) {//REGISTRAR_REGISTRAR_ADD_NAME
        registrar_add();
    } else if (registrar_add_zone_) {//REGISTRAR_REGISTRAR_ADD_ZONE_NAME
        registrar_add_zone();
    } else if (registrar_create_certification_) {//REGISTRAR_REGISTRAR_CREATE_CERTIFICATION_NAME
        registrar_create_certification();
    } else if (registrar_create_group_) {//REGISTRAR_REGISTRAR_CREATE_GROUP_NAME
        registrar_create_group();
    } else if (registrar_into_group_) {//REGISTRAR_REGISTRAR_INTO_GROUP_NAME
    	registrar_into_group();
    } else if (registrar_list_) {//REGISTRAR_LIST_NAME
        list();
    } else if (registrar_show_opts_) {//REGISTRAR_SHOW_OPTS_NAME
        //show_opts();
    } else if (zone_ns_add_) {//REGISTRAR_ZONE_NS_ADD_NAME
        zone_ns_add();
    } else if (registrar_acl_add_) {//REGISTRAR_REGISTRAR_ACL_ADD_NAME
        registrar_acl_add();
    } else if (price_add_) {//REGISTRAR_PRICE_ADD_NAME
        price_add();
    }
}

void
RegistrarClient::list()
{

    LibFred::Registrar::Manager::AutoPtr regMan
             = LibFred::Registrar::Manager::create(m_db);

    LibFred::Registrar
        ::RegistrarList::AutoPtr reg_list(regMan->createList());

    std::unique_ptr<Database::Filters::Registrar>
        regFilter ( new Database::Filters::RegistrarImpl(true));

    if (registrar_list_params_.id.is_value_set())//ID_NAME
        regFilter->addId().setValue(
                Database::ID(registrar_list_params_.id.get_value()));
    if (registrar_list_params_.handle.is_value_set())//HANDLE_NAME
        regFilter->addHandle().setValue(
                registrar_list_params_.handle.get_value());
    if (registrar_list_params_.name_name.is_value_set())//NAME_NAME
        regFilter->addName().setValue(
                registrar_list_params_.name_name.get_value());
    if (registrar_list_params_.organization.is_value_set())//ORGANIZATION_NAME
        regFilter->addOrganization().setValue(
                registrar_list_params_.organization.get_value());
    if (registrar_list_params_.city.is_value_set())//CITY_NAME
        regFilter->addCity().setValue(
                registrar_list_params_.city.get_value());
    if (registrar_list_params_.email.is_value_set())//EMAIL_NAME
        regFilter->addEmail().setValue(
                registrar_list_params_.email.get_value());
    if (registrar_list_params_.country.is_value_set())//COUNTRY_NAME
        regFilter->addCountryCode().setValue(
                registrar_list_params_.country.get_value());

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
                << "\t\t\t<pass></pass>\n"
                << "\t\t</ACL>\n";
        }
        std::cout
            << "\t</registrar>\n";
    }
    std::cout << "</object>" << std::endl;
}

void
RegistrarClient::zone_add()
{
    std::string fqdn = zone_add_params_.zone_fqdn;//REGISTRAR_ZONE_FQDN_NAME
    int exPeriodMin = 12;
    int exPeriodMax = 120;
    int ttl = 18000;
    std::string hostmaster = "hostmaster@localhost";
    int updateRetr = 300;
    int refresh = 900;
    int expiry = 604800;
    int minimum = 900;
    std::string nsFqdn("localhost");

    if (zone_add_params_.ex_period_min.is_value_set()) {
        exPeriodMin = zone_add_params_.ex_period_min.get_value();
    };//REGISTRAR_EX_PERIOD_MIN_NAME
    if (zone_add_params_.ex_period_max.is_value_set()) {
        exPeriodMax = zone_add_params_.ex_period_max.get_value();
    };//REGISTRAR_EX_PERIOD_MAX_NAME
    if (zone_add_params_.ttl.is_value_set()) {
        ttl = zone_add_params_.ttl.get_value();
    };//REGISTRAR_TTL_NAME
    if (zone_add_params_.hostmaster.is_value_set()) {
        hostmaster = zone_add_params_.hostmaster.get_value();
    };//REGISTRAR_HOSTMASTER_NAME
    if (zone_add_params_.update_retr.is_value_set()) {
        updateRetr = zone_add_params_.update_retr.get_value();
    };//REGISTRAR_UPDATE_RETR_NAME
    if (zone_add_params_.refresh.is_value_set()) {
        refresh = zone_add_params_.refresh.get_value();
    };//REGISTRAR_REFRESH_NAME
    if (zone_add_params_.expiry.is_value_set()) {
        expiry = zone_add_params_.expiry.get_value();
    };//REGISTRAR_EXPIRY_NAME
    if (zone_add_params_.minimum.is_value_set()) {
        minimum = zone_add_params_.minimum.get_value();
    };//REGISTRAR_MINIMUM_NAME
    if (zone_add_params_.ns_fqdn.is_value_set()) {
        nsFqdn = zone_add_params_.ns_fqdn.get_value();
    };//REGISTRAR_NS_FQDN_NAME
    Admin::Zone::add_zone(fqdn, exPeriodMin, exPeriodMax,
            ttl, hostmaster, refresh, updateRetr, expiry, minimum, nsFqdn);
}

void
RegistrarClient::zone_ns_add()
{
    std::string zone = zone_ns_add_params_.zone_fqdn;//REGISTRAR_ZONE_FQDN_NAME
    std::string fqdn = zone_ns_add_params_.ns_fqdn;//REGISTRAR_NS_FQDN_NAME
    std::vector<boost::asio::ip::address> addrs;
    if (!zone_ns_add_params_.addrs.empty()) {//REGISTRAR_ADDR_NAME
        std::for_each(zone_ns_add_params_.addrs.begin(),
                zone_ns_add_params_.addrs.end(),
                [&addrs](const std::string& s) { addrs.push_back(boost::asio::ip::address::from_string(s));});
    }
    Admin::Zone::add_zone_ns(zone, fqdn, addrs);
}

void
RegistrarClient::registrar_add()
{
    LibFred::Registrar::Manager::AutoPtr regMan
             = LibFred::Registrar::Manager::create(m_db);
    LibFred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();
    registrar->setHandle(registrar_add_params_.handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry(registrar_add_params_.country);//REGISTRAR_COUNTRY_NAME
    if (registrar_add_params_.ico.is_value_set()) {
        registrar->setIco(registrar_add_params_.ico.get_value());
    };//REGISTRAR_ICO_NAME
    if (registrar_add_params_.dic.is_value_set()) {
        registrar->setDic(registrar_add_params_.dic.get_value());
    };//REGISTRAR_DIC_NAME
    if (registrar_add_params_.varsymb.is_value_set()) {
        registrar->setVarSymb(registrar_add_params_.varsymb.get_value());
    };//REGISTRAR_VAR_SYMB_NAME
    if (registrar_add_params_.reg_name.is_value_set()) {
        registrar->setName(registrar_add_params_.reg_name.get_value());
    };//REGISTRAR_ADD_NAME_NAME
    if (registrar_add_params_.organization.is_value_set()) {
        registrar->setOrganization(registrar_add_params_.organization.get_value());
    };//REGISTRAR_ORGANIZATION_NAME
    if (registrar_add_params_.street1.is_value_set()) {
        registrar->setStreet1(registrar_add_params_.street1.get_value());
    };//REGISTRAR_STREET1_NAME
    if (registrar_add_params_.street2.is_value_set()) {
        registrar->setStreet2(registrar_add_params_.street2.get_value());
    };//REGISTRAR_STREET2_NAME
    if (registrar_add_params_.street3.is_value_set()) {
        registrar->setStreet3(registrar_add_params_.street3.get_value());
    };//REGISTRAR_STREET3_NAME
    if (registrar_add_params_.city.is_value_set()) {
        registrar->setCity(registrar_add_params_.city.get_value());
    };//REGISTRAR_CITY_NAME
    if (registrar_add_params_.stateorprovince.is_value_set()) {
        registrar->setProvince(registrar_add_params_.stateorprovince.get_value());
    };//REGISTRAR_STATEORPROVINCE_NAME
    if (registrar_add_params_.postalcode.is_value_set()) {
        registrar->setPostalCode(registrar_add_params_.postalcode.get_value());
    };//REGISTRAR_POSTALCODE_NAME
    if (registrar_add_params_.telephone.is_value_set()) {
        registrar->setTelephone(registrar_add_params_.telephone.get_value());
    };//REGISTRAR_TELEPHONE_NAME
    if (registrar_add_params_.fax.is_value_set()) {
        registrar->setFax(registrar_add_params_.fax.get_value());
    };//REGISTRAR_FAX_NAME
    if (registrar_add_params_.email.is_value_set()) {
        registrar->setEmail(registrar_add_params_.email.get_value());
    };//REGISTRAR_EMAIL_NAME
    if (registrar_add_params_.url.is_value_set()) {
        registrar->setURL(registrar_add_params_.url.get_value());
    };//REGISTRAR_URL_NAME
    if (registrar_add_params_.system) {//REGISTRAR_SYSTEM_NAME
        registrar->setSystem(true);
    } else {
        registrar->setSystem(false);
    }
    if (registrar_add_params_.no_vat) {//REGISTRAR_NO_VAT_NAME
        registrar->setVat(false);
    } else {
        registrar->setVat(true);
    }
    registrar->save();
}

void
RegistrarClient::registrar_acl_add()
{
    LibFred::Registrar::Manager::AutoPtr regMan
        = LibFred::Registrar::Manager::create(m_db);
    std::string handle = registrar_acl_add_params_.handle;//REGISTRAR_ADD_HANDLE_NAME
    std::string cert = registrar_acl_add_params_.certificate;//REGISTRAR_CERT_NAME
    std::string pass = registrar_acl_add_params_.password;//REGISTRAR_PASSWORD_NAME
    try {
        regMan->addRegistrarAcl(handle, cert, pass);
    } catch (...) {
        std::cerr << "An error has occured" << std::endl;
    }
}

void
RegistrarClient::registrar_add_zone()
{
    std::string zone = registrar_add_zone_params_.zone_fqdn;//REGISTRAR_ZONE_FQDN_NAME
    std::string registrar = registrar_add_zone_params_.handle;//REGISTRAR_ADD_HANDLE_NAME
    Database::Date fromDate;
    Database::Date toDate;
    if (registrar_add_zone_params_.from_date.is_value_set()) {//REGISTRAR_FROM_DATE_NAME
        fromDate.from_string(registrar_add_zone_params_.from_date.get_value());
    }
    if (registrar_add_zone_params_.to_date.is_value_set()) {//REGISTRAR_TO_DATE_NAME
        toDate.from_string(registrar_add_zone_params_.to_date.get_value());
    }
    LibFred::Registrar::addRegistrarZone(registrar, zone, fromDate, toDate);
    return;
}

void
RegistrarClient::registrar_create_certification()
{
    std::string cert_eval_file =
            registrar_create_certification_params_.certification_evaluation;//REGISTRAR_CERTIFICATION_EVALUATION_NAME

    std::string cert_eval_file_mimetype =
            registrar_create_certification_params_.certification_evaluation_mime_type;//REGISTRAR_CERTIFICATION_EVALUATION_MIME_TYPE_NAME

    int score = registrar_create_certification_params_.certification_score;//REGISTRAR_CERTIFICATION_SCORE_NAME
    if((score < 0) || (score > 5))
        throw std::runtime_error("Invalid value of score");

    std::string registrar_handle = registrar_create_certification_params_.handle;//REGISTRAR_ADD_HANDLE_NAME
    Database::Date fromDate;
    Database::Date toDate;
    if (registrar_create_certification_params_.from_date.is_value_set())//REGISTRAR_FROM_DATE_NAME
    {
        fromDate.from_string(
                registrar_create_certification_params_.from_date.get_value());
    }
    if (registrar_create_certification_params_.to_date.is_value_set())//REGISTRAR_TO_DATE_NAME
    {
        toDate.from_string(registrar_create_certification_params_.to_date.get_value());
    }

    // init file manager
    CorbaClient corba_client(0, 0, m_nsAddr
            , nameservice_context);//NS_CONTEXT_NAME
    FileManagerClient fm_client(corba_client.getNS());
    LibFred::File::ManagerPtr file_manager(
            LibFred::File::Manager::create(&fm_client));

    unsigned long long evaluation_file_id
        = file_manager->upload(cert_eval_file
        		, cert_eval_file_mimetype//"application/pdf"
        		, 6);
    if(evaluation_file_id < 1)
        throw std::runtime_error("Invalid value of evaluation_file_id");

    DBSharedPtr nodb;
    LibFred::Registrar::Manager::AutoPtr regman(
             LibFred::Registrar::Manager::create(nodb));
     ///create registrar certification
     regman->createRegistrarCertification( registrar_handle
             , fromDate//Database::Date(makeBoostDate(from))
             , toDate//Database::Date(makeBoostDate(to))
             , static_cast<LibFred::Registrar::RegCertClass>(score)
             , evaluation_file_id);

    return;
}

void
RegistrarClient::registrar_create_group()
{
    //callHelp(m_conf, registrar_create_group_help);
    std::string group_name =
            registrar_create_group_params_.registrar_group;//REGISTRAR_GROUP_NAME
    if(group_name.empty())
        throw std::runtime_error("RegistrarClient::registrar_create_group "
                "error: group name is empty");
    DBSharedPtr nodb;
    LibFred::Registrar::Manager::AutoPtr regman(
             LibFred::Registrar::Manager::create(nodb));
     ///create registrar certification
     regman->createRegistrarGroup(group_name);
    return;
}

void
RegistrarClient::registrar_into_group()
{
    std::string registrar_handle
        = registrar_into_group_params_.handle;//REGISTRAR_ADD_HANDLE_NAME

    Database::Date fromDate = Database::Date(NOW);
    Database::Date toDate;
    if (registrar_into_group_params_.from_date)//REGISTRAR_FROM_DATE_NAME
    {
        fromDate.from_string(
                registrar_into_group_params_.from_date);

        if (fromDate.is_special())
                throw std::runtime_error("RegistrarClient::registrar_into_group "
                                "error: invalid from_date");
    }


    if (registrar_into_group_params_.to_date)//REGISTRAR_TO_DATE_NAME
    {
        toDate.from_string(registrar_into_group_params_.to_date);

        if (toDate.is_special())
                throw std::runtime_error("RegistrarClient::registrar_into_group "
                                "error: invalid to_date");
    }

    if (fromDate > toDate)
        throw std::runtime_error("RegistrarClient::registrar_into_group "
                        "error: from_date > to_date");

    std::string group_name =
            registrar_into_group_params_.registrar_group;//REGISTRAR_GROUP_NAME

    if(group_name.empty())
        throw std::runtime_error("RegistrarClient::registrar_into_group "
                "error: group name is empty");

    DBSharedPtr nodb;
    LibFred::Registrar::Manager::AutoPtr regman(
             LibFred::Registrar::Manager::create(nodb));
     ///create registrar group membership
     regman->createRegistrarGroupMembership(
             registrar_handle,group_name,fromDate,toDate);
    return;
}

void
RegistrarClient::price_add()
{
    LibFred::Zone::Manager::ZoneManagerPtr zoneMan
        = LibFred::Zone::Manager::create();
    Database::DateTime validFrom(Database::NOW_UTC);
    if (price_add_params_.valid_from.is_value_set()) {//REGISTRAR_VALID_FROM_NAME
        validFrom.from_string(price_add_params_.valid_from.get_value());
    }
    Database::DateTime validTo;
    if (price_add_params_.valid_to.is_value_set()) {//REGISTRAR_VALID_TO_NAME
        validTo.from_string(price_add_params_.valid_to.get_value());
    }
    Money price(price_add_params_.operation_price.get_value());//REGISTRAR_PRICE_NAME
    int period = 1;
    if (price_add_params_.period.is_value_set()) {//REGISTRAR_PERIOD_NAME
        period = price_add_params_.period.get_value();
    }
    if (!(price_add_params_.zone_fqdn.is_value_set() ||//REGISTRAR_ZONE_FQDN_NAME
            price_add_params_.zone_id.is_value_set())) {//REGISTRAR_ZONE_ID_NAME
        std::cerr << "You have to specity either ``--zone_fqdn'' or ``--zone_id''." << std::endl;
        return;
    }

    if (!price_add_params_.operation.is_value_set())
    {
        throw std::runtime_error("RegistrarClient::price_add: operation not set");
    }
    std::string operation = price_add_params_.operation.get_value();

    if (price_add_params_.zone_fqdn.is_value_set()) {//REGISTRAR_ZONE_FQDN_NAME
        std::string zone = price_add_params_.zone_fqdn.get_value();//REGISTRAR_ZONE_FQDN_NAME
        zoneMan->addPrice(zone, operation, validFrom,
                validTo, price, period
                , price_add_params_.enable_postpaid_operation);
    } else {
        unsigned int zoneId = price_add_params_.zone_id.get_value();//REGISTRAR_ZONE_ID_NAME
        zoneMan->addPrice(zoneId, operation, validFrom,
                validTo, price, period
                , price_add_params_.enable_postpaid_operation);
    }

}

} // namespace Admin;

