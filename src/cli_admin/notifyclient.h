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

#ifndef _NOTIFYCLIENT_H_
#define _NOTIFYCLIENT_H_

#include "util/hp/hpmail.h"
#include "util/subprocess.h"

#include <boost/program_options.hpp>
#include <iostream>


#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "fredlib/registry.h"
#include "baseclient.h"

#include "notify_params.h"

namespace Admin {

class NotifyClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;

    std::string nameservice_context;

    optional_string docgen_path;
    optional_string docgen_template_path;
    optional_string fileclient_path;
    bool restricted_handles;

    bool notify_state_changes;
    NotifyStateChangesArgs notify_state_changes_params;
    bool notify_letters_postservis_send;
    optional_string hpmail_config;
    bool notify_sms_send;
    optional_string cmdline_sms_command;
    optional_string configfile_sms_command;

    static const struct options m_opts[];
public:
    NotifyClient()
    : restricted_handles(false)
    , notify_state_changes(false)
    , notify_letters_postservis_send(false)
    , notify_sms_send(false)
    { }
    NotifyClient(
            const std::string &connstring
            , const std::string &nsAddr
            , const std::string& _nameservice_context
            , const optional_string& _docgen_path
            , const optional_string& _docgen_template_path
            , const optional_string& _fileclient_path
            , bool _restricted_handles
            , bool _notify_state_changes
            , const NotifyStateChangesArgs& _notify_state_changes_params
            , bool _notify_letters_postservis_send
            , const optional_string& _hpmail_config
            , bool _notify_sms_send
            , const optional_string& _cmdline_sms_command
            , const optional_string& _configfile_sms_command
            )
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , docgen_path(_docgen_path)
    , docgen_template_path(_docgen_template_path)
    , fileclient_path(_fileclient_path)
    , restricted_handles(_restricted_handles)
    , notify_state_changes(_notify_state_changes)
    , notify_state_changes_params(_notify_state_changes_params)
    , notify_letters_postservis_send(_notify_letters_postservis_send)
    , hpmail_config(_hpmail_config)
    , notify_sms_send(_notify_sms_send)
    , cmdline_sms_command(_cmdline_sms_command)
    , configfile_sms_command(_configfile_sms_command)
    {
        m_db = connect_DB(connstring
                , std::runtime_error("NotifyClient db connection failed"));
    }
    ~NotifyClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void state_changes();
    void letters_create();
    void letters_send();
    void sms_send();
    void file_send();
    void sendFile(const std::string &filename, const std::string &conf_file); 
    void sendLetters(std::auto_ptr<Fred::File::Transferer> fileman
            , const std::string &conf_file);
    void sendSMS(const std::string& command , const std::string& param_quote_by);
    HPCfgMap readHPConfig(const std::string &conf_file);
    void send_letters_impl(
        Fred::File::Transferer* fileman
        , const HPCfgMap& hpmail_config
        , Fred::Messages::LetterProcInfo& proc_letters
        , std::string& new_status
        , std::string& batch_id);

}; // class NotifyClient

void notify_registered_letters_manual_send_impl(const std::string& nameservice_host_port
        , const std::string& nameservice_context
        , const RegisteredLettersManualSendArgs& params
        );


} // namespace Admin;

#endif // _NOTIFYCLIENT_H_

