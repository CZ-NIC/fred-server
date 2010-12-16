#ifndef USER_IMPL_H_
#define USER_IMPL_H_

#include <string>

class ccReg_User_i : public POA_Registry::User,
                     public PortableServer::RefCountServantBase {
  ccReg::TID m_id;
  std::string m_username;
  std::string m_firstname;
  std::string m_surname;
 public:
  ccReg_User_i(ccReg::TID _id, const std::string& _username);
  ccReg_User_i(ccReg::TID _id, const std::string& _username,
  		const std::string& _firstname, const std::string& _surname);
  ~ccReg_User_i() { }
  ccReg::TID id() { return m_id; }
  void id(ccReg::TID _id) { m_id = _id; }
  char* username() { return CORBA::string_dup(m_username.c_str()); }
  void username(const char* _username) { m_username = _username; }
  char* firstname() { return CORBA::string_dup(m_firstname.c_str()); }
  void firstname(const char* _firstname) { m_firstname = _firstname; }
  char* surname() { return CORBA::string_dup(m_surname.c_str()); }
  void surname(const char* _surname) { m_surname = _surname; }
  CORBA::Boolean hasNPermission(const char* _np);
};

#endif /*USER_IMPL_H_*/
