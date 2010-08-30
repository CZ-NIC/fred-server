//messages_impl.cc
#include "messages_impl.h"

#include "db_settings.h"
#include "log/logger.h"
#include "log/context.h"

#include <stdexcept>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Registry
{
namespace MessagesImpl
{


void send_sms_impl(const char* contact_handle
        , const char* phone
        , const char* content)
{
    try
    {//TODO: turn impl into private methods
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        std::string cid_query
            = "SELECT id, historyid FROM object_registry WHERE name=$1::text";
        Database::QueryParams cid_qparams
            = Database::query_param_list (contact_handle);//$1
        Database::Result cid_res = conn.exec_params( cid_query, cid_qparams );
        unsigned long long contact_id = 0;
        unsigned long long contact_historyid = 0;
        if ((cid_res.size() == 1) && (cid_res[0].size() == 2))
        {
            //get data
            contact_id = static_cast<unsigned long long>(cid_res[0][0]);
            contact_historyid = static_cast<unsigned long long>(cid_res[0][1]);
        }//if cid_res size
        else
            throw std::runtime_error
                (std::string("MessagesImpl::send_sms_impl: unexpected cid_query"
                        " result, invalid contact_handle: ") + contact_handle);

        std::string msg_query
            = "INSERT INTO message_archive"
              " (crdate, moddate, attempt, status, comm_type_id, message_type_id)"
              " VALUES (CURRENT_TIMESTAMP, $1::timestamp without time zone "
              " , $2::smallint, $3::integer,"
              " (SELECT id FROM comm_type WHERE type ='sms'), $4::integer)";
        Database::QueryParams msg_qparams
            = Database::query_param_list
              (Database::QPNull)//$1 moddate
              (0)//$2 attempt
              (1)//$3 status
              (Database::QPNull)//$4 message_type_id
            ;

        conn.exec_params( msg_query, msg_qparams );

        std::string msgid_query
            = "SELECT currval('message_archive_id_seq') AS id";
        Database::Result msgid_res = conn.exec( msgid_query);
        unsigned long long message_archive_id = 0;

        if (msgid_res.size() == 1)
            message_archive_id
                = static_cast<unsigned long long>(msgid_res[0][0]);

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
        LOGGER(PACKAGE).error(boost::format("MessagesImpl::send_sms_impl exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("MessagesImpl::send_sms_impl error");
    }
}

void send_letter_impl(const char* contact_handle
        , const PostalAddress& address
        , const ByteBuffer& file_content
        , const char* file_type)
{
    try
    {
        Database::Connection conn = Database::Manager::acquire();

    }//try
    catch(const std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("MessagesImpl::send_letter_impl exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("MessagesImpl::send_letter_impl error");
    }
}

}//namespace MessagesImpl
}//namespace Registry
