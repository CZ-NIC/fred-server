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
  const std::string& fqdn
) throw (INTERNAL_ERROR, REGISTRAR_NOT_FOUND, NSSET_NOT_FOUND)
{
  try {
    ccReg::Lists fqdns;
    fqdns.length(1) ;
    fqdns[0] = fqdn.c_str();
    CORBA::String_var reg = registrar.c_str();
    CORBA::String_var nss = nsset.c_str();
    tc->checkNssetAsynch(reg,nss,0,false,true,ccReg::CHKR_EPP,fqdns);
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
