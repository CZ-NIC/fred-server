#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <map>
#include "singleton.h"
#include "log/logger.h"

class Settings_ {
public:
  Settings_() {
  }
  
  void set(const std::string& _name, const std::string& _value) {
    LOGGER("db").info(boost::format("settings -- attribut `%1%' set to `%2%'") % _name % _value);
    parameters_[_name] = _value;
  }
  
  std::string get(const std::string& _name) const {
    std::map<std::string, std::string>::const_iterator it = parameters_.find(_name);
    if (it != parameters_.end()) {
      return it->second;
    }
    else {
      return "not_set";
    }
  }
  
protected:
  std::map<std::string, std::string> parameters_;
};

// typedef Singleton<Settings_> Settings;
typedef Settings_ Settings;


#endif /*SETTINGS_H_*/
