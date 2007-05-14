#include "tech_check.h"

TechCheckManager::TechCheckManager(NameService *ns)
  throw (RESOLVE_FAILED)
{
  try {
    tc = ccReg::TechCheck::_narrow(ns->resolve("TechCheck"));
  } catch (...) { throw RESOLVE_FAILED(); }
}

void 
TechCheckManager::checkFromRegistrar(
  const std::string& registrar, const std::string& nsset, 
  int level, const FQDNList& fqdns, const char *cltrid
) throw (INTERNAL_ERROR, REGISTRAR_NOT_FOUND, NSSET_NOT_FOUND)
{
  try {
    ccReg::Lists cfqdns;
    cfqdns.length(fqdns.size());
    for (unsigned i=0; i<fqdns.size(); i++)
      cfqdns[i] = fqdns[i].c_str();
    CORBA::String_var reg = registrar.c_str();
    CORBA::String_var nss = nsset.c_str();
    tc->checkNssetAsynch(reg,nss,level,false,true,ccReg::CHKR_EPP,cfqdns,cltrid);
  }
  catch (ccReg::TechCheck::InternalError) {
    throw INTERNAL_ERROR();
  }
  catch (ccReg::TechCheck::NssetNotFound) {
    throw NSSET_NOT_FOUND();
  }
  catch (ccReg::TechCheck::RegistrarNotFound) {
    throw REGISTRAR_NOT_FOUND();
  }
}
