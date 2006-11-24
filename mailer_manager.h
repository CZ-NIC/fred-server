#ifndef MAILER_MANAGER_H_
#define MAILER_MANAGER_H_

#include "ccReg.hh"
#include "register/mailer.h"
#include "nameservice.h"

/// Implementation of register mailer 
/** Use remote CORBA mailer to implement register mailer functionality.
 * This implementation is connected to the rest of register by supplying 
 * generic pointer to abstract mailer interface */ 
class MailerManager : public Register::Mailer::Manager
{
  ccReg::Mailer_var mailer;
 public:
  class RESOLVE_FAILED {};
  MailerManager(NameService *ns) throw (RESOLVE_FAILED);
  virtual unsigned long sendEmail(
    const std::string& from,
    const std::string& to,
    const std::string& subject,
    const std::string& mailTemplate,
    Register::Mailer::Parameters params,
    Register::Mailer::Handles handles
  ); 
};

#endif
