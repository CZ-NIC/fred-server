/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#ifndef ZONE_NS_FILTER_HH_F93CFDCB1F7F485B8DEDC955AAB3D9AC
#define ZONE_NS_FILTER_HH_F93CFDCB1F7F485B8DEDC955AAB3D9AC

#include "src/util/db/query/base_filters.hh"

namespace Database {
namespace Filters {

class ZoneNs:
    virtual public Compound {
public:
    virtual ~ZoneNs()
    { }

    virtual Table &joinZoneNsTable() = 0;
    virtual Value<Database::ID> &addId() = 0;
    virtual Value<Database::ID> &addZoneId() = 0;
    virtual Value<std::string> &addFqdn() = 0;
    virtual Value<std::string> &addAddrs() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class ZoneNs

class ZoneNsImpl:
    virtual public ZoneNs {
public:
    ZoneNsImpl(bool set_active = false);
    virtual ~ZoneNsImpl();

    virtual Table &joinZoneNsTable();
    virtual Value<Database::ID> &addId();
    virtual Value<Database::ID> &addZoneId();
    virtual Value<std::string> &addFqdn();
    virtual Value<std::string> &addAddrs();

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version) 
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ZoneNs);
    }
}; // class ZoneNsImpl

} // namespace Filters
} // namespace Database

#endif
