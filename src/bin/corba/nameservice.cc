/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

#include <iostream>
#include <boost/lexical_cast.hpp>

#include "src/bin/corba/nameservice.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"

void NameService::_connect()// throw (NOT_RUNNING)
{
  try {
    CORBA::Object_var obj;

    if (hostname.empty()) {
      obj = orb->resolve_initial_references("NameService");
    }
    else {
      obj = orb->string_to_object(("corbaname::" + hostname).c_str());
    }

    rootContext = CosNaming::NamingContext::_narrow(obj);

    if (CORBA::is_nil(rootContext)) {
      throw NOT_RUNNING();
    }
  }
  catch (...) {
    throw NOT_RUNNING();
  }
}


NameService::NameService(CORBA::ORB_ptr orb)// throw (NOT_RUNNING)
                        : orb(orb),
                          hostname(""),
                          context("")
{
    _connect();
}


NameService::NameService(CORBA::ORB_ptr orb,
                         const std::string& _hostname,
                         const std::string& _context)// throw (NOT_RUNNING)
                                                      : orb(orb),
                                                        hostname(_hostname),
                                                        context(_context)
{
    _connect();
}


NameService::NameService(CORBA::ORB_ptr orb,
                         const std::string& _hostname,
                         const unsigned int _port,
                         const std::string& _context)// throw (NOT_RUNNING)
                        : orb(orb),
                          hostname(_hostname + ":" + boost::lexical_cast<std::string>(_port)),
                          context(_context) 
{
    _connect();
}


NameService::~NameService() {
}


void NameService::bind(const std::string& name, 
                       CORBA::Object_ptr objref)// throw (NOT_RUNNING, BAD_CONTEXT)
{
  try {
    Logging::Context ctx("nameservice");

    LOGGER.debug(boost::format("requested object bind: %1%/%2%")
                                        % context
                                        % name);

    CosNaming::Name contextName;
    contextName.length(1);
    contextName[0].id   = (const char*) context.c_str(); 
    contextName[0].kind = (const char*) "context";
    CosNaming::NamingContext_var testContext;
    try {
      /* Bind the context to root */
      testContext = rootContext->bind_new_context(contextName);
    }
    catch (CosNaming::NamingContext::AlreadyBound& ex) {
      /**
       * If the context already exists, this exception will be raised.
       * In this case, just resolve the name and assign testContext
       * to the object returned
       */
      CORBA::Object_var obj;
      obj = rootContext->resolve(contextName);
      testContext = CosNaming::NamingContext::_narrow(obj);
      if (CORBA::is_nil(testContext)) throw BAD_CONTEXT();
    }
    CosNaming::Name objectName;
    objectName.length(1);
    objectName[0].id   = name.c_str();
    objectName[0].kind = (const char*) "Object";

    try {
      testContext->bind(objectName, objref);
   }
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      testContext->rebind(objectName, objref);
    }
  }
  catch(CORBA::TRANSIENT& ex) {
    throw NOT_RUNNING();
  }
  catch(CORBA::SystemException& ex) {
    throw NOT_RUNNING();
  }
}

void NameService::bind_process_object( const std::string& process_context_name,
            const std::string& object_name,
            CORBA::Object_ptr objref)
{
    try {
      Logging::Context ctx("nameservice");

      LOGGER.debug(boost::format("requested object bind: %1%/%2%/%3%")
                                      % context % process_context_name % object_name);

      //Fred
      CosNaming::Name context1Name;
      context1Name.length(1);
      context1Name[0].id   = (const char*) context.c_str();
      context1Name[0].kind = (const char*) "context";
      CosNaming::NamingContext_var test1Context;
      try {
        // Bind the context to root
        test1Context = rootContext->bind_new_context(context1Name);
      }
      catch (CosNaming::NamingContext::AlreadyBound& ex)
      {
        //
        // If the context already exists, this exception will be raised.
        // In this case, just resolve the name and assign testContext
        // to the object returned
        ///
        CORBA::Object_var obj;
        obj = rootContext->resolve(context1Name);
        test1Context = CosNaming::NamingContext::_narrow(obj);
        if (CORBA::is_nil(test1Context)) throw BAD_CONTEXT();
      }//catch (CosNaming::NamingContext::AlreadyBound& ex)

      //process
      CosNaming::Name context2Name;
      context2Name.length(1);
      context2Name[0].id   = (const char*) process_context_name.c_str();
      context2Name[0].kind = (const char*) "context";
      CosNaming::NamingContext_var test2Context;
      try {
        // Bind the context to root
        test2Context = test1Context->bind_new_context(context2Name);
      }
      catch (CosNaming::NamingContext::AlreadyBound& ex)
      {
        ///
        // If the context already exists, this exception will be raised.
        // In this case, just resolve the name and assign testContext
        // to the object returned
        ////
        CORBA::Object_var obj;
        obj = test1Context->resolve(context2Name);
        test2Context = CosNaming::NamingContext::_narrow(obj);
        if (CORBA::is_nil(test2Context)) throw BAD_CONTEXT();
      }//catch (CosNaming::NamingContext::AlreadyBound& ex)

      //object
      CosNaming::Name objectName;
      objectName.length(1);
      objectName[0].id   = object_name.c_str();
      objectName[0].kind = (const char*) "Object";
      try {
        test2Context->bind(objectName, objref);
     }
      catch(CosNaming::NamingContext::AlreadyBound& ex)
      {
        test2Context->rebind(objectName, objref);
      }//catch(CosNaming::NamingContext::AlreadyBound& ex)

    }
    catch(CORBA::TRANSIENT& ex) {
      throw NOT_RUNNING();
    }
    catch(CORBA::SystemException& ex) {
      throw NOT_RUNNING();
    }
}//NameService::bind_process_object


