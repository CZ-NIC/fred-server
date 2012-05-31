#ifndef _REQUEST_IMPL_H_
#define _REQUEST_IMPL_H_

#include "common_impl.h"
#include "request.h"

namespace Fred {
namespace Logger {

class RequestImpl : public Fred::CommonObjectImpl,
                 virtual public Request {
private:
  DateTime time_begin;
  DateTime time_end;
  std::string source_ip;
  // types changed from ServiceType and RequestType to std::string because pagetable should now return it as strings
  std::string serv_type;
  std::string request_type_id;
  ID session_id;
  std::string user_name;
  ID user_id;
  bool is_monitoring;
  std::string raw_request;
  std::string raw_response;
  boost::shared_ptr<RequestProperties> props;
  boost::shared_ptr<ObjectReferences> refs;
  int rc_code;
  std::string rc_name;

public:
  RequestImpl(ID &_id, DateTime &_time_begin, DateTime &_time_end,
          std::string &_serv_type, std::string &_source_ip,
          std::string &_request_type_id, ID &_session_id,
          std::string &_user_name, ID &_user_id,
          bool &_is_monitoring,
          std::string & _raw_request, std::string & _raw_response,
          std::auto_ptr<RequestProperties>  _props,
          std::auto_ptr<ObjectReferences>   _refs,
          const int _rc_code = 0, const std::string &_rc_name = std::string()) :
    CommonObjectImpl(_id),
    time_begin(_time_begin),
    time_end(_time_end),
    source_ip(_source_ip),
    serv_type(_serv_type),
    request_type_id(_request_type_id),
    session_id(_session_id),
        user_name(_user_name),
        user_id(_user_id),
    is_monitoring(_is_monitoring),
    raw_request(_raw_request),
    raw_response(_raw_response),
    props(_props),
        refs(_refs),
        rc_code(_rc_code),
        rc_name(_rc_name)
  {
  }

  virtual const ptime  getTimeBegin() const {
    return time_begin;
  }
  virtual const ptime  getTimeEnd() const {
    return time_end;
  }
  virtual const std::string& getServiceType() const {
    return serv_type;
  }
  virtual const std::string& getSourceIp() const {
    return source_ip;
  }
  virtual const std::string& getUserName() const {
        return user_name;
  }
  virtual const ID& getUserId() const {
        return user_id;
  }
  virtual const std::string& getActionType() const {
    return request_type_id;
  }
  virtual const ID& getSessionId() const {
    return session_id;
  }
  virtual const bool& getIsMonitoring() const {
    return is_monitoring;
  }
  virtual const std::string& getRawRequest() const {
    return raw_request;
  }
  virtual const std::string& getRawResponse() const {
    return raw_response;
  }
  virtual boost::shared_ptr<RequestProperties> getProperties() {
    return props;
  }
  virtual boost::shared_ptr<ObjectReferences> getReferences() {
        return refs;
  }
  virtual const std::pair<int, std::string> getResultCode() const {
    return std::make_pair(rc_code, rc_name);
  }
  virtual const std::string& getResultCodeName() const {
    return rc_name;
  }
};

}
}

#endif // _REQUEST_IMPL_H_

