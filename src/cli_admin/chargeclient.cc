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

    void ChargeClient::chargeRequestFeeOneReg(const std::string &handle)
    {

        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec_params(
                "SELECT id FROM registrar WHERE handle = $1::text",
                Database::query_param_list(handle));

        if (result.size() != 1) {
            throw std::runtime_error("failed to find specified registrar in database");
        }

        Database::ID reg_id = result[0][0];

        std::auto_ptr<Fred::Invoicing::Manager> invMan(
          Fred::Invoicing::Manager::create());

        invMan->chargeRequestFee(reg_id);
    }

    void ChargeClient::chargeRequestFeeAllRegs(const std::string &except_handles)
    {
        std::auto_ptr<Fred::Invoicing::Manager> invMan(
        Fred::Invoicing::Manager::create());

        Database::Connection conn = Database::Manager::acquire();
        Database::Result res
            = conn.exec("SELECT id, handle FROM registrar WHERE system = false");

        if(!except_handles.empty()) {
            throw std::runtime_error("not implemented yet");
        }

        for(unsigned i=0;i<res.size();++i) {
            invMan->chargeRequestFee(res[0][0]);
        }
    }

}; // namespace Admin
