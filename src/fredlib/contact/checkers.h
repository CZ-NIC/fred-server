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

std::string email_phone_protection_period() { return "1MONTH"; }

const boost::regex& phone_pattern()
{
    static const boost::regex pattern("[[:space:]]*\\+[0-9]{1,3}\\.[0-9]{1,14}[[:space:]]*");
    return pattern;
};

struct check_contact_name
{
    check_contact_name(const InfoContactData &_data);
    bool success()const { return !(first_name_absents || last_name_absents); }
    bool first_name_absents:1;
    bool last_name_absents:1;
};

struct check_contact_mailing_address
{
    check_contact_mailing_address(const InfoContactData &_data);
    bool success()const { return !(street1_absents || city_absents || postalcode_absents || country_absents); }
    bool street1_absents:1;
    bool city_absents:1;
    bool postalcode_absents:1;
    bool country_absents:1;
};

struct check_contact_email_presence
{
    check_contact_email_presence(const InfoContactData &_data);
    bool success()const { return !absents; }
    bool absents:1;
};

struct check_contact_email_validity
{
    check_contact_email_validity(const InfoContactData &_data);
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct check_contact_email_availability:check_contact_email_presence
{
    check_contact_email_availability(const InfoContactData &_data, OperationContext &_ctx);
    bool success()const { return this->check_contact_email_presence::success() && !used_recently; }
    bool used_recently:1;
};

struct check_contact_phone_presence
{
    check_contact_phone_presence(const InfoContactData &_data);
    bool success()const { return !absents; }
    bool absents:1;
};

struct check_contact_phone_validity
{
    check_contact_phone_validity(const InfoContactData &_data);
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct check_contact_phone_availability:check_contact_phone_presence
{
    check_contact_phone_availability(const InfoContactData &_data, OperationContext &_ctx);
    bool success()const { return this->check_contact_phone_presence::success() && !used_recently; }
    bool used_recently:1;
};

struct check_contact_fax_validity
{
    check_contact_fax_validity(const InfoContactData &_data);
    bool success()const { return !invalid; }
    bool invalid:1;
};

namespace MojeID {

const boost::regex& username_pattern()
{
    static const boost::regex pattern("[0-9A-Za-z]([-0-9A-Za-z]{0,28}[0-9A-Za-z])?");
    return pattern;
};

struct check_contact_username
{
    check_contact_username(const InfoContactData &_data);
    bool success()const { return !(absents || invalid); }
    bool absents:1;
    bool invalid:1;
};

struct check_contact_birthday_validity
{
    check_contact_birthday_validity(const InfoContactData &_data);
    bool success()const { return !invalid; }
    bool invalid:1;
};

}//Fred::PublicRequest::MojeID

}//Fred

#endif//CHECKERS_H_D5C22F5DFA53E06F6886FD6DE0FD30C6
