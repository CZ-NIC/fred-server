#ifndef SETTINGS_HH_2C2C96874AC44B308147B6B81E4DF6C2
#define SETTINGS_HH_2C2C96874AC44B308147B6B81E4DF6C2

#include <map>
#include "util/singleton.hh"
#include "util/log/logger.hh"

class Settings_ {
public:
  Settings_() {
  }
  
  void set(const std::string& _name, const std::string& _value) {
    LOGGER.info(boost::format("settings -- attribut `%1%' set to `%2%'") % _name % _value);
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
