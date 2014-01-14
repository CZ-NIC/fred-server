/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @contact_verification_password.cc
 *  passwords implementation
 */

#include <stdexcept>

#include "contact_verification_password.h"
#include "fredlib/db_settings.h"
#include "log/logger.h"
#include "util/map_at.h"
#include "util/util.h"
#include "util/xmlgen.h"

namespace Fred {
namespace PublicRequest {

const ContactVerificationPassword::MessageData ContactVerificationPassword::collectMessageData()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result result = conn.exec_params(
            "SELECT c.name, c.organization, c.street1, c.city,"
            " c.stateorprovince, c.postalcode, c.country, c.email,"
            " oreg.historyid, c.telephone, ec.country, ec.country_cs"
            " FROM contact c"
            " JOIN object_registry oreg ON oreg.id = c.id"
            " JOIN enum_country ec ON ec.id = c.country "
            " WHERE c.id = $1::integer",
            Database::query_param_list(prai_ptr_->getObject(0).id));
    if (result.size() != 1)
        throw std::runtime_error("unable to get data for"
                " password messages");

    MessageData data;

    std::string name = static_cast<std::string>(result[0][0]);
    std::size_t pos = name.find_last_of(" ");
    data["firstname"] = name.substr(0, pos);
    data["lastname"] = name.substr(pos + 1);
    data["organization"] = static_cast<std::string>(result[0][1]);
    data["street"] = static_cast<std::string>(result[0][2]);
    data["city"] = static_cast<std::string>(result[0][3]);
    data["stateorprovince"] = static_cast<std::string>(result[0][4]);
    data["postalcode"] = static_cast<std::string>(result[0][5]);
    data["country"] = static_cast<std::string>(result[0][6]);
    data["email"] = static_cast<std::string>(result[0][7]);
    data["hostname"] = prai_ptr_->getPublicRequestManager()\
            ->getIdentificationMailAuthHostname();
    data["identification"] = prai_ptr_->getIdentification();
    data["handle"] = boost::algorithm::to_lower_copy(
            prai_ptr_->getObject(0).handle);
    /* password split */
    const std::string password = prai_ptr_->getPassword();
    data["pin1"] = password.substr(
            0, -get_password_chunk_length() + password.length());
    data["pin2"] = password.substr(
            -get_password_chunk_length() + password.length());
    data["pin3"] = password;
    data["reqdate"] = boost::gregorian::to_iso_extended_string(
            prai_ptr_->getCreateTime().date());
    data["contact_id"] = boost::lexical_cast<std::string>(
            prai_ptr_->getObject(0).id);
    data["contact_hid"] = static_cast<std::string>(result[0][8]);
    data["phone"] = static_cast<std::string>(result[0][9]);
    data["country_name"] = static_cast<std::string>(result[0][10]);
    data["country_cs_name"] = static_cast<std::string>(result[0][11]);

