//#include <boost/thread/thread.hpp>
//#include <boost/thread/mutex.hpp>
#include <boost/format.hpp>
#include "logger.h"

//struct TestLogger {
//	TestLogger(int _id) : m_id(_id) { }
//	void operator()() {
//		Logging::Manager::instance_ref()->get("file").info(boost::format("msg from thread id %1%") % m_id);
//		Logging::Manager::instance_ref()->get("console").alert(boost::format("msg from thread id %1%") % m_id);
//		sleep(2);
//		Logging::Manager::instance_ref()->get("syslog").error(boost::format("msg from thread id %1%") % m_id);
//		sleep(5);
//	}
//	
//	int m_id;
//};


int main() {
	
//	Logging::Manager::instance_ref()->add("file", new Log(new FileLog("filelog.log")));
//	Logging::Manager::instance_ref()->add("console", new Log(new ConsoleLog()));
//	Logging::Manager::instance_ref()->add("syslog", new Log(new SysLog(2)));
//	
	Logging::Logger &l = Logging::Manager::instance_ref();
	//l.add("console", new Logging::Log(new Logging::ConsoleLog()));
	l.get("console").addHandler(Logging::Log::LT_CONSOLE);
	l.get("console").addHandler(Logging::Log::LT_FILE);
	l.get("console").setLevel(Logging::Log::LL_DEBUG);
	l.get("console").debug("singleton template message");
	l.get("console").info("singleton template message 2");
	
//	boost::thread_group threads;
//	for (int i = 0; i < 100; ++i) {
//		threads.create_thread(TestLogger(i)); 
//	}
//	threads.join_all();
	
	return 0;
}
