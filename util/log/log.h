#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <iostream>
#include <boost/format.hpp>
#include <deque>

namespace Logging {

class BaseLogType;

class Log {
public:
	enum Level {
	  LL_EMERG,
	  LL_ALERT,
	  LL_CRIT,
	  LL_ERR,
	  LL_WARNING,
	  LL_NOTICE,
	  LL_INFO,
	  LL_DEBUG,
	  LL_TRACE
	};

	enum Type {
	  LT_CONSOLE,
	  LT_FILE,
	  LT_SYSLOG
	};

	Log(const std::string& _ctx = "");
	~Log();
	void addHandler(Log::Type _type, const std::string& _params = std::string());

	void setLevel(Log::Level _ll);
	Log::Level getLevel() const;
	void setContext(const std::string &_ctx);
	
	void trace(const std::string& _msg);
	void trace(const boost::format& _frmt);
	void debug(const std::string& _msg);
	void debug(const boost::format& _frmt);
	void info(const std::string& _msg);
	void info(const boost::format& _frmt);
	void notice(const std::string& _msg);
	void notice(const boost::format& _frmt);
	void warning(const std::string& _msg);
	void warning(const boost::format& _frmt);
	void error(const std::string& _msg);
	void error(const boost::format& _frmt);
	void critical(const std::string& _msg);
	void critical(const boost::format& _frmt);
	void alert(const std::string& _msg);
	void alert(const boost::format& _frmt);
	void emerg(const std::string& _msg);
	void emerg(const boost::format& _frmt);
	
protected:	
	std::deque<BaseLogType* > handlers;
	Log::Level level;
	std::string context;
};

}

#endif /*LOG_H_*/
