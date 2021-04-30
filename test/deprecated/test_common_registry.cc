/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "test/deprecated/test_common_registry.hh"

#include "src/util/time_clock.hh"
#include "src/deprecated/libfred/credit.hh"
#include "src/bin/corba/file_manager_client.hh"

#include "src/deprecated/libfred/banking/bank_common.hh"
#include "src/deprecated/libfred/exceptions.hh"

#include "libfred/opcontext.hh"
#include "libfred/poll/create_request_fee_info_message.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/epp_auth/add_registrar_epp_auth.hh"
#include "libfred/registrar/zone_access/add_registrar_zone_access.hh"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>
#include <map>
#include <queue>
#include <sys/time.h>
#include <time.h>


// TODO use it where it's needed
Database::ID get_zone_cz_id()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::ID zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];
    return zone_cz_id;
}


LibFred::Registrar::Registrar::AutoPtr createTestRegistrarClass()
{
    std::string time_string(TimeStamp::microsec());
    std::string registrar_handle = std::string("REG-FREDTEST") + time_string;
    ::LibFred::Registrar::Manager::AutoPtr regMan
             = ::LibFred::Registrar::Manager::create(DBSharedPtr());
    ::LibFred::Registrar::Registrar::AutoPtr registrar = regMan->createRegistrar();
    static unsigned long long var_symbol = 5556660000;

    registrar->setName(registrar_handle + "_Name");
    registrar->setOrganization(registrar_handle + "_Organization");
    registrar->setCity("Brno");
    registrar->setStreet1("Street 1");
    registrar->setStreet2("Street 2");
    registrar->setStreet3("Street 3");
    registrar->setDic("1234567889");
    registrar->setEmail("info@nic.cz");
    registrar->setFax("+420.123456");
    registrar->setIco("92345678899");
    registrar->setPostalCode("11150");
    registrar->setProvince("noprovince");
    registrar->setTelephone("+420.987654");
    registrar->setVarSymb(std::to_string(var_symbol++));
    registrar->setURL("http://ucho.cz");

    registrar->setHandle(registrar_handle);//REGISTRAR_ADD_HANDLE_NAME
    registrar->setCountry("CZ");//REGISTRAR_COUNTRY_NAME
    registrar->setVat(true);
    ::LibFred::Registrar::ACL* const registrar_acl = registrar->newACL();
    registrar_acl->setCertificateMD5("");
    registrar_acl->set_password("");

    ::LibFred::OperationContextCreator ctx;
    const unsigned long long registrar_id = ::LibFred::CreateRegistrar(registrar_handle)
            .set_name(registrar->getName())
            .set_organization(registrar->getOrganization())
            .set_city(registrar->getCity())
            .set_street1(registrar->getStreet1())
            .set_street2(registrar->getStreet2())
            .set_street3(registrar->getStreet3())
            .set_dic(registrar->getDic())
            .set_email(registrar->getEmail())
            .set_fax(registrar->getFax())
            .set_ico(registrar->getIco())
            .set_postalcode(registrar->getPostalCode())
            .set_stateorprovince(registrar->getProvince())
            .set_telephone(registrar->getTelephone())
            .set_variable_symbol(registrar->getVarSymb())
            .set_url(registrar->getURL())
            .set_country(registrar->getCountry())
            .set_vat_payer(registrar->getVat())
            .exec(ctx);
    registrar->setId(registrar_id);
    const unsigned long long epp_auth_id =
            ::LibFred::Registrar::EppAuth::AddRegistrarEppAuth(registrar_handle, "", "").exec(ctx);
    registrar_acl->setId(epp_auth_id);

    //add registrar into zone
    std::string rzzone ("cz");//REGISTRAR_ZONE_FQDN_NAME
    boost::gregorian::date rzfromDate(boost::gregorian::day_clock::local_day());
    ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, rzzone, rzfromDate).exec(ctx);
    ::LibFred::Registrar::ZoneAccess::AddRegistrarZoneAccess(registrar_handle, "0.2.4.e164.arpa", rzfromDate)
            .exec(ctx);
    ctx.commit_transaction();
    return registrar;
}

Database::ID createTestRegistrar()
{
    ::LibFred::Registrar::Registrar::AutoPtr registrar = createTestRegistrarClass();
    return registrar->getId();
}

bool check_std_exception_invoice_prefix(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("Missing invoice prefix")) != std::string::npos);
}

