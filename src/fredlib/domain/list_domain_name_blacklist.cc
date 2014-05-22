/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @file list_domain_name_blacklist.cc
 *  list domain name blacklist
 */

#include "src/fredlib/domain/list_domain_name_blacklist.h"
#include "src/fredlib/domain/get_blocking_status_desc_list.h"
#include "src/fredlib/domain/get_object_state_id_map.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "src/fredlib/object.h"
#include "cli_admin/domain_params.h"

#include <boost/algorithm/string.hpp>

namespace Fred
{

    DomainNameBlacklistItem::DomainNameBlacklistItem(Id _id, const std::string &_domain, const Time &_valid_from,
        const Optional< Time > &_valid_to, const std::string &_reason)
    :   id_(_id),
        domain_(_domain),
        valid_from_(_valid_from),
        valid_to_(_valid_to),
        reason_(_reason)
    {}

    DomainNameBlacklistItem::DomainNameBlacklistItem(const DomainNameBlacklistItem &_src)
    :   id_(_src.id_),
        domain_(_src.domain_),
        valid_from_(_src.valid_from_),
        valid_to_(_src.valid_to_),
        reason_(_src.reason_)
    {}

    DomainNameBlacklistItem& DomainNameBlacklistItem::operator=(const DomainNameBlacklistItem &_src)
    {
        id_ = _src.id_;
        domain_ = _src.domain_;
        valid_from_ = _src.valid_from_;
        valid_to_ = _src.valid_to_;
        reason_ = _src.reason_;
        return *this;
    }

    ListDomainNameBlacklist::ListDomainNameBlacklist()
    {}

    DomainNameBlacklist& ListDomainNameBlacklist::exec(OperationContext &_ctx)
    {
        blacklist_.clear();
        enum ResultIdx
        {
            COLUMN_ID_IDX         = 0,
            COLUMN_DOMAIN_IDX     = 1,
            COLUMN_VALID_FROM_IDX = 2,
            COLUMN_VALID_TO_IDX   = 3,
            COLUMN_REASON_IDX     = 4,
        };
        Database::Result blacklist_result = _ctx.get_conn().exec(
            "SELECT id,regexp,valid_from,valid_to,reason "
            "FROM domain_blacklist "
            "WHERE CURRENT_TIMESTAMP<valid_to OR valid_to IS NULL");
        if (blacklist_result.size() <= 0) {
            return blacklist_;
        }
        blacklist_.reserve(blacklist_result.size());
        for (Database::Result::Iterator pRow = blacklist_result.begin(); pRow != blacklist_result.end(); ++pRow) {
            Optional< DomainNameBlacklistItem::Time > valid_to = (*pRow)[COLUMN_VALID_TO_IDX].isnull()
              ? Optional< DomainNameBlacklistItem::Time >()
              : Optional< DomainNameBlacklistItem::Time >(static_cast< const DomainNameBlacklistItem::Time& >((*pRow)[COLUMN_VALID_TO_IDX]));
            blacklist_.push_back(DomainNameBlacklistItem((*pRow)[COLUMN_ID_IDX],
                                                         (*pRow)[COLUMN_DOMAIN_IDX],
                                                         (*pRow)[COLUMN_VALID_FROM_IDX],
                                                         valid_to,
                                                         (*pRow)[COLUMN_REASON_IDX]));
        }
        return blacklist_;
    }//ListDomainNameBlacklist::exec

}//namespace Fred
