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


#include "src/bin/cli/bankclient.hh"
#include "src/bin/cli/commonclient.hh"
#include "src/libfred/banking/bank_manager.hh"
#include "src/libfred/registry.hh"
#include "src/bin/corba/file_manager_client.hh"

#include <iostream>
#include <fstream>
#include <string>

namespace Admin {


void
BankClient::runMethod()
{
    if (bank_add_account) {//BANK_ADD_ACCOUNT_NAME
        add_bank_account();
    }
}

void
BankClient::add_bank_account()
{
    //callHelp(m_conf, add_bank_account_help);
    std::string account_number = add_account_params.bank_account_number;//BANK_ACCOUNT_NUMBER_NAME
    std::string bank_code = add_account_params.bank_code;//BANK_BANK_CODE_NAME

    std::string account_name;
    if (add_account_params.account_name.is_value_set()) {//BANK_ACCOUNT_NAME_NAME
        account_name = add_account_params.account_name.get_value();
    }
    std::string zone_fqdn;
    if (add_account_params.zone_fqdn.is_value_set()) {//BANK_ZONE_NAME_NAME
        zone_fqdn = add_account_params.zone_fqdn.get_value();
    }

    // init file manager
    CorbaClient corba_client(0, 0, m_nsAddr, nameservice_context);
    FileManagerClient fm_client(corba_client.getNS());
    LibFred::File::ManagerPtr file_manager(LibFred::File::Manager::create(&fm_client));

    // bank manager
    LibFred::Banking::ManagerPtr bank_manager(LibFred::Banking::Manager::create(file_manager.get()));
    bank_manager->addBankAccount(account_number, bank_code, zone_fqdn, account_name);
}

} // namespace Admin;

