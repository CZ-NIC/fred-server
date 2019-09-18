/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef KEYSET_FILTER_HH_26AEA8517BF34970B574BEF10682B753
#define KEYSET_FILTER_HH_26AEA8517BF34970B574BEF10682B753

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "src/deprecated/model/object_filter.hh"
#include "src/deprecated/model/contact_filter.hh"

namespace Database {
namespace Filters {

class KeySet: virtual public Object {
public:
    virtual ~KeySet() { }

    virtual Table       &joinKeySetTable() = 0;
    virtual Value<ID>   &addId() = 0;
    virtual Value<std::string> &addHandle() = 0;
    virtual Contact     &addTechContact() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &_ar,
            const unsigned int) {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
    }
};

class KeySetImpl: public ObjectImpl, virtual public KeySet {
public:
    KeySetImpl();
    virtual ~KeySetImpl();

    virtual ObjectType getType() const {
        return TKEYSET;
    }

    virtual Table       &joinKeySetTable();
    virtual void        _joinPolymorphicTables();

    virtual Value<ID>   &addId();
    virtual Value<std::string> &addHandle();
    virtual Contact     &addTechContact();

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &_ar,
            const unsigned int) {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(KeySet);
    }
};

class KeySetHistoryImpl: public ObjectHistoryImpl, virtual public KeySet {
    public:
        KeySetHistoryImpl();
        virtual ~KeySetHistoryImpl();

        virtual ObjectType getType() const {
            return TKEYSET;
        }

    virtual Table       &joinKeySetTable();
    virtual void        _joinPolymorphicTables();

    virtual Value<ID>   &addId();
    virtual Value<std::string> &addHandle();
    virtual Contact     &addTechContact();

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &_ar,
            const unsigned int) {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(KeySet);
    }
};

} // namespace Filters
} // namespace Database

#endif
