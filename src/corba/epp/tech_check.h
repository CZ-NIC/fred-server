#ifndef TECH_CHECK_MANAGER_H_
#define TECH_CHECK_MANAGER_H_

#include <vector>
#include <stdexcept>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "corba/TechCheck.hh"
#include "corba/nameservice.h"

class TechCheckManager {
 public:
  typedef std::vector<std::string> FQDNList;
  class RESOLVE_FAILED : public std::runtime_error
  {
  public:
      RESOLVE_FAILED()
      : std::runtime_error("TechCheckManager RESOLVE_FAILED")
      {}
  };

  class INTERNAL_ERROR : public std::runtime_error
  {
  public:
      INTERNAL_ERROR()
      : std::runtime_error("TechCheckManager INTERNAL_ERROR")
      {}
  };
  class REGISTRAR_NOT_FOUND : public std::runtime_error
  {
  public:
      REGISTRAR_NOT_FOUND()
      : std::runtime_error("TechCheckManager REGISTRAR_NOT_FOUND")
      {}
  };
  class NSSET_NOT_FOUND : public std::runtime_error
  {
  public:
      NSSET_NOT_FOUND()
      : std::runtime_error("TechCheckManager NSSET_NOT_FOUND")
      {}
  };
  TechCheckManager(NameService *ns);
  void checkFromRegistrar(
    const std::string& registrar, const std::string& nsset, 
    int level, const FQDNList& fqdns, const char *cltrid
  );
  
 private:
  NameService          *ns_ptr;
  ccReg::TechCheck_var tc;
  boost::mutex         mutex;

  void _resolveInit();
};

#endif
