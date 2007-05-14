#ifndef TECH_CHECK_MANAGER_H_
#define TECH_CHECK_MANAGER_H_

#include "ccReg.hh"
#include "nameservice.h"
#include <vector>

class TechCheckManager {
  ccReg::TechCheck_var tc;
 public:
  typedef std::vector<std::string> FQDNList;
  class RESOLVE_FAILED {};
  class INTERNAL_ERROR {};
  class REGISTRAR_NOT_FOUND {};
  class NSSET_NOT_FOUND {};
  TechCheckManager(NameService *ns) throw (RESOLVE_FAILED);
  void checkFromRegistrar(
    const std::string& registrar, const std::string& nsset, 
    int level, const FQDNList& fqdns, const char *cltrid
  ) throw (INTERNAL_ERROR, REGISTRAR_NOT_FOUND, NSSET_NOT_FOUND);
};

#endif
