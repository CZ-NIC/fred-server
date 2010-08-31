/*  
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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

/**
 *  @file corba_wrapper.h
 *  manipulation with common CORBA instances
 */

#ifndef CORBA_WRAPPER_H_
#define CORBA_WRAPPER_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <memory>

#include <omniORB4/CORBA.h>
#include "corba/nameservice.h"

#include "corba_wrapper_decl.h"

//static init
CorbaContainer::CorbaContainerPtr CorbaContainer::instance_ptr(0);

//impl
CorbaContainer::CorbaContainer(int argc, char ** argv
        , const std::string& nameservice_host
        , unsigned nameservice_port
        , const std::string& nameservice_context)
: orb(CORBA::ORB_init( argc,argv))
, root_poa_initial_ref(orb->resolve_initial_references("RootPOA"))
, root_poa(PortableServer::POA::_narrow(root_poa_initial_ref))
, poa_mgr(root_poa->the_POAManager())
, ns(orb, nameservice_host, nameservice_port, nameservice_context)
{
    //poa for persistent refs
    CORBA::PolicyList pols;
    pols.length(2);
    pols[0] = root_poa->create_lifespan_policy(PortableServer::PERSISTENT);
    pols[1] = root_poa->create_id_assignment_policy(PortableServer::USER_ID);
    poa_persistent = root_poa->create_POA("RegistryPOA", poa_mgr.in(), pols);
}

CorbaContainer::~CorbaContainer()
{
    orb->destroy();
}

///NameService resolve with simple
CORBA::Object_var CorbaContainer::nsresolve(const std::string& context, const std::string& object_name)
{
    return ns.resolve(context, object_name);
}

///NameService resolve using default context
CORBA::Object_var CorbaContainer::nsresolve(const std::string& object_name)
{
    return ns.resolve(object_name);
}

NameService* CorbaContainer::getNS()
{
    return &ns;
}

void CorbaContainer::set_instance(int argc, char** argv
        , const std::string& nameservice_host
        , unsigned nameservice_port
        , const std::string& nameservice_context)
{
    CorbaContainerPtr tmp_instance(new CorbaContainer
            (argc, argv, nameservice_host, nameservice_port, nameservice_context)
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

#endif //CORBA_WRAPPER_H_
