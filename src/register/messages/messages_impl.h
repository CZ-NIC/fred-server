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
#include <memory>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "register/db_settings.h"
#include "model/model_filters.h"


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


/// message attributes and specific parameters
struct Message
{
    unsigned long long id;
    boost::posix_time::ptime crdate;
    boost::posix_time::ptime moddate;
    unsigned attempt;
    std::string status;
    std::string comm_type;
    std::string message_type;
};

typedef boost::shared_ptr<Message> MessagePtr;

/// member identification (i.e. for sorting)
enum MemberType {
  MT_ID, ///< id
  MT_CRDATE, ///< create datetime
  MT_MODDATE, ///< modification datetime
  MT_ATTEMPT, ///< send attempt
  MT_STATUS, ///< message status
  MT_COMMTYPE, ///< type of communication channel
  MT_MSGTYPE ///< type of message
};


class CompareId
{
    bool asc_;
public:
    CompareId(bool _asc)
      : asc_(_asc) { }
  bool operator()(MessagePtr _left, MessagePtr _right) const
  {
    return (asc_ ? (_left->id < _right->id) : (_left->id > _right->id));
  }
};//class CompareId

class CompareCrDate
{
    bool asc_;
public:
    CompareCrDate(bool _asc)
      : asc_(_asc) { }
  bool operator()(MessagePtr _left, MessagePtr _right) const
  {
    return (asc_ ? (_left->crdate < _right->crdate) : (_left->crdate > _right->crdate));
  }
};//class CompareCrDate

class CompareModDate
{
    bool asc_;
public:
    CompareModDate(bool _asc)
      : asc_(_asc) { }
  bool operator()(MessagePtr _left, MessagePtr _right) const
  {
    return (asc_ ? (_left->moddate < _right->moddate) : (_left->moddate > _right->moddate));
  }
};//class CompareModDate

class CompareAttempt
{
    bool asc_;
public:
    CompareAttempt(bool _asc)
      : asc_(_asc) { }
  bool operator()(MessagePtr _left, MessagePtr _right) const
  {
    return (asc_ ? (_left->attempt < _right->attempt) : (_left->attempt > _right->attempt));
  }
};//class CompareAttempt

class CompareStatus
{
    bool asc_;
public:
    CompareStatus(bool _asc)
      : asc_(_asc) { }
  bool operator()(MessagePtr _left, MessagePtr _right) const
  {
    return (asc_ ? (_left->status < _right->status) : (_left->status > _right->status));
  }
};//class CompareStatus

class CompareCommType
{
    bool asc_;
public:
    CompareCommType(bool _asc)
      : asc_(_asc) { }
  bool operator()(MessagePtr _left, MessagePtr _right) const
  {
    return (asc_ ? (_left->comm_type < _right->comm_type) : (_left->comm_type > _right->comm_type));
  }
};//class CompareCommType

class CompareMsgType
{
    bool asc_;
public:
    CompareMsgType(bool _asc)
      : asc_(_asc) { }
  bool operator()(MessagePtr _left, MessagePtr _right) const
  {
    return (asc_ ? (_left->message_type < _right->message_type) : (_left->message_type > _right->message_type));
  }
};//class CompareMsgType



class MessageList
{
    typedef std::vector<MessagePtr> ListType;
    ListType ml_;

    bool m_loadLimitActive;

    public:
    MessagePtr get(std::size_t row)
    {
        return ml_.at(row);
    }

    std::size_t size()
    {
        return ml_.size();
    }

    void sort(MemberType _member, bool _asc)
    {
        switch (_member)
        {
          case MT_ID:
            std::stable_sort(ml_.begin(), ml_.end(), CompareId(_asc));
            break;
          case MT_CRDATE:
            std::stable_sort(ml_.begin(), ml_.end(), CompareCrDate(_asc));
            break;
          case MT_MODDATE:
            std::stable_sort(ml_.begin(), ml_.end(), CompareModDate(_asc));
            break;
          case MT_ATTEMPT:
            std::stable_sort(ml_.begin(), ml_.end(), CompareAttempt(_asc));
            break;
          case MT_STATUS:
            std::stable_sort(ml_.begin(), ml_.end(), CompareAttempt(_asc));
            break;
          case MT_COMMTYPE:
            std::stable_sort(ml_.begin(), ml_.end(), CompareCommType(_asc));
            break;
          case MT_MSGTYPE:
            std::stable_sort(ml_.begin(), ml_.end(), CompareMsgType(_asc));
            break;
        }

    }

    void reload(Database::Filters::Union &uf)
    {


    }

    void clear()
    {
        ml_.clear();
    }

    unsigned long long getRealCount(Database::Filters::Union &filter)
    {

        return 0;
    }

    virtual Register::Messages::MessagePtr findId(unsigned long long id) const
    {
        for(ListType::const_iterator it =ml_.begin()
                ; it != ml_.end(); ++it)
        {
            if((*it)->id == id)
                return *it;
        }

        LOGGER(PACKAGE).debug(boost::format("object list miss! object id=%1% not found")
        % id);
        throw std::runtime_error("message id not found");
    }

    bool isLimited() const
    {
        return m_loadLimitActive;
    }

};


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
};

typedef boost::shared_ptr<Manager> ManagerPtr;
ManagerPtr create_manager();


}//namespace Messages
}//namespace Register
#endif //MESSAGES_IMPL_H_
