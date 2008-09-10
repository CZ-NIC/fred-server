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

#define PUBLICREQ_LIST_NAME             "public-request-list"
#define PUBLICREQ_LIST_NAME_DESC        "list of all public requests"

#define PUBLICREQ_LIST_HELP_NAME        "public-request-list-name"
#define PUBLICREQ_LIST_HELP_NAME_DESC   "help for public request list"

#define PUBLICREQ_STATUS_NAME               "status"
#define PUBLICREQ_STATUS_NAME_DESC          "status description"
#define PUBLICREQ_ANSWER_EMAIL_NAME         "answer-email"
#define PUBLICREQ_ANSWER_EMAIL_NAME_DESC    "answer email description"
#define PUBLICREQ_ANSWER_EMAIL_ID_NAME      "answer-email-id"
#define PUBLICREQ_ANSWER_EMAIL_ID_NAME_DESC "answer email id description"
#define PUBLICREQ_REASON_NAME               "reason"
#define PUBLICREQ_REASON_NAME_DESC          "reason description"
#define PUBLICREQ_EPP_ID_NAME               "epp-id"
#define PUBLICREQ_EPP_ID_NAME_DESC          "epp id description"
#define PUBLICREQ_EPP_CLTRID_NAME           "epp-cltrid"
#define PUBLICREQ_EPP_CLTRID_NAME_DESC      "epp clTRID description"
#define PUBLICREQ_EPP_SVTRID_NAME           "epp-svtrid"
#define PUBLICREQ_EPP_SVTRID_NAME_DESC      "epp svTRID description"
#define PUBLICREQ_EPP_CODE_RESPONSE_NAME    "epp-code-response"
#define PUBLICREQ_EPP_CODE_RESPONSE_NAME_DESC "epp-code-response description"
#define PUBLICREQ_EPP_RESPONSE_NAME         "epp-response"
#define PUBLICREQ_EPP_RESPONSE_NAME_DESC    "epp response description"
#define PUBLICREQ_EPP_TYPE_NAME             "epp-type"
#define PUBLICREQ_EPP_TYPE_NAME_DESC        "epp type description"

#define PUBLICREQ_RESDATE_FROM_NAME         "res-date-from"
#define PUBLICREQ_RESDATE_FROM_NAME_DESC    "response datetime from"
#define PUBLICREQ_RESDATE_TO_NAME           "res-date-to"
#define PUBLICREQ_RESDATE_TO_NAME_DESC      "response datetime to"

#include <iostream>
#include <boost/program_options.hpp>
#include "old_utils/dbsql.h"

#include "old_utils/log.h"
#include "old_utils/conf.h"
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
    boost::program_options::variables_map m_varMap;
    ccReg::EPP_var m_epp;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
public:
    PublicRequestClient();
    PublicRequestClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~PublicRequestClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    void list();
    void list_help();
};

} // namespace Admin;

#endif // _PUBLICREQCLIENT_H_

