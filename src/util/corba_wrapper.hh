/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file corba_wrapper.h
 *  manipulation with common CORBA instances
 */

#ifndef CORBA_WRAPPER_HH_6CC50150E4C742CC81BC5922DB214312
#define CORBA_WRAPPER_HH_6CC50150E4C742CC81BC5922DB214312

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>

#include <omniORB4/CORBA.h>
#include "src/bin/corba/nameservice.hh"

#include "src/util/corba_wrapper_decl.hh"

//static init
CorbaContainer::CorbaContainerPtr CorbaContainer::instance_ptr;

void OrbThread::operator ()() //run orb in thread
{
    CorbaContainer::get_instance()->orb->run();
}//OrbThr::operator()



//impl

ThreadPtr CorbaContainer::run_orb_thread()
{
    return std::make_unique<boost::thread>(OrbThread());
}

PortableServer::POA_var CorbaContainer::create_persistent_poa()
{
    //poa for persistent refs
    CORBA::PolicyList pols;
    pols.length(2);
    pols[0] = root_poa->create_lifespan_policy(PortableServer::PERSISTENT);
    pols[1] = root_poa->create_id_assignment_policy(PortableServer::USER_ID);

    PortableServer::POA_var persistent_poa = root_poa->create_POA("RegistryPOA", poa_mgr.in(), pols);
    return persistent_poa;
}

CorbaContainer::CorbaContainer(int argc, char ** argv
        , const std::string& nameservice_host
        , unsigned nameservice_port
        , const std::string& nameservice_context)
: orb(CORBA::ORB_init( argc,argv))
, root_poa_initial_ref(orb->resolve_initial_references("RootPOA"))
, root_poa(PortableServer::POA::_narrow(root_poa_initial_ref))
, poa_mgr(root_poa->the_POAManager())
, ns_ptr(new NameService(orb, nameservice_host, nameservice_port, nameservice_context))
{
    //poa for persistent refs
    poa_persistent = create_persistent_poa();
}

CorbaContainer::CorbaContainer(int& argc, char ** argv)
: orb(CORBA::ORB_init( argc,argv))
, root_poa_initial_ref(orb->resolve_initial_references("RootPOA"))
, root_poa(PortableServer::POA::_narrow(root_poa_initial_ref))
, poa_mgr(root_poa->the_POAManager())
, ns_ptr(nullptr)
{
    //poa for persistent refs
    poa_persistent = create_persistent_poa();
}

void CorbaContainer::setNameService ( const std::string& nameservice_host
, unsigned nameservice_port
, const std::string& nameservice_context)
{
    ns_ptr.reset( new NameService(orb, nameservice_host, nameservice_port, nameservice_context));
}



CorbaContainer::~CorbaContainer()
{
    orb->destroy();
}

CORBA::Object_var CorbaContainer::nsresolve(const std::string& context, const std::string& object_name)
{
    if(ns_ptr.get() == 0)
        throw std::runtime_error(
                "CorbaContainer::nsresolve error: NameService not set");

    return ns_ptr->resolve(context, object_name);
}

CORBA::Object_var CorbaContainer::nsresolve(const std::string& object_name)
{
    if(ns_ptr.get() == 0)
        throw std::runtime_error(
                "CorbaContainer::nsresolve error: NameService not set");

    return ns_ptr->resolve(object_name);
}


CORBA::Object_var CorbaContainer::nsresolve_process_object(
        const std::string& context
        , const std::string& process_name
        , const std::string& object_name)
{
    if(ns_ptr.get() == 0)
        throw std::runtime_error(
                "CorbaContainer::nsresolve_process_object error: NameService not set");

    return ns_ptr->resolve_process_object(context, process_name, object_name);
}

CORBA::Object_var CorbaContainer::nsresolve_process_object(
        const std::string& process_name
        , const std::string& object_name)
{
    if(ns_ptr.get() == 0)
        throw std::runtime_error(
                "CorbaContainer::nsresolve_process_object error: NameService not set");

    return ns_ptr->resolve_process_object(process_name, object_name);
}


NameService* CorbaContainer::getNS()
{
    if(ns_ptr.get() == 0)
        throw std::runtime_error(
                "CorbaContainer::getNS error: NameService not set");

    return ns_ptr.get();
}

void CorbaContainer::set_instance(int argc, char** argv
        , const std::string& nameservice_host
        , unsigned nameservice_port
        , const std::string& nameservice_context)
{
    destroy_instance();
    CorbaContainerPtr tmp_instance(new CorbaContainer
            (argc, argv, nameservice_host, nameservice_port, nameservice_context)
    );

    instance_ptr = std::move(tmp_instance);
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

#endif
