/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 */

#ifndef CREATE_H_81A29A35F62E4C92373D668458E7619E//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_H_81A29A35F62E4C92373D668458E7619E

#include "src/fredlib/opcontext.h"
#include "src/epp/keyset/info_data.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct KeysetCreateResult
{
    unsigned long long id;
    unsigned long long create_history_id;
    boost::posix_time::ptime crdate;
};

/**
 * @throws AuthErrorServerClosingConnection
 * @throws ObjectExists
 * @throws AggregatedParamErrors
 */
ContactCreateResult keyset_create(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    const std::string &_auth_info_pw,
    const KeysetInfoData::TechContacts &_tech_contacts,
    const KeysetInfoData::DsRecords &_ds_records,
    const KeysetInfoData::DnsKeys &_dns_keys,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id);

}

#endif//CREATE_H_81A29A35F62E4C92373D668458E7619E
