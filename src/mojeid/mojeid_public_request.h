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
 *  header of mojeid public request classes
 */

#ifndef MOJEID_PUBLIC_REQUEST_H_77C5F1C5C56F30200BE16A2E48A104EB//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID_PUBLIC_REQUEST_H_77C5F1C5C56F30200BE16A2E48A104EB

#include "src/fredlib/public_request/public_request_auth_type_iface.h"

namespace Fred {

std::string conditional_contact_identification_generate_passwords();

namespace MojeID {

std::string contact_transfer_request_generate_passwords();

std::string contact_identification_generate_passwords();

namespace PublicRequest {

class ContactConditionalIdentification:public PublicRequestAuthTypeIface
{
public:
    ~ContactConditionalIdentification() { }
    std::string get_public_request_type()const;
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    static std::string get_pin1_part(const std::string &_summary_password);
    static std::string get_pin2_part(const std::string &_summary_password);
private:
    std::string generate_passwords()const;
};

class ContactIdentification:public PublicRequestAuthTypeIface
{
public:
    ~ContactIdentification() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    std::string generate_passwords()const;
};

class ContactReidentification:public PublicRequestAuthTypeIface
{
public:
    ~ContactReidentification() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    std::string generate_passwords()const;
};

class ContactValidation:public PublicRequestTypeIface
{
public:
    ~ContactValidation() { }
    const PublicRequestTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
};

class ConditionallyIdentifiedContactTransfer:public PublicRequestAuthTypeIface
{
public:
    ~ConditionallyIdentifiedContactTransfer() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    std::string generate_passwords()const;
};

class IdentifiedContactTransfer:public PublicRequestAuthTypeIface
{
public:
    ~IdentifiedContactTransfer() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    std::string generate_passwords()const;
};

class PrevalidatedUnidentifiedContactTransfer:public PublicRequestAuthTypeIface
{
public:
    ~PrevalidatedUnidentifiedContactTransfer() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    std::string generate_passwords()const;
};

class PrevalidatedContactTransfer:public PublicRequestAuthTypeIface
{
public:
    ~PrevalidatedContactTransfer() { }
    const PublicRequestAuthTypeIface& iface()const { return *this; }
    std::string get_public_request_type()const;
private:
    std::string generate_passwords()const;
};

}//Fred::MojeID::PublicRequest
}//Fred::MojeID
}//Fred

#endif//MOJEID_PUBLIC_REQUEST_H_77C5F1C5C56F30200BE16A2E48A104EB
