#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

#include <vector>
#include <map>

#include <string>
#include <ostream>
#include <sstream>
#include <iterator>

namespace Logging {



class Context {
public:
  Context(); 

  Context(const std::string& _name);

  ~Context();
  
  static void push(const std::string& _name);
 
  static void pop();

  static std::string top();

  static std::string getNDC();

  static void add(const std::string& _attr, const std::string& _val);

  static void rem(const std::string& _attr);

  static std::string getMDC();

  static std::string get();

  static void clear();

private:
  static const std::string ndc_separator;

  struct NDCData_ {
    std::string name;
    std::string path;
  };

  typedef std::vector<NDCData_>               Stack;
  typedef std::map<std::string, std::string>  Map;
  
  struct PerThreadData_ {
    Context::Stack stack_;
    Context::Map   map_;
  };

  Context(const Context& _ctx);

  Context& operator=(const Context& _ctx);

  static Context::Stack* _getThreadStack();
  static Context::Map*   _getThreadMap();

  
  static boost::thread_specific_ptr<PerThreadData_> data_;
};




}

#endif /*CONTEXT_H_*/

