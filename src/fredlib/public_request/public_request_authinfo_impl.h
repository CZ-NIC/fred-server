#ifndef PUBLIC_REQUEST_AUTHINFO_IMPL_H_
#define PUBLIC_REQUEST_AUTHINFO_IMPL_H_

#include "public_request_impl.h"
#include "types/stringify.h"

#define SERVER_TRANSFER_PROHIBITED 3
#define SERVER_UPDATE_PROHIBITED 4

namespace Fred {
namespace PublicRequest {



class AuthInfoRequestImpl : public PublicRequestImpl {
public:
  /// check if object exists & no serverTransferProhibited
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = !checkState(getObject(i).id,SERVER_TRANSFER_PROHIBITED);
    return res;
  }
  virtual void processAction(bool _check) {
    if (_check && !check());
  }
  std::string getAuthInfo() const {
    // just one object is supported
    if (!getObjectSize() || getObjectSize() > 1) return "";
    Database::SelectQuery sql;
    sql.buffer() << "SELECT o.AuthInfoPw "
                 << "FROM object o "
                 << "WHERE o.id=" << getObject(0).id;
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec(sql);
    if (!res.size()) {
      return "";
    }
    return (*res.begin())[0];
  }
  /// fill mail template with common param for authinfo requeste templates
  virtual void fillTemplateParams(Mailer::Parameters& params) const {
    std::ostringstream buf;
    buf << getRegistrarName() << " (" << getRegistrarUrl() << ")";
    params["registrar"] = buf.str();
    params["reqdate"] = stringify(getCreateTime().date());
    params["reqid"] = stringify(getId());
    if (getObjectSize()) {
      params["type"] = stringify(getObject(0).type);
      params["handle"] = getObject(0).handle;
    }
    params["authinfo"] = getAuthInfo();
  }
};

class AuthInfoRequestEPPImpl : public AuthInfoRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "sendauthinfo_epp";
  }
  /// after creating, this type of request is processed
  virtual void postCreate() {
    // cannot call process(), object need to be loaded completly
    man_->processRequest(getId(),false,false, this->getRequestId());
  }
};

class AuthInfoRequestPIFAutoImpl : public AuthInfoRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "sendauthinfo_pif";
  }
  /// after creating, this type of request is processed
  virtual void postCreate() {
    // cannot call process(), object need to be loaded completly
    man_->processRequest(getId(),false,false,this->getRequestId());
  }
};

class AuthInfoRequestPIFEmailImpl : public AuthInfoRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "sendauthinfo_pif";
  }
  /// answer is sent to requested email
  std::string getEmails() const {
    return getEmailToAnswer();
  }
};

class AuthInfoRequestPIFPostImpl : public AuthInfoRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "sendauthinfo_pif";
  }
  /// answer is sent to requested email
  std::string getEmails() const {
    return getEmailToAnswer();
  }
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 1;
  }
};



}
}


#endif /* PUBLIC_REQUEST_AUTHINFO_IMPL_H_ */
 
