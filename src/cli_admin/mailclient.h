/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
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

#ifndef _MAILCLIENT_H_
#define _MAILCLIENT_H_

#define MAIL_LIST_NAME              "mail-list"
#define MAIL_LIST_NAME_DESC         "list all mails"
#define MAIL_LIST_HELP_NAME         "mail-list-help"
#define MAIL_LIST_HELP_NAME_DESC    "help for mail list"

#define MAIL_STATUS_NAME            "status"
#define MAIL_STATUS_NAME_DESC       "status description"
#define MAIL_ATTEMPT_NAME           "attempt"
#define MAIL_ATTEMPT_NAME_DESC      "attempt description"
#define MAIL_MESSAGE_NAME           "message"
#define MAIL_MESSAGE_NAME_DESC      "message desc"
#define MAIL_ATTACHMENT_ID_NAME           "attachment-id"
#define MAIL_ATTACHMENT_ID_NAME_DESC      "attachment-id desc"
#define MAIL_ATTACHMENT_NAME_NAME         "attachment-name"
#define MAIL_ATTACHMENT_NAME_NAME_DESC    "attachment-name desc"

#include <iostream>
#include <boost/program_options.hpp>
#include "old_utils/dbsql.h"

#include "old_utils/log.h"
#include "old_utils/conf.h"
#include "register/register.h"
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"

namespace Admin {

class MailClient {
private:
    std::string m_connstring;
    std::string m_nsAddr;
    CORBA::Long m_clientId;
    DB m_db;
    Database::Manager *m_dbman;
    boost::program_options::variables_map m_varMap;
    ccReg::EPP_var m_epp;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
public:
    MailClient();
    MailClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~MailClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    void list();
    void list_help();
};

} // namespace Admin;

#endif // _MAILCLIENT_H_

