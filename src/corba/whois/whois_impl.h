#ifndef _WHOIS_IMPL_H_
#define _WHOIS_IMPL_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include "session_impl.h"
#include "corba/mailer_manager.h"
#include "fredlib/registry.h"
#include "fredlib/invoice.h"
#include "old_utils/dbsql.h"
#include "model/model_filters.h"
#include "bankinginvoicing_impl.h"

#include "conf/manager.h"

class NameService;

class ccReg_Whois_i : public POA_ccReg::Whois,
  public PortableServer::RefCountServantBase {
private:
  std::string m_connection_string;
  std::string server_name_;
  bool registry_restricted_handles_;

  void fillRegistrar(ccReg::AdminRegistrar& creg,
                     Fred::Registrar::Registrar *reg);

public:

  ccReg_Whois_i(const std::string& database, const std::string& _server_name
          , bool _registry_restricted_handles);
  virtual ~ccReg_Whois_i();

  ccReg::AdminRegistrar* getRegistrarByHandle(const char* handle);

};//class ccReg_Whois_i

#endif //WHOIS_IMPL_H
