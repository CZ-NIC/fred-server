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

#ifndef MESSAGES_IMPL_HH_9D52FB7BDA1B4718A5F1E03C5FD37EB1
#define MESSAGES_IMPL_HH_9D52FB7BDA1B4718A5F1E03C5FD37EB1

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/utility.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>

#include "libfred/db_settings.hh"
#include "src/deprecated/model/model_filters.hh"

#include "src/deprecated/libfred/obj_types.hh"


namespace LibFred
{
namespace Messages
{


struct MessageCopyProhibited : virtual std::exception
{
    const char* what() const noexcept
    {
        return "message copy is prohibited by status";
    }
};


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

class WrongPostalAddress : public std::runtime_error
{
public:
    WrongPostalAddress(const std::string& what,const PostalAddress & wrong_postal_address)
    : std::runtime_error(std::string("wrong postal address: ") + what
        + std::string(" name: ") + wrong_postal_address.name
        + std::string(" org: ") + wrong_postal_address.org
        + std::string(" street1: ") + wrong_postal_address.street1
        + std::string(" street2: ") + wrong_postal_address.street2
        + std::string(" street3: ") + wrong_postal_address.street3
        + std::string(" state: ") + wrong_postal_address.state
        + std::string(" city: ") + wrong_postal_address.city
        + std::string(" code: ") + wrong_postal_address.code
        + std::string(" country: ") + wrong_postal_address.country
        )
    {}
};

/**
 * Check that postal address fields: name, street1, city, postal code and country are not empty after trimming spaces.
 * @throw std::runtime_error if not filled address field found
 */
void check_postal_address(const PostalAddress& address);

unsigned long long get_filetype_id(std::string file_type);

struct letter_proc {
    unsigned long long file_id;
    unsigned long long letter_id;
    unsigned attempt;
    std::string message_type;
    std::string fname;
    PostalAddress postal_address;
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

//for detail
struct SmsInfo
{
    std::string phone_number;
    std::string content;
};
struct LetterInfo
{
    unsigned long long file_id;
    std::string fname;
    std::string batch_id;
    PostalAddress postal_address;
};


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
	enum MemberOrderType {
	  MT_ID, ///< id
	  MT_CRDATE, ///< create datetime
	  MT_MODDATE, ///< modification datetime
	  MT_ATTEMPT, ///< send attempt
	  MT_STATUS, ///< message status
	  MT_COMMTYPE, ///< type of communication channel
	  MT_MSGTYPE ///< type of message
	};// columns order identification
	enum Columns { columns = 7};//number of columns

	//member conversion type specification
	static ObjMemberConversion::Type member_conversion_type(MemberOrderType col)
	{
	    ObjMemberConversion::Type mct[columns];

        mct[MT_ID] = ObjMemberConversion::MC_NONE; //id
        mct[MT_CRDATE] = ObjMemberConversion::MC_DATETIME; //crdate
        mct[MT_MODDATE] = ObjMemberConversion::MC_DATETIME; //moddate
        mct[MT_ATTEMPT] = ObjMemberConversion::MC_ULONG; //attempt
        mct[MT_STATUS] = ObjMemberConversion::MC_NONE; //status
        mct[MT_COMMTYPE] = ObjMemberConversion::MC_NONE; //commtype
        mct[MT_MSGTYPE] = ObjMemberConversion::MC_NONE; //msgtype

	    return mct[col];
	}//member_conversion_type

};//struct MessageMetaInfo

typedef ObjType<MessageMetaInfo> Message;//object type used by pagetable
typedef Message::ObjPtr MessagePtr;// made from shared_ptr

class MessageReload //RELOAD_FUNCTOR for messages
{
	typedef ObjType<MessageMetaInfo> Object;
	typedef Object::ObjPtr  ObjPtr;
	typedef std::vector<ObjPtr> ListType;
public:
	MessageReload()	{}
	void operator()(Database::Filters::Union &uf, ListType& list
			, std::size_t& _limit,  bool& loadLimitActive_);
};//class MessageReload

typedef ObjList<MessageMetaInfo, MessageReload > MessageList;


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
            , const std::string& comm_type //letter or registered_letter
            , bool do_check_postal_address //if true then check postall that address fields: name, street1, city, postalcode and country contains something beside the spaces, if false then don't
            );

    //cancel letter sending if hasn't been already sent
    void cancel_letter_send(unsigned long long letter_id);

    //copy message for later send
    unsigned long long copy_letter_to_send(unsigned long long letter_id);

    //copy sms for later send
    unsigned long long copy_sms_to_send(unsigned long long sms_id);

    //load saved letters
    LetterProcInfo load_letters_to_send(std::size_t batch_size_limit
            , const std::string &comm_type, const std::string& service_handle
            , std::size_t max_attempts_limit);

    //load saved sms
    SmsProcInfo load_sms_to_send(std::size_t batch_size_limit, const std::string& service_handle, std::size_t max_attempts_limit);

    //set send result into letter status
    void set_letter_status(const LetterProcInfo& letters
            ,const std::string& new_status, const std::string& batch_id
            , const std::string &comm_type
            , const std::string& service_handle
            , const std::size_t max_attempts_limit);

    //set send result into sms status
    void set_sms_status(const SmsProcInfo& messages
            , const std::string& service_handle
            , const std::size_t max_attempts_limit);

    //get sms data by id
    SmsInfo get_sms_info_by_id(unsigned long long id);
    //get letter data by id
    LetterInfo get_letter_info_by_id(unsigned long long id);

    ///list factory
    typedef std::unique_ptr<MessageList> MessageListPtr;
    MessageListPtr createList();

    ///status names
    EnumList getStatusList();

    ///communication types
    EnumList getCommTypeList();

    ///message types
    EnumList getMessageTypeList();
};

typedef std::shared_ptr<Manager> ManagerPtr;
ManagerPtr create_manager();


} // namespace Messages
} // namespace LibFred
#endif
