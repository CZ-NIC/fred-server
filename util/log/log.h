#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <iostream>
#include <boost/format.hpp>
#include <boost/any.hpp>
#include <deque>

#include <stdio.h>
#include <stdarg.h>

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

	Log(const std::string& _name);
	~Log();

  void setName(const std::string &_name);
	void addHandler(Log::Type _type, const boost::any& _param = boost::any());

	void setLevel(Log::Level _ll);
	Log::Level getLevel() const;
		
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

  /**
   * support for old style formatting log
   */
  void message(int _prio, const char *_format, ...);
	
protected:	
	std::deque<BaseLogType* > handlers;
	Log::Level level;
  std::string name;
};

}

#endif /*LOG_H_*/
