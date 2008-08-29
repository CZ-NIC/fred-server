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

#ifndef _INFOBUFFCLIENT_H_
#define _INFOBUFFCLIENT_H_

#define INFOBUFF_MAKE_INFO_NAME     "infobuff-make-info"
#define INFOBUFF_GET_CHUNK_NAME     "infobuff-get-chunk"
#define INFOBUFF_REQUEST_NAME       "infobuff-request"

#include <iostream>
#include <boost/program_options.hpp>
#include "old_utils/dbsql.h"

#include "old_utils/log.h"
#include "old_utils/conf.h"
#include "register/register.h"
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"

namespace Admin {

class InfoBuffClient {
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
    InfoBuffClient();
    InfoBuffClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~InfoBuffClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    int make_info();
    int get_chunk();

};

} // namespace Admin;

#endif // _INFOBUFFCLIENT_H_

