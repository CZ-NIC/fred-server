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
#include "db_settings.h"
#include "model/model_filters.h"
#include "log/logger.h"
#include "log/context.h"

#include <stdexcept>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "register/db_settings.h"

namespace Register
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
    LOGGER(PACKAGE).debug(boost::format(
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
        " (crdate, moddate, attempt, status_id, comm_type_id, message_type_id)"
        " VALUES (CURRENT_TIMESTAMP, $1::timestamp"// without time zone "
        " , $2::smallint"
        " , (SELECT id FROM message_status WHERE status_name = $3::text)"
        " , (SELECT id FROM comm_type WHERE type = $4::text), $5::integer)"
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

unsigned long long get_filetype_id(std::string file_type)
{
    unsigned long long filetype_id=0;
    try
    {
        LOGGER(PACKAGE).debug(boost::format(
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
        LOGGER(PACKAGE).error(boost::format(
                "Messages::get_filetype_id exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("Messages::get_filetype_id error");
    }

    return filetype_id;
}

// not processed letters should have status set back
void set_status_back(const std::string& comm_type)
{
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("UPDATE message_archive SET "
            "status_id = (SELECT id FROM message_status WHERE status_name = 'ready') "
            " WHERE status_id = (SELECT id FROM message_status WHERE status_name = 'being_sent') "
            " AND comm_type_id = (SELECT id FROM comm_type "
            " WHERE type = $1::text)"
            , Database::query_param_list(comm_type));
}

//Manager factory
ManagerPtr create_manager()
{
    return ManagerPtr(new Manager);
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
        LOGGER(PACKAGE).debug(boost::format(
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
        LOGGER(PACKAGE).error(boost::format(
                "Messages::save_sms_to_send exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("Messages::save_sms_to_send error");
    }
    return message_archive_id;
}


unsigned long long Manager::save_letter_to_send(const char* contact_handle
        , const PostalAddress& address
        , unsigned long long file_id
        , const char* message_type
        , unsigned long contact_object_registry_id
        , unsigned long contact_history_historyid
        )
{
    unsigned long long message_archive_id=0;
    try
    {
        LOGGER(PACKAGE).debug(boost::format(
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
                      );

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        message_archive_id
        = save_message(Database::QPNull, 0, "ready",message_type, "letter", true
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
    catch(const std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format(
                "Messages::save_letter_to_send exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("Messages::save_letter_to_send error");
    }

    return message_archive_id;
}

LetterProcInfo Manager::load_letters_to_send(std::size_t batch_size_limit)
{
    Database::Connection conn = Database::Manager::acquire();

    // return info of records being processed
    LetterProcInfo proc_letters;

    /* now bail out if other process (presumably another instance of this
     * method) is already doing something with this table. No support
     * for multithreaded access - we would have to remember which
     * records exactly are we processing
     */
    Database::Result res = conn.exec("SELECT EXISTS "
            "(SELECT * FROM message_archive ma "
            "JOIN comm_type ct ON ma.comm_type_id = ct.id "
            "JOIN message_status ms ON ma.status_id = ms.id "
            " WHERE ms.status_name = 'being_sent' AND ct.type = 'letter')");

    if ((bool)res[0][0])
    {
           LOGGER(PACKAGE).notice("the files are already being processed. "
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
        conn.exec("UPDATE message_archive SET "
            "status_id = (SELECT id FROM message_status WHERE status_name = 'being_sent') "
            " WHERE (status_id = (SELECT id FROM message_status WHERE status_name = 'ready') "
            " OR status_id = (SELECT id FROM message_status WHERE status_name = 'send_failed')) "
            " AND comm_type_id = (SELECT id FROM comm_type WHERE type = 'letter') "
        );
    }
    else//limited batch
    {
        conn.exec_params("UPDATE message_archive SET "
            "status_id = (SELECT id FROM message_status WHERE status_name = 'being_sent') "
            " WHERE id IN (SELECT id FROM message_archive "
            " WHERE (status_id = (SELECT id FROM message_status WHERE status_name = 'ready') "
            " OR status_id = (SELECT id FROM message_status WHERE status_name = 'send_failed')) "
            " AND comm_type_id = (SELECT id FROM comm_type "
            " WHERE type = 'letter') ORDER BY id LIMIT $1::integer) "
                , Database::query_param_list (batch_size_limit)
        );
    }
    // is there anything to send?
    res = conn.exec("SELECT la.file_id, la.id, ma.attempt , f.name "
        " FROM message_archive ma JOIN letter_archive la ON ma.id = la.id "
        " JOIN files f ON la.file_id = f.id "
        " JOIN message_status ms ON ma.status_id = ms.id "
        " WHERE ms.status_name='being_sent'");

    trans.commit();

    if(res.size() == 0) {
        LOGGER(PACKAGE).notice("no files ready for processing; exiting");
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
             proc_letters.push_back(mp);
    }

    return proc_letters;
}

SmsProcInfo Manager::load_sms_to_send(std::size_t batch_size_limit)
{

    Database::Connection conn = Database::Manager::acquire();

    // return info of records being processed
    SmsProcInfo proc_sms;

    /* now bail out if other process (presumably another instance of this
     * method) is already doing something with this table. No support
     * for multithreaded access - we would have to remember which
     * records exactly are we processing
     */
    Database::Result res = conn.exec("SELECT EXISTS "
            "(SELECT * FROM message_archive ma "
            "JOIN comm_type ct ON ma.comm_type_id = ct.id "
            "JOIN message_status ms ON ma.status_id = ms.id "
            " WHERE ms.status_name = 'being_sent' AND ct.type = 'sms')");

    if ((bool)res[0][0])
    {
           LOGGER(PACKAGE).notice("the messages are already being processed. "
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
        conn.exec("UPDATE message_archive SET "
            "status_id = (SELECT id FROM message_status WHERE status_name = 'being_sent') "
            " WHERE (status_id = (SELECT id FROM message_status WHERE status_name = 'ready') "
            " OR status_id = (SELECT id FROM message_status WHERE status_name = 'send_failed')) "
            " AND comm_type_id = (SELECT id FROM comm_type WHERE type = 'sms') "
        );
    }
    else//limited batch
    {
        conn.exec_params("UPDATE message_archive SET "
            "status_id = (SELECT id FROM message_status WHERE status_name = 'being_sent') "
            " WHERE id IN (SELECT id FROM message_archive "
            " WHERE (status_id = (SELECT id FROM message_status WHERE status_name = 'ready') "
            " OR status_id = (SELECT id FROM message_status WHERE status_name = 'send_failed')) "
            " AND comm_type_id = (SELECT id FROM comm_type "
            " WHERE type = 'sms') ORDER BY id LIMIT $1::integer) "
                , Database::query_param_list (batch_size_limit)
        );
    }

    // is there anything to send?
    res = conn.exec("SELECT  sa.id, ma.attempt, sa.phone_number, sa.content  "
        " FROM message_archive ma JOIN sms_archive sa ON ma.id = sa.id "
        " JOIN message_status ms ON ma.status_id = ms.id "
        " WHERE ms.status_name='being_sent'");

    trans.commit();

    if(res.size() == 0) {
        LOGGER(PACKAGE).notice("no messages ready for processing; exiting");
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
        ,const std::string& new_status, const std::string& batch_id)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction trans2(conn);

    // processed letters update
    for (Register::Messages::LetterProcInfo::const_iterator it = letters.begin()
            ; it!=letters.end(); ++it)
    {
          unsigned int new_attempt = it->attempt + 1;
          conn.exec_params("UPDATE message_archive SET "
              "status_id = (SELECT id FROM message_status WHERE status_name = $1::text), "
              "attempt = $2::integer, "
              "moddate = CURRENT_TIMESTAMP WHERE id = $3::integer"
              " AND comm_type_id = (SELECT id FROM comm_type "
              " WHERE type = 'letter')"
              , Database::query_param_list
                   (new_status)
                   (new_attempt)
                   (it->letter_id)
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
    set_status_back("letter");

    trans2.commit();

}

//set send result into sms status
void Manager::set_sms_status(const SmsProcInfo& messages)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction trans2(conn);

    // processed sms update
    for (Register::Messages::SmsProcInfo::const_iterator it = messages.begin()
            ; it!=messages.end(); ++it)
    {
          unsigned int new_attempt = it->attempt + 1;
          conn.exec_params("UPDATE message_archive SET "
                  "status_id = (SELECT id FROM message_status WHERE status_name = $1::text), "
                      "attempt = $2::integer, "
                      "moddate = CURRENT_TIMESTAMP WHERE id = $3::integer"
                      " AND comm_type_id = (SELECT id FROM comm_type "
                      " WHERE type = 'sms')"
                  , Database::query_param_list
                   (it->new_status)
                   (new_attempt)
                   (it->sms_id)
                  );
    }
    // not processed letters should have status set back (set moddate? status?)
    set_status_back("sms");

    trans2.commit();
}

Manager::MessageListPtr Manager::createList()
{
	return Manager::MessageListPtr( new MessageList(
			std::auto_ptr <MessageReload>(new MessageReload())
			));
}

///status names
EnumList Manager::getStatusList()
{
    return getStatusListImpl();
}//Manager::getStatusList

///communication types
EnumList Manager::getCommTypeList()
{
    return getCommTypeListImpl();
}//Manager::getCommTypeList
///message types
EnumList Manager::getMessageTypeList()
{
    return getMessageTypeListImpl();
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
    if (res.size() == 1)
    {
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
    }

    return li;
}



///status names
EnumList getStatusListImpl()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec("SELECT  id, status_name FROM message_status "
            "ORDER BY id");
    if(res.size() == 0)
    {
        const char* err_msg = "getStatusList: no data in table message_status";
        LOGGER(PACKAGE).error(err_msg);
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
///communication types
EnumList getCommTypeListImpl()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec("SELECT  id, type FROM comm_type "
            "ORDER BY id");
    if(res.size() == 0)
    {
        const char* err_msg = "getCommTypeList: no data in table comm_type";
        LOGGER(PACKAGE).error(err_msg);
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
///message types
EnumList getMessageTypeListImpl()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec("SELECT  id, type FROM message_type "
            "ORDER BY id");
    if(res.size() == 0)
    {
        const char* err_msg = "getMessageTypeList: no data in table message_type";
        LOGGER(PACKAGE).error(err_msg);
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

}//namespace Messages
}//namespace Register
