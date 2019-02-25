/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef REMOVE_DELETE_CANDIDATES_HH_59422E14C17140D59C681C1D74277797
#define REMOVE_DELETE_CANDIDATES_HH_59422E14C17140D59C681C1D74277797

#include "util/nonconvertible.hh"

#include <chrono>
#include <set>
#include <string>

#include <boost/optional.hpp>

namespace Admin {

template <typename E>
using DbHandle = Util::Nonconvertible<std::string>::Named<E>;

template <typename N>
using DbId = Util::Nonconvertible<unsigned long long>::Named<N>;

template <typename E>
DbHandle<E> to_db_handle(E src);

template <typename E>
E from_db_handle(const DbHandle<E>& src);

enum class ObjectType
{
    contact,
    domain,
    keyset,
    nsset
};

template <>
DbHandle<ObjectType> to_db_handle(ObjectType enum_value);

template <>
ObjectType from_db_handle(const DbHandle<ObjectType>& handle);

ObjectType object_type_from_cli_option(const std::string& cli_option);

typedef std::set<ObjectType> SetOfObjectTypes;

boost::optional<SetOfObjectTypes> construct_set_of_object_types_from_cli_options(const std::string& cli_options);

typedef std::chrono::duration<double> Seconds;

enum class Debug
{
    on,
    off
};

template <Debug d>
void delete_objects_marked_as_delete_candidate(
        int fraction,
        const boost::optional<unsigned>& max_number_of_selected_candidates,
        const boost::optional<SetOfObjectTypes>& types,
        const Seconds& spread_deletion_in_time);

} // namespace Admin

#endif
