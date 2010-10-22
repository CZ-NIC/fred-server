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

#define NOTIFY_SHOW_OPTS_NAME           "notify_show_opts"
#define NOTIFY_SHOW_OPTS_NAME_DESC      "show all notify command line options"
#define NOTIFY_STATE_CHANGES_NAME       "notify_state_changes"
#define NOTIFY_STATE_CHANGES_NAME_DESC  "send emails to contacts about object state changes"
#define NOTIFY_LETTERS_CREATE_NAME      "notify_letters_create"
#define NOTIFY_LETTERS_CREATE_NAME_DESC "generate pdf with domain registration warning"
#define NOTIFY_LETTERS_SEND_NAME        "notify_letters_postservis_send"
#define NOTIFY_LETTERS_SEND_NAME_DESC   "send generated PDF notification letters to postservis"
#define NOTIFY_SMS_SEND_NAME            "notify_sms_send"
#define NOTIFY_SMS_SEND_NAME_DESC       "send generated SMS notification messages"
#define NOTIFY_FILE_SEND_NAME           "notify_send_file"
#define NOTIFY_FILE_SEND_NAME_DESC      "send specified PDF file via postservis"

#define NOTIFY_EXCEPT_TYPES_NAME        "notify_except_types"
#define NOTIFY_EXCEPT_TYPES_NAME_DESC   "list of notification types ignored in notification"
#define NOTIFY_USE_HISTORY_TABLES_NAME  "notify_use_history_tables"
#define NOTIFY_USE_HISTORY_TABLES_NAME_DESC "slower queries into historic tables, but can handle deleted objects"
#define NOTIFY_LIMIT_NAME               "notify_limit"
#define NOTIFY_LIMIT_NAME_DESC          "limit for nubmer of emails generated in one pass (0=no limit)"
#define NOTIFY_DEBUG_NAME               "notify_debug"
#define NOTIFY_DEBUG_NAME_DESC          "debug"
#define NOTIFY_HPMAIL_CONFIG_NAME       "hpmail_config"
#define NOTIFY_HPMAIL_CONFIG_NAME_DESC  "Configuration file for Postservis client (hpmail)"
#define NOTIFY_SMS_COMMAND_NAME         "sms_command"
#define NOTIFY_SMS_COMMAND_NAME_DESC    "Command to send saved sms messages"

#include <boost/program_options.hpp>
#include <iostream>


#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "register/register.h"
#include "baseclient.h"

namespace Admin {

class NotifyClient : public BaseClient {
private:
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    NotifyClient()
    { }
    NotifyClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
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
    void sendLetters(std::auto_ptr<Register::File::Transferer> fileman
            , const std::string &conf_file);
    void sendSMS(const std::string& command , const std::string& param_quote_by);
    HPCfgMap readHPConfig(const std::string &conf_file);
    void send_letters_impl(
        Register::File::Transferer* fileman
        , const HPCfgMap& hpmail_config
        , Register::Messages::LetterProcInfo& proc_letters
        , std::string& new_status
        , std::string& batch_id);

}; // class NotifyClient

} // namespace Admin;

#endif // _NOTIFYCLIENT_H_

