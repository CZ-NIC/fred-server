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

Log& Logger::get(const std::string& _ctx) {
  std::string prefixed_ctx = ctx_prefix + (ctx_prefix.empty() ? "" : "/") + _ctx;
  std::map<std::string, Log*>::iterator it = logs.find(prefixed_ctx);
  if (it != logs.end()) {
    return *it->second;
  } else {
    Log *l = new Log();
    l->setContext(prefixed_ctx);
    logs.insert(std::make_pair<std::string, Log*>(prefixed_ctx, l));
    return *l;
  }
}

void Logger::add(const std::string& _ctx, Log *_l) {
  std::string prefixed_ctx = ctx_prefix + (ctx_prefix.empty() ? "" : "/") + _ctx;
  std::map<std::string, Log*>::iterator it = logs.find(prefixed_ctx);
  if (it == logs.end()) {
    _l->setContext(prefixed_ctx);
    logs.insert(std::make_pair<std::string, Log*>(prefixed_ctx, _l));
  }
}

void Logger::prefix(const std::string& _prefix) {
  if (ctx_prefix.empty()) {
    ctx_prefix = _prefix;
  }
}

}
