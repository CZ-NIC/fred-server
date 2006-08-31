#include "nameservice.h"

NameService::NameService(CORBA::ORB_ptr orb, const std::string& nameServiceIOR)
  throw (NOT_RUNNING)
{
  try {
    CORBA::Object_var obj;
    if (nameServiceIOR.empty())
      obj = orb->resolve_initial_references(nameServiceIOR.c_str());
    else
      obj = orb->string_to_object(nameServiceIOR.c_str());
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

void 
NameService::bind(const std::string& name, CORBA::Object_ptr objref)
  throw (NOT_RUNNING, BAD_CONTEXT)
{
  try {
    CosNaming::Name contextName;
    contextName.length(1);
    contextName[0].id   = (const char*) "ccReg";  // string copied
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

NameService::~NameService()
{
}
