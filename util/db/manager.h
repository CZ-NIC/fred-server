#ifndef MANAGER_H_
#define MANAGER_H_

#include <string>
#include <vector>
#include "psql_connection.h"

namespace DBase {

class Manager {
public:
	Manager(const std::string& _conn_info) :
		conn_info(_conn_info) {
	}
	Manager(const char* _conn_info) :
		conn_info(_conn_info) {
	}
	virtual ~Manager() {
	}
	virtual Connection* getConnection() = 0;

protected:
	std::string conn_info;
};

class PSQLManager : public Manager {
public:
	PSQLManager(const std::string& _conn_info) :
		Manager(_conn_info) {
	}
	PSQLManager(const char* _conn_info) :
		Manager(_conn_info) {
	}
	~PSQLManager() {
	}
	Connection* getConnection() {
		return new PSQLConnection(conn_info);
	}
};

}
#endif /*MANAGER_H_*/
