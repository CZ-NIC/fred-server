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

#ifndef _COMMONCLIENT_H_
#define _COMMONCLIENT_H_

#include <iostream>
#include <vector>
#include <memory>

#include "corba/nameservice.h"

extern const char *corbaOpts[][2];
extern const char g_prog_name[];
extern const char g_version[];

class CorbaClient {
    CORBA::ORB_var orb;
    std::auto_ptr<NameService> ns;
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
std::vector<std::string> separateSpaces(const char *string);

void print_version();
void print_moo();

#endif // _COMMONCLIENT_H_
