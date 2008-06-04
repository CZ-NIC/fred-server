#ifndef LOG_TYPES_H_
#define LOG_TYPES_H_

#include <string>
#include <fstream>
#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "log.h"

namespace Logging {

class BaseLogType {
public:
	virtual ~BaseLogType();
	virtual void msg(Log::Level _ll, const std::string& _msg, const std::string& _ctx) = 0;
	virtual std::string level2str(Log::Level _ll) const;

protected:
	boost::mutex mutex;
};

class FileLog : public BaseLogType {
public:
	FileLog(const std::string& _name);
	virtual ~FileLog();
	virtual void msg(Log::Level _ll, const std::string& _msg, const std::string& _ctx);
	
protected:
	const std::string name;
	std::ofstream _ofs;
};

class ConsoleLog : public BaseLogType {
public:
	ConsoleLog();
	virtual ~ConsoleLog();
	virtual void msg(Log::Level _ll, const std::string& _msg, const std::string& _ctx);
};

class SysLog : public BaseLogType {
public:
	SysLog(int _facility = 2);
	virtual ~SysLog();
	virtual void msg(Log::Level _ll, const std::string& _msg, const std::string& _ctx);

protected:
	int facility;
};

}

#endif /*LOG_TYPES_H_*/
