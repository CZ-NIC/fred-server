#ifndef LOGGER_H_
#define LOGGER_H_

#include <map>
#include <string>
#include <iostream>
#include <boost/format.hpp>

#include "log.h"
#include "log_types.h"
#include "singleton.h"

namespace Logging {

class Logger {
public:
	Logger();
	~Logger();
	Log& get(const std::string& _ctx);
	void add(const std::string& _ctx, Log *_l);
	void prefix(const std::string& _prefix);

private:
	std::map<std::string, Log*> logs;
	std::string ctx_prefix;
};

typedef Singleton<Logger> Manager;

}

#define LOGGER(name) Logging::Manager::instance_ref().get(name)
#define TRACE(msg) LOGGER("tracer").trace(msg)

#endif /*LOGGER_H_*/