CORBA::Object_ptr NameService::resolve(const std::string& name)
    //throw (NOT_RUNNING, BAD_CONTEXT)
{
    return resolve(context, name);
}


CORBA::Object_ptr NameService::resolve(const std::string& nsctx,
                                       const std::string& name)
    //throw (NOT_RUNNING, BAD_CONTEXT)
{

  Logging::Context ctx("nameservice");

  LOGGER.debug(boost::format("requested object resolve: %1%/%2%")
                                      % nsctx
                                      % name);

  /* Create a name object, containing the name test/context */
  CosNaming::Name contextName;
  contextName.length(2);

  contextName[0].id   = (const char*) nsctx.c_str(); 
  contextName[0].kind = (const char*) "context";
  contextName[1].id   = name.c_str();
  contextName[1].kind = (const char*) "Object";
  /**
   * Note on kind: The kind field is used to indicate the type
   * of the object. This is to avoid conventions such as that used
   * by files (name.type -- e.g. test.ps = postscript etc.)
   */

  try {
    /* Resolve the name to an object reference */
    return rootContext->resolve(contextName);
  }

  catch(CosNaming::NamingContext::NotFound& ex) {
    /**
     * This exception is thrown if any of the components of the
     * path [contexts or the object] aren't found
     */
    CosNaming::Name cname = ex.rest_of_name;
    std::string cname_str;
    for (unsigned i = 0; i < cname.length(); ++i) {
      cname_str += std::string(cname[i].id) + "(" + std::string(cname[i].kind) + ")" + (cname.length() != i + 1 ? "/" : "" );
    }
    LOGGER.error(boost::format("Context [%1%] not found.") % cname_str);
    throw BAD_CONTEXT();
  }

  catch(CORBA::TRANSIENT& ex) {
    throw NOT_RUNNING();
  }
  catch(CORBA::SystemException& ex) {
    throw NOT_RUNNING();
  }

  return CORBA::Object::_nil();
}//NameService::resolve

CORBA::Object_ptr NameService::resolve_process_object(const std::string& process_context
          , const std::string& name)
{
    return resolve_process_object(context, process_context, name);
}

CORBA::Object_ptr NameService::resolve_process_object(const std::string& root_context,
                           const std::string& process_context,
                          const std::string& object_name)
{
    Logging::Context ctx("nameservice");

    LOGGER.debug(boost::format("requested object resolve: %1%/%2%/%3%")
                                        % root_context
                                        % process_context
                                        % object_name);

    /* Create a name object, containing the name test/context */
    CosNaming::Name contextName;
    contextName.length(3);

    contextName[0].id   = (const char*) root_context.c_str();
    contextName[0].kind = (const char*) "context";
    contextName[1].id   = (const char*) process_context.c_str();
    contextName[1].kind = (const char*) "context";
    contextName[2].id   = object_name.c_str();
    contextName[2].kind = (const char*) "Object";
    /**
     * Note on kind: The kind field is used to indicate the type
     * of the object. This is to avoid conventions such as that used
     * by files (name.type -- e.g. test.ps = postscript etc.)
     */

    try {
      /* Resolve the name to an object reference */
      return rootContext->resolve(contextName);
    }

    catch(CosNaming::NamingContext::NotFound& ex) {
      /**
       * This exception is thrown if any of the components of the
       * path [contexts or the object] aren't found
       */
      CosNaming::Name cname = ex.rest_of_name;
      std::string cname_str;
      for (unsigned i = 0; i < cname.length(); ++i) {
        cname_str += std::string(cname[i].id) + "(" + std::string(cname[i].kind) + ")" + (cname.length() != i + 1 ? "/" : "" );
      }
      LOGGER.error(boost::format("Context [%1%] not found.") % cname_str);
      throw BAD_CONTEXT();
    }

    catch(CORBA::TRANSIENT& ex) {
      throw NOT_RUNNING();
    }
    catch(CORBA::SystemException& ex) {
      throw NOT_RUNNING();
    }

    return CORBA::Object::_nil();
}//NameService::resolve_process_object

const std::string& NameService::getHostName() {
  return hostname;
}

