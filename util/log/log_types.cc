#include "boost/date_time/posix_time/posix_time.hpp"
#include <time.h>

#include "log_types.h"
#include "syslog.h"
#include "context.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace Logging {

BaseLogType::~BaseLogType() {
}

const char* BaseLogType::level2str(Log::Level _ll) const {
    static const char debug[] = "debug";
    static const char info[] = "info";
    static const char notice[] = "notice";
    static const char warning[] = "warning";
    static const char error[] = "error";
    static const char critical[] = "critical";
    static const char alert[] = "alert";
    static const char emerg[] = "emerg";
    static const char trace[] = "trace";
    static const char unknown[] = "unknown";

  switch (_ll) {
  case Log::LL_DEBUG:   return debug;
  case Log::LL_INFO:    return info;
  case Log::LL_NOTICE:  return notice;
  case Log::LL_WARNING: return warning;
  case Log::LL_ERR:     return error;
  case Log::LL_CRIT:    return critical;
  case Log::LL_ALERT:   return alert;
  case Log::LL_EMERG:   return emerg;
  case Log::LL_TRACE:   return trace;
  default:              return unknown;
	}
}

FileLog::FileLog(const std::string& _name)
    : name(_name), pFile (fopen (_name.c_str(),"a"))
{
    if(pFile == NULL)
    {
        throw std::runtime_error("FileLog: fopen failed");
    }
    _ofs.open(name.c_str(), std::ios_base::app);
}

FileLog::~FileLog() {
    fclose(pFile);
  _ofs.flush();
	_ofs.close();
}

void FileLog::msg(Log::Level _ll, const std::string& _msg,
		const std::string& _name) {
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
	
	if (_name.empty()) {	
	  _ofs << boost::format("[%1%] %|5t|[%2%] %|20t|%3%")
	                        % str_now 
                          % level2str(_ll) 
                          % _msg;
  }
	else if (!Context::get().empty()) {
	  _ofs << boost::format("[%1%] %|20t|[%2%] %|40t|[%3%] %|50t|[%4%] -- %5%")
	                        % str_now 
                          % _name 
                          % level2str(_ll)
                          % Context::get()
                          % _msg;
  }
  else {
	  _ofs << boost::format("[%1%] %|20t|[%2%] %|40t|[%3%] %|50t|%4%")
	                        % str_now 
                          % _name 
                          % level2str(_ll)
                          % _msg;
  }
  _ofs << std::endl;
}

void FileLog::smsg(Log::Level _ll, const char* _msg) throw()//simple msg
{
    try
    {
        std::string str_now;
        ptime now = ptime(second_clock::local_time());
        str_now = to_simple_string(now);

        boost::mutex::scoped_lock scoped_lock(mutex);

        fprintf(pFile,"[%s] [%s] -- %s",str_now.c_str(),  level2str(_ll), _msg);
    }
    catch (std::exception& ex)
    {
        fprintf(pFile,"smsg exception: %s -- %s", ex.what(),_msg);
    }
    catch (...)
    {
        fprintf(pFile,"smsg error: unknown exception -- %s",_msg);
    }
}



ConsoleLog::ConsoleLog() {
}

ConsoleLog::~ConsoleLog() {
  std::cout.flush();
}

void ConsoleLog::msg(Log::Level _ll, const std::string& _msg,
		const std::string& _name) {
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
  
	if (_name.empty()) {	
	  std::cout << boost::format("[%1%] %|5t|[%2%] %|20t|%3%")
	                             % str_now 
                               % level2str(_ll) 
                               % _msg;
  }
	else if (!Context::get().empty()) {
	  std::cout << boost::format("[%1%] %|20t|[%2%] %|40t|[%3%] %|50t|[%4%] -- %5%")
	                             % str_now 
                               % _name 
                               % level2str(_ll)
                               % Context::get()
                               % _msg;
  }
  else {
	  std::cout << boost::format("[%1%] %|20t|[%2%] %|40t|[%3%] %|50t|%4%")
	                             % str_now 
                               % _name 
                               % level2str(_ll)
                               % _msg;
  }
  std::cout << std::endl;
}

void ConsoleLog::smsg(Log::Level _ll, const char* _msg) throw() //simple msg
{
    try
    {
        std::string str_now;
        ptime now = ptime(second_clock::local_time());
        str_now = to_simple_string(now);

        boost::mutex::scoped_lock scoped_lock(mutex);

        fprintf(stdout,"[%s] [%s] %s",str_now.c_str(),  level2str(_ll), _msg);
    }
    catch (std::exception& ex)
    {
        fprintf(stdout,"smsg exception: %s", ex.what());
    }
    catch (...)
    {
        fprintf(stderr,"smsg error: unknown exception");
    }
}

SysLog::SysLog(int _facility) :
	facility(_facility) {
}

SysLog::~SysLog() {
}

void SysLog::msg(Log::Level _ll, const std::string& _msg,
		const std::string& _name) {
	std::string prefix = (Context::get().empty() ? "" : "[" + Context::get() + "] -- ");
	if (_ll == Log::LL_TRACE)
	  _ll = Log::LL_DEBUG;
	
	syslog(_ll | ((LOG_FAC(LOG_LOCAL0) + facility) << 3), "%s", std::string(prefix + _msg).c_str());
}

void SysLog::smsg(Log::Level _ll, const char* _msg) throw() //simple msg
{
    if (_ll == Log::LL_TRACE)
      _ll = Log::LL_DEBUG;

    syslog(_ll | ((LOG_FAC(LOG_LOCAL0) + facility) << 3), "%s", _msg);
}

}
