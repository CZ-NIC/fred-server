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
 *  @file corba_wrapper_decl.h
 *  declaration for CORBA wrapper class CorbaContainer
 */

#ifndef CORBA_WRAPPER_DECL_H_
#define CORBA_WRAPPER_DECL_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <memory>

#include <boost/utility.hpp>
#include <omniORB4/CORBA.h>

#include "corba/nameservice.h"

#include <boost/thread/thread.hpp>

struct OrbThread
{
    //OrbThread() {}
    void operator()();
};//struct OrbThread

typedef std::auto_ptr< boost::thread > ThreadPtr;

/**
 * \class CorbaContainer
 * \brief for manipulation with CORBA orb poa and nameservice
 */

class CorbaContainer : boost::noncopyable
{
    typedef std::auto_ptr<CorbaContainer> CorbaContainerPtr;
public:
    CORBA::ORB_var orb;
    CORBA::Object_var root_poa_initial_ref;
    PortableServer::POA_var root_poa;
    PortableServer::POAManager_var poa_mgr;
private:
    std::auto_ptr<NameService> ns_ptr;
public:
    PortableServer::POA_var poa_persistent;
private:
    static CorbaContainerPtr instance_ptr;

    CorbaContainer(int argc, char ** argv
            , const std::string& nameservice_host
            , unsigned nameservice_port
            , const std::string& nameservice_context);

    CorbaContainer(int& argc, char ** argv);

friend class std::auto_ptr<CorbaContainer>;
protected:
    ~CorbaContainer();
private:
    PortableServer::POA_var create_persistent_poa();

public:
    ///NameService resolve context/object name
    CORBA::Object_var nsresolve(
            const std::string& context, const std::string& object_name);

    ///NameService resolve object name using default context
    CORBA::Object_var nsresolve(const std::string& object_name);

    ///NameService resolve context/process/object name
    CORBA::Object_var nsresolve_process_object(
            const std::string& context
            , const std::string& process_name
            , const std::string& object_name);

    ///NameService resolve process/object name using default context
    CORBA::Object_var nsresolve_process_object(
            const std::string& process_name
            , const std::string& object_name);


    //set NameService after set_instance
    void setNameService ( const std::string& nameservice_host
    , unsigned nameservice_port
    , const std::string& nameservice_context);

    //static interface
    static void set_instance(int& argc , char** argv);
    static void set_instance(int argc , char** argv
            , const std::string& nameservice_host
            , unsigned nameservice_port
            , const std::string& nameservice_context);
    static CorbaContainer* get_instance();
    static void destroy_instance();

    NameService *getNS();

    //register server object with persistent poa and nameservice and with ownership transfer
    template <class T> void  register_server(T* new_server_object_ptr
            , const std::string& name)
    {
        PortableServer::ObjectId_var tObjectId
            = PortableServer::string_to_ObjectId(name.c_str());

        CorbaContainer::get_instance()->poa_persistent
            ->activate_object_with_id(tObjectId, new_server_object_ptr);

        CORBA::Object_var tObj = new_server_object_ptr->_this();
        new_server_object_ptr->_remove_ref();

        //register within corba nameservice
        CorbaContainer::get_instance()->getNS()->bind(name,tObj);
    }//register_server

    //register server object in process context with persistent poa
    //and nameservice and with ownership transfer
    template <class T> void  register_server_process_object(
            T* new_server_object_ptr
            , const std::string& process_name
            , const std::string& object_name)
    {
        PortableServer::ObjectId_var tObjectId
            = PortableServer::string_to_ObjectId(
                    (process_name+"-"+object_name).c_str());

        CorbaContainer::get_instance()->poa_persistent
            ->activate_object_with_id(tObjectId, new_server_object_ptr);

        CORBA::Object_var tObj = new_server_object_ptr->_this();
        new_server_object_ptr->_remove_ref();

        //register within corba nameservice
        CorbaContainer::get_instance()->getNS()
                ->bind_process_object(process_name, object_name,tObj);
    }//register_server_process_object

    ThreadPtr run_orb_thread();

};//class CorbaContainer


#endif // CORBA_WRAPPER_DECL_H_
