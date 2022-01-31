/*
 * Copyright (C) 2011-2022  CZ.NIC, z. s. p. o.
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

#include "src/bin/cli/chargeclient.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"
#include "src/deprecated/libfred/db_settings.hh"

#include "util/util.hh"


namespace Admin {

void ChargeClient::runMethod()
{
    // parse period_to input
    boost::gregorian::date poll_msg_period_to;
    if(params.poll_msg_period_to.is_value_set()) {
        poll_msg_period_to = from_simple_string(params.poll_msg_period_to.get_value());

        if(poll_msg_period_to.is_special()) {
            throw std::runtime_error("charge: Invalid poll_msg_period_to.");
        }
    }

    // validate input and determine what period to use
    boost::gregorian::date local_today = boost::gregorian::day_clock::local_day();

    if(poll_msg_period_to.is_special()) {
        poll_msg_period_to = date(local_today.year(), local_today.month(), 1);
    } else {
        if(poll_msg_period_to.month() > local_today.month()) {
            std::string msg("Warning: charging for requests in the future.");
            std::cout << msg << std::endl;
            LOGGER.warning(msg);
        }
    }

    if(params.only_registrar.is_value_set()) {
        chargeRequestFeeOneReg(params.only_registrar, poll_msg_period_to);
    } else if(params.except_registrars.is_value_set()) {
        chargeRequestFeeAllRegs(params.except_registrars, poll_msg_period_to);
    } else {
        chargeRequestFeeAllRegs(std::string(), poll_msg_period_to);
    }
}

unsigned long long ChargeClient::getRegistrarID(const std::string &handle) {
    Database::Connection conn = Database::Manager::acquire();
    Database::Result result = conn.exec_params(
            "SELECT id FROM registrar WHERE handle = $1::text",
            Database::query_param_list(handle));

    if (result.size() != 1) {
        boost::format msg("Registrar with handle %1% not found in database.");
        msg % handle;
        throw std::runtime_error(msg.str());
    }

    return result[0][0];
}

void ChargeClient::chargeRequestFeeOneReg(const std::string &handle, const date &poll_msg_period_to)
{
    Database::ID reg_id = getRegistrarID(handle);

    std::unique_ptr<LibFred::Invoicing::Manager> invMan(
      LibFred::Invoicing::Manager::create());

    invMan->chargeRequestFee(reg_id, poll_msg_period_to);
}

void ChargeClient::chargeRequestFeeAllRegs(const std::string &except_handles, const date &poll_msg_period_to)
{
    std::unique_ptr<LibFred::Invoicing::Manager> invMan(
    LibFred::Invoicing::Manager::create());

    Database::Connection conn = Database::Manager::acquire();

    unsigned zone_id;
    LibFred::Invoicing::getRequestFeeParams(&zone_id);

    date poll_msg_period_from;

    if(poll_msg_period_to.day() == 1) {
        poll_msg_period_from = poll_msg_period_to - months(1);
    } else {
        poll_msg_period_from = date(poll_msg_period_to.year(), poll_msg_period_to.month(), 1);
    }

    Database::Result result;
    if(except_handles.empty()) {
        result = conn.exec_params("SELECT r.id "
                "FROM registrar r "
                "JOIN registrarinvoice ri "
                    "ON ri.registrarid = r.id "
                    "AND ( "
                        "($1::date <= ri.fromdate AND $2::date > ri.fromdate) "
                        "OR ($1::date >= ri.fromdate AND  ((ri.todate IS NULL) OR (ri.todate >= $1::date))) "
                    ") "
                "WHERE r.system IS NOT NULL AND NOT r.system AND NOT r.is_internal "
                    "AND ri.zone=$3::integer "
                "ORDER BY r.id",
                Database::query_param_list(poll_msg_period_from)
                                          (poll_msg_period_to)
                                          (zone_id)
            );
    } else {

        std::stringstream parse(except_handles);
        std::string element;

        std::vector<Database::ID> reg_id_list;

        while(std::getline(parse, element, ',')) {

            Database::ID reg_id = getRegistrarID(element);
            reg_id_list.push_back(reg_id);
        }

        std::string id_array = "{" + Util::container2comma_list(reg_id_list) + "}";

        result = conn.exec_params("SELECT r.id "
                "FROM registrar r "
                "JOIN registrarinvoice ri "
                    "ON ri.registrarid = r.id "
                    "AND ( "
                        "($1::date <= ri.fromdate AND $2::date > ri.fromdate) "
                        "OR ($1::date >= ri.fromdate AND  ((ri.todate IS NULL) OR (ri.todate > $1::date))) "
                    ") "
                "WHERE r.system IS NOT NULL AND NOT r.system AND NOT r.is_internal "
                    "AND ri.zone = $3::integer "
                    "AND r.id != ALL ($4::bigint[]) "
                "ORDER BY r.id",

                Database::query_param_list(poll_msg_period_from)
                                        (poll_msg_period_to)
                                        (zone_id)
                                        (id_array)
        );
    }

    Database::Transaction tx(conn);

    for(unsigned i=0;i<result.size();++i) {
        // let exceptions in charging terminate the whole transaction
        if( !invMan->chargeRequestFee(result[i][0], poll_msg_period_to) ) {
            boost::format msg("Balance not sufficient for charging requests for registrar ID %1%");
            msg % result[i][0];
            LOGGER.error(msg);
            throw std::runtime_error(msg.str());
        }
    }

    tx.commit();
}

}//namespace Admin
