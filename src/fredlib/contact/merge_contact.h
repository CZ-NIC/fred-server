/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file merge_contact.h
 *  contact merge
 */

#ifndef MERGE_CONTACT_H
#define MERGE_CONTACT_H

#include <string>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"

namespace Fred
{

    class MergeContact
    {
        const std::string src_contact_handle_;//source contact identifier
        const std::string dst_contact_handle_;//destination contact identifier
        const std::string registrar_;//registrar used for object updates
        Optional<unsigned long long> logd_request_id_; //id of the new entry in log_entry
    public:
        MergeContact(const std::string& from_contact_handle, const std::string& to_contact_handle, const std::string& registrar);
        MergeContact& set_logd_request_id(unsigned long long logd_request_id);
        void exec(OperationContext& ctx, std::string* dry_run = 0);
    };//class MergeContact

}//namespace Fred

#endif//MERGE_CONTACT_H