    return data;
}

size_t ContactVerificationPassword::get_password_chunk_length()
{
    static const size_t PASSWORD_CHUNK_LENGTH = 8;
    return PASSWORD_CHUNK_LENGTH;
}

ContactVerificationPassword::ContactVerificationPassword(PublicRequestAuthImpl* _prai_ptr)
: prai_ptr_(_prai_ptr)
{}

void ContactVerificationPassword::sendEmailPassword(const std::string& mailTemplate //db table mail_type.name
        )
{
    LOGGER(PACKAGE).debug("public request auth - send email password");

    MessageData data = this->collectMessageData();

    Fred::Mailer::Attachments attach;
    Fred::Mailer::Handles handles;
    Fred::Mailer::Parameters params;

    params["firstname"] = map_at(data, "firstname");
    params["lastname"]  = map_at(data, "lastname");
    params["email"]     = map_at(data, "email");
    params["hostname"]  = map_at(data, "hostname");
    params["handle"]    = map_at(data, "handle");
    params["identification"] = map_at(data, "identification");

    /*
     * If public request password is one chunk long then
     * pin1 is empty because of substr in collectMessageData() method.
     * In this case password is stored in pin3.
     */
    if (!map_at(data, "pin1").empty()) {
        params["passwd"]    = map_at(data, "pin1");
    }
    else {
        params["passwd"]    = map_at(data, "pin3");
    }

    Database::Connection conn = Database::Manager::acquire();

    handles.push_back(prai_ptr_->getObject(0).handle);

    unsigned long long id = prai_ptr_->getPublicRequestManager()
        ->getMailerManager()->sendEmail(
            "",           /* default sender */
            params["email"],
            "",           /* default subject */
            mailTemplate,
            params,
            handles,
            attach
            );

    Database::Transaction tx(conn);
    conn.exec_params("INSERT INTO public_request_messages_map "
            " (public_request_id, message_archive_id, mail_archive_id) "
            " VALUES ($1::integer, $2::integer, $3::integer)",
            Database::query_param_list
                (prai_ptr_->getId())
                (Database::QPNull)
                (id));
    tx.commit();
}

void ContactVerificationPassword::sendLetterPassword( const std::string& custom_tag //tag in template xml params: "pin2",  "pin3"
        , Fred::Document::GenerationType doc_type //type for document generator
        , const std::string& message_type //for message_archive: "contact_verification_pin2", "contact_verification_pin3"
        , const std::string& comm_type //for message_archive: "letter"
        )
{
    LOGGER(PACKAGE).debug("public request auth - send letter password");

    MessageData data = collectMessageData();

    std::string addr_country = ((map_at(data, "country_cs_name")).empty()
            ? map_at(data, "country_name")
            : map_at(data, "country_cs_name"));

    std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");

    Util::XmlTagPair("contact_auth", Util::vector_of<Util::XmlCallback>
        (Util::XmlTagPair("user", Util::vector_of<Util::XmlCallback>
            (Util::XmlTagPair("actual_date", Util::XmlUnparsedCData(map_at(data, "reqdate"))))
            (Util::XmlTagPair("name", Util::XmlUnparsedCData(map_at(data, "firstname")+ " " + map_at(data, "lastname"))))
            (Util::XmlTagPair("organization", Util::XmlUnparsedCData(map_at(data, "organization"))))
            (Util::XmlTagPair("street", Util::XmlUnparsedCData(map_at(data, "street"))))
            (Util::XmlTagPair("city", Util::XmlUnparsedCData(map_at(data, "city"))))
            (Util::XmlTagPair("stateorprovince", Util::XmlUnparsedCData(map_at(data, "stateorprovince"))))
            (Util::XmlTagPair("postal_code", Util::XmlUnparsedCData(map_at(data, "postalcode"))))
            (Util::XmlTagPair("country", Util::XmlUnparsedCData(addr_country)))
            (Util::XmlTagPair("account", Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("username", Util::XmlUnparsedCData(map_at(data, "handle"))))
                (Util::XmlTagPair("first_name", Util::XmlUnparsedCData(map_at(data, "firstname"))))
                (Util::XmlTagPair("last_name", Util::XmlUnparsedCData(map_at(data, "lastname"))))
                (Util::XmlTagPair("email", Util::XmlUnparsedCData(map_at(data, "email"))))
            ))
            (Util::XmlTagPair("auth", Util::vector_of<Util::XmlCallback>
                (Util::XmlTagPair("codes", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair(custom_tag, Util::XmlUnparsedCData(map_at(data, custom_tag))))
                ))
                (Util::XmlTagPair("link"
                    , Util::XmlUnparsedCData(((message_type == "contact_verification_pin3") ?
                        std::string("https://") + map_at(data, "hostname")
                            + std::string("/verification/identify/letter/?handle=") + map_at(data, "handle")
                        : map_at(data, "hostname"))
                    )
                ))
            ))
        ))
    )(letter_xml);

    std::stringstream xmldata;
    xmldata << letter_xml;

        unsigned long long file_id = prai_ptr_->getPublicRequestManager()
            ->getDocumentManager()->generateDocumentAndSave(
                doc_type,
                xmldata,
                "identification_request-"
                    + boost::lexical_cast<std::string>(prai_ptr_->getId())
                    + ".pdf",
                7,
                "");

        Fred::Messages::PostalAddress pa;
        pa.name    = map_at(data, "firstname") + " "
                + map_at(data, "lastname");
        pa.org     = map_at(data, "organization");
        pa.street1 = map_at(data, "street");
        pa.street2 = std::string("");
        pa.street3 = std::string("");
        pa.city    = map_at(data, "city");
        pa.state   = map_at(data, "stateorprovince");
        pa.code    = map_at(data, "postalcode");
        pa.country = map_at(data, "country_name");

        unsigned long long message_id =
            prai_ptr_->getPublicRequestManager()->getMessagesManager()
                ->save_letter_to_send(
                    map_at(data, "handle").c_str()//contact handle
                    , pa
                    , file_id
                    , message_type.c_str()
                    , boost::lexical_cast<unsigned long >(map_at(data
                            , "contact_id"))//contact object_registry.id
                    , boost::lexical_cast<unsigned long >(map_at(data
                            , "contact_hid"))//contact_history.historyid
                    , comm_type.c_str()//comm_type letter or registered_letter
                    );

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        conn.exec_params("INSERT INTO public_request_messages_map "
                " (public_request_id, message_archive_id, mail_archive_id) "
                " VALUES ($1::integer, $2::integer, $3::integer)",
                Database::query_param_list
                    (prai_ptr_->getId())
                    (message_id)
                    (Database::QPNull));
        tx.commit();
}


