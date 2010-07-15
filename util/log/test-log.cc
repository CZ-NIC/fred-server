#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/format.hpp>
#include <boost/any.hpp>
#include <iostream>
#include <string>

#include "logger.h"
#include "context.h"

class TestA {
public:
  TestA() : ctx_("TestA") {
  }

  void meth1() {
    ctx_.push("meth1");
    LOGGER("test-log").info("call");
    ctx_.pop();
  }

  void meth2() {
    ctx_.push("meth2");
    LOGGER("test-log").info("call");
    meth3();
    ctx_.pop();
  }

  void meth3() {
    ctx_.push("meth3");
    LOGGER("test-log").info("call");
    ctx_.pop();
  }

private:
  Logging::Context ctx_;
};

struct TestContext {
  TestContext(int _id) : id_(_id) { }
  void operator()() {
    std::stringstream tmp;
    tmp << id_;
    Logging::Context ctx("thread-" + tmp.str());
    LOGGER("test-log").trace(boost::format("start"));

    TestA ta;
    ta.meth1();
    ta.meth2();
    ta.meth3();

    LOGGER("test-log").trace(boost::format("finish"));
  }

  int id_;
};

int main()
{
  try {
    Logging::Logger &l = Logging::Manager::instance_ref();
    l.get("test-log").addHandler(Logging::Log::LT_CONSOLE);
    l.get("test-log").setLevel(Logging::Log::LL_TRACE);

    LOGGER("test-log").trace("creating threads");

    boost::thread_group threads;
    for (int i = 0; i < 5; ++i) {
        threads.create_thread(TestContext(i)); 
    }
    threads.join_all();
    return 0;
  }
  catch (std::exception &_e) {
    std::cerr << "error occured (" << _e.what() << ")" << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "exception occured" << std::endl;
    return 2;
  }
}
