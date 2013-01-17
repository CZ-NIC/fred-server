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
 *  @file create_contact.h
 *  create contact
 */


#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "fredlib/contact/create_contact.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    CreateContact::CreateContact(const std::string& handle
                , const std::string& registrar)
    : handle_(handle)
    , registrar_(registrar)
    {}

    CreateContact::CreateContact(const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& authinfo
            )
    : handle_(handle)
    , registrar_(registrar)
    , authinfo_(authinfo)
    {}

    CreateContact& CreateContact::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    std::string exec(OperationContext& ctx)
    {
        std::string timestamp;

        return timestamp;
    }

}//namespace Fred

