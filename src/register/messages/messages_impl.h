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
 *  @messages_impl.h
 *  header of registry messages
 */

#ifndef MESSAGES_IMPL_H_
#define MESSAGES_IMPL_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>

#include "register/db_settings.h"
#include "model/model_filters.h"

#include "util/obj_types.h"


namespace Register
{
namespace Messages
{

struct PostalAddress
{
    std::string name;
    std::string org;
    std::string street1;
    std::string street2;
    std::string street3;
    std::string city;
    std::string state;
    std::string code;
    std::string country;
};//struct PostalAddress

unsigned long long get_filetype_id(std::string file_type);

struct letter_proc {
    unsigned long long file_id;
    unsigned long long letter_id;
    unsigned attempt;
    std::string fname;
};//info for letter processing

typedef std::vector<letter_proc> LetterProcInfo;

struct sms_proc {
    unsigned long long sms_id;
    unsigned attempt;
    std::string phone_number;
    std::string content;
    std::string new_status;
};//info for sms processing

typedef std::vector<sms_proc> SmsProcInfo;

struct EnumListItem
{
  unsigned long long id;
  std::string name;
};
typedef std::vector<EnumListItem> EnumList;

//status names
EnumList getStatusListImpl();
//communication types
EnumList getCommTypeListImpl();
//message types
EnumList getMessageTypeListImpl();

/// template param OBJECT_META_INFO implementation
struct MessageMetaInfo
{
	enum MemberType {
	  MT_ID, ///< id
	  MT_CRDATE, ///< create datetime
	  MT_MODDATE, ///< modification datetime
	  MT_ATTEMPT, ///< send attempt
	  MT_STATUS, ///< message status
	  MT_COMMTYPE, ///< type of communication channel
	  MT_MSGTYPE ///< type of message
	};// columns identification
	enum Columns { columns = 7};//number of columns
};//struct MessageMetaInfo

typedef ObjType<MessageMetaInfo> Message;//object type used by pagetable
typedef Message::ObjPtr MessagePtr;// made from shared_ptr

class MessageReload //RELOAD_FUNCTOR for messages
{
	typedef ObjType<MessageMetaInfo> Object;
	typedef Object::ObjPtr  ObjPtr;
	typedef std::vector<ObjPtr> ListType;

public:
	MessageReload()
	{}

