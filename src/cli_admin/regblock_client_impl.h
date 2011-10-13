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

using namespace Fred::Registrar;

namespace Admin {

class RegBlockClient {
private:
    RegBlockArgs params;
    Fred::Registrar::Manager::AutoPtr regMan;

public:
    RegBlockClient(
            const RegBlockArgs &p) :
               params(p),
               regMan(Fred::Registrar::Manager::create(DBDisconnectPtr(0)))
    { }

    void runMethod() {
        if(params.over_limit) {
            block_clients_over_limit();
        } else if (params.list_only) {
            // TODO
            throw std::runtime_error("Not implemented yet ");
        } else if (params.block_id.get_value() != 0) {
            init_corba_container();
            std::auto_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());

            if(!regMan->blockRegistrar(params.block_id, epp_cli.get())) {
                std::cout << "Registrar not blocked: see log for details" << std::endl;
            }
        } else if (params.unblock_id.get_value() != 0) {
            regMan->unblockRegistrar(params.unblock_id, 0);
        }
    }


private:
    void init_corba_container()
    {
        // ORB init
        FakedArgs orb_fa = CfgArgGroups::instance()->fa;

        HandleCorbaNameServiceArgsGrp* ns_args_ptr=CfgArgGroups::instance()->
                   get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

        CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
               , ns_args_ptr->get_nameservice_host()
               , ns_args_ptr->get_nameservice_port()
               , ns_args_ptr->get_nameservice_context());
    }

    void send_block_notification()
    {
       // check if sendmail is present
       SubProcessOutput sub_output_test = ShellCmd("ls /usr/sbin/sendmail", params.shell_cmd_timeout).execute();
       if (!sub_output_test.stderr.empty()) {
           LOGGER(PACKAGE).error(sub_output_test.stderr);
       }

       BlockedRegistrars blocked_registrars = regMan->getRegistrarsBlockedToday();

       std::string cmd;
       if(blocked_registrars->empty()) {
           cmd = (boost::format("{\n"
                   "echo \"Subject: No registrars blocked, date $(date +'%%Y-%%m-%%d')\n"
                   "Content-Type: text/plain; charset=UTF-8; format=flowed"
                   "\nContent-Transfer-Encoding: 8bit\n\n \";"
                   "\n} | /usr/sbin/sendmail %1%" ) % params.notify_email).str();
       } else {
           // there are some entries to send
           std::ostringstream msg;

           for (
                   std::vector<BlockedReg>::iterator it = blocked_registrars->begin();
                   it != blocked_registrars->end();
                   ++it) {

               // TODO: include price as before: "Registrar %1% blocked: price limit %2%, current price: %3%,
               // e-mail: %4%, phone: %5%, link: https://manager.nic.cz/registrar/detail/?id=%6% \n")
               msg << (boost::format(
               "Registrar %1% blocked: price limit %2%, e-mail: %3%, phone: %4%, "
               "link: https://manager.nic.cz/registrar/detail/?id=%5% \n")
                   % it->reg_handle
                   % it->price_limit
                   % it->email
                   % it->telephone
                   % it->reg_id).str()
               << std::endl;
           }
           cmd = (boost::format("{\n"
             "echo \"Subject: REGISTRARS BLOCKED - requests over limit, date $(date +'%%Y-%%m-%%d')\n"
             "Content-Type: text/plain; charset=UTF-8; format=flowed"
             "\nContent-Transfer-Encoding: 8bit\n\n%1% \n\";"
             "\n} | /usr/sbin/sendmail %2%") % msg.str() % params.notify_email).str();
       }

       SubProcessOutput sub_output = ShellCmd(cmd, params.shell_cmd_timeout).execute();
       if (!sub_output.stderr.empty()) {
           throw std::runtime_error(sub_output.stderr);
       }
    }

    void block_clients_over_limit()
    {
       init_corba_container();
       std::auto_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());
       std::auto_ptr<Fred::Logger::LoggerClient> log_cli(new Fred::Logger::LoggerCorbaClientImpl());

       regMan->blockRegistrarsOverLimit(
                   epp_cli.get(),
                   log_cli.get()
                   );

       if(!params.notify_email.empty()) {
           send_block_notification();
       } else {
           LOGGER(PACKAGE).info("No email specified, not trying to send information about blocked registrars");
       }
    }
};

};


#endif // _REGBLOCK_CLIENT_IMPL_H_
