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

#ifndef _AUTHINFOCLIENT_H_
#define _AUTHINFOCLIENT_H_

#define AUTHINFO_PDF_NAME         "authinfopdf"
#define AUTHINFO_PDF_NAME_DESC    "generate pdf of authorization info request"

#define AUTHINFO_PDF_HELP_NAME    "authinfopdf_help"
#define AUTHINFO_PDF_HELP_NAME_DESC "help for authinfo pdf creation"

#define AUTHINFO_SHOW_OPTS_NAME     "authinfo_show_opts"
#define AUTHINFO_SHOW_OPTS_NAME_DESC "show all authinfo command line options"


#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "baseclient.h"

namespace Admin {

class AuthInfoClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
public:
    AuthInfoClient();
    AuthInfoClient(std::string connstring,
            std::string nsAddr);
    ~AuthInfoClient();
    void init(std::string connstring,
            std::string nsAddr,
            Config::Conf &conf);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;
    void show_opts() const;

    int pdf();

    void pdf_help();

    //int list();
};

} // namespace Admin;

#endif // _AUTHINFOCLIENT_H_