bool check_std_exception_out_of_range(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("Out of range")) != std::string::npos);
}

bool check_std_exception_archiveInvoices(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("archiveInvoices")) != std::string::npos);
}

bool check_std_exception_createAccountInvoice(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("createAccountInvoice")) != std::string::npos);
}

bool check_std_exception_billing_fail(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("Billing failure")) != std::string::npos);
}

bool check_dummy(std::exception const &)
{
    return true;
}

//insertInvoicePrefix
void try_insert_invoice_prefix()
{
    //db
    Database::Connection conn = Database::Manager::acquire();
    unsigned long long zone_cz_id = conn.exec("select id from zone where fqdn='cz'")[0][0];

    std::unique_ptr<::LibFred::Invoicing::Manager> invMan(
            ::LibFred::Invoicing::Manager::create());

    for (int year = 2000; year < boost::gregorian::day_clock::universal_day().year() + 10 ; ++year)
    {
        try{
        invMan->insertInvoicePrefix(
                 zone_cz_id//zoneId
                , 0//type
                , year//year
                , year*10000//prefix
                );
        }catch(...){}

        try{
        invMan->insertInvoicePrefix(
                zone_cz_id//zoneId
                , 1//type
                , year//year
                , year*10000 + 1000//prefix
                );
        }catch(...){}

}//for insertInvoicePrefix
}

Money getOperationPrice(unsigned op, Database::ID zone_id, unsigned requested_quantity)
{
    //db
    Database::Connection conn = Database::Manager::acquire();

    // TODO more detailed
    Database::Result price_res = conn.exec_params(
        "SELECT"
            " price, quantity"
        " FROM price_list where zone_id = $1::integer and operation_id = $2::integer"
            " AND valid_from < now()"
            " AND ((valid_to is null) or (valid_to > now())) order by valid_from desc limit 1 ",
                              Database::query_param_list(zone_id)
                                                        (op));

    if(price_res.size() != 1) {
        throw std::runtime_error("Fetching record from price_list");
    }

    Decimal base_period = std::string(price_res[0][1]);
    Money base_price = std::string(price_res[0][0]);
    if(base_period > Decimal("0")) {
        return (base_price
                    * Decimal(boost::lexical_cast<std::string>(requested_quantity))
                    / base_period
                ).round_half_up(2);

    } else {
        return base_price;
    }


}

//// Test reuqest fee charging
// TODO use in testcases.
const Decimal get_credit(Database::ID reg_id, Database::ID zone_id)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::Result credit_res = conn.exec_params(zone_registrar_credit_query
            , Database::query_param_list(zone_id)(reg_id));

    if(credit_res.size() == 1 && credit_res[0].size() == 1) {
        return Decimal(std::string(credit_res[0][0]));
    }
    return Decimal("0");
}

void get_vat(int &vat_percent, std::string &vat_koef, date taxdate)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::Result vat_details = conn.exec_params(
                "select vat, koef from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
                , Database::query_param_list(taxdate));

    if(vat_details.size() == 1 && vat_details[0].size() == 2) {
        vat_percent = vat_details[0][0];
        vat_koef = std::string(vat_details[0][1]);
    } else {
        throw std::runtime_error("Entry in price_vat not found.");
    }
}


// using namespace ::LibFred::Invoicing;

// poll message for given time period,
// default is poll message for the last month
void insert_poll_request_fee(Database::ID reg_id,
        Decimal price,
        date poll_from,
        date poll_to
        )
{
    Database::ID zone_cz_id = get_zone_cz_id();

    Decimal req_unit_price = getOperationPrice(::LibFred::Invoicing::INVOICING_GeneralOperation, zone_cz_id, 1);
    Decimal req_count = price / req_unit_price;

    if(poll_from == date()) {
        date local_today = day_clock::local_day();
        poll_from = date(local_today.year(), local_today.month(), 1) - months(1);
    }
    if(poll_to == date()) {
        poll_to = poll_from + months(1);
    }

    ::LibFred::OperationContextCreator ctx;
    ::LibFred::Poll::CreateRequestFeeInfoMessage(reg_id,
                                            ptime(poll_from),
                                            ptime(poll_to),
                                            0,
                                            boost::lexical_cast<unsigned long long>(
                                                boost::lexical_cast<double>(req_count.get_string())),
                                            price,
                                            "Europe/Prague").exec(ctx);
    ctx.commit_transaction();
}
