#include "logger.h"

namespace Logging {

Logger::Logger() {
}

Logger::~Logger() {
  std::map<std::string, Log*>::const_iterator it = logs.begin();
  for (; it != logs.end(); ++it) {
    delete it->second;
  }
}

Log& Logger::get(const std::string& _name) {
  std::map<std::string, Log*>::iterator it = logs.find(_name);
  if (it != logs.end()) {
    return *it->second;
  } else {
    Log *l = new Log(_name);
    logs.insert(std::make_pair<std::string, Log*>(_name, l));
    return *l;
  }
}

void Logger::add(const std::string& _name, Log *_l) {
  std::map<std::string, Log*>::iterator it = logs.find(_name);
  if (it == logs.end()) {
    _l->setName(_name);
    logs.insert(std::make_pair<std::string, Log*>(_name, _l));
  }
}


}
