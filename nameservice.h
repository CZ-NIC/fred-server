#ifndef NAMESERVICE_H_
#define NAMESERVICE_H_

#include <string>
#include <omniORB4/CORBA.h>

class NameService
{
  CORBA::ORB_ptr orb;
  CosNaming::NamingContext_var rootContext;
 public:
  /// exception when NameService not running or not in omniORB config file
  struct NOT_RUNNING {};
  /// exception when cannot bind context 
  struct BAD_CONTEXT {};
  /// config rootContext from supplied orb
        NameService(CORBA::ORB_ptr orb)
    throw (NOT_RUNNING);
	NameService(CORBA::ORB_ptr orb, const std::string& nameServiceIOR) 
    throw (NOT_RUNNING);
  /// bind object into ccReg.context/'name'.Object
  void bind(const std::string& name, CORBA::Object_ptr objref)
    throw (NOT_RUNNING, BAD_CONTEXT);
  /// resolve object into ccReg.context/'name'.Object get IOR
  CORBA::Object_ptr resolve( const std::string& name)
    throw (NOT_RUNNING, BAD_CONTEXT);

  /// destroy NameServiceObject
	virtual ~NameService();
};

#endif /*NAMESERVICE_H_*/
