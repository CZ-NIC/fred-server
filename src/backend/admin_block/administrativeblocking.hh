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
 *  @administrativeblocking.h
 *  header of administrativeblocking implementation
 */

#ifndef ADMINISTRATIVEBLOCKING_HH_148AC7F4CEE549DEB1E2489BBCC73B55
#define ADMINISTRATIVEBLOCKING_HH_148AC7F4CEE549DEB1E2489BBCC73B55

#include "libfred/object_state/get_blocking_status_desc_list.hh"
#include "util/db/nullable.hh"

#include <list>
#include <set>
#include <string>

namespace Fred {
namespace Backend {
namespace AdministrativeBlocking {

struct IdlOwnerChange
{
    unsigned long long domain_id;
    std::string domain_handle;
    unsigned long long old_owner_id;
    std::string old_owner_handle;
    unsigned long long new_owner_id;
    std::string new_owner_handle;

};

typedef std::list<IdlOwnerChange> IdlOwnerChangeList;

typedef std::set<LibFred::ObjectId> IdSet;
typedef IdSet IdlDomainIdList;

typedef std::set<std::string> StringSet;
typedef StringSet IdlStatusList;

enum IdlOwnerBlockMode
{
    OWNER_BLOCK_MODE_KEEP_OWNER,
    OWNER_BLOCK_MODE_BLOCK_OWNER,
    OWNER_BLOCK_MODE_BLOCK_OWNER_COPY

};

struct EX_INTERNAL_SERVER_ERROR
{
    std::string what;

};

struct EX_DOMAIN_ID_NOT_FOUND
{

    typedef IdSet Type;

    Type what;

};

struct EX_UNKNOWN_STATUS
{

    typedef StringSet Type;

    Type what;

};

struct EX_DOMAIN_ID_ALREADY_BLOCKED
{
    struct Item
    {
        unsigned long long domain_id;
        std::string domain_handle;
        bool operator<(const struct Item& _b) const
        {
            return domain_id < _b.domain_id;
        }

    };

    typedef std::set<struct Item> Type;

    Type what;

};

struct EX_OWNER_HAS_OTHER_DOMAIN
{
    struct Item
    {
        std::string owner_handle;
        EX_DOMAIN_ID_ALREADY_BLOCKED::Type domain;

    };

    typedef std::map<unsigned long long, struct Item> Type;

    Type what;

};

struct EX_DOMAIN_ID_NOT_BLOCKED
{
    struct Item
    {
        unsigned long long domain_id;
        std::string domain_handle;
        bool operator<(const struct Item& _b) const
        {
            return domain_id < _b.domain_id;
        }

    };

    typedef std::set<struct Item> Type;

    Type what;

};

struct EX_CONTACT_BLOCK_PROHIBITED
{
    struct Item
    {
        unsigned long long contact_id;
        std::string contact_handle;
        bool operator<(const struct Item& _b) const
        {
            return contact_id < _b.contact_id;
        }

    };

    typedef std::set<struct Item> Type;

    Type what;

};

struct EX_NEW_OWNER_DOES_NOT_EXISTS
{
    std::string what;

};

class BlockingImpl
{
public:
    BlockingImpl(const std::string& _server_name)
        : server_name_(_server_name)
    {
    }

    virtual ~BlockingImpl()
    {
    }

    const std::string& get_server_name() const
    {
        return server_name_;
    }

    LibFred::GetBlockingStatusDescList::StatusDescList getBlockingStatusDescList(const std::string& _lang);


    IdlOwnerChangeList blockDomainsId(
            const IdlDomainIdList& _domain_list,
            const LibFred::StatusList& _status_list,
            IdlOwnerBlockMode _owner_block_mode,
            const Nullable<boost::gregorian::date>& _block_to_date,
            const std::string& _reason,
            unsigned long long _log_req_id);


    void restorePreAdministrativeBlockStatesId(
            const IdlDomainIdList& _domain_list,
            const Nullable<std::string>& _new_owner,
            const std::string& _reason,
            unsigned long long _log_req_id);


    void updateBlockDomainsId(
            const IdlDomainIdList& _domain_list,
            const LibFred::StatusList& _status_list,
            const Nullable<boost::gregorian::date>& _block_to_date,
            const std::string& _reason,
            unsigned long long _log_req_id);


    void unblockDomainsId(
            const IdlDomainIdList& _domain_list,
            const Nullable<std::string>& _new_owner,
            bool _remove_admin_c,
            const std::string& _reason,
            unsigned long long _log_req_id);


    void blacklistAndDeleteDomainsId(
            const IdlDomainIdList& _domain_list,
            const Nullable<boost::gregorian::date>& _blacklist_to_date,
            const std::string& _reason,
            unsigned long long _log_req_id);


    void blacklistDomainsId(
            const IdlDomainIdList& _domain_list,
            const Nullable<boost::gregorian::date>& _blacklist_to_date,
            bool _with_delete,
            unsigned long long _log_req_id);


private:
    std::string server_name_;
};

} // namespace Fred::Backend::AdministrativeBlocking
} // namespace Fred::Backend
} // namespace Fred

#endif
