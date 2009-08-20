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

#ifndef _PUBLICREQCLIENT_H_
#define _PUBLICREQCLIENT_H_

#define PUBLICREQ_SHOW_OPTS_NAME        "public_request_show_opts"
#define PUBLICREQ_SHOW_OPTS_NAME_DESC   "show all public request command line options"
#define PUBLICREQ_LIST_NAME             "public_request_list"
#define PUBLICREQ_LIST_NAME_DESC        "list of all public requests"

#define PUBLICREQ_LIST_HELP_NAME        "public_request_list_help"
#define PUBLICREQ_LIST_HELP_NAME_DESC   "help for public request list"

#define PUBLICREQ_STATUS_NAME               "status"
#define PUBLICREQ_STATUS_NAME_DESC          "status description"
#define PUBLICREQ_ANSWER_EMAIL_NAME         "answer_email"
#define PUBLICREQ_ANSWER_EMAIL_NAME_DESC    "answer email description"
#define PUBLICREQ_ANSWER_EMAIL_ID_NAME      "answer_email_id"
#define PUBLICREQ_ANSWER_EMAIL_ID_NAME_DESC "answer email id description"
#define PUBLICREQ_REASON_NAME               "reason"
#define PUBLICREQ_REASON_NAME_DESC          "reason description"
#define PUBLICREQ_REGISTRAR_ID_NAME	    "registrar id"	
#define PUBLICREQ_REGISTRAR_ID_NAME_DESC    "registrar id description"	
#define PUBLICREQ_REGISTRAR_HANDLE_NAME	    "registrar handle"
#define PUBLICREQ_REGISTRAR_HANDLE_NAME_DESC "registrar handle description"
#define PUBLICREQ_REGISTRAR_NAME_NAME	    "registrar name"
#define PUBLICREQ_REGISTRAR_NAME_NAME_DESC  "registrar name description"

#define PUBLICREQ_RESDATE_NAME              "resdate"
#define PUBLICREQ_RESDATE_NAME_DESC         "response date"
#define PUBLICREQ_TYPE_NAME                 "public_request_type"
#define PUBLICREQ_TYPE_NAME_DESC            "......."

#include <iostream>
#include <boost/program_options.hpp>
#include "old_utils/dbsql.h"

#include "register/register.h"
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"
#include "baseclient.h"


namespace Admin {

class PublicRequestClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    PublicRequestClient()
    { }
    PublicRequestClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
    }
    ~PublicRequestClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void list();
    void list_help();
}; // class PublicRequestClient

} // namespace Admin;

#endif // _PUBLICREQCLIENT_H_

