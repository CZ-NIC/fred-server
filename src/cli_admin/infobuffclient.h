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

#ifndef _INFOBUFFCLIENT_H_
#define _INFOBUFFCLIENT_H_

#define INFOBUFF_SHOW_OPTS_NAME         "info_buffer_show_opts"
#define INFOBUFF_SHOW_OPTS_NAME_DESC    "show all info buffer command line options"
#define INFOBUFF_MAKE_INFO_NAME         "info_buffer_make_info"
#define INFOBUFF_MAKE_INFO_NAME_DESC    "invoke generation if list of o for given registrar"
#define INFOBUFF_GET_CHUNK_NAME         "info_buffer_get_chunk"
#define INFOBUFF_GET_CHUNK_NAME_DESC    "output chunk of buffer for given registrar"
#define INFOBUFF_REQUEST_NAME           "info_buffer_request"
#define INFOBUFF_REQUEST_NAME_DESC      "handle for query"
#define INFOBUFF_REGISTRAR_NAME         "info_buffer_registrar"
#define INFOBUFF_REGISTRAR_NAME_DESC    "id of registrar for buffer selection"

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "baseclient.h"

namespace Admin {

class InfoBuffClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    InfoBuffClient()
    { }
    InfoBuffClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        DB* db= new DB;
        if (!db->OpenDatabase(connstring.c_str()))
         {
            throw std::runtime_error("InfoBuffClient db connection failed");
         }
        m_db = DBDisconnectPtr(db);
    }
    ~InfoBuffClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts() ;
    void make_info();
    void get_chunk();

}; // class InfoBuffClient

} // namespace Admin;

#endif // _INFOBUFFCLIENT_H_

