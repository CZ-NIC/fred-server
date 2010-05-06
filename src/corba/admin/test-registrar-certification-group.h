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
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/version.hpp>
#include <boost/utility.hpp>
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

class CorbaContainer : boost::noncopyable
{
public:
    typedef std::auto_ptr<CorbaContainer> CorbaContainerPtr;
private:
    int argc;
    CORBA::ORB_var orb;
    CORBA::Object_var root_poa_initial_ref;
    PortableServer::POA_var root_poa;
    std::string name_service_corbaname;
    std::string default_name_service_context;
    CORBA::Object_var nameservice_initial_ref;
    CosNaming::NamingContext_var root_nameservice_context;
    static CorbaContainerPtr instance_ptr;

    CorbaContainer(FakedArgs fa
            , const std::string& nameservice_host
            , unsigned nameservice_port
            , const std::string& nameservice_context)
    : argc(fa.get_argc())
    , orb(CORBA::ORB_init( argc,fa.get_argv()))
    , root_poa_initial_ref(orb->resolve_initial_references("RootPOA"))
    , root_poa(PortableServer::POA::_narrow(root_poa_initial_ref))
    , name_service_corbaname(nameservice_host.empty() ? std::string("")
        : "corbaname::" + nameservice_host + ":"
          + boost::lexical_cast<std::string>(nameservice_port))
    , default_name_service_context(nameservice_context)
    , nameservice_initial_ref(name_service_corbaname.empty()
        ? orb->resolve_initial_references("NameService")
        : orb->string_to_object(name_service_corbaname.c_str()))
    , root_nameservice_context(CosNaming::NamingContext::_narrow(
        nameservice_initial_ref.in()))
    {}
friend class std::auto_ptr<CorbaContainer>;
protected:
    ~CorbaContainer()
    {
        orb->destroy();
    }
public:
    ///NameService resolve with simple
    CORBA::Object_var nsresolve(const std::string& context, const std::string& object_name)
    {
        CORBA::Object_var obj;

        CosNaming::Name context_name;
        context_name.length(2);
        context_name[0].id   = context.c_str();
        context_name[0].kind = "context";
        context_name[1].id   = object_name.c_str();
        context_name[1].kind = "Object";
        obj = root_nameservice_context->resolve(context_name);
        return obj;
    }
    ///NameService resolve using default context
    CORBA::Object_var nsresolve(const std::string& object_name)
    {
        return nsresolve(default_name_service_context, object_name);
    }

    //static interface
    static void set_instance(FakedArgs fa
            , const std::string& nameservice_host
            , unsigned nameservice_port
            , const std::string& nameservice_context);
    static CorbaContainer* get_instance();
    static void destroy_instance();

};//class CorbaContainer

//static init
CorbaContainer::CorbaContainerPtr CorbaContainer::instance_ptr(0);

void CorbaContainer::set_instance(FakedArgs fa
        , const std::string& nameservice_host
        , unsigned nameservice_port
        , const std::string& nameservice_context)
{
    CorbaContainerPtr tmp_instance(new CorbaContainer
            (fa,nameservice_host,nameservice_port, nameservice_context)
    );

    instance_ptr = tmp_instance;
}

CorbaContainer* CorbaContainer::get_instance()
{
    CorbaContainer* ret = instance_ptr.get();
    if (ret == 0)
        throw std::runtime_error("error: CorbaContainer instance not set");
    return ret;
}

void CorbaContainer::destroy_instance()
{
    instance_ptr.reset(0);
}

unsigned registrar_certification_test()
{
    return 0;
}


#endif // TEST_REGISTRAR_CERTIFICATION_GROUP_H_
