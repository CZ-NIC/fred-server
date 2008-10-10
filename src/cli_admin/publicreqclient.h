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

#ifndef _PUBLICREQCLIENT_H_
#define _PUBLICREQCLIENT_H_

#define PUBLICREQ_SHOW_OPTS_NAME        "public_request_show_opts"
#define PUBLICREQ_SHOW_OPTS_NAME_DESC   "show all public request command line options"
#define PUBLICREQ_LIST_NAME             "public_request_list"
#define PUBLICREQ_LIST_NAME_DESC        "list of all public requests"

#define PUBLICREQ_LIST_HELP_NAME        "public_request_list_name"
#define PUBLICREQ_LIST_HELP_NAME_DESC   "help for public request list"

#define PUBLICREQ_STATUS_NAME               "status"
#define PUBLICREQ_STATUS_NAME_DESC          "status description"
#define PUBLICREQ_ANSWER_EMAIL_NAME         "answer_email"
#define PUBLICREQ_ANSWER_EMAIL_NAME_DESC    "answer email description"
#define PUBLICREQ_ANSWER_EMAIL_ID_NAME      "answer_email_id"
#define PUBLICREQ_ANSWER_EMAIL_ID_NAME_DESC "answer email id description"
#define PUBLICREQ_REASON_NAME               "reason"
#define PUBLICREQ_REASON_NAME_DESC          "reason description"
#define PUBLICREQ_EPP_ID_NAME               "epp_id"
#define PUBLICREQ_EPP_ID_NAME_DESC          "epp id description"
#define PUBLICREQ_EPP_CLTRID_NAME           "epp_cltrid"
#define PUBLICREQ_EPP_CLTRID_NAME_DESC      "epp clTRID description"
#define PUBLICREQ_EPP_SVTRID_NAME           "epp_svtrid"
#define PUBLICREQ_EPP_SVTRID_NAME_DESC      "epp svTRID description"
#define PUBLICREQ_EPP_CODE_RESPONSE_NAME    "epp_code_response"
#define PUBLICREQ_EPP_CODE_RESPONSE_NAME_DESC "epp_code_response description"
#define PUBLICREQ_EPP_RESPONSE_NAME         "epp_response"
#define PUBLICREQ_EPP_RESPONSE_NAME_DESC    "epp response description"
#define PUBLICREQ_EPP_TYPE_NAME             "epp_type"
#define PUBLICREQ_EPP_TYPE_NAME_DESC        "epp type description"

#define PUBLICREQ_RESDATE_FROM_NAME         "res_date_from"
#define PUBLICREQ_RESDATE_FROM_NAME_DESC    "response datetime from"
#define PUBLICREQ_RESDATE_TO_NAME           "res_date_to"
#define PUBLICREQ_RESDATE_TO_NAME_DESC      "response datetime to"
#define PUBLICREQ_TYPE_NAME                 "public_request_type"
#define PUBLICREQ_TYPE_NAME_DESC            "......."

#include <iostream>
#include <boost/program_options.hpp>
#include "old_utils/dbsql.h"

#include "register/register.h"
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"

namespace Admin {

class PublicRequestClient {
private:
    std::string m_connstring;
    std::string m_nsAddr;
    CORBA::Long m_clientId;
    DB m_db;
    Database::Manager *m_dbman;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
public:
    PublicRequestClient();
    PublicRequestClient(std::string connstring,
            std::string nsAddr);
    ~PublicRequestClient();
    void init(std::string connstring,
            std::string nsAddr,
            Config::Conf &conf);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;
    void show_opts() const;

    void list();
    void list_help();
};

} // namespace Admin;

#endif // _PUBLICREQCLIENT_H_

