/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _REGBLOCK_CLIENT_IMPL_H_
#define _REGBLOCK_CLIENT_IMPL_H_

#include "src/corba/logger_client_impl.h"
#include "src/corba/epp_corba_client_impl.h"

namespace Admin {

class RegBlockClient {
private:
    bool over_limit;
    bool list_only;
    optional_ulonglong block_id;
    optional_ulonglong unblock_id;
    Fred::Registrar::Manager::AutoPtr regMan;

public:
    RegBlockClient(
            const RegBlockArgs &params) :
               over_limit(params.over_limit),
               list_only(params.list_only),
               block_id(params.block_id),
               unblock_id(params.unblock_id),
               regMan(Fred::Registrar::Manager::create(DBDisconnectPtr(0)))
    { }

    void runMethod() {
        if(over_limit) {
            init_corba_container();
            std::auto_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());
            std::auto_ptr<Fred::Logger::LoggerClient> log_cli(new Fred::Logger::LoggerCorbaClientImpl());

            regMan->blockClientsOverLimit(epp_cli.get(), log_cli.get());

        } else if (list_only) {
            // TODO
            throw std::runtime_error("Not implemented yet ");
        } else if (block_id.get_value() != 0) {
            init_corba_container();
            std::auto_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

            if(!regMan->blockRegistrar(block_id, epp_cli.get())) {
                std::cout << "Registrar not blocked: see log for details" << std::endl;
            }
        } else if (unblock_id.get_value() != 0) {
            regMan->unblockRegistrar(unblock_id, 0);
        }
    }


private:
    void init_corba_container() {
        // ORB init
        FakedArgs orb_fa = CfgArgGroups::instance()->fa;

        HandleCorbaNameServiceArgsGrp* ns_args_ptr=CfgArgGroups::instance()->
                   get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

        CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
               , ns_args_ptr->get_nameservice_host()
               , ns_args_ptr->get_nameservice_port()
               , ns_args_ptr->get_nameservice_context());
    }
};

};


#endif // _REGBLOCK_CLIENT_IMPL_H_
