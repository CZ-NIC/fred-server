
//
// Example class implementing IDL interface ccReg::Whois
//
class ccReg_Whois_i: public POA_ccReg::Whois {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~ccReg_Whois_i();
public:
  // standard constructor
  ccReg_Whois_i();
  virtual ~ccReg_Whois_i();

  // methods corresponding to defined IDL attributes and operations
  ccReg::DomainWhois* Domain(const char* domain_name);

};


