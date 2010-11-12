/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @messages.h
 *  corba server header of registry messages
 */

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

#include "log/logger.h"
#include "log/context.h"
#include "corba/connection_releaser.h"


//code for implementing IDL interfaces in file /usr/local/share/idl/fred//Messages.idl
#include "corba/Messages.hh"

//class implementing IDL interface Registry::Messages
class Registry_Messages_i: public POA_Registry::Messages
{
private:
    Fred::Messages::ManagerPtr msgmgr_;
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  virtual ~Registry_Messages_i();
public:
  // standard constructor
  Registry_Messages_i(Fred::Messages::ManagerPtr msgmgr);

  // methods corresponding to defined IDL attributes and operations
  CORBA::ULongLong saveSmsToSend(const char* contact_handle
          ,const char* phone
          ,const char* content
          ,const char* message_type
          , CORBA::ULongLong contact_object_registry_id
          , CORBA::ULongLong contact_history_historyid
          );

  CORBA::ULongLong saveLetterToSend(const char* contact_handle
          ,const Registry::Messages::PostalAddress& address
          ,const Registry::Messages::ByteBuffer& file_content
          ,const char* file_name
          ,const char* file_type
          ,const char* message_type
          , CORBA::ULongLong contact_object_registry_id
          , CORBA::ULongLong contact_history_historyid
          ,const char* comm_type
          );

  Registry::Messages::EnumList* getStatusList();
  Registry::Messages::EnumList* getCommTypeList();
  Registry::Messages::EnumList* getMessageTypeList();

};//class Registry_Messages_i

extern const char* server_name;//for logging contxt

#endif //MESSAGES_H_
