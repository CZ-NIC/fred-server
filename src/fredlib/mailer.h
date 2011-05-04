#ifndef __MAILER_H__
#define __MAILER_H__

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include "types.h"

namespace Fred
{
  namespace Mailer
  {
    typedef std::map<std::string,std::string> Parameters;
    typedef std::vector<std::string> Handles;
    typedef std::vector<TID> Attachments;
    // Exception thrown when mail cannot be send
    struct NOT_SEND : public std::runtime_error
    {
        NOT_SEND()
                : std::runtime_error("Mailer NOT_SEND")
        {}
    };
    class Manager
    {
     public:
      virtual ~Manager() {}
      virtual TID sendEmail(
        const std::string& from,
        const std::string& to,
        const std::string& subject,
        const std::string& mailTemplate,
        const Parameters &params,
        const Handles &handles,
        const Attachments &attach,
        const std::string& reply_to = std::string("")
      ) throw (NOT_SEND) = 0;
      virtual bool checkEmailList(std::string &_email_list) const = 0;
    }; // Manager
  }; // Mailer
}; // Fred

#endif
