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
#include "config.h"
#include "nameservice.h"
#include "log/logger.h"

NameService::NameService(CORBA::ORB_ptr orb)
  throw (NOT_RUNNING)
{
  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_var obj;
    obj = orb->resolve_initial_references("NameService");

    // Narrow the reference returned.
    rootContext = CosNaming::NamingContext::_narrow(obj);
    if( CORBA::is_nil(rootContext) ) throw NOT_RUNNING();
 }
  catch (CORBA::NO_RESOURCES&) {
    throw NOT_RUNNING();
  }
  catch (CORBA::ORB::InvalidName&) {
    throw NOT_RUNNING();
  }

}

NameService::NameService(CORBA::ORB_ptr orb, const std::string& _hostname)
  throw (NOT_RUNNING)
 : hostname(_hostname)
{
  try {
    CORBA::Object_var obj;
    if (hostname.empty())
      obj = orb->resolve_initial_references("NameService");
    else
      obj = orb->string_to_object(("corbaname::" + hostname).c_str());
    rootContext = CosNaming::NamingContext::_narrow(obj);
    if( CORBA::is_nil(rootContext) ) throw NOT_RUNNING();
  }
  catch (...) {
    throw NOT_RUNNING();
  }
}

void 
NameService::bind(const std::string& name, CORBA::Object_ptr objref)
  throw (NOT_RUNNING, BAD_CONTEXT)
{
  try {
    CosNaming::Name contextName;
    contextName.length(1);
    contextName[0].id   = (const char*) CONTEXTNAME;  // string copied
    contextName[0].kind = (const char*) "context"; // string copied
    CosNaming::NamingContext_var testContext;
    try {
      // Bind the context to root.
      testContext = rootContext->bind_new_context(contextName);
    }
    catch (CosNaming::NamingContext::AlreadyBound& ex) {
      // If the context already exists, this exception will be raised.
      // In this case, just resolve the name and assign testContext
      // to the object returned:
      CORBA::Object_var obj;
      obj = rootContext->resolve(contextName);
      testContext = CosNaming::NamingContext::_narrow(obj);
      if (CORBA::is_nil(testContext)) throw BAD_CONTEXT();
    }
    CosNaming::Name objectName;
    objectName.length(1);
    objectName[0].id   = name.c_str(); // string copied
    objectName[0].kind = (const char*) "Object"; // string copied
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



CORBA::Object_ptr NameService::resolve(  const std::string& name )
 throw (NOT_RUNNING, BAD_CONTEXT)
{

  // Create a name object, containing the name test/context:
  CosNaming::Name contextName;
  contextName.length(2);

  contextName[0].id   =  (const char*) CONTEXTNAME; 
  contextName[0].kind = (const char*) "context"; // string copied
  contextName[1].id   = name.c_str();
  contextName[1].kind = (const char*) "Object";
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (name.type -- e.g. test.ps = postscript etc.)

  try {
    // Resolve the name to an object reference.
    return rootContext->resolve(contextName);
  }

  catch(CosNaming::NamingContext::NotFound& ex) {
    // This exception is thrown if any of the components of the
    // path [contexts or the object] aren't found:
    CosNaming::Name name = ex.rest_of_name;
    std::string name_str;
    for (unsigned i = 0; i < name.length(); ++i) {
      name_str += std::string(name[i].id) + "(" + std::string(name[i].kind) + ")" + (name.length() != i + 1 ? "/" : "" );
    }
    LOGGER("corba").error(boost::format("Context [%1%] not found.") % name_str);
    throw BAD_CONTEXT();
  }

  catch(CORBA::TRANSIENT& ex) {
    throw NOT_RUNNING();
  }
  catch(CORBA::SystemException& ex) {
    throw NOT_RUNNING();
  }

  return CORBA::Object::_nil();
}

const std::string& 
NameService::getHostName()
{
  return hostname;
}

NameService::~NameService()
{
}
