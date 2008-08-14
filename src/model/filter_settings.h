#ifndef FILTER_SETTINGS_H_
#define FILTER_SETTINGS_H_

#include <map>
#include "singleton.h"
#include "log/logger.h"

namespace Database {
namespace Filters {

class Settings_ {
public:
  Settings_() {
    set("history", "on");
  }
  
  void set(const std::string& _name, const std::string& _value) {
    LOGGER("db").info(boost::format("Filter settings -- attribut `%1%' set to `%2%'") % _name % _value);
    parameters_[_name] = _value;
  }
  
  std::string get(const std::string& _name) {
    return parameters_[_name];
  }
  
protected:
  std::map<std::string, std::string> parameters_;
};

typedef Singleton<Settings_> Settings;

}
}

#endif /*FILTER_SETTINGS_H_*/
