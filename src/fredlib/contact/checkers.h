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
std::string email_phone_protection_period() { return "1MONTH"; }

/**
 * Regular expression which match correct phone number.
 * @return pattern usable in boost::regex_match for checking correct phone number format
 */
const boost::regex& phone_pattern()
{
    static const boost::regex pattern("[[:space:]]*\\+[0-9]{1,3}\\.[0-9]{1,14}[[:space:]]*");
    return pattern;
};

/**
 * Contact name verification.
 */
struct check_contact_name
{
    /**
     * Executes check.
     * @param _data data to verification
     */
    check_contact_name(const InfoContactData &_data);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success()const { return !(first_name_absents || last_name_absents); }
    bool first_name_absents:1;///< contact doesn't have first name
    bool last_name_absents:1; ///< contact doesn't have last name
};

/**
 * Contact mailing address verification.
 */
struct check_contact_mailing_address
{
    /**
     * Executes check.
     * @param _data data to verification
     */
    check_contact_mailing_address(const InfoContactData &_data);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success()const { return !(street1_absents || city_absents || postalcode_absents || country_absents); }
    bool street1_absents:1;   ///< contact doesn't have street1 entry
    bool city_absents:1;      ///< contact doesn't have city entry
    bool postalcode_absents:1;///< contact doesn't have postal code entry
    bool country_absents:1;   ///< contact doesn't have country entry
};

/**
 * Contact e-mail presence checking.
 */
struct check_contact_email_presence
{
    /**
     * Executes check.
     * @param _data data to verification
     */
    check_contact_email_presence(const InfoContactData &_data);
    /**
     * Contact e-mail presents.
     * @return true if check was successfully
     */
    bool success()const { return !absents; }
    bool absents:1;///< contact e-mail doesn't present
};

/**
 * Contact e-mail format verification.
 */
struct check_contact_email_validity
{
    /**
     * Executes check.
     * @param _data data to verification
     */
    check_contact_email_validity(const InfoContactData &_data);
    /**
     * Contact e-mail is valid.
     * @return true if check was successfully
     */
    bool success()const { return !invalid; }
    bool invalid:1;///< contact e-mail format fails to meet the requirements
};

/**
 * Contact e-mail availability verification.
 */
struct check_contact_email_availability
{
    /**
     * Executes check.
     * @param _data data to verification
     * @param _ctx operation context used to check processing
     */
    check_contact_email_availability(const InfoContactData &_data, OperationContext &_ctx);
    /**
     * Contact e-mail is available for using in next identification request.
     * @return true if check was successfully
     */
    bool success()const { return !(absents || used_recently); }
    bool absents:1;      ///< contact e-mail doesn't present
    bool used_recently:1;///< contact e-mail used for identification request recently
};

/**
 * Contact phone presence checking.
 */
struct check_contact_phone_presence
{
    /**
     * Executes check.
     * @param _data data to verification
     */
    check_contact_phone_presence(const InfoContactData &_data);
    /**
     * Contact phone presents.
     * @return true if check was successfully
     */
    bool success()const { return !absents; }
    bool absents:1;///< contact phone doesn't present
};

/**
 * Contact phone format verification.
 */
struct check_contact_phone_validity
{
    /**
     * Executes check.
     * @param _data data to verification
     */
    check_contact_phone_validity(const InfoContactData &_data);
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
struct check_contact_phone_availability
{
    /**
     * Executes check.
     * @param _data data to verification
     * @param _ctx operation context used to check processing
     */
    check_contact_phone_availability(const InfoContactData &_data, OperationContext &_ctx);
    /**
     * Contact phone is available for using in next identification request.
     * @return true if check was successfully
     */
    bool success()const { return !(absents || used_recently); }
    bool absents:1;      ///< contact phone doesn't present
    bool used_recently:1;///< contact phone used for identification request recently
};

/**
 * Contact fax format verification.
 */
struct check_contact_fax_validity
{
    /**
     * Executes check.
     * @param _data data to verification
     */
    check_contact_fax_validity(const InfoContactData &_data);
    /**
     * Contact fax is valid.
     * @return true if check was successfully
     */
    bool success()const { return !invalid; }
    bool invalid:1;///< contact fax format fails to meet the requirements
};

/// MojeID
namespace MojeID {

/**
 * Regular expression which match correct mojeID contact handle.
 * @return pattern usable in boost::regex_match for checking correct username format
 */
const boost::regex& username_pattern()
{
    static const boost::regex pattern("[0-9A-Za-z]([-0-9A-Za-z]{0,28}[0-9A-Za-z])?");
    return pattern;
};

/**
 * MojeID contact handle verification.
 */
struct check_contact_username
{
    /**
     * Executes check.
     * @param _data data to verification
     */
    check_contact_username(const InfoContactData &_data);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success()const { return !(absents || invalid); }
    bool absents:1;///< mojeID contact handle doesn't present
    bool invalid:1;///< mojeID contact handle format fails to meet the requirements
};

/**
 * MojeID contact birthday format verification.
 */
struct check_contact_birthday_validity
{
    /**
     * Executes check.
     * @param _data data to verification
     */
    check_contact_birthday_validity(const InfoContactData &_data);
    /**
     * MojeID contact birthday is valid.
     * @return true if check was successfully
     */
    bool success()const { return !invalid; }
    bool invalid:1;///< mojeID contact birthday format fails to meet the requirements
};

}//Fred::PublicRequest::MojeID

}//Fred

#endif//CHECKERS_H_D5C22F5DFA53E06F6886FD6DE0FD30C6
