#ifndef __MAILER_H__
#define __MAILER_H__

#include <map>
#include <vector>
#include <string>

namespace Register
{
  namespace Mailer
  {
    typedef std::map<std::string,std::string> Parameters;
    typedef std::vector<std::string> Handles;
    class Manager
    {
     public:
      virtual ~Manager() {}
      virtual unsigned long sendEmail(
        const std::string& from,
        const std::string& to,
        const std::string& subject,
        const std::string& mailTemplate,
        Parameters params,
        Handles handles
      ) = 0;
    }; // Manager
  }; // Mailer
}; // Register

#endif