	void operator()(Database::Filters::Union &uf
			, ListType& list
			, std::size_t& _limit
			,  bool& loadLimitActive_)
	{
	    list.clear();
	    uf.clearQueries();

	    bool at_least_one = false;
	    Database::SelectQuery info_query;
	    std::auto_ptr<Database::Filters::Iterator> fit(uf.createIterator());
	    for (fit->first(); !fit->isDone(); fit->next())
	    {
	    	Database::Filters::Message *f =
	          dynamic_cast<Database::Filters::Message*>(fit->get());
	      if (!f)
	        continue;

	      Database::SelectQuery *tmp = new Database::SelectQuery();
	        tmp->addSelect(
				"id crdate moddate attempt status_id comm_type_id message_type_id"
				,f->joinMessageArchiveTable());
	        tmp->order_by() << "1 DESC";

	      uf.addQuery(tmp);
	      at_least_one = true;
	    }//for filters
	    if (!at_least_one) {
	      LOGGER(PACKAGE).error("wrong filter passed for reload!");
	      return;
	    }
	    uf.serialize(info_query);
	    std::string info_query_str = str(boost::format("%1% LIMIT %2%")
			% info_query.str() % (_limit+1));//try select more than limit
	    LOGGER(PACKAGE).debug(boost::format("reload(uf) ObjList query: %1%")
			% info_query_str);
	    try
	    {
	      Database::Connection conn = Database::Manager::acquire();
	      Database::Result res = conn.exec(info_query_str);

	      std::size_t result_size = res.size();

	      if( result_size > _limit )//check if selected more than limit
	      {
	          loadLimitActive_ = true;
	          result_size = _limit;//copy only limited number of rows
	      }
	      else
	      loadLimitActive_= false;

	      //enumlists
	      //status names
	      EnumList status_names_ = getStatusListImpl();
	      std::map<std::size_t, std::string> status_names;
	      for (EnumList::const_iterator i = status_names_.begin()
	    		  ; i != status_names_.end(); ++i)
			  status_names[i->id] = i->name;

	      //communication types
	      EnumList comm_types_ = getCommTypeListImpl();
	      std::map<std::size_t, std::string> comm_types;
	      for (EnumList::const_iterator i = comm_types_.begin()
	    		  ; i != comm_types_.end(); ++i)
	    	  comm_types[i->id] = i->name;

	      //message types
	      EnumList msg_types_ = getMessageTypeListImpl();
	      std::map<std::size_t, std::string> msg_types;
	      for (EnumList::const_iterator i = msg_types_.begin()
	    		  ; i != msg_types_.end(); ++i)
	    	  msg_types[i->id] = i->name;


	      list.reserve(result_size);//allocate list by size
	      list.clear();
	      for (std::size_t i=0; i < result_size; i++)
	      {
	          ObjPtr objptr(new Object);
	          for (std::size_t j=0; j < res[i].size(); j++)
	          {/*
	        	  LOGGER(PACKAGE).debug(
	        			  boost::format("i: %1% j: %2% data: %3%")
						  % i % j % res[i][j]);
	        	  */

	        	  switch(j)
	        	  {
	        	  case MessageMetaInfo::MT_STATUS :
		        	  objptr->set(static_cast<MessageMetaInfo::MemberType>(j)
		            		  ,status_names[static_cast<std::size_t>(res[i][j])]);//for j col
		        	  break;
	        	  case MessageMetaInfo::MT_COMMTYPE :
		        	  objptr->set(static_cast<MessageMetaInfo::MemberType>(j)
		            		  ,comm_types[static_cast<std::size_t>(res[i][j])]);//for j col
		        	  break;
	        	  case MessageMetaInfo::MT_MSGTYPE :
		        	  objptr->set(static_cast<MessageMetaInfo::MemberType>(j)
		            		  ,msg_types[static_cast<std::size_t>(res[i][j])]);//for j col
		        	  break;
				  default :
		        	  objptr->set(static_cast<MessageMetaInfo::MemberType>(j)
		            		  ,res[i][j]);//for j col
		        	  break;
	        	  }


	          }
	          list.push_back(objptr);
	      }//for i row
	    }//try
	    catch (std::exception& ex)
	    {
	      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
	    }
	}
};//class MessageReload

typedef ObjList<MessageMetaInfo
	, MessageReload > MessageList;


class Manager : boost::noncopyable
{
public:

    //save message for later send
    unsigned long long save_sms_to_send(const char* contact_handle
            , const char* phone
            , const char* content
            , const char* message_type
            , unsigned long contact_object_registry_id
            , unsigned long contact_history_historyid
            );

    //save message for later send
    unsigned long long save_letter_to_send(const char* contact_handle
            , const PostalAddress& address
            , unsigned long long file_id
            , const char* message_type
            , unsigned long contact_object_registry_id
            , unsigned long contact_history_historyid
            );

    //load saved letters
    LetterProcInfo load_letters_to_send(std::size_t batch_size_limit);

    //load saved sms
    SmsProcInfo load_sms_to_send(std::size_t batch_size_limit);

    //set send result into letter status
    void set_letter_status(const LetterProcInfo& letters
            ,const std::string& new_status, const std::string& batch_id);

    //set send result into sms status
    void set_sms_status(const SmsProcInfo& messages);

    ///list factory
    typedef std::auto_ptr<MessageList> MessageListPtr;
    MessageListPtr createList();

    ///status names
    EnumList getStatusList();

    ///communication types
    EnumList getCommTypeList();

    ///message types
    EnumList getMessageTypeList();
};

typedef boost::shared_ptr<Manager> ManagerPtr;
ManagerPtr create_manager();


}//namespace Messages
}//namespace Register
#endif //MESSAGES_IMPL_H_
