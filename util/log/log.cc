#include <boost/utility.hpp>
#include "log.h"
#include "log_types.h"

namespace Logging {

Log::Log(const std::string& _ctx) :
	level(LL_INFO), context(_ctx) {
}

Log::~Log() {
  std::for_each(handlers.begin(),
                handlers.end(),
                boost::checked_deleter<BaseLogType>());}

void Log::addHandler(Log::Type _type, const std::string& _params) {
  BaseLogType *type = 0;
  switch (_type) {
	  case Log::LT_CONSOLE:
	    type = new Logging::ConsoleLog();
	    break;
	  case Log::LT_SYSLOG:
	    type = new Logging::SysLog();
	    break;
	  case Log::LT_FILE:
	    type = new Logging::FileLog(_params);
	    break;
	}
	handlers.push_back(type);
}

void Log::setLevel(Log::Level _ll) {
	level = _ll;
}

void Log::setContext(const std::string& _ctx) {
	context = _ctx;
}

Log::Level Log::getLevel() const {
	return level;
}

void Log::trace(const std::string& _msg) {
	if (level < LL_TRACE)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_TRACE, _msg, context);
	}
}

void Log::debug(const std::string& _msg) {
	if (level < LL_DEBUG)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_DEBUG, _msg, context);
	}
}

void Log::info(const std::string& _msg) {
	if (level < LL_INFO)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_INFO, _msg, context);
	}
}

void Log::notice(const std::string& _msg) {
	if (level < LL_NOTICE)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_NOTICE, _msg, context);
	}
}

void Log::warning(const std::string& _msg) {
	if (level < LL_WARNING)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_WARNING, _msg, context);
	}
}

void Log::error(const std::string& _msg) {
	if (level < LL_ERR)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_ERR, _msg, context);
	}
}

void Log::critical(const std::string& _msg) {
	if (level < LL_CRIT)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_CRIT, _msg, context);
	}
}

void Log::alert(const std::string& _msg) {
	if (level < LL_ALERT)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_ALERT, _msg, context);
	}
}

void Log::emerg(const std::string& _msg) {
	if (level < LL_EMERG)
		return;
	std::deque<BaseLogType*>::iterator it = handlers.begin();
	for (; it != handlers.end(); ++it) {
		(*it)->msg(LL_EMERG, _msg, context);
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

}
