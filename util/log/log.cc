#include <boost/utility.hpp>
#include "log.h"
#include "log_types.h"
#include "context.h"

namespace Logging {

Log::Log(const std::string& _name) :
	level(LL_INFO), name(_name) {
}

Log::~Log() {
  std::for_each(handlers.begin(),
                handlers.end(),
                boost::checked_deleter<BaseLogType>());}

void Log::addHandler(Log::Type _type, const boost::any& _param) {
  BaseLogType *type = 0;
  switch (_type) {
	  case Log::LT_CONSOLE:
	    type = new Logging::ConsoleLog();
	    break;
	  case Log::LT_SYSLOG:
	    type = new Logging::SysLog(boost::any_cast<unsigned>(_param));
	    break;
	  case Log::LT_FILE:
	    type = new Logging::FileLog(boost::any_cast<std::string>(_param));
	    break;
	}
	handlers.push_back(type);
}

void Log::setLevel(Log::Level _ll) {
	level = _ll;
}

void Log::setName(const std::string& _name) {
	name = _name;
}

Log::Level Log::getLevel() const {
	return level;
}

void Log::trace(const std::string& _msg) {
	if (level < LL_TRACE)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_TRACE, _msg, name);
	}
}

void Log::debug(const std::string& _msg) {
	if (level < LL_DEBUG)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_DEBUG, _msg, name);
	}
}

void Log::info(const std::string& _msg) {
	if (level < LL_INFO)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_INFO, _msg, name);
	}
}

void Log::notice(const std::string& _msg) {
	if (level < LL_NOTICE)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_NOTICE, _msg, name);
	}
}

void Log::warning(const std::string& _msg) {
	if (level < LL_WARNING)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_WARNING, _msg, name);
	}
}

void Log::error(const std::string& _msg) {
	if (level < LL_ERR)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_ERR, _msg, name);
	}
}

void Log::critical(const std::string& _msg) {
	if (level < LL_CRIT)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_CRIT, _msg, name);
	}
}

void Log::alert(const std::string& _msg) {
	if (level < LL_ALERT)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_ALERT, _msg, name);
	}
}

void Log::emerg(const std::string& _msg) {
	if (level < LL_EMERG)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_EMERG, _msg, name);
	}
}

void Log::trace(const boost::format& _frmt) {
	trace(_frmt.str());
}

void Log::debug(const boost::format& _frmt) {
	debug(_frmt.str());
}

void Log::info(const boost::format& _frmt) {
	info(_frmt.str());
}

void Log::notice(const boost::format& _frmt) {
	notice(_frmt.str());
}

void Log::warning(const boost::format& _frmt) {
	warning(_frmt.str());
}

void Log::error(const boost::format& _frmt) {
	error(_frmt.str());
}

void Log::critical(const boost::format& _frmt) {
	critical(_frmt.str());
}

void Log::alert(const boost::format& _frmt) {
	alert(_frmt.str());
}

void Log::emerg(const boost::format& _frmt) {
	emerg(_frmt.str());
}


void Log::message(int _prio, const char *_format, ...) {
  if (level < _prio)
    return;

  char msg[2048];
  va_list args;
  va_start(args, _format);
  vsnprintf(msg, 2047, _format, args);

 	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(static_cast<Logging::Log::Level>(_prio), msg, name);
	}

  va_end(args);
}

}
