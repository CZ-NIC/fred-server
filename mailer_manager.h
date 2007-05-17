#ifndef MAILER_MANAGER_H_
#define MAILER_MANAGER_H_

#include <boost/date_time/posix_time/posix_time.hpp>

#include "ccReg.hh"
#include "register/mailer.h"
#include "nameservice.h"

using namespace boost::posix_time;

/// Implementation of register mailer 
/** Use remote CORBA mailer to implement register mailer functionality.
 * This implementation is connected to the rest of register by supplying 
 * generic pointer to abstract mailer interface */ 
class MailerManager : public Register::Mailer::Manager
{ 
  ccReg::Mailer_var mailer;
 public:
  class RESOLVE_FAILED {};
  class LOAD_ERROR {};
  MailerManager(NameService *ns) throw (RESOLVE_FAILED);
  virtual Register::TID sendEmail(
    const std::string& from,
    const std::string& to,
    const std::string& subject,
    const std::string& mailTemplate,
    Register::Mailer::Parameters params,
    Register::Mailer::Handles handles,
    Register::Mailer::Attachments attach
  ) throw (Register::Mailer::NOT_SEND) ;
  struct Filter 
  {
    Filter();
    void clear();
    Register::TID id;
    time_period crTime;
    time_period modTime;
    long type;
    long status;
    std::string content;
    std::string handle;
    std::string attachment;
  };
  struct Detail
  {
    Register::TID id;
    std::string createTime;
    std::string modTime;
    long type;
    std::string typeDesc;
    long status;
    std::string content;
    std::vector<std::string> handles;
    std::vector<Register::TID> attachments;   
  };
  typedef std::vector<Detail> List;  
 private:
  List mailList;
 public:
  List& getMailList();
  void reload(Filter& mf) throw (LOAD_ERROR);
};

#endif
