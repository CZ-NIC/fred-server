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
 *  header of mojeid2 public request classes
 */

#ifndef MOJEID_PUBLIC_REQUEST_H_77C5F1C5C56F30200BE16A2E48A104EB//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID_PUBLIC_REQUEST_H_77C5F1C5C56F30200BE16A2E48A104EB

#include "src/fredlib/public_request/public_request_type_iface.h"
#include "src/fredlib/public_request/public_request_auth_type_iface.h"

namespace Fred {

std::string conditional_contact_identification_generate_passwords();

namespace MojeID {

std::string contact_transfer_request_generate_passwords();

namespace PublicRequest {

class ContactConditionalIdentification:private PublicRequestAuthTypeIface
{
public:
    virtual ~ContactConditionalIdentification() { }
    static const PublicRequestAuthTypeIface& iface()
    {
        static const ContactConditionalIdentification instance;
        return static_cast< const PublicRequestAuthTypeIface& >(instance);
    }
private:
    ContactConditionalIdentification() { }
    std::string get_public_request_type()const;
    std::string generate_passwords()const;
};

class ConditionallyIdentifiedContactTransfer:private PublicRequestAuthTypeIface
{
public:
    virtual ~ConditionallyIdentifiedContactTransfer() { }
    static const PublicRequestAuthTypeIface& iface()
    {
        static const ConditionallyIdentifiedContactTransfer instance;
        return static_cast< const PublicRequestAuthTypeIface& >(instance);
    }
private:
    ConditionallyIdentifiedContactTransfer() { }
    std::string get_public_request_type()const;
    std::string generate_passwords()const;
};

class IdentifiedContactTransfer:private PublicRequestAuthTypeIface
{
public:
    virtual ~IdentifiedContactTransfer() { }
    static const PublicRequestAuthTypeIface& iface()
    {
        static const IdentifiedContactTransfer instance;
        return static_cast< const PublicRequestAuthTypeIface& >(instance);
    }
private:
    IdentifiedContactTransfer() { }
    std::string get_public_request_type()const;
    std::string generate_passwords()const;
};

}//Fred::MojeID::PublicRequest
}//Fred::MojeID
}//Fred

#endif//MOJEID_PUBLIC_REQUEST_H_77C5F1C5C56F30200BE16A2E48A104EB
