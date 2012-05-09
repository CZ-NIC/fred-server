#include <vector>
#include <map>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

// logger
#include "log/logger.h"
#include "log/context.h"
#include "old_utils/log.h"

class NumberSessionLimit : public std::runtime_error {
public:
    NumberSessionLimit(const std::string &msg) : std::runtime_error(msg)
    {  }
};

struct Session
{
public:
  int registrarID;
  int language;
  long long timestamp;

  Session()
  : registrarID(0)
  , language(0)
  , timestamp(0)
  { }

  Session(int _registrarID, int _language, long long _timestamp) :
      registrarID(_registrarID),
      language(_language),
      timestamp(_timestamp)
  { }

};

class EppSessionContainer {
private:
    boost::mutex session_mutex;

    unsigned session_timeout_sec;

    unsigned max_total_sessions;
    unsigned max_reg_sessions;

    std::map<unsigned long long, Session> sessions;
    std::map<int, int> registrar_session_count;

public:
    EppSessionContainer(unsigned max_total , unsigned max_reg, unsigned wait) :
        session_timeout_sec(wait),
        max_total_sessions(max_total),
        max_reg_sessions(max_reg),
        sessions(),
        registrar_session_count()
    {  }

    /// returns loginID, begins with garbage procedure
    unsigned long long login_session(int regId, int lang);
    void logout_session(unsigned long long loginId);
    void destroy_all_registrar_sessions(long regId);
    int get_registrar_id(unsigned long long loginId);
    int get_registrar_lang(unsigned long long loginId);
    /// session_mutext must already be acquired
    void update_timestamp(unsigned long long loginId);

private:

    // session_mutex must already be acquired
    void logout_session_worker(unsigned long long loginId);
    // session_mutex must already be acquired
    void garbage_session_internal();

};
