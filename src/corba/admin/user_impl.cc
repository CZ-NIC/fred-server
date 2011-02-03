#include <corba/Admin.hh>
#include "user_impl.h"

ccReg_User_i::ccReg_User_i(ccReg::TID _id, const std::string& _username) :
	m_id(_id), m_username(_username), m_firstname(), m_surname() {
}

ccReg_User_i::ccReg_User_i(ccReg::TID _id, const std::string& _username,
		const std::string& _firstname, const std::string& _surname) :
	m_id(_id), m_username(_username), m_firstname(_firstname),
			m_surname(_surname) {
}

CORBA::Boolean ccReg_User_i::hasNPermission(const char* _np) {
	return false;
}
