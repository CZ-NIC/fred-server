#ifndef LOGGER_HH_FF92F982C11C49949EC827AE9795E29A
#define LOGGER_HH_FF92F982C11C49949EC827AE9795E29A

#include <map>
#include <string>
#include <iostream>
#include <boost/format.hpp>

#include "config.h"

#include "src/util/log/log.hh"
#include "src/util/log/log_types.hh"
#include "src/util/singleton.hh"

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
