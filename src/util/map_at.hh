/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAP_AT_HH_0F40C4D88AA5475C91EF7F998E0B968A
#define MAP_AT_HH_0F40C4D88AA5475C91EF7F998E0B968A

#include <stdexcept>


template<typename T>
const typename T::mapped_type& map_at(
        const T &_container, const typename T::key_type &_key)
{
    typename T::const_iterator it;
    if ((it = _container.find(_key)) == _container.end()) {
        throw std::out_of_range("map_at: not found");
    }
    return it->second;
}

/**
 * if value is found in map by the key, return set optional map value, if not return unset optional map value
 * ReturnOptionalTypeTemplate might be Optional or Nullable template
 */
template<template <typename> class ReturnOptionalTypeTemplate, typename T>
ReturnOptionalTypeTemplate<typename T::mapped_type> optional_map_at(
        const T &_container, const typename T::key_type &_key)
{
    typename T::const_iterator it;
    if ((it = _container.find(_key)) == _container.end()) {
        return ReturnOptionalTypeTemplate<typename T::mapped_type>();
    }
    return ReturnOptionalTypeTemplate<typename T::mapped_type>(it->second);
}

template<typename CONTAINER, class EXCEPTION>
const typename CONTAINER::mapped_type& map_at_ex(
        const CONTAINER &_container
        , const typename CONTAINER::key_type &_key
        ,  EXCEPTION ex)//exception to throw when _key not fond in _container
{
    typename CONTAINER::const_iterator it;
    if ((it = _container.find(_key)) == _container.end()) {
        throw ex;
    }
    return it->second;
}


#endif /*MAP_AT_H_*/
