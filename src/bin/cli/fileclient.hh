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

#ifndef FILECLIENT_HH_A6D880632E134B91850CFBC572CF2A46
#define FILECLIENT_HH_A6D880632E134B91850CFBC572CF2A46

#include <iostream>
#include <boost/program_options.hpp>


#include "src/deprecated/libfred/registry.hh"
#include "src/bin/corba/admin/admin_impl.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/bin/cli/baseclient.hh"

#include "src/bin/cli/file_params.hh"

namespace Admin {

class FileClient : public BaseClient {
private:
    ccReg::EPP_var m_epp;
    std::string nameservice_context;
    bool file_list;
    FileListArgs file_list_params;
    static const struct options m_opts[];
public:
    FileClient()
    : file_list(false)
    { }
    FileClient(
            const std::string &connstring
            , const std::string &nsAddr
            , const std::string& _nameservice_context
            , const bool _file_list
            , const FileListArgs& _file_list_params
            )
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , file_list(_file_list)
    , file_list_params(_file_list_params)
    { }
    ~FileClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void list();
    void list_help();
}; // class FileClient

} // namespace Admin;

#endif // _FILECLIENT_H_

