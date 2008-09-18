#ifndef NAMESERVICE_H_
#define NAMESERVICE_H_

#include <string>
#include <omniORB4/CORBA.h>

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
  struct NOT_RUNNING {};

  /**
   * exception when cannot bind context 
   */
  struct BAD_CONTEXT {};

  /**
   * config rootContext from supplied orb
   */
  NameService(CORBA::ORB_ptr orb) throw (NOT_RUNNING);

  /**
   * config rootContext from supplied orb and hostname
   */
	NameService(CORBA::ORB_ptr orb, 
              const std::string& hostname, 
              const std::string& _context) throw (NOT_RUNNING);

  /**
   * d-tor
   */	
  virtual ~NameService();

  /**
   * bind object into nameservice.context/'name'.Object
   */
  void bind(const std::string& name, 
            CORBA::Object_ptr objref) throw (NOT_RUNNING, BAD_CONTEXT);

  /**
   * resolve object from nameservice.context/'name'.Object get IOR
   */
  CORBA::Object_ptr resolve(const std::string& name) throw (NOT_RUNNING, BAD_CONTEXT);

  /**
   * get nameservice hostname used
   */
  const std::string& getHostName();

};

#endif /*NAMESERVICE_H_*/
