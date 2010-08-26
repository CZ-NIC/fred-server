//messages.h

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include "messages/messages_impl.h"

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "register/db_settings.h"
#include "corba_wrapper.h"
#include "log/logger.h"
#include "log/context.h"

#include "pidfile.h"
#include "daemonize.h"

#include "config_handler.h"
#include "handle_general_args.h"
#include "handle_database_args.h"
#include "handle_corbanameservice_args.h"

//code for implementing IDL interfaces in file /usr/local/share/idl/fred//Messages.idl
#include "corba/Messages.hh"

//class implementing IDL interface Registry::Messages
class Registry_Messages_i: public POA_Registry::Messages
{
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  virtual ~Registry_Messages_i();
public:
  // standard constructor
  Registry_Messages_i();

  // methods corresponding to defined IDL attributes and operations
  void sendSms(const char* contact_handle
          ,const char* phone
          ,const char* content);

  void sendLetter(const char* contact_handle
          ,const Registry::Messages::PostalAddress& address
          ,const Registry::Messages::ByteBuffer& file_content
          ,const char* file_type);

};//class Registry_Messages_i

#endif //MESSAGES_H_
