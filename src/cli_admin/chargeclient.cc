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

#include "chargeclient.h"
#include "invoicing/invoice.h"
#include "util/util.h"

namespace Admin {
    void ChargeClient::runMethod()
    {
        if(params.only_registrar.is_value_set()) {
            chargeRequestFeeOneReg(params.only_registrar);
        } else if(params.except_registrars.is_value_set()) {
            chargeRequestFeeAllRegs(params.except_registrars);
        } else {
            chargeRequestFeeAllRegs(std::string());
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

    void ChargeClient::chargeRequestFeeOneReg(const std::string &handle)
    {
        Database::ID reg_id = getRegistrarID(handle);

        std::auto_ptr<Fred::Invoicing::Manager> invMan(
          Fred::Invoicing::Manager::create());

        invMan->chargeRequestFee(reg_id);
    }

    void ChargeClient::chargeRequestFeeAllRegs(const std::string &except_handles)
    {
        std::auto_ptr<Fred::Invoicing::Manager> invMan(
        Fred::Invoicing::Manager::create());

        Database::Connection conn = Database::Manager::acquire();

        Database::Result result;
        if(except_handles.empty()) {
            result = conn.exec("SELECT id FROM registrar WHERE system = false");
        } else {

            // TODO split string
            std::cout << "Split string: " << std::endl;
            std::stringstream parse(except_handles);
            std::string element;

            std::vector<Database::ID> reg_id_list;

            while(std::getline(parse, element, ',')) {
                std::cout << "Registrar handle: " << element << " " << std::endl;

                Database::ID reg_id = getRegistrarID(element);

                reg_id_list.push_back(reg_id);
            }

            std::string id_array = "{" + Util::container2comma_list(reg_id_list) + "}";
            result = conn.exec_params("SELECT id FROM registrar "
                    "WHERE system = false "
                    "AND id != ALL ($1::bigint[])",
                    Database::query_param_list(id_array));
        }

        for(unsigned i=0;i<result.size();++i) {
            if( !invMan->chargeRequestFee(result[i][0]) ) {
                boost::format msg("Balance not sufficient for charging requests for registrar ID %1%");
                msg % result[i][0];
                LOGGER(PACKAGE).warning(msg);
            }
        }
    }

}; // namespace Admin
