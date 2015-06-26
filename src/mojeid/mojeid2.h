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

#include "src/mojeid/mojeid2_checkers.h"

#include <vector>

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
        const Fred::InfoContactData &_contact,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident);

    typedef boost::mpl::list< Fred::check_contact_name,
                              Fred::check_contact_place_address,
                              Fred::check_contact_addresses_mailing,
                              Fred::check_contact_addresses_billing,
                              Fred::check_contact_addresses_shipping,
                              Fred::check_contact_addresses_shipping2,
                              Fred::check_contact_addresses_shipping3,
                              Fred::check_contact_email_presence,
                              Fred::check_contact_email_validity,
                              Fred::check_contact_phone_presence,
                              Fred::check_contact_phone_validity > check_create_contact_prepare;
    typedef boost::mpl::list< Fred::MojeID::check_contact_username_availability,
                              Fred::MojeID::Check::new_contact_email_availability,
                              Fred::MojeID::Check::new_contact_phone_availability > check_create_contact_prepare_ctx;
    typedef Fred::Check< boost::mpl::list< check_create_contact_prepare,
                                           check_create_contact_prepare_ctx > > CheckCreateContactPrepare;
    typedef CheckCreateContactPrepare CreateContactPrepareError;
private:
    const std::string server_name_;
    const std::string mojeid_registrar_handle_;
};//class MojeID2Impl

}//namespace Registry::MojeID
}//namespace Registry

#endif // MOJEID2_H_06D795C17DD0FF3D98B375032F99493A