void ContactVerificationPassword::sendSmsPassword(const boost::format& sms_template
        , const std::string& message_type //for message_archive: "contact_verification_pin2"
        )
{
    LOGGER(PACKAGE).debug("public request auth - send sms password");

    MessageData data = collectMessageData();

    boost::format sms_content = sms_template;
    sms_content % map_at(data, "pin2");

    unsigned long long message_id =
            prai_ptr_->getPublicRequestManager()->getMessagesManager()
                ->save_sms_to_send(
            map_at(data, "handle").c_str()
            , map_at(data, "phone").c_str()
            , sms_content.str().c_str()
            , message_type.c_str()//"contact_verification_pin2"
            , boost::lexical_cast<unsigned long >(map_at(data
                    , "contact_id"))
            , boost::lexical_cast<unsigned long >(map_at(data
                    , "contact_hid"))
            );

    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);
    conn.exec_params("INSERT INTO public_request_messages_map "
            " (public_request_id, message_archive_id, mail_archive_id) "
            " VALUES ($1::integer, $2::integer, $3::integer)",
            Database::query_param_list
                (prai_ptr_->getId())
                (message_id)
                (Database::QPNull));
    tx.commit();
}


std::string ContactVerificationPassword::generateRandomPassword(const size_t _length)
{
    return Random::string_from(_length,
            "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789");
}


std::string ContactVerificationPassword::generateRandomPassword()
{
    return generateRandomPassword(get_password_chunk_length());
}


std::string ContactVerificationPassword::generateAuthInfoPassword()
{
    unsigned long long contact_id = prai_ptr_->getObject(0).id;

    Database::Connection conn = Database::Manager::acquire();
    Database::Result rauthinfo = conn.exec_params(
            "SELECT substr(replace(o.authinfopw, ' ', ''), 1, $1::integer) "
            " FROM object o JOIN contact c ON c.id = o.id"
            " WHERE c.id = $2::integer",
            Database::query_param_list(get_password_chunk_length())
                                      (contact_id));
    if (rauthinfo.size() != 1)
    {
        throw std::runtime_error(str(boost::format(
                    "cannot retrieve authinfo for contact id=%1%")
                    % contact_id));
    }
    std::string passwd;
    /* pin1 */
    if (rauthinfo[0][0].isnull())\
    {
        passwd = generateRandomPassword(get_password_chunk_length());
    }
    else
    {
        passwd = static_cast<std::string>(rauthinfo[0][0]);
        LOGGER(PACKAGE).debug(boost::format("authinfo w/o spaces='%s'")
            % passwd);
        /* fill with random to PASSWORD_CHUNK_LENGTH size */
        size_t to_fill = 0;
        if ((to_fill = (get_password_chunk_length() - passwd.length())
                ) > 0)
        {
            passwd += generateRandomPassword(to_fill);
            LOGGER(PACKAGE).debug(boost::format("authinfo filled='%s'")
                % passwd);
        }
    }
    return passwd;
}

}}


