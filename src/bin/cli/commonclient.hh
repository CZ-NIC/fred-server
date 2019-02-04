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

#ifndef COMMONCLIENT_HH_C591C9EB2DE14B0AA7E40F61B5089DB4
#define COMMONCLIENT_HH_C591C9EB2DE14B0AA7E40F61B5089DB4

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <string>

#include <boost/lexical_cast.hpp>

#include "src/bin/corba/nameservice.hh"
#include "util/types/datetime_interval.hh"
#include "util/types/date_interval.hh"
#include "src/deprecated/util/dbsql.hh"

#include "src/bin/corba/EPP.hh"

extern const char *corbaOpts[][2];
extern const char g_prog_name[];
extern const char g_version[];

class CorbaClient {
    CORBA::ORB_var orb;
    std::unique_ptr<NameService> ns;
public:
    CorbaClient(int argc, char **argv, const std::string &nshost, const std::string &nscontext)
    {
        orb = CORBA::ORB_init(argc, argv, "", corbaOpts);
        ns.reset(new NameService(orb, nshost, nscontext));
    }
    ~CorbaClient()
    {
        if (!CORBA::is_nil(orb))
            //orb->shutdown(0);
            orb->destroy();
    }
    NameService *getNS()
    {
        return ns.get();
    }
};

std::vector<std::string> separate(const std::string str, int ch = ' ');

#define DATETIME_FROM   1
#define DATETIME_TO     2

#define PARSE_DATETIME  1
#define PARSE_DATEONLY  2

std::string createDateTime(std::string datetime, int interval=DATETIME_FROM, int type=PARSE_DATEONLY);
Database::DateInterval *parseDate(std::string str);
Database::DateTimeInterval *parseDateTime(std::string str);

void print_version();
void print_moo();
void help_dates();

#endif
