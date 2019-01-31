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

#include "src/deprecated/libfred/contact_verification/contact_verification_password.hh"
#include "libfred/db_settings.hh"
#include "util/log/logger.hh"
#include "util/map_at.hh"
#include "util/util.hh"
#include "src/util/xmlgen.hh"

namespace LibFred {
namespace PublicRequest {

ContactVerificationPassword::MessageData ContactVerificationPassword::collectMessageData()const
{
    Database::Connection conn = Database::Manager::acquire();
    MessageData data;
    data["hostname"] = prai_ptr_->getPublicRequestManager()
            ->getIdentificationMailAuthHostname();
    data["identification"] = prai_ptr_->getIdentification();
    /* password split */
    const std::string password = prai_ptr_->getPassword();
    data["pin1"] = password.substr(
            0, -get_password_chunk_length() + password.length());
    data["pin2"] = password.substr(
            -get_password_chunk_length() + password.length());
    data["pin3"] = password;
    data["reqdate"] = boost::gregorian::to_iso_extended_string(
            prai_ptr_->getCreateTime().date());
    return collect_message_data(prai_ptr_->getObject(0).id, conn, data);
}

size_t ContactVerificationPassword::get_password_chunk_length()const
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
    LOGGER.debug("public request auth - send email password");

    MessageData data = this->collectMessageData();

    LibFred::Mailer::Attachments attach;
    LibFred::Mailer::Handles handles;
    LibFred::Mailer::Parameters params;

    params["firstname"] = map_at(data, "firstname");
    params["lastname"]  = map_at(data, "lastname");
    params["email"]     = map_at(data, "email");
    params["hostname"]  = map_at(data, "hostname");
    params["handle"]    = map_at(data, "handle");
    params["identification"] = map_at(data, "identification");
    params["telephone"] = map_at(data, "phone");

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
        , LibFred::Document::GenerationType doc_type //type for document generator
        , const std::string& message_type //for message_archive: "contact_verification_pin2", "contact_verification_pin3"
        , const std::string& comm_type //for message_archive: "letter"
        )
{
    LOGGER.debug("public request auth - send letter password");

    MessageData data = collectMessageData();

    std::string addr_country = ((map_at(data, "country_cs_name")).empty()
            ? map_at(data, "country_name")
            : map_at(data, "country_cs_name"));

    std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");

    /* HACK: here we create input data for template but we take advantage of the fact
     * that document is created without errors even if there are unknown input data tags for given template.
     * (for mojeid_pin3 letter send by OPTYS service there are two extra tags - 'sex' and 'mobile')
     * Appropritate template for service type is deduced before call.
     *
     * It's quite a mess :(
     */
    const std::string lastname = map_at(data, "lastname");
    static const char female_suffix[] = "á"; // utf-8 encoded
    enum { FEMALE_SUFFIX_LEN = sizeof(female_suffix) - 1 };
    const std::string sex = (FEMALE_SUFFIX_LEN <= lastname.length()) &&
                            (std::strcmp(lastname.c_str() + lastname.length() - FEMALE_SUFFIX_LEN,
                                         female_suffix) == 0) ? "female" : "male";

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
                (Util::XmlTagPair("last_name", Util::XmlUnparsedCData(lastname)))
                (Util::XmlTagPair("sex", Util::XmlUnparsedCData(sex)))
                (Util::XmlTagPair("email", Util::XmlUnparsedCData(map_at(data, "email"))))
                (Util::XmlTagPair("mobile", Util::XmlUnparsedCData(map_at(data, "phone"))))
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

        LibFred::Messages::PostalAddress pa;
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
                    , true //check postal address filed
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
    LOGGER.debug("public request auth - send sms password");

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
        LOGGER.debug(boost::format("authinfo w/o spaces='%s'")
            % passwd);
        /* fill with random to PASSWORD_CHUNK_LENGTH size */
        size_t to_fill = 0;
        if ((to_fill = (get_password_chunk_length() - passwd.length())
                ) > 0)
        {
            passwd += generateRandomPassword(to_fill);
            LOGGER.debug(boost::format("authinfo filled='%s'")
                % passwd);
        }
    }
    return passwd;
}

ContactVerificationPassword::MessageData& collect_message_data(
    unsigned long long _contact_id,
    Database::Connection &_conn,
    ContactVerificationPassword::MessageData &_data)
{
    const Database::Result result = _conn.exec_params( // contact mailing address first
        "SELECT c.name,c.organization,"
               "CASE WHEN mc.id IS NULL THEN c.street1 ELSE mc.street1 END,"
               "CASE WHEN mc.id IS NULL THEN c.city ELSE mc.city END,"
               "CASE WHEN mc.id IS NULL THEN c.stateorprovince ELSE mc.stateorprovince END,"
               "CASE WHEN mc.id IS NULL THEN c.postalcode ELSE mc.postalcode END,"
               "CASE WHEN mc.id IS NULL THEN c.country ELSE mc.country END,"
               "c.email,oreg.historyid,c.telephone,"
               "CASE WHEN mc.id IS NULL THEN cc.country ELSE mcc.country END,"
               "CASE WHEN mc.id IS NULL THEN cc.country_cs ELSE mcc.country_cs END,"
               "LOWER(oreg.name),NOW()::DATE,"
               "EXISTS(SELECT 1 FROM object_state os "
                      "WHERE os.object_id=c.id AND "
                            "os.state_id=(SELECT id FROM enum_object_states "
                                         "WHERE name='validatedContact') AND "
                            "os.valid_to IS NULL) "
        "FROM contact c "
        "JOIN enum_country cc ON cc.id=c.country "
        "JOIN object_registry oreg ON oreg.id=c.id "
        "LEFT JOIN contact_address mc ON mc.contactid=c.id AND mc.type='MAILING' "
        "LEFT JOIN enum_country mcc ON mcc.id=mc.country "
        "WHERE c.id=$1::INTEGER LIMIT 1",
        Database::query_param_list(_contact_id));
    if (result.size() != 1) {
        throw std::runtime_error("unable to get data for password messages");
    }

    const std::string name   = static_cast<std::string>(result[0][0]);
    const std::size_t pos    = name.find_last_of(" ");
    _data["firstname"]       = name.substr(0, pos);
    _data["lastname"]        = name.substr(pos + 1);
    _data["organization"]    = static_cast<std::string>(result[0][1]);
    _data["street"]          = static_cast<std::string>(result[0][2]);
    _data["city"]            = static_cast<std::string>(result[0][3]);
    _data["stateorprovince"] = static_cast<std::string>(result[0][4]);
    _data["postalcode"]      = static_cast<std::string>(result[0][5]);
    _data["country"]         = static_cast<std::string>(result[0][6]);
    _data["email"]           = static_cast<std::string>(result[0][7]);
    _data["contact_id"]      = boost::lexical_cast<std::string>(_contact_id);
    _data["contact_hid"]     = static_cast<std::string>(result[0][8]);
    _data["phone"]           = static_cast<std::string>(result[0][9]);
    _data["country_name"]    = static_cast<std::string>(result[0][10]);
    _data["country_cs_name"] = static_cast<std::string>(result[0][11]);
    _data["handle"]          = static_cast<std::string>(result[0][12]);
    _data["reqdate"]         = static_cast<std::string>(result[0][13]);
    if (static_cast< bool >(result[0][14])) {
        _data["state"] = "validated";
    }

    return _data;
}

}}


