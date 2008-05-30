#ifndef TECH_CHECK_MANAGER_H_
#define TECH_CHECK_MANAGER_H_

#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "corba/ccReg.hh"
#include "corba/nameservice.h"

class TechCheckManager {
 public:
  typedef std::vector<std::string> FQDNList;
  class RESOLVE_FAILED {};
  class INTERNAL_ERROR {};
  class REGISTRAR_NOT_FOUND {};
  class NSSET_NOT_FOUND {};
  TechCheckManager(NameService *ns);
  void checkFromRegistrar(
    const std::string& registrar, const std::string& nsset, 
    int level, const FQDNList& fqdns, const char *cltrid
  ) throw (INTERNAL_ERROR, REGISTRAR_NOT_FOUND, NSSET_NOT_FOUND);
  
 private:
  NameService          *ns_ptr;
  ccReg::TechCheck_var tc;
  boost::mutex         mutex;

  void _resolveInit() throw (RESOLVE_FAILED);
};

#endif
