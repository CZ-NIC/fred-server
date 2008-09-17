#ifndef LOGGER_H_
#define LOGGER_H_

#include <map>
#include <string>
#include <iostream>
#include <boost/format.hpp>

#include "config.h"

#include "log.h"
#include "log_types.h"
#include "singleton.h"

namespace Logging {

class Logger {
public:
	Logger();
	~Logger();
	Log& get(const std::string& _name);
	void add(const std::string& _name, Log *_l);

private:
	std::map<std::string, Log*> logs;
};

typedef Singleton<Logger> Manager;

}

#define LOGGER(name) Logging::Manager::instance_ref().get(name)
#define TRACE(msg) LOGGER(PACKAGE).trace(msg)

#define LOG LOGGER(PACKAGE).message

#endif /*LOGGER_H_*/
