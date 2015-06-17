/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  @file
 *  declaration of partial contact check classes
 */

#ifndef CHECKERS_H_D5C22F5DFA53E06F6886FD6DE0FD30C6//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CHECKERS_H_D5C22F5DFA53E06F6886FD6DE0FD30C6

#include "src/fredlib/contact/check_collector.h"
#include "src/fredlib/contact/info_contact_data.h"
#include "src/fredlib/opcontext.h"

/// Fred
namespace Fred {

/**
 * How long can't be email or phone used for next identification request.
 * @return string value usable as parameter of INTERVAL type in SQL query
 */
inline std::string email_phone_protection_period() { return "1MONTH"; }

/**
 * Regular expression which match correct phone number.
 * @return pattern usable in boost::regex_match for checking correct phone number format
 */
inline const boost::regex& phone_pattern()
{
    static const boost::regex pattern("[[:space:]]*\\+[0-9]{1,3}\\.[0-9]{1,14}[[:space:]]*");
    return pattern;
};

/// General check classes
namespace GeneralCheck
{
    
/**
 * Contact name verification.
 */
struct contact_name
{
    /**
     * Executes check.
     * @param _name contact name to verify
     */
    contact_name(const Nullable< std::string > &_name);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success()const { return !(first_name_absent || last_name_absent); }
    bool first_name_absent:1;///< contact doesn't have first name
    bool last_name_absent:1; ///< contact doesn't have last name
};

/**
 * Contact mailing address verification.
 */
struct contact_mailing_address
{
    /**
     * Executes check.
     * @param _street1 contact address part to verify
     * @param _city contact address part to verify
     * @param _postalcode contact address part to verify
     * @param _country contact address part to verify
     */
    contact_mailing_address(
        const std::string &_street1,
        const std::string &_city,
        const std::string &_postalcode,
        const std::string &_country);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success()const { return !(street1_absent || city_absent || postalcode_absent || country_absent); }
    bool street1_absent:1;   ///< contact doesn't have street1 entry
    bool city_absent:1;      ///< contact doesn't have city entry
    bool postalcode_absent:1;///< contact doesn't have postal code entry
    bool country_absent:1;   ///< contact doesn't have country entry
};

/**
 * Contact e-mail presence checking.
 */
struct contact_email_presence
{
    /**
     * Executes check.
     * @param _email contact email to verify
     */
    contact_email_presence(const Nullable< std::string > &_email);
    /**
     * Contact e-mail presents.
     * @return true if check was successfully
     */
    bool success()const { return !absent; }
    bool absent:1;///< contact e-mail doesn't present
};

/**
 * Contact e-mail format verification.
 */
struct contact_email_validity
{
    /**
     * Executes check.
     * @param _email contact email to verify
     */
    contact_email_validity(const Nullable< std::string > &_email);
    /**
     * Contact e-mail is valid or doesn't present.
     * @return true if check was successfully
     */
    bool success()const { return !invalid; }
    bool invalid:1;///< contact e-mail presents but format fails to meet the requirements
};

/**
 * Contact e-mail availability verification.
 */
struct contact_email_availability
{
    /**
     * Executes check.
     * @param _email contact email to verify
     * @param _id contact id
     * @param _ctx operation context used to check processing
     */
    contact_email_availability(
        const Nullable< std::string > &_email,
        unsigned long long _id,
        OperationContext &_ctx);
    /**
     * Contact e-mail is available for using in next identification request.
     * @return true if check was successfully
     */
    bool success()const { return !(absent || used_recently); }
    bool absent:1;       ///< contact e-mail doesn't present
    bool used_recently:1;///< contact e-mail used for identification request recently
};

/**
 * Contact notify e-mail format verification.
 */
struct contact_notifyemail_validity
{
    /**
     * Executes check.
     * @param _notifyemail contact email to verify
     */
    contact_notifyemail_validity(const Nullable< std::string > &_notifyemail);
    /**
     * Contact notify e-mail is valid or doesn't present.
     * @return true if check was successfully
     */
    bool success()const { return !invalid; }
    bool invalid:1;///< contact notify e-mail presents and its format fails to meet the requirements
};

/**
 * Contact phone presence checking.
 */
struct contact_phone_presence
{
    /**
     * Executes check.
     * @param _telephone contact phone number to verify
     */
    contact_phone_presence(const Nullable< std::string > &_telephone);
    /**
     * Contact phone presents.
     * @return true if check was successfully
     */
    bool success()const { return !absent; }
    bool absent:1;///< contact phone doesn't present
};

/**
 * Contact phone format verification.
 */
struct contact_phone_validity
{
    /**
     * Executes check.
     * @param _telephone contact phone number to verify
     */
    contact_phone_validity(const Nullable< std::string > &_telephone);
    /**
     * Contact phone is valid.
     * @return true if check was successfully
     */
    bool success()const { return !invalid; }
    bool invalid:1;///< contact phone format fails to meet the requirements
};

/**
 * Contact phone availability verification.
 */
struct contact_phone_availability
{
    /**
     * Executes check.
     * @param _telephone contact phone number to verify
     * @param _id contact id
     * @param _ctx operation context used to check processing
     */
    contact_phone_availability(
        const Nullable< std::string > &_telephone,
        unsigned long long _id,
        OperationContext &_ctx);
    /**
     * Contact phone is available for using in next identification request.
     * @return true if check was successfully
     */
    bool success()const { return !(absent || used_recently); }
    bool absent:1;       ///< contact phone doesn't present
    bool used_recently:1;///< contact phone used for identification request recently
};

/**
 * Contact fax format verification.
 */
struct contact_fax_validity
{
    /**
     * Executes check.
     * @param _fax contact fax number to verify
     */
    contact_fax_validity(const Nullable< std::string > &_fax);
    /**
     * Contact fax is valid.
     * @return true if check was successfully
     */
    bool success()const { return !invalid; }
    bool invalid:1;///< contact fax format fails to meet the requirements
};

/// MojeID
namespace MojeID {

enum { USERNAME_LENGTH_LIMIT = 30 };

/**
 * Regular expression which match correct mojeID contact handle.
 * @return pattern usable in boost::regex_match for checking correct username format
 */
inline const boost::regex& username_pattern()
{
    static const boost::regex pattern("[0-9A-Za-z](-?[0-9A-Za-z])*");
    return pattern;
};

/**
 * MojeID contact handle verification.
 */
struct contact_username
{
    /**
     * Executes check.
     * @param _handle contact handle to verify
     */
    contact_username(const std::string &_handle);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success()const { return !(absent || invalid); }
    bool absent:1; ///< mojeID contact handle doesn't present
    bool invalid:1;///< mojeID contact handle format fails to meet the requirements
};

/**
 * MojeID contact birthday verification.
 */
struct contact_birthday
{
    /**
     * Executes check.
     * @param _ssntype type of personal identification
     * @param _ssn personal identification to verify
     */
    contact_birthday(
        const Nullable< std::string > &_ssntype,
        const Nullable< std::string > &_ssn);
    /**
     * MojeID contact birthday presents and is correct.
     * @return true if check was successfully
     */
    bool success()const { return !(absent || invalid); }
    bool absent:1; ///< mojeID contact birthday doesn't present
    bool invalid:1;///< mojeID contact birthday format fails to meet the requirements
};

/**
 * MojeID contact birthday format verification.
 */
struct contact_birthday_validity
{
    /**
     * Executes check.
     * @param _ssntype type of personal identification
     * @param _ssn personal identification to verify
     */
    contact_birthday_validity(
        const Nullable< std::string > &_ssntype,
        const Nullable< std::string > &_ssn);
    /**
     * MojeID contact birthday is valid or doesn't present.
     * @return true if check was successfully
     */
    bool success()const { return !invalid; }
    bool invalid:1;///< mojeID contact birthday format fails to meet the requirements
};

/**
 * MojeID contact vat_id presence checking.
 */
struct contact_vat_id_presence
{
    /**
     * Executes check.
     * @param _ssntype type of personal identification
     * @param _ssn personal identification to verify
     */
    contact_vat_id_presence(
        const Nullable< std::string > &_ssntype,
        const Nullable< std::string > &_ssn);
    /**
     * MojeID contact vat_id presents.
     * @return true if check was successfully
     */
    bool success()const { return !absent; }
    bool absent:1;///< mojeID contact vat_id doesn't present
};

}//Fred::GeneralCheck::MojeID
}//Fred::GeneralCheck

struct check_contact_name:GeneralCheck::contact_name
{
    check_contact_name(const InfoContactData &_data)
    :   GeneralCheck::contact_name(_data.name)
    { }
};

struct check_contact_place_address:GeneralCheck::contact_mailing_address
{
    check_contact_place_address(const Contact::PlaceAddress &_data)
    :   GeneralCheck::contact_mailing_address(
            _data.street1,
            _data.city,
            _data.postalcode,
            _data.country)
    { }
};

struct check_contact_mailing_address:check_contact_place_address
{
    check_contact_mailing_address(const InfoContactData &_data)
    :   check_contact_place_address(_data.get_address< ContactAddressType::MAILING >())
    { }
};

struct check_contact_email_presence:GeneralCheck::contact_email_presence
{
    check_contact_email_presence(const InfoContactData &_data)
    :   GeneralCheck::contact_email_presence(_data.email)
    { }
};

struct check_contact_email_validity:GeneralCheck::contact_email_validity
{
    check_contact_email_validity(const InfoContactData &_data)
    :   GeneralCheck::contact_email_validity(_data.email)
    { }
};

struct check_contact_email_availability:GeneralCheck::contact_email_availability
{
    check_contact_email_availability(const InfoContactData &_data, OperationContext &_ctx)
    :   GeneralCheck::contact_email_availability(_data.email, _data.id, _ctx)
    { }
};

struct check_contact_notifyemail_validity:GeneralCheck::contact_notifyemail_validity
{
    check_contact_notifyemail_validity(const InfoContactData &_data)
    :   GeneralCheck::contact_notifyemail_validity(_data.notifyemail)
    { }
};

struct check_contact_phone_presence:GeneralCheck::contact_phone_presence
{
    check_contact_phone_presence(const InfoContactData &_data)
    :   GeneralCheck::contact_phone_presence(_data.telephone)
    { }
};

struct check_contact_phone_validity:GeneralCheck::contact_phone_validity
{
    check_contact_phone_validity(const InfoContactData &_data)
    :   GeneralCheck::contact_phone_validity(_data.telephone)
    { }
};

struct check_contact_phone_availability:GeneralCheck::contact_phone_availability
{
    check_contact_phone_availability(const InfoContactData &_data, OperationContext &_ctx)
    :   GeneralCheck::contact_phone_availability(_data.telephone, _data.id, _ctx)
    { }
};

struct check_contact_fax_validity:GeneralCheck::contact_fax_validity
{
    check_contact_fax_validity(const InfoContactData &_data)
    :   GeneralCheck::contact_fax_validity(_data.fax)
    { }
};

/// MojeID
namespace MojeID {

struct check_contact_username:GeneralCheck::MojeID::contact_username
{
    check_contact_username(const InfoContactData &_data)
    :   GeneralCheck::MojeID::contact_username(_data.handle)
    { }
};

struct check_contact_birthday:GeneralCheck::MojeID::contact_birthday
{
    check_contact_birthday(const InfoContactData &_data)
    :   GeneralCheck::MojeID::contact_birthday(_data.ssntype, _data.ssn)
    { }
};

struct check_contact_birthday_validity:GeneralCheck::MojeID::contact_birthday_validity
{
    check_contact_birthday_validity(const InfoContactData &_data)
    :   GeneralCheck::MojeID::contact_birthday_validity(_data.ssntype, _data.ssn)
    { }
};

struct check_contact_vat_id_presence:GeneralCheck::MojeID::contact_vat_id_presence
{
    check_contact_vat_id_presence(const InfoContactData &_data)
    :   GeneralCheck::MojeID::contact_vat_id_presence(_data.ssntype, _data.ssn)
    { }
};

}//Fred::MojeID

}//Fred

#endif//CHECKERS_H_D5C22F5DFA53E06F6886FD6DE0FD30C6
