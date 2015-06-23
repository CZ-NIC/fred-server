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
 *  header of mojeid2 implementation
 */

#ifndef MOJEID2_H_06D795C17DD0FF3D98B375032F99493A//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID2_H_06D795C17DD0FF3D98B375032F99493A

#include "util/db/nullable.h"

#include <string>
#include <vector>

namespace Fred {
namespace MojeID {

struct Address
{
    std::string street1;
    Nullable< std::string > street2;
    Nullable< std::string > street3;
    std::string city;
    Nullable< std::string > state;
    std::string postal_code;
    std::string country;
};

struct ShippingAddress
{
    Nullable< std::string > company_name;
    std::string street1;
    Nullable< std::string > street2;
    Nullable< std::string > street3;
    std::string city;
    Nullable< std::string > state;
    std::string postal_code;
    std::string country;
};

struct CreateContact
{
    std::string username;
    std::string first_name;
    std::string last_name;
    Nullable< std::string > organization;
    Nullable< std::string > vat_reg_num;
    Nullable< std::string > birth_date;
    Nullable< std::string > id_card_num;
    Nullable< std::string > passport_num;
    Nullable< std::string > ssn_id_num;
    Nullable< std::string > vat_id_num;
    Address permanent;
    Nullable< Address > mailing;
    Nullable< Address > billing;
    Nullable< ShippingAddress > shipping;
    Nullable< ShippingAddress > shipping2;
    Nullable< ShippingAddress > shipping3;
    std::string email;
    Nullable< std::string > notify_email;
    std::string teplephone;
    Nullable< std::string > fax;
};

}//Fred::MojeID
}//Fred

namespace Registry {
namespace MojeID {

typedef unsigned long long ContactId;
typedef unsigned long long LogRequestId;

typedef std::vector< std::string > HandleList;

class MojeID2Impl
{
public:
    MojeID2Impl(const std::string &_server_name);
    ~MojeID2Impl();

    const std::string& get_server_name()const;

    static const ContactId contact_handles_start = 0;
    static const ContactId contact_handles_end_reached = 0;
    HandleList& get_unregistrable_contact_handles(
        ::size_t _chunk_size,
        ContactId &_start_from,
        HandleList &_result)const;

    ContactId create_contact_prepare(
        const Fred::MojeID::CreateContact &_contact,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident);
private:
    const std::string server_name_;
    const std::string mojeid_registrar_handle_;
};//class MojeID2Impl

}//namespace Registry::MojeID
}//namespace Registry

#endif // MOJEID2_H_06D795C17DD0FF3D98B375032F99493A
