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
            init_corba_container();
            std::auto_ptr<EppCorbaClient> epp_cli(new EppCorbaClientImpl());
            std::auto_ptr<Fred::Logger::LoggerClient> log_cli(new Fred::Logger::LoggerCorbaClientImpl());

            if(params.notify_email.empty()) {
                throw std::runtime_error("No notify email specified, use parametre --email ");
            }

            std::auto_ptr<Fred::Registrar::RequestFeeDataMap> blocked_registrars
                = regMan->blockClientsOverLimit(
                        epp_cli.get(),
                        log_cli.get()
                        );

            if(!blocked_registrars->empty()) {
                SubProcessOutput sub_output_test = ShellCmd("ls /usr/sbin/sendmail", params.shell_cmd_timeout).execute();
                if (!sub_output_test.stderr.empty()) {
                    throw std::runtime_error(sub_output_test.stderr);
                }
            }

            Database::Connection conn = Database::Manager::acquire();
            std::ostringstream msg;

            // send some notification that registrars were blocked
            for ( Fred::Registrar::RequestFeeDataMap::iterator it = blocked_registrars->begin();
                    it != blocked_registrars->end();
                    ++it) {
                Fred::Registrar::RequestFeeData rfd = it->second;

                Result res_contacts = conn.exec_params("SELECT email, telephone FROM request_fee_registrar_parameter WHERE registrar_id=$1::bigint",
                        Database::query_param_list(rfd.reg_id));

                msg << (boost::format("Registrar %1% blocked: price limit %2%, current price: %3%, e-mail: %4%, phone: %5%, link: https://manager.nic.cz/registrar/detail/?id=%6% \n")
                        % it->first
                        % rfd.price_limit
                        % rfd.price
                        % res_contacts[0][0]
                        % res_contacts[0][1]
                        % rfd.reg_id).str()
                    << std::endl;

                //check if sendmail is present in the system
            }

            std::string cmd;
            if(blocked_registrars->begin() == blocked_registrars->end()) {
                cmd = (boost::format("{\n"
                        "echo \"Subject: No registrars blocked, date $(date +'%%Y-%%m-%%d')\n"
                        "Content-Type: text/plain; charset=UTF-8; format=flowed"
                        "\nContent-Transfer-Encoding: 8bit\n\n \";"
                        "\n} | /usr/sbin/sendmail %1%" ) % params.notify_email).str();

            } else {
                cmd = (boost::format("{\n"
              "echo \"Subject: REGISTRARS BLOCKED - requests over limit $(date +'%%Y-%%m-%%d')\n"
              "Content-Type: text/plain; charset=UTF-8; format=flowed"
              "\nContent-Transfer-Encoding: 8bit\n\n%1% \n\";"
              "\n} | /usr/sbin/sendmail %2%") % msg.str() % params.notify_email).str();
            }

            SubProcessOutput sub_output = ShellCmd(cmd, params.shell_cmd_timeout).execute();
            if (!sub_output.stderr.empty()) {
                throw std::runtime_error(sub_output.stderr);
            }

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
