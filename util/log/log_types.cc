#include "boost/date_time/posix_time/posix_time.hpp"
#include <time.h>

#include "log_types.h"
#include "syslog.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace Logging {

BaseLogType::~BaseLogType() {
}

std::string BaseLogType::level2str(Log::Level _ll) const {
	switch (_ll) {
	case Log::LL_DEBUG: 	return "debug";
	case Log::LL_INFO: 	return "info";
	case Log::LL_NOTICE: 	return "notice";
	case Log::LL_WARNING: 	return "warning";
	case Log::LL_ERR: 	return "error";
	case Log::LL_CRIT: 	return "critical";
	case Log::LL_ALERT: 	return "alert";
	case Log::LL_EMERG: 	return "emerg";
	case Log::LL_TRACE: 	return "trace";
	default: 		return "unknown";
	}
}

FileLog::FileLog(const std::string& _name) :
	name(_name) {
	_ofs.open(name.c_str(), std::ios_base::app);
}

FileLog::~FileLog() {
	_ofs.close();
}

void FileLog::msg(Log::Level _ll, const std::string& _msg,
		const std::string& _ctx) {
	std::string str_now;
	try {
		ptime now = ptime(second_clock::local_time());
		str_now = to_simple_string(now);
	}
	catch (...) {
		// Valgrind throws exception even if everything is ok.
		std::cerr << "ERROR: boost posix time library" << std::endl;
	}

	boost::mutex::scoped_lock scoped_lock(mutex);
//	_ofs << "[" << str_now << "] " 
//	     << (_ctx.empty() ? "" : "[" + _ctx + "] ") 
//	     << "[" << level2str(_ll) << "] " << _msg << std::endl;
	
	if (_ctx.empty())  
	  std::cout << boost::format("[%1%] %|5t|[%2%] %|20t|%3%\n")
	      % str_now % level2str(_ll) % _msg;
	else
	  std::cout << boost::format("[%1%] %|20t|[%2%] %|40t|[%3%] %|50t|%4%\n")
	      % str_now % _ctx % level2str(_ll) % _msg;
}

ConsoleLog::ConsoleLog() {
}

ConsoleLog::~ConsoleLog() {
}

void ConsoleLog::msg(Log::Level _ll, const std::string& _msg,
		const std::string& _ctx) {
	std::string str_now;
	try {
		ptime now = ptime(second_clock::local_time());
		str_now = to_simple_string(now);
	}
	catch (...) {
		// Valgrind throws exception even if everything is ok.
		std::cerr << "ERROR: boost posix time library" << std::endl;
	}

	boost::mutex::scoped_lock scoped_lock(mutex);
  
//	std::cout << "[" << str_now << "] " 
//	          << (_ctx.empty() ? "" : "[" + _ctx + "] ") 
//	          << "[" << level2str(_ll) << "] " << _msg << std::endl;
	
	if (_ctx.empty())	
	  std::cout << boost::format("[%1%] %|5t|[%2%] %|20t|%3%\n")
	      % str_now % level2str(_ll) % _msg;
	else
	  std::cout << boost::format("[%1%] %|20t|[%2%] %|40t|[%3%] %|50t|%4%\n")
	      % str_now % _ctx % level2str(_ll) % _msg;
}

SysLog::SysLog(int _facility) :
	facility(_facility) {
}

SysLog::~SysLog() {
}

void SysLog::msg(Log::Level _ll, const std::string& _msg,
		const std::string& _ctx) {
	std::string ctx = (_ctx.empty() ? "" : "[" + _ctx + "] ");
	if (_ll == Log::LL_TRACE)
	  _ll = Log::LL_DEBUG;
	
	syslog(_ll | ((LOG_FAC(LOG_LOCAL0) + facility) << 3), std::string(ctx + _msg).c_str());
}

}
