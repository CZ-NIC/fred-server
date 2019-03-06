/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
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
#ifndef NAMESERVICE_HH_DB9E9C5A05B14A618530FC9F2EF96FB2
#define NAMESERVICE_HH_DB9E9C5A05B14A618530FC9F2EF96FB2

#include <string>
#include <omniORB4/CORBA.h>
#include <stdexcept>

class NameService
{
private:
  CORBA::ORB_ptr               orb;
  CosNaming::NamingContext_var rootContext;
  std::string                  hostname;
  std::string                  context;

public:
  /**
   * exception when NameService not running 
   * or not in omniORB config file
   */
  struct NOT_RUNNING : public std::runtime_error
  {
  public:
    NOT_RUNNING()
        : std::runtime_error("CORBA NameService not running")
    {};
  };

  /**
   * exception when cannot bind context 
   */
  struct BAD_CONTEXT : public std::runtime_error
  {
  public:
      BAD_CONTEXT()
         : std::runtime_error("CORBA cannot bind context")
    {};
  };


  /**
   * config rootContext from supplied orb
   */
  NameService(CORBA::ORB_ptr orb) ;//throw (NOT_RUNNING);

  /**
   * config rootContext from supplied orb and hostname
   */
  NameService(CORBA::ORB_ptr orb, 
              const std::string& hostname, 
              const std::string& _context);// throw (NOT_RUNNING);

  /**
   * config rootContext from supplied orb and hostname and port
   */
  NameService(CORBA::ORB_ptr orb, 
              const std::string& hostname, 
              const unsigned int _port,
              const std::string& _context);// throw (NOT_RUNNING);

  /**
   * d-tor
   */
  virtual ~NameService();

  /**
   * bind object into nameservice.context/'name'.Object
   */
  void bind(const std::string& name, 
            CORBA::Object_ptr objref);// throw (NOT_RUNNING, BAD_CONTEXT);

  /**
   * bind object into nameservice.context/process.context/'name'.Object
   */
  void bind_process_object( const std::string& process_context_name,
            const std::string& object_name,
            CORBA::Object_ptr objref);


  /**
   * resolve object from nameservice.context/'name'.Object get IOR
   */
  CORBA::Object_ptr resolve(const std::string& name);// throw (NOT_RUNNING, BAD_CONTEXT);

  /**
   * resolve object from 'context'.context/'name'.Object get IOR
   */
  CORBA::Object_ptr resolve(const std::string& context,
                            const std::string& name);// throw (NOT_RUNNING, BAD_CONTEXT);

  /**
   * resolve object from nameservice.context/'process'.context/'name'.Object get IOR
   */
  CORBA::Object_ptr resolve_process_object(const std::string& process_context
          , const std::string& name);

  /**
   * resolve object from 'context'.context/'process'.context/'name'.Object get IOR
   */
  CORBA::Object_ptr resolve_process_object(const std::string& root_context,
                             const std::string& process_context,
                            const std::string& object_name);

  /**
   * get nameservice hostname used
   */
  const std::string& getHostName();


private:
  void _connect();// throw (NOT_RUNNING);

};

#endif /*NAMESERVICE_H_*/
