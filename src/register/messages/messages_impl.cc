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
 *  @messages_impl.cc
 *  implementation of registry messages
 */

#include "messages_impl.h"
#include "messages/messages_filemanager.h"
#include "db_settings.h"
#include "log/logger.h"
#include "log/context.h"

#include <stdexcept>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Registry
{
namespace MessagesImpl
{

//insert data into message_archive and optionally into message_contact_history_map
//return new message_archive id
unsigned long long save_message(Database::QueryParam moddate// = Database::QPNull
        , Database::QueryParam attempt// = 0
        , Database::QueryParam status// = 1
        , const std::string message_type// = Database::QPNull
        , const std::string comm_type// = std::string("")//sms, letter, email
        , bool save_contact_reference// = true
        , const std::string contact_handle// = std::string("")
        , unsigned long contact_object_registry_id
        , unsigned long contact_history_historyid
        )
{
    LOGGER(PACKAGE).debug(boost::format(
            "MessagesImpl::save_message "
            " moddate: %1% "
            " attempt: %2% "
            " status: %3% "
            " message_type: %4% "
            " comm_type: %5% "
            " save_contact_reference: %6% "
            " contact_handle: %7% "
            " contact_object_registry_id: %8%"
            " contact_history_historyid: %9%"

            )
                  % moddate.print_buffer()
                  % attempt.print_buffer()
                  % status.print_buffer()
                  % message_type
                  % comm_type
                  % save_contact_reference
                  % contact_handle
                  % contact_object_registry_id
                  % contact_history_historyid
                  );

    Database::Connection conn = Database::Manager::acquire();

    std::string message_type_id_query
        = "SELECT id FROM message_type WHERE type= $1::text";
    Database::QueryParams  message_type_id_qparams = Database::query_param_list
            (message_type);
    Database::Result message_type_id_res = conn.exec_params( message_type_id_query
            , message_type_id_qparams);
    unsigned long long message_type_id = 0;
    if (message_type_id_res.size() == 1)
        message_type_id
            = static_cast<unsigned long long>(message_type_id_res[0][0]);

    std::string msg_query
        = "INSERT INTO message_archive"
          " (crdate, moddate, attempt, status, comm_type_id, message_type_id)"
          " VALUES (CURRENT_TIMESTAMP, $1::timestamp"// without time zone "
          " , $2::smallint, $3::integer,"
          " (SELECT id FROM comm_type WHERE type = $4::text), $5::integer)";
    Database::QueryParams msg_qparams
        = Database::query_param_list
          (Database::QueryParam())//$1 moddate
          (attempt)//$2 attempt
          (status)//$3 status
          (comm_type)//$4 comm_type
          (message_type_id)//$5 message_type_id
        ;

    conn.exec_params( msg_query, msg_qparams );

    std::string msgid_query
        = "SELECT currval('message_archive_id_seq') AS id";
    Database::Result msgid_res = conn.exec( msgid_query);
    unsigned long long message_archive_id = 0;

    if (msgid_res.size() == 1)
        message_archive_id
            = static_cast<unsigned long long>(msgid_res[0][0]);

    if(save_contact_reference)
    {

        std::string msg_contact_query
            = "INSERT INTO message_contact_history_map"
              " (contact_object_registry_id, contact_history_historyid"
              " , message_archive_id)"
              " VALUES ( $1::integer, $2::integer, $3::integer)";
        Database::QueryParams msg_contact_qparams
            = Database::query_param_list
              (contact_object_registry_id)//$1 object_registry id
              (contact_history_historyid)//$2 object_registry historyid
              (message_archive_id)//$3 message_archive id
            ;

        conn.exec_params( msg_contact_query, msg_contact_qparams );
    }//if(save_contact_reference)

    return message_archive_id;
}

void send_sms_impl(const char* contact_handle
        , const char* phone
        , const char* content
        , const char* message_type
        , unsigned long contact_object_registry_id
        , unsigned long contact_history_historyid

        )
{
    try
    {
        LOGGER(PACKAGE).debug(boost::format(
                "MessagesImpl::send_sms_impl "
                " contact_handle: %1% "
                " phone: %2% "
                " content: %3% "
                " message_type: %4% "
                " contact_object_registry_id: %5%"
                " contact_history_historyid: %6%"
                )
                      % contact_handle
                      % phone
                      % content
                      % message_type
                      % contact_object_registry_id
                      % contact_history_historyid
                      );

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        unsigned long long message_archive_id
        = save_message(
                Database::QueryParam()
                , 0
                , 1
                , message_type
                ,"sms"
                , true
                , contact_handle
                , contact_object_registry_id
                , contact_history_historyid
                );

        std::string sms_query
            = "INSERT INTO sms_archive"
              " (id, phone_number, content)"
              " VALUES ( $1::integer, $2::varchar(64), $3::text)";
        Database::QueryParams sms_qparams
            = Database::query_param_list
              (message_archive_id)//$1 id
              (phone)//$2 phone_number
              (content)//$3 status
            ;

        conn.exec_params( sms_query, sms_qparams );
        tx.commit();

    }//try
    catch(const std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format(
                "MessagesImpl::send_sms_impl exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("MessagesImpl::send_sms_impl error");
    }
}

void send_letter_impl(const char* contact_handle
        , const PostalAddress& address
        , const ByteBuffer& file_content
        , const char* file_name
        , const char* file_type
        , const char* message_type
        , unsigned long contact_object_registry_id
        , unsigned long contact_history_historyid
        )
{
    try
    {
        LOGGER(PACKAGE).debug(boost::format(
                "MessagesImpl::send_letter_impl "
                " contact_handle: %1% "
                " file_content.size(): %2%"
                " address.name: %3%"
                " address.org: %4%"
                " address.street1: %5%"
                " address.street2: %6%"
                " address.street3: %7%"
                " address.city: %8%"
                " address.state: %9%"
                " address.code: %10%"
                " address.county: %11%"
                " file_name: %12% "
                " file_type: %13% "
                " message_type: %14% "
                " contact_object_registry_id: %15%"
                " contact_history_historyid: %16%"
                )
                      % contact_handle
                      % file_content.size
                      % address.name
                      % address.org
                      % address.street1
                      % address.street2
                      % address.street3
                      % address.city
                      % address.state
                      % address.code
                      % address.country
                      % file_name
                      % file_type
                      % message_type
                      % contact_object_registry_id
                      % contact_history_historyid
                      );



        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        unsigned long long message_archive_id
        = save_message(Database::QPNull, 0, 1,message_type, "letter", true
                , contact_handle, contact_object_registry_id
                , contact_history_historyid
);

        std::string filetype_id_query
            = "SELECT id FROM enum_filetype WHERE \"name\"= $1::text";
        Database::QueryParams  filetype_id_qparams = Database::query_param_list
                (file_type);
        Database::Result filetype_id_res = conn.exec_params( filetype_id_query
                , filetype_id_qparams);
        unsigned long long filetype_id = 0;
        if (filetype_id_res.size() == 1)
            filetype_id
                = static_cast<unsigned long long>(filetype_id_res[0][0]);

        std::vector<char> file_buffer(file_content.buffer, file_content.buffer+file_content.size) ;

        unsigned long long file_id = 0;

        //call filemanager client
        file_id = save_file(file_buffer
                , file_name
                , "application/pdf"
                , filetype_id );


        std::string letter_query
            = "INSERT INTO letter_archive"
              " (id, file_id" //$1 $2
              "  , postal_address_name"//$3
              "  , postal_address_organization"//$4
              "  , postal_address_street1"//$5
              "  , postal_address_street2"//$6
              "  , postal_address_street3"//$7
              "  , postal_address_city"//$8
              "  , postal_address_stateorprovince"//$9
              "  , postal_address_postalcode"//$10
              "  , postal_address_country"//$11
              " )"
              " VALUES ( $1::integer,$2::integer"
              ", $3::text"
              ", $4::text"
              ", $5::text"
              ", $6::text"
              ", $7::text"
              ", $8::text"
              ", $9::text"
              ", $10::text"
              ", $11::text"
              ")";

        Database::QueryParams letter_qparams
            = Database::query_param_list
              (message_archive_id)//$1 id
              (file_id)//$2 file_id
              (address.name)//$3
              (address.org)//$4
              (address.street1)//$5
              (address.street2)//$6
              (address.street3)//$7
              (address.city)//$8
              (address.state)//$9
              (address.code)//$10
              (address.country)//$11
            ;

        conn.exec_params( letter_query, letter_qparams );
        tx.commit();

    }//try
    catch(const std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format(
                "MessagesImpl::send_letter_impl exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("MessagesImpl::send_letter_impl error");
    }
}

}//namespace MessagesImpl
}//namespace Registry
