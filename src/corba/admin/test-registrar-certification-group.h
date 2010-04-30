/*
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEST_REGISTRAR_CERTIFICATION_GROUP_H_
#define TEST_REGISTRAR_CERTIFICATION_GROUP_H_

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <boost/format.hpp>
#include "register/db_settings.h"
#include "log/logger.h"
#include "log/context.h"
#include <boost/test/included/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/version.hpp>
#include <fstream>
#include <queue>
#include <sys/time.h>
#include <time.h>
#include <boost/thread/barrier.hpp>
#include <boost/date_time.hpp>

#include <iostream>
#include <map>
#include <exception>
#include <corba/ccReg.hh>

using namespace std;

struct corba_container
{
    //here may be useful mutex
    CORBA::ORB_var orb;
    CORBA::Object_var root_initial_ref;
    PortableServer::POA_var poa;
    CORBA::Object_var nameservice_ref;
    CosNaming::NamingContext_var root_nameservice_context;
    ccReg::Admin_var admin_ref;
    Registry::Registrar::Group::Manager_var group_manager;
    Registry::Registrar::Certification::Manager_var cert_manager;
};//struct corba_container

class CorbaSingleton
{
private:
    static CorbaSingleton* instance_ptr;
    CorbaSingleton(){}
    ~CorbaSingleton(){}
public:
    static CorbaSingleton* instance();
    corba_container cc;
};//class CorbaSingleton

CorbaSingleton* CorbaSingleton::instance_ptr= 0;

CorbaSingleton* CorbaSingleton::instance()
{//first call from single thread
    if(instance_ptr == 0)
        instance_ptr = new CorbaSingleton();
    return instance_ptr;
}




unsigned registrar_certification_test()
{

    return 0;
}


#endif // TEST_REGISTRAR_CERTIFICATION_GROUP_H_
