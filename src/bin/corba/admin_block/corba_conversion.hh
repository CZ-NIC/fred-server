/*
 * Copyright (C) 2013-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CORBA_CONVERSION_HH_4787F0F5B8964E3282F494229EA4B01B
#define CORBA_CONVERSION_HH_4787F0F5B8964E3282F494229EA4B01B

#include "src/backend/admin_block/administrativeblocking.hh"
#include "corba/AdministrativeBlocking.hh"
#include "src/bin/corba/common_wrappers.hh"
#include "libfred/object_state/get_blocking_status_desc_list.hh"
#include "util/db/nullable.hh"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <memory>
#include <string>

namespace CorbaConversion {
namespace AdministrativeBlocking {

Registry::Administrative::NullableString* corba_wrap_nullable_string(const Nullable<std::string>& _v)
{
    if (_v.isnull())
    {
        return 0;
    }
    return new Registry::Administrative::NullableString(_v.get_value().c_str());
}


Registry::Administrative::NullableBoolean* corba_wrap_nullable_boolean(const Nullable<bool>& _v)
{
    if (_v.isnull())
    {
        return 0;
    }
    return new Registry::Administrative::NullableBoolean(_v.get_value());
}


Registry::Administrative::NullableBoolean* corba_wrap_nullable_boolean(const bool _v)
{
    return new Registry::Administrative::NullableBoolean(_v);
}


Nullable<std::string> corba_unwrap_nullable_string(const Registry::Administrative::NullableString* _v)
{
    if (_v)
    {
        return std::string(_v->_value());
    }
    return Nullable<std::string>();
}


bool is_not_normal(int c)
{
    return c == '\r' || c == '\n' || c == '\t';
}


Nullable<std::string> corba_unwrap_normalize_nullable_string(
        const Registry::Administrative::NullableString* _v)
{
    if (_v)
    {
        std::string tmp(_v->_value());
        // normalize
        tmp.erase(
                std::remove_if(
                        tmp.begin(),
                        tmp.end(),
                        is_not_normal),
                tmp.end());

        return tmp;
    }
    return Nullable<std::string>();
}


Nullable<bool> corba_unwrap_nullable_boolean(const Registry::Administrative::NullableBoolean* _v)
{
    if (_v)
    {
        return _v->_value();
    }
    return Nullable<bool>();
}


bool corba_unwrap_nullable_boolean(
        const Registry::Administrative::NullableBoolean* _v,
        const bool null_value)
{
    if (_v)
    {
        return _v->_value();
    }
    return null_value;
}


Registry::Administrative::StatusDescList* corba_wrap_status_desc_list(
        const LibFred::GetBlockingStatusDescList::StatusDescList& _desc_list)
{
    std::unique_ptr<Registry::Administrative::StatusDescList> result(
            new Registry::Administrative::StatusDescList);
    result->length(_desc_list.size());
    int n = 0;
    for (LibFred::GetBlockingStatusDescList::StatusDescList::const_iterator pItem = _desc_list.begin();
         pItem != _desc_list.end();
         ++n, ++pItem)
    {
        Registry::Administrative::StatusDesc& item = (*result)[n];
        item.id = pItem->state_id;
        item.shortName = corba_wrap_string(pItem->status);
        item.name = corba_wrap_string(pItem->desc);
    }
    return result.release();
}


Registry::Administrative::DomainIdHandleOwnerChangeList* corba_wrap_owner_change_list(
        const Fred::Backend::AdministrativeBlocking::IdlOwnerChangeList& _owner_change_list)
{
    std::unique_ptr<Registry::Administrative::DomainIdHandleOwnerChangeList> result(
            new Registry::Administrative::DomainIdHandleOwnerChangeList);
    result->length(_owner_change_list.size());
    int n = 0;
    for (Fred::Backend::AdministrativeBlocking::IdlOwnerChangeList::const_iterator pItem =
             _owner_change_list.begin();
         pItem != _owner_change_list.end();
         ++n, ++pItem)
    {
        Registry::Administrative::DomainIdHandleOwnerChange& item = (*result)[n];
        item.domainId = pItem->domain_id;
        item.domainHandle = corba_wrap_string(pItem->domain_handle);
        item.oldOwnerId = pItem->old_owner_id;
        item.oldOwnerHandle = corba_wrap_string(pItem->old_owner_handle);
        item.newOwnerId = pItem->new_owner_id;
        item.newOwnerHandle = corba_wrap_string(pItem->new_owner_handle);
    }
    return result.release();
}


Registry::Administrative::INTERNAL_SERVER_ERROR corba_wrap_exception(
        const Fred::Backend::AdministrativeBlocking::EX_INTERNAL_SERVER_ERROR& _e)
{
    return Registry::Administrative::INTERNAL_SERVER_ERROR(corba_wrap_string(_e.what));
}


Registry::Administrative::DOMAIN_ID_NOT_FOUND corba_wrap_exception(
        const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND& _e)
{
    Registry::Administrative::DOMAIN_ID_NOT_FOUND ex;
    ex.what.length(_e.what.size());
    int n = 0;
    for (Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND::Type::const_iterator pItem =
             _e.what.begin();
         pItem != _e.what.end();
         ++n, ++pItem)
    {
        ex.what[n] = *pItem;
    }
    return ex;
}


Registry::Administrative::UNKNOWN_STATUS corba_wrap_exception(
        const Fred::Backend::AdministrativeBlocking::EX_UNKNOWN_STATUS& _e)
{
    Registry::Administrative::UNKNOWN_STATUS ex;
    ex.what.length(_e.what.size());
    int n = 0;
    for (Fred::Backend::AdministrativeBlocking::EX_UNKNOWN_STATUS::Type::const_iterator pItem =
             _e.what.begin();
         pItem != _e.what.end();
         ++n, ++pItem)
    {
        ex.what[n] = corba_wrap_string(*pItem);
    }
    return ex;
}


Registry::Administrative::DOMAIN_ID_ALREADY_BLOCKED corba_wrap_exception(
        const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_ALREADY_BLOCKED& _e)
{
    Registry::Administrative::DOMAIN_ID_ALREADY_BLOCKED ex;
    ex.what.length(_e.what.size());
    int n = 0;
    for (Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_ALREADY_BLOCKED::Type::const_iterator pItem =
             _e.what.begin();
         pItem != _e.what.end();
         ++n, ++pItem)
    {
        Registry::Administrative::DomainIdHandle& item = ex.what[n];
        item.domainId = pItem->domain_id;
        item.domainHandle = corba_wrap_string(pItem->domain_handle);
    }
    return ex;
}


Registry::Administrative::OWNER_HAS_OTHER_DOMAIN corba_wrap_exception(
        const Fred::Backend::AdministrativeBlocking::EX_OWNER_HAS_OTHER_DOMAIN& _e)
{
    Registry::Administrative::OWNER_HAS_OTHER_DOMAIN ex;
    ex.what.length(_e.what.size());
    int n = 0;
    for (Fred::Backend::AdministrativeBlocking::EX_OWNER_HAS_OTHER_DOMAIN::Type::const_iterator pItem =
             _e.what.begin();
         pItem != _e.what.end();
         ++n, ++pItem)
    {
        Registry::Administrative::OwnerDomain& item = ex.what[n];
        item.ownerId = pItem->first;
        item.ownerHandle = corba_wrap_string(pItem->second.owner_handle);
        item.otherDomainList.length(pItem->second.domain.size());
        int n = 0;
        for (Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_ALREADY_BLOCKED::Type::const_iterator pDomain
                 = pItem->second.domain.begin();
             pDomain != pItem->second.domain.end();
             ++n, ++pDomain)
        {
            Registry::Administrative::DomainIdHandle& domain = item.otherDomainList[n];
            domain.domainId = pDomain->domain_id;
            domain.domainHandle = corba_wrap_string(pDomain->domain_handle);
        }
    }
    return ex;
}


Registry::Administrative::DOMAIN_ID_NOT_BLOCKED corba_wrap_exception(
        const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_BLOCKED& _e)
{
    Registry::Administrative::DOMAIN_ID_NOT_BLOCKED ex;
    ex.what.length(_e.what.size());
    int n = 0;
    for (Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_BLOCKED::Type::const_iterator pItem =
             _e.what.begin();
         pItem != _e.what.end();
         ++n, ++pItem)
    {
        Registry::Administrative::DomainIdHandle& item = ex.what[n];
        item.domainId = pItem->domain_id;
        item.domainHandle = corba_wrap_string(pItem->domain_handle);
    }
    return ex;
}


Registry::Administrative::CONTACT_BLOCK_PROHIBITED corba_wrap_exception(
        const Fred::Backend::AdministrativeBlocking::EX_CONTACT_BLOCK_PROHIBITED& _e)
{
    Registry::Administrative::CONTACT_BLOCK_PROHIBITED ex;
    ex.what.length(_e.what.size());
    int n = 0;
    for (Fred::Backend::AdministrativeBlocking::EX_CONTACT_BLOCK_PROHIBITED::Type::const_iterator pItem =
             _e.what.begin();
         pItem != _e.what.end();
         ++n, ++pItem)
    {
        Registry::Administrative::ContactIdHandle& item = ex.what[n];
        item.contactId = pItem->contact_id;
        item.contactHandle = corba_wrap_string(pItem->contact_handle);
    }
    return ex;
}


Registry::Administrative::NEW_OWNER_DOES_NOT_EXISTS corba_wrap_exception(
        const Fred::Backend::AdministrativeBlocking::EX_NEW_OWNER_DOES_NOT_EXISTS& _e)
{
    return Registry::Administrative::NEW_OWNER_DOES_NOT_EXISTS(corba_wrap_string(_e.what));
}


Fred::Backend::AdministrativeBlocking::IdlDomainIdList corba_unwrap_domain_id_list(
        const Registry::Administrative::DomainIdList& _domain_id_list)
{
    Fred::Backend::AdministrativeBlocking::IdlDomainIdList result;
    for (::size_t idx = 0; idx < _domain_id_list.length(); ++idx)
    {
        result.insert(_domain_id_list[idx]);
    }
    return result;
}


LibFred::StatusList corba_unwrap_status_list(const Registry::Administrative::StatusList& _status_list)
{
    LibFred::StatusList result;
    for (::size_t idx = 0; idx < _status_list.length(); ++idx)
    {
        result.insert(_status_list[idx].in());
    }
    return result;
}


Fred::Backend::AdministrativeBlocking::IdlOwnerBlockMode corba_unwrap_owner_block_mode(
        Registry::Administrative::OwnerBlockMode _owner_block_mode)
{
    switch (_owner_block_mode)
    {
        case Registry::Administrative::KEEP_OWNER:
            return Fred::Backend::AdministrativeBlocking::OWNER_BLOCK_MODE_KEEP_OWNER;

        case Registry::Administrative::BLOCK_OWNER:
            return Fred::Backend::AdministrativeBlocking::OWNER_BLOCK_MODE_BLOCK_OWNER;

        case Registry::Administrative::BLOCK_OWNER_COPY:
            return Fred::Backend::AdministrativeBlocking::OWNER_BLOCK_MODE_BLOCK_OWNER_COPY;

        default:
            throw std::runtime_error("bad owner_block_mode value");
    }
}


} // namespace CorbaConversion::AdministrativeBlocking
} // namespace CorbaConversion

#endif
