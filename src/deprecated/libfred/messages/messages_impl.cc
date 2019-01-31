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

#include "src/deprecated/libfred/messages/messages_impl.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/model/model_filters.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"

#include <memory>
#include <stdexcept>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <utility>

#include "libfred/db_settings.hh"

namespace LibFred
{
namespace Messages
{

//insert data into message_archive and optionally into message_contact_history_map
//return new message_archive id
unsigned long long save_message(Database::QueryParam moddate// = Database::QPNull
        , Database::QueryParam attempt// = 0
        , const std::string& status_name
        , const std::string& message_type// = Database::QPNull
        , const std::string& comm_type// = std::string("")//sms, letter, email
        , bool save_contact_reference// = true
        , const std::string contact_handle// = std::string("")
        , unsigned long contact_object_registry_id
        , unsigned long contact_history_historyid
        )
{
    LOGGER.debug(boost::format(
            "Messages::save_message "
            " moddate: %1% "
            " attempt: %2% "
            " status_name: %3% "
            " message_type: %4% "
            " comm_type: %5% "
            " save_contact_reference: %6% "
            " contact_handle: %7% "
            " contact_object_registry_id: %8%"
            " contact_history_historyid: %9%"

            )
                  % moddate.print_buffer()
                  % attempt.print_buffer()
                  % status_name
                  % message_type
                  % comm_type
                  % save_contact_reference
                  % contact_handle
                  % contact_object_registry_id
                  % contact_history_historyid
                  );

    Database::Connection conn = Database::Manager::acquire();

    Database::Result message_type_id_res = conn.exec_params(
            "SELECT id FROM message_type WHERE type= $1::text"
            , Database::query_param_list (message_type));

    unsigned long long message_type_id = 0;
    if (message_type_id_res.size() == 1)
        message_type_id
            = static_cast<unsigned long long>(message_type_id_res[0][0]);

    conn.exec_params(
        "INSERT INTO message_archive"
        " (crdate, moddate, attempt, status_id, comm_type_id, message_type_id, service_handle)"
        " VALUES (CURRENT_TIMESTAMP, $1::timestamp"// without time zone "
        " , $2::smallint"
        " , (SELECT id FROM enum_send_status WHERE status_name = $3::text)"
        " , (SELECT id FROM comm_type WHERE type = $4::text), $5::integer "
        " , (SELECT service_handle FROM message_type_forwarding_service_map WHERE message_type_id = $5::integer) "
        " )"
        , Database::query_param_list
          (Database::QueryParam())//$1 moddate
          (attempt)//$2 attempt
          (status_name)//$3 status
          (comm_type)//$4 comm_type
          (message_type_id)//$5 message_type_id
          );

    Database::Result msgid_res = conn.exec(
            "SELECT currval('message_archive_id_seq') AS id");
    unsigned long long message_archive_id = 0;

    if (msgid_res.size() == 1)
        message_archive_id
            = static_cast<unsigned long long>(msgid_res[0][0]);

    if(save_contact_reference)
    {
        conn.exec_params(
            "INSERT INTO message_contact_history_map"
            " (contact_object_registry_id, contact_history_historyid"
            " , message_archive_id)"
            " VALUES ( $1::integer, $2::integer, $3::integer)"
            , Database::query_param_list
            (contact_object_registry_id)//$1 object_registry id
            (contact_history_historyid)//$2 object_registry historyid
            (message_archive_id)//$3 message_archive id
        );
    }//if(save_contact_reference)

    return message_archive_id;
}//save_message

std::string get_file_name_by_file_id(unsigned long long file_id)
{
    std::string fname;
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec_params(
        "SELECT \"name\" FROM files WHERE id= $1::integer"
        , Database::query_param_list (file_id));
    if (res.size() == 1)
    {
        fname = std::string(res[0][0]);
    }
    return fname;
}

void check_postal_address(const PostalAddress& address)
{
    std::string trimed_address_name_string
        = boost::algorithm::trim_copy(static_cast<std::string>(address.name));
    if(trimed_address_name_string.empty()) throw WrongPostalAddress("empty address.name",address);

    std::string trimed_address_street1_string
        = boost::algorithm::trim_copy(static_cast<std::string>(address.street1));
    if(trimed_address_street1_string.empty()) throw WrongPostalAddress("empty address.street1",address);

    std::string trimed_address_city_string
        = boost::algorithm::trim_copy(static_cast<std::string>(address.city));
    if(trimed_address_city_string.empty()) throw WrongPostalAddress("empty address.city",address);

    std::string trimed_address_code_string
        = boost::algorithm::trim_copy(static_cast<std::string>(address.code));
    if(trimed_address_code_string.empty()) throw WrongPostalAddress("empty address.code",address);

    std::string trimed_address_country_string
        = boost::algorithm::trim_copy(static_cast<std::string>(address.country));
    if(trimed_address_country_string.empty()) throw WrongPostalAddress("empty address.country",address);
}

unsigned long long get_filetype_id(std::string file_type)
{
    unsigned long long filetype_id=0;
    try
    {
        LOGGER.debug(boost::format(
                "Messages::get_file_type_id "
                " file_type: %1% "
                )% file_type);

        Database::Connection conn = Database::Manager::acquire();

        Database::Result filetype_id_res = conn.exec_params(
            "SELECT id FROM enum_filetype WHERE \"name\"= $1::text"
            , Database::query_param_list (file_type));

        if (filetype_id_res.size() == 1)
            filetype_id
                = static_cast<unsigned long long>(filetype_id_res[0][0]);

    }//try
    catch(const std::exception& ex)
    {
        LOGGER.error(boost::format(
                "Messages::get_filetype_id exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER.error("Messages::get_filetype_id error");
    }

    return filetype_id;
}

// not processed letters should have status set back according to send attempts
void set_status_back(const std::string& comm_type, const std::string& service_handle, const std::size_t max_attempts_limit)
{
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("UPDATE message_archive SET "
            " status_id = (SELECT id FROM enum_send_status WHERE status_name = 'ready') "
            " WHERE status_id = (SELECT id FROM enum_send_status WHERE status_name = 'being_sent') "
            " AND comm_type_id = (SELECT id FROM comm_type WHERE type = $1::text) "
            " AND attempt < $2::integer "
            " AND service_handle = $3::message_forwarding_service "
            , Database::query_param_list(comm_type)(max_attempts_limit)(service_handle));

    conn.exec_params("UPDATE message_archive SET "
            " status_id = (SELECT id FROM enum_send_status WHERE status_name = 'no_processing') "
            " WHERE status_id = (SELECT id FROM enum_send_status WHERE status_name = 'send_failed') "
            " AND comm_type_id = (SELECT id FROM comm_type WHERE type = $1::text) "
            " AND attempt >= $2::integer "
            " AND service_handle = $3::message_forwarding_service "
            , Database::query_param_list(comm_type)(max_attempts_limit)(service_handle));

}

//Manager factory
ManagerPtr create_manager()
{
    return std::make_shared<Manager>();
}

//Manager impl

unsigned long long Manager::save_sms_to_send(const char* contact_handle
        , const char* phone
        , const char* content
        , const char* message_type
        , unsigned long contact_object_registry_id
        , unsigned long contact_history_historyid
        )
{
    unsigned long long message_archive_id =0;
    try
    {
        LOGGER.debug(boost::format(
                "Messages::save_sms_to_send "
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

        //check phone
        std::string trimed_phone_string
            = boost::algorithm::trim_copy(static_cast<std::string>(phone));
        if(trimed_phone_string.empty()) throw std::runtime_error("empty phone");

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        message_archive_id= save_message(
                Database::QueryParam()
                , 0
                , "ready"
                , message_type
                ,"sms"
                , true
                , contact_handle
                , contact_object_registry_id
                , contact_history_historyid
                );

        conn.exec_params(
                "INSERT INTO sms_archive"
                " (id, phone_number, content)"
                " VALUES ( $1::integer, $2::varchar(64), $3::text)"
                , Database::query_param_list
                (message_archive_id)//$1 id
                (phone)//$2 phone_number
                (content)//$3 status
        );
        tx.commit();
    }//try
    catch(const std::exception& ex)
    {
        LOGGER.error(boost::format(
                "Messages::save_sms_to_send exception: %1%") % ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER.error("Messages::save_sms_to_send error");
        throw;
    }
    return message_archive_id;
}


unsigned long long Manager::save_letter_to_send(const char* contact_handle
        , const PostalAddress& address
        , unsigned long long file_id
        , const char* message_type
        , unsigned long contact_object_registry_id
        , unsigned long contact_history_historyid
        , const std::string& comm_type //letter , registered_letter
        , bool do_check_postal_address
        )
{
    unsigned long long message_archive_id=0;
    try
    {
        LOGGER.debug(boost::format(
                "Messages::save_letter_to_send "
                " contact_handle: %1% "
                " file_id: %2%"
                " address.name: %3%"
                " address.org: %4%"
                " address.street1: %5%"
                " address.street2: %6%"
                " address.street3: %7%"
                " address.city: %8%"
                " address.state: %9%"
                " address.code: %10%"
                " address.county: %11%"
                " message_type: %12% "
                " contact_object_registry_id: %13%"
                " contact_history_historyid: %14%"
                " comm_type: %15%"
                )
                      % contact_handle
                      % file_id
                      % address.name
                      % address.org
                      % address.street1
                      % address.street2
                      % address.street3
                      % address.city
                      % address.state
                      % address.code
                      % address.country
                      % message_type
                      % contact_object_registry_id
                      % contact_history_historyid
                      % comm_type
                      );

        if(do_check_postal_address)
        {
            //check snail mail address (name, street1, city, postalcode, country)
            check_postal_address(address);
        }

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        message_archive_id
        = save_message(Database::QPNull, 0, "ready",message_type, comm_type, true
                , contact_handle, contact_object_registry_id
                , contact_history_historyid
                );

        conn.exec_params(
                "INSERT INTO letter_archive"
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
                 ")"
                , Database::query_param_list
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
                );

        tx.commit();
    }//try
    catch (LibFred::Messages::WrongPostalAddress& e)
    {
        LOGGER.warning(boost::format("Messages::save_letter_to_send exception: %1%") % e.what());
        throw;
    }
    catch(const std::exception& ex)
    {
        LOGGER.error(boost::format(
                "Messages::save_letter_to_send exception: %1%") % ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER.error("Messages::save_letter_to_send error");
        throw;
    }

    return message_archive_id;
}


void Manager::cancel_letter_send(unsigned long long letter_id)
{
    try
    {
        LOGGER.debug(boost::format("Messages::cancel_letter_send(%1%)") % letter_id);
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        Database::Result r = conn.exec_params(
            "UPDATE message_archive AS ma "
               "SET moddate = now(), "
                   "status_id = (SELECT id FROM enum_send_status WHERE status_name = 'no_processing') "
               "FROM letter_archive AS la "
               "WHERE "
                    "la.id = ma.id "
                    "AND ma.id = $1::bigint "
                    "AND ma.status_id IN (SELECT id FROM enum_send_status WHERE status_name IN ('sent_failed', 'ready'))",
            Database::query_param_list(letter_id));
        tx.commit();
    }
    catch (const std::exception &ex)
    {
        LOGGER.error(boost::format("Messages::cancel_letter_send(%1%) (ex: %2%)") % letter_id % ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error(boost::format("Messages::cancel_letter_send(%1%)") % letter_id);
        throw;
    }

}


unsigned long long Manager::copy_letter_to_send(unsigned long long letter_id)
{
    try
    {
        LOGGER.debug(boost::format(
                "Messages::copy_letter_to_send "
                " letter_id: %1%"
                )
                      % letter_id
                      );

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        Database::Result rcopy_ok = conn.exec_params(
                "SELECT ess.status_name IN ('sent', 'no_processing') AS copy_allowed "
                    "FROM message_archive ma "
                    "JOIN enum_send_status ess ON ess.id = ma.status_id "
                  "WHERE ma.id = $1::bigint ",
                  Database::query_param_list(letter_id));
        if (rcopy_ok.size() <= 0)
        {
            throw Database::NoDataFound("letter not found");
        }
        if (!static_cast<bool>(rcopy_ok[0]["copy_allowed"]))
        {
            throw MessageCopyProhibited();
        }

        Database::Result res = conn.exec_params(
                "INSERT INTO message_archive "
                       "(crdate,"
                        "moddate,"
                        "attempt,"
                        "status_id,"
                        "comm_type_id,"
                        "message_type_id,"
                        "service_handle) "
                 "SELECT CURRENT_TIMESTAMP,"
                        "NULL,"
                        "0,"
                        "(SELECT id FROM enum_send_status WHERE status_name='ready'),"
                        "ma.comm_type_id,"
                        "ma.message_type_id,"
                        "ma.service_handle "
                 "FROM message_archive ma "
                 "JOIN letter_archive la ON la.id=ma.id "
                 "WHERE ma.id=$1::bigint "
                "RETURNING id",
                Database::query_param_list(letter_id));
        if (res.size() <= 0) {
            throw Database::NoDataFound("not found");
        }
        const unsigned long long message_archive_id = static_cast< unsigned long long >(res[0][0]);

        res = conn.exec_params(
                "INSERT INTO message_contact_history_map "
                 "(contact_object_registry_id,"
                  "contact_history_historyid,"
                  "message_archive_id"
                 ") "
                 "SELECT "
                  "contact_object_registry_id,"
                  "contact_history_historyid,"
                  "$1::bigint "
                 "FROM message_contact_history_map "
                 "WHERE message_archive_id=$2::bigint "
                "RETURNING id",
                Database::query_param_list(message_archive_id)(letter_id)
                );
        if (res.size() <= 0) {
            throw Database::NoDataFound("not found in message_contact_history_map");
        }

        res = conn.exec_params(
                "INSERT INTO letter_archive "
                 "(id,file_id,"
                  "postal_address_name,"
                  "postal_address_organization,"
                  "postal_address_street1,"
                  "postal_address_street2,"
                  "postal_address_street3,"
                  "postal_address_city,"
                  "postal_address_stateorprovince,"
                  "postal_address_postalcode,"
                  "postal_address_country,"
                  "postal_address_id"
                 ") "
                 "SELECT "
                  "$1::bigint,"//new letter_id
                  "file_id,"
                  "postal_address_name,"
                  "postal_address_organization,"
                  "postal_address_street1,"
                  "postal_address_street2,"
                  "postal_address_street3,"
                  "postal_address_city,"
                  "postal_address_stateorprovince,"
                  "postal_address_postalcode,"
                  "postal_address_country,"
                  "postal_address_id "
                 "FROM letter_archive "
                 "WHERE id=$2::bigint "//source letter_id
                "RETURNING id",
                Database::query_param_list(message_archive_id)(letter_id)
                );
        if (res.size() <= 0) {
            throw Database::NoDataFound("not found in letter_archive");
        }

        tx.commit();
        return message_archive_id;
    }//try
    catch (const Database::NoDataFound &ex) {
        LOGGER.error(boost::format(
            "Messages::copy_letter_to_send(letter_id:%1%) Database::NoDataFound(%2%)")
            % letter_id
            % ex.what());
        throw;
    }
    catch (const Database::Exception &ex) {
        LOGGER.error(boost::format(
            "Messages::copy_letter_to_send(letter_id:%1%) Database::Exception(%2%)")
            % letter_id
            % ex.what());
        throw;
    }
    catch(const std::exception &ex) {
        LOGGER.error(boost::format(
            "Messages::copy_letter_to_send(letter_id:%1%) std::exception(%2%)")
            % letter_id
            % ex.what());
        throw;
    }
    catch(...) {
        LOGGER.error(boost::format(
            "Messages::copy_letter_to_send(letter_id:%1%) error") % letter_id);
        throw;
    }
}

unsigned long long Manager::copy_sms_to_send(unsigned long long sms_id)
{
    try
    {
        LOGGER.debug(boost::format(
                "Messages::copy_sms_to_send "
                " sms_id: %1%"
                )
                      % sms_id
                      );

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        Database::Result res = conn.exec_params(
                "INSERT INTO message_archive "
                       "(crdate,"
                        "moddate,"
                        "attempt,"
                        "status_id,"
                        "comm_type_id,"
                        "message_type_id,"
                        "service_handle) "
                 "SELECT CURRENT_TIMESTAMP,"
                        "NULL,"
                        "0,"
                        "(SELECT id FROM enum_send_status WHERE status_name='ready'),"
                        "ma.comm_type_id,"
                        "ma.message_type_id,"
                        "ma.service_handle "
                 "FROM message_archive ma "
                 "JOIN sms_archive sa ON sa.id=ma.id "
                 "WHERE sa.id=$1::bigint "
                "RETURNING id",
                Database::query_param_list(sms_id));
        if (res.size() <= 0) {
            throw Database::NoDataFound("not found");
        }
        const unsigned long long message_archive_id = static_cast< unsigned long long >(res[0][0]);

        res = conn.exec_params(
                "INSERT INTO message_contact_history_map "
                 "(contact_object_registry_id,"
                  "contact_history_historyid,"
                  "message_archive_id"
                 ") "
                 "SELECT "
                  "contact_object_registry_id,"
                  "contact_history_historyid,"
                  "$1::bigint "
                 "FROM message_contact_history_map "
                 "WHERE message_archive_id=$2::bigint "
                "RETURNING id",
                Database::query_param_list(message_archive_id)(sms_id)
                );
        if (res.size() <= 0) {
            throw Database::NoDataFound("not found in message_contact_history_map");
        }

        res = conn.exec_params(
                "INSERT INTO sms_archive "
                 "(id,"
                  "phone_number,"
                  "phone_number_id,"
                  "content"
                 ") "
                 "SELECT "
                  "$1::bigint,"//new sms_id
                  "phone_number,"
                  "phone_number_id,"
                  "content "
                 "FROM sms_archive "
                 "WHERE id=$2::bigint "//source sms_id
                "RETURNING id",
                Database::query_param_list(message_archive_id)(sms_id)
                );
        if (res.size() <= 0) {
            throw Database::NoDataFound("not found in sms_archive");
        }

        tx.commit();
        return message_archive_id;
    }//try
    catch (const Database::NoDataFound &ex) {
        LOGGER.error(boost::format(
            "Messages::copy_sms_to_send(sms_id:%1%) Database::NoDataFound(%2%)")
            % sms_id
            % ex.what());
        throw;
    }
    catch (const Database::Exception &ex) {
        LOGGER.error(boost::format(
            "Messages::copy_sms_to_send(sms_id:%1%) Database::Exception(%2%)")
            % sms_id
            % ex.what());
        throw;
    }
    catch(const std::exception &ex) {
        LOGGER.error(boost::format(
            "Messages::copy_sms_to_send(sms_id:%1%) std::exception(%2%)")
            % sms_id
            % ex.what());
        throw;
    }
    catch(...) {
        LOGGER.error(boost::format(
            "Messages::copy_sms_to_send(sms_id:%1%) error") % sms_id);
        throw;
    }
}

LetterProcInfo Manager::load_letters_to_send(std::size_t batch_size_limit
        , const std::string &comm_type, const std::string& service_handle, std::size_t max_attempts_limit)
{
    Database::Connection conn = Database::Manager::acquire();

    // return info of records being processed
    LetterProcInfo proc_letters;

    /* now bail out if other process (presumably another instance of this
     * method) is already doing something with this table. No support
     * for multithreaded access - we would have to remember which
     * records exactly are we processing
     */
    Database::Result res = conn.exec_params("SELECT EXISTS "
            "(SELECT * FROM message_archive ma "
            "JOIN comm_type ct ON ma.comm_type_id = ct.id "
            "JOIN enum_send_status ms ON ma.status_id = ms.id "
            " WHERE ms.status_name = 'being_sent' AND ct.type = $1::text "
            " AND ma.attempt < $2::integer AND ma.service_handle = $3::message_forwarding_service) "
            , Database::query_param_list (comm_type)(max_attempts_limit)(service_handle)
            );

    if ((bool)res[0][0])
    {
           LOGGER.notice("the files are already being processed. "
                   "no action; exiting");
           return proc_letters;
    }

    // transaction is needed for 'ON COMMIT DROP' functionality
    Database::Transaction trans(conn);
    // acquire lock which conflicts with itself but not basic locks used
    // by select and stuff..
    conn.exec("LOCK TABLE message_archive IN SHARE UPDATE EXCLUSIVE MODE");
    // application level lock and notification about data processing

    if(batch_size_limit == 0)//unlimited batch
    {
        conn.exec_params("UPDATE message_archive SET "
            "status_id = (SELECT id FROM enum_send_status WHERE status_name = 'being_sent') "
            " WHERE (status_id = (SELECT id FROM enum_send_status WHERE status_name = 'ready') "
            " OR status_id = (SELECT id FROM enum_send_status WHERE status_name = 'send_failed')) "
            " AND comm_type_id = (SELECT id FROM comm_type WHERE type = $1::text) "
            " AND attempt < $2::integer AND service_handle = $3::message_forwarding_service"
            , Database::query_param_list (comm_type)(max_attempts_limit)(service_handle)
        );
    }
    else//limited batch
    {
        conn.exec_params("UPDATE message_archive SET "
            "status_id = (SELECT id FROM enum_send_status WHERE status_name = 'being_sent') "
            " WHERE id IN (SELECT id FROM message_archive "
            " WHERE (status_id = (SELECT id FROM enum_send_status WHERE status_name = 'ready') "
            " OR status_id = (SELECT id FROM enum_send_status WHERE status_name = 'send_failed')) "
            " AND comm_type_id = (SELECT id FROM comm_type "
            " WHERE type = $1::text) "
            " AND attempt < $2::integer AND service_handle = $3::message_forwarding_service "
            " ORDER BY id LIMIT $4::integer) "
            , Database::query_param_list (comm_type)(max_attempts_limit)(service_handle)(batch_size_limit)
        );
    }
    // is there anything to send?
    res = conn.exec_params("SELECT la.file_id, la.id, ma.attempt , f.name "
        " , la.postal_address_name, la.postal_address_organization "
        " , la.postal_address_street1 , la.postal_address_street2, la.postal_address_street3 "
        " , la.postal_address_city, la.postal_address_stateorprovince"
        " , la.postal_address_postalcode , la.postal_address_country, mt.type "
        " FROM message_archive ma JOIN letter_archive la ON ma.id = la.id "
        " JOIN message_type mt ON ma.message_type_id = mt.id "
        " JOIN files f ON la.file_id = f.id "
        " JOIN enum_send_status ms ON ma.status_id = ms.id "
        " JOIN comm_type ct ON ma.comm_type_id = ct.id "
        " WHERE ms.status_name='being_sent' "
        " AND ct.type = $1::text "
        " AND ma.attempt < $2::integer AND ma.service_handle = $3::message_forwarding_service"
        , Database::query_param_list (comm_type)(max_attempts_limit)(service_handle));

    trans.commit();

    if(res.size() == 0) {
        LOGGER.notice("no files ready for processing; exiting");
        return proc_letters;
    }

    proc_letters.reserve(res.size());

    for(unsigned i=0;i<res.size();i++)
    {
         letter_proc mp;
         mp.file_id = res[i][0];
         mp.letter_id = res[i][1];
         mp.attempt = res[i][2];
         mp.fname = std::string(res[i][3]);

         mp.postal_address.name = std::string(res[i][4]);
         mp.postal_address.org = std::string(res[i][5]);
         mp.postal_address.street1 = std::string(res[i][6]);
         mp.postal_address.street2 = std::string(res[i][7]);
         mp.postal_address.street3 = std::string(res[i][8]);
         mp.postal_address.city = std::string(res[i][9]);
         mp.postal_address.state = std::string(res[i][10]);
         mp.postal_address.code = std::string(res[i][11]);
         mp.postal_address.country = std::string(res[i][12]);

         mp.message_type = std::string(res[i][13]);

         proc_letters.push_back(mp);
    }

    return proc_letters;
}

SmsProcInfo Manager::load_sms_to_send(std::size_t batch_size_limit, const std::string& service_handle, std::size_t max_attempts_limit)
{

    Database::Connection conn = Database::Manager::acquire();

    // return info of records being processed
    SmsProcInfo proc_sms;

    /* now bail out if other process (presumably another instance of this
     * method) is already doing something with this table. No support
     * for multithreaded access - we would have to remember which
     * records exactly are we processing
     */
    Database::Result res = conn.exec_params("SELECT EXISTS "
            "(SELECT * FROM message_archive ma "
            "JOIN comm_type ct ON ma.comm_type_id = ct.id "
            "JOIN enum_send_status ms ON ma.status_id = ms.id "
            " WHERE ms.status_name = 'being_sent' AND ct.type = 'sms'"
            " AND ma.attempt < $1::integer AND ma.service_handle = $2::message_forwarding_service) "
            , Database::query_param_list (max_attempts_limit)(service_handle));

    if ((bool)res[0][0])
    {
           LOGGER.notice("the messages are already being processed. "
                   "no action; exiting");
           return proc_sms;
    }

    // transaction is needed for 'ON COMMIT DROP' functionality and lock
    Database::Transaction trans(conn);
    // acquire lock which conflicts with itself but not basic locks used
    // by select and stuff..
    conn.exec("LOCK TABLE message_archive IN SHARE UPDATE EXCLUSIVE MODE");
    // application level lock and notification about data processing

    if(batch_size_limit == 0)//unlimited batch
    {
        conn.exec_params("UPDATE message_archive SET "
            "status_id = (SELECT id FROM enum_send_status WHERE status_name = 'being_sent') "
            " WHERE (status_id = (SELECT id FROM enum_send_status WHERE status_name = 'ready') "
            " OR status_id = (SELECT id FROM enum_send_status WHERE status_name = 'send_failed')) "
            " AND comm_type_id = (SELECT id FROM comm_type WHERE type = 'sms') "
            " AND attempt < $1::integer AND service_handle = $2::message_forwarding_service"
            , Database::query_param_list (max_attempts_limit)(service_handle)
        );
    }
    else//limited batch
    {
        conn.exec_params("UPDATE message_archive SET "
            " status_id = (SELECT id FROM enum_send_status WHERE status_name = 'being_sent') "
            " WHERE id IN (SELECT id FROM message_archive "
            " WHERE (status_id = (SELECT id FROM enum_send_status WHERE status_name = 'ready') "
            " OR status_id = (SELECT id FROM enum_send_status WHERE status_name = 'send_failed')) "
            " AND comm_type_id = (SELECT id FROM comm_type "
            " WHERE type = 'sms') "
            " AND attempt < $1::integer AND service_handle = $2::message_forwarding_service"
            " ORDER BY id LIMIT $3::integer) "
            , Database::query_param_list (max_attempts_limit)(service_handle)(batch_size_limit)
        );
    }

    // is there anything to send?
    res = conn.exec_params("SELECT  sa.id, ma.attempt, sa.phone_number, sa.content  "
        " FROM message_archive ma JOIN sms_archive sa ON ma.id = sa.id "
        " JOIN enum_send_status ms ON ma.status_id = ms.id "
        " WHERE ms.status_name='being_sent' "
        " AND ma.attempt < $1::integer AND ma.service_handle = $2::message_forwarding_service"
        , Database::query_param_list (max_attempts_limit)(service_handle)
        );

    trans.commit();

    if(res.size() == 0) {
        LOGGER.notice("no messages ready for processing; exiting");
        return proc_sms;
    }

    proc_sms.reserve(res.size());

    for(unsigned i=0;i<res.size();i++)
    {
             sms_proc mp;
             mp.sms_id = res[i][0];
             mp.attempt = res[i][1];
             mp.phone_number = std::string(res[i][2]);
             mp.content = std::string(res[i][3]);
             proc_sms.push_back(mp);
    }
    return proc_sms;
}


void Manager::set_letter_status(const LetterProcInfo& letters
        ,const std::string& new_status, const std::string& batch_id
        , const std::string &comm_type
        , const std::string& service_handle
        , const std::size_t max_attempts_limit )
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction trans2(conn);

    // processed letters update
    for (LibFred::Messages::LetterProcInfo::const_iterator it = letters.begin()
            ; it!=letters.end(); ++it)
    {
          unsigned int new_attempt = it->attempt + 1;
          conn.exec_params("UPDATE message_archive SET "
              "status_id = (SELECT id FROM enum_send_status WHERE status_name = $1::text), "
              "attempt = $2::integer, "
              "moddate = CURRENT_TIMESTAMP WHERE id = $3::integer"
              " AND comm_type_id = (SELECT id FROM comm_type "
              " WHERE type = $4::text)"
              " AND service_handle = $5::message_forwarding_service"
              , Database::query_param_list
                   (new_status)
                   (new_attempt)
                   (it->letter_id)
                   (comm_type)
                   (service_handle)
              );

          conn.exec_params("UPDATE letter_archive SET  "
                              "batch_id = $1::text "
                              " WHERE id = $2::integer"
                          , Database::query_param_list
                                   (batch_id)
                                   (it->letter_id)
                          );
    }
    // not processed letters should have status set back (set moddate? status?)
    set_status_back(comm_type, service_handle, max_attempts_limit);

    trans2.commit();

}

//set send result into sms status
void Manager::set_sms_status(const SmsProcInfo& messages, const std::string& service_handle, const std::size_t max_attempts_limit)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction trans2(conn);

    // processed sms update
    for (LibFred::Messages::SmsProcInfo::const_iterator it = messages.begin()
            ; it!=messages.end(); ++it)
    {
          unsigned int new_attempt = it->attempt + 1;
          conn.exec_params("UPDATE message_archive SET "
                  "status_id = (SELECT id FROM enum_send_status WHERE status_name = $1::text), "
                      "attempt = $2::integer, "
                      "moddate = CURRENT_TIMESTAMP WHERE id = $3::integer AND service_handle = $4::message_forwarding_service"
                      " AND comm_type_id = (SELECT id FROM comm_type "
                      " WHERE type = 'sms')"
                  , Database::query_param_list
                   (it->new_status)
                   (new_attempt)
                   (it->sms_id)
                   (service_handle)
                  );
    }
    // not processed letters should have status set back (set moddate? status?)
    set_status_back("sms", service_handle, max_attempts_limit);

    trans2.commit();
}

Manager::MessageListPtr Manager::createList()
{
	return Manager::MessageListPtr( new MessageList(
			std::unique_ptr <MessageReload>(new MessageReload())
			));
}

///status names
EnumList Manager::getStatusList()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec("SELECT  id, status_name FROM enum_send_status "
            "ORDER BY id");
    if(res.size() == 0)
    {
        const char* err_msg = "getStatusList: no data in table enum_send_status";
        LOGGER.error(err_msg);
        throw std::runtime_error(err_msg);
    }
    EnumList el;
    for(unsigned i=0;i<res.size();i++)
    {
        EnumListItem eli;
        eli.id = res[i][0];
        eli.name = std::string(res[i][1]);
        el.push_back(eli);
    }
    return el;
}
//Manager::getStatusList

///communication types
EnumList Manager::getCommTypeList()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec("SELECT  id, type FROM comm_type "
            "ORDER BY id");
    if(res.size() == 0)
    {
        const char* err_msg = "getCommTypeList: no data in table comm_type";
        LOGGER.error(err_msg);
        throw std::runtime_error(err_msg);
    }
    EnumList el;
    for(unsigned i=0;i<res.size();i++)
    {
        EnumListItem eli;
        eli.id = res[i][0];
        eli.name = std::string(res[i][1]);
        el.push_back(eli);
    }
    return el;
}//Manager::getCommTypeList
///message types
EnumList Manager::getMessageTypeList()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec("SELECT  id, type FROM message_type "
            "ORDER BY id");
    if(res.size() == 0)
    {
        const char* err_msg = "getMessageTypeList: no data in table message_type";
        LOGGER.error(err_msg);
        throw std::runtime_error(err_msg);
    }
    EnumList el;
    for(unsigned i=0;i<res.size();i++)
    {
        EnumListItem eli;
        eli.id = res[i][0];
        eli.name = std::string(res[i][1]);
        el.push_back(eli);
    }
    return el;
}//Manager::getMessageTypeList


//get sms data by id
SmsInfo Manager::get_sms_info_by_id(unsigned long long id)
{
    Database::Connection conn = Database::Manager::acquire();
    SmsInfo si;
    Database::Result res = conn.exec_params(
        "SELECT phone_number, content FROM sms_archive WHERE id= $1::integer"
        , Database::query_param_list (id));
    if (res.size() == 1)
    {
        si.phone_number = std::string(res[0][0]);
        si.content = std::string(res[0][1]);
    }
    return si;
}
//get letter data by id
LetterInfo Manager::get_letter_info_by_id(unsigned long long id)
{
    Database::Connection conn = Database::Manager::acquire();
    LetterInfo li;
    Database::Result res = conn.exec_params(
        "SELECT file_id, batch_id"
         "  , postal_address_name"
         "  , postal_address_organization"
         "  , postal_address_street1"
         "  , postal_address_street2"
         "  , postal_address_street3"
         "  , postal_address_city"
         "  , postal_address_stateorprovince"
         "  , postal_address_postalcode"
         "  , postal_address_country"
         " FROM letter_archive WHERE id= $1::integer"
        , Database::query_param_list (id));

    if (res.size() != 1) {
        throw std::runtime_error("ccReg_Session_i::createMessageDetail message_content");
    }

    li.file_id = res[0][0];
    li.batch_id = std::string(res[0][1]);
    li.postal_address.name = std::string(res[0][2]);
    li.postal_address.org = std::string(res[0][3]);
    li.postal_address.street1 = std::string(res[0][4]);
    li.postal_address.street2 = std::string(res[0][5]);
    li.postal_address.street3 = std::string(res[0][6]);
    li.postal_address.city = std::string(res[0][7]);
    li.postal_address.state = std::string(res[0][8]);
    li.postal_address.code = std::string(res[0][9]);
    li.postal_address.country = std::string(res[0][10]);

    li.fname = get_file_name_by_file_id(li.file_id);

    return li;
}

void MessageReload::operator ()
        (Database::Filters::Union &uf
        , ListType& list
        , std::size_t& _limit
        ,  bool& loadLimitActive_)
{
    list.clear();
    uf.clearQueries();

    bool at_least_one = false;
    Database::SelectQuery info_query;
    std::unique_ptr<Database::Filters::Iterator> fit(uf.createIterator());
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
      LOGGER.error("wrong filter passed for reload!");
      return;
    }
    uf.serialize(info_query);
    std::string info_query_str = str(boost::format("%1% LIMIT %2%")
        % info_query.str() % (_limit+1));//try select more than limit
    LOGGER.debug(boost::format("reload(uf) ObjList query: %1%")
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

      LibFred::Messages::ManagerPtr msg_mgr
          = LibFred::Messages::create_manager();

      //enumlists
      //status names
      EnumList status_names_ = msg_mgr->getStatusList();
      std::map<std::size_t, std::string> status_names;
      for (EnumList::const_iterator i = status_names_.begin()
              ; i != status_names_.end(); ++i)
          status_names[i->id] = i->name;

      //communication types
      EnumList comm_types_ = msg_mgr->getCommTypeList();
      std::map<std::size_t, std::string> comm_types;
      for (EnumList::const_iterator i = comm_types_.begin()
              ; i != comm_types_.end(); ++i)
          comm_types[i->id] = i->name;

      //message types
      EnumList msg_types_ = msg_mgr->getMessageTypeList();
      std::map<std::size_t, std::string> msg_types;
      for (EnumList::const_iterator i = msg_types_.begin()
              ; i != msg_types_.end(); ++i)
          msg_types[i->id] = i->name;


      list.reserve(result_size);
      list.clear();
      for (std::size_t i=0; i < result_size; i++)
      {
          ObjPtr objptr(new Object);
          for (std::size_t j=0; j < res[i].size(); j++)
          {/*
              LOGGER.debug(
                      boost::format("i: %1% j: %2% data: %3%")
                      % i % j % res[i][j]);
              */

              switch(j)
              {
              case MessageMetaInfo::MT_STATUS :
                  objptr->conv_set(static_cast<MessageMetaInfo::MemberOrderType>(j)
                          ,status_names[static_cast<std::size_t>(res[i][j])]);//for j col
                  break;
              case MessageMetaInfo::MT_COMMTYPE :
                  objptr->conv_set(static_cast<MessageMetaInfo::MemberOrderType>(j)
                          ,comm_types[static_cast<std::size_t>(res[i][j])]);//for j col
                  break;
              case MessageMetaInfo::MT_MSGTYPE :
                  objptr->conv_set(static_cast<MessageMetaInfo::MemberOrderType>(j)
                          ,msg_types[static_cast<std::size_t>(res[i][j])]);//for j col
                  break;
              default :
                  objptr->conv_set(static_cast<MessageMetaInfo::MemberOrderType>(j)
                          ,res[i][j]);//for j col
                  break;
              }
          }
          list.push_back(objptr);
      }//for i row
    }//try
    catch (std::exception& ex)
    {
      LOGGER.error(boost::format("%1%") % ex.what());
      throw;
    }
}


} // namespace Messages
} // namespace LibFred
