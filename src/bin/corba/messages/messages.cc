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
 *  @messages.cc
 *  corba server implementation of registry messages
 */

#include "src/bin/corba/messages/messages.hh"
#include "src/bin/corba/messages/messages_filemanager.hh"

using namespace std;

//implementational code for IDL interface Registry::Messages
Registry_Messages_i::Registry_Messages_i(LibFred::Messages::ManagerPtr msgmgr)
    : msgmgr_(msgmgr)
{
  // add extra constructor code here
}
Registry_Messages_i::~Registry_Messages_i()
{
  // add extra destructor code here
}
//   Methods corresponding to IDL attributes and operations
CORBA::ULongLong Registry_Messages_i::saveSmsToSend(const char* contact_handle
        , const char* phone
        , const char* content
        , const char* message_type
        , CORBA::ULongLong contact_object_registry_id
        , CORBA::ULongLong contact_history_historyid
       )
{
    Logging::Context ctx(server_name);
    ConnectionReleaser releaser;

    try
    {
        return msgmgr_->save_sms_to_send(contact_handle,phone, content
                , message_type
                , contact_object_registry_id
                , contact_history_historyid
                );//call of impl
    }//try
    catch(std::exception& ex)
    {
        throw Registry::Messages::ErrorReport(ex.what());
    }
    catch(...)
    {
        throw Registry::Messages::ErrorReport("unknown exception");
    }
}//Registry_Messages_i::saveSmsToSend

CORBA::ULongLong Registry_Messages_i::saveLetterToSend(const char* contact_handle
        , const Registry::Messages::PostalAddress& address
        , const Registry::Messages::ByteBuffer& file_content
        , const char* file_name
        , const char* file_type
        , const char* message_type
        , CORBA::ULongLong contact_object_registry_id
        , CORBA::ULongLong contact_history_historyid
        , const char* comm_type //letter or registered_letter
        )
{
    Logging::Context ctx(server_name);
    ConnectionReleaser releaser;

    try
    {
        LOGGER.debug(boost::format(
                  "Registry_Messages_i::saveLetterToSend"
                " contact_handle: %1%")
            % contact_handle);

        LibFred::Messages::PostalAddress address_impl;
        address_impl.name = std::string(address.name.in());
        address_impl.org = std::string(address.org.in());
        address_impl.street1 = std::string(address.street1.in());
        address_impl.street2 = std::string(address.street2.in());
        address_impl.street3 = std::string(address.street3.in());
        address_impl.city = std::string(address.city.in());
        address_impl.state = std::string(address.state.in());
        address_impl.code = std::string(address.code.in());
        address_impl.country = std::string(address.country.in());

        std::size_t file_content_size= file_content.length();
        unsigned char* file_content_buffer
            = const_cast<unsigned char*>(file_content.get_buffer());

        unsigned long long filetype_id = LibFred::Messages::get_filetype_id(file_type);

        std::vector<char> file_buffer(file_content_buffer, file_content_buffer+file_content_size) ;

        unsigned long long file_id = 0;

        //call filemanager client
        file_id = save_file(file_buffer
                , file_name
                , "application/pdf"
                , filetype_id );

        return msgmgr_->save_letter_to_send(contact_handle
                , address_impl
                , file_id
                , message_type
                , contact_object_registry_id
                , contact_history_historyid
                , comm_type
                , true
                );//call of impl

    }//try
    catch(std::exception& ex)
    {
        throw Registry::Messages::ErrorReport(ex.what());
    }
    catch(...)
    {
        throw Registry::Messages::ErrorReport("unknown exception");
    }
}//Registry_Messages_i::saveLetterToSend

Registry::Messages::EnumList* Registry_Messages_i::getStatusList()
{
    Logging::Context ctx(server_name);
    ConnectionReleaser releaser;

    LOGGER.debug("Registry_Messages_i::getStatusList");
    try
    {
        LibFred::Messages::EnumList el = msgmgr_->getStatusList();
        Registry::Messages::EnumList_var ret = new Registry::Messages::EnumList;
        ret->length(el.size());
        for(std::size_t i = 0; i < el.size(); ++i)
        {
            ret[i].id = el[i].id;
            ret[i].name = CORBA::string_dup(el[i].name.c_str());
        }
        return ret._retn();
    }//try
    catch(std::exception& ex)
    {
        throw Registry::Messages::ErrorReport(ex.what());
    }
    catch(...)
    {
        throw Registry::Messages::ErrorReport("unknown exception");
    }
}//Registry_Messages_i::getStatusList

Registry::Messages::EnumList* Registry_Messages_i::getCommTypeList()
{
    Logging::Context ctx(server_name);
    ConnectionReleaser releaser;

    LOGGER.debug("Registry_Messages_i::getCommTypeList");
    try
    {
        LibFred::Messages::EnumList el = msgmgr_->getCommTypeList();
        Registry::Messages::EnumList_var ret = new Registry::Messages::EnumList;
        ret->length(el.size());
        for(std::size_t i = 0; i < el.size(); ++i)
        {
            ret[i].id = el[i].id;
            ret[i].name = CORBA::string_dup(el[i].name.c_str());
        }
        return ret._retn();
    }//try
    catch(std::exception& ex)
    {
        throw Registry::Messages::ErrorReport(ex.what());
    }
    catch(...)
    {
        throw Registry::Messages::ErrorReport("unknown exception");
    }
}//Registry_Messages_i::getCommTypeList

Registry::Messages::EnumList* Registry_Messages_i::getMessageTypeList()
{
    Logging::Context ctx(server_name);
    ConnectionReleaser releaser;

    LOGGER.debug("Registry_Messages_i::getMessageTypeList");
    try
    {
        LibFred::Messages::EnumList el = msgmgr_->getMessageTypeList();
        Registry::Messages::EnumList_var ret = new Registry::Messages::EnumList;
        ret->length(el.size());
        for(std::size_t i = 0; i < el.size(); ++i)
        {
            ret[i].id = el[i].id;
            ret[i].name = CORBA::string_dup(el[i].name.c_str());
        }
        return ret._retn();
    }//try
    catch(std::exception& ex)
    {
        throw Registry::Messages::ErrorReport(ex.what());
    }
    catch(...)
    {
        throw Registry::Messages::ErrorReport("unknown exception");
    }
}//Registry_Messages_i::getMessageTypeList
