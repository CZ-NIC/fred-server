#include "ccReg.hh"
#include <string>

// implementace interface Admin
class ccReg_Admin_i: public POA_ccReg::Admin, 
                     public PortableServer::RefCountServantBase {
  std::string database;
 public:
  ccReg_Admin_i(const std::string database);
  virtual ~ccReg_Admin_i();

  //session
  virtual char* login(const char* username, const char* password);
  virtual ccReg::Session_ptr getSession(const char* sessionID);

  // vypis registratoru
  ccReg::RegistrarList* getRegistrars();
  ccReg::Registrar* getRegistrarByHandle(const char* handle) 
    throw (ccReg::Admin::ObjectNotFound);

  // primitivni vypis
  ccReg::Lists*  ObjectList( char* table , char *fname );
  ccReg::Lists* ListRegistrar();
  ccReg::Lists* ListDomain();
  ccReg::Lists* ListContact();
  ccReg::Lists* ListNSSet();


  /// testovaci fce na typ objektu
  void checkHandle(const char* handle, ccReg::CheckHandleType_out ch);
  
};
