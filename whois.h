#include "ccReg.hh"
#include <string>

//
// Example class implementing IDL interface ccReg::Whois
//
class ccReg_Whois_i: public POA_ccReg::Whois , 
                     public PortableServer::RefCountServantBase  {
  std::string database;
 public:
  ccReg_Whois_i(const std::string database);
  virtual ~ccReg_Whois_i();


  // dotaz na domenu
  ccReg::DomainWhois* Domain(const char* domain_name);
};

