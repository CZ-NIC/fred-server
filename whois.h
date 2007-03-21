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


  // whois ASK to domain
  // methods corresponding to defined IDL attributes and operations
  ccReg::DomainWhois* getDomain(const char* domain_name, CORBA::String_out timestamp);

};

