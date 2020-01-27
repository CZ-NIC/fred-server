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
#ifndef ONLINE_STATEMENT_FILTER_HH_03DC25A73FD640A482F54B85D7E16A85
#define ONLINE_STATEMENT_FILTER_HH_03DC25A73FD640A482F54B85D7E16A85

#include "src/util/db/query/base_filters.hh"

namespace Database {
namespace Filters {

class OnlineStatement: virtual public Compound {
public:
    virtual ~OnlineStatement() 
    { }

    virtual Table &joinOnlineStatementTable() = 0;
    virtual Value<Database::ID> &addId() = 0;
    virtual Value<Database::ID> &addAccountId() = 0;
    virtual Interval<Database::DateTimeInterval> &addCreateTime() = 0;
    virtual Value<std::string> &addAccountNumber() = 0;
    virtual Value<std::string> &addBankCode() = 0;
    virtual Value<std::string> &addConstSymbol() = 0;
    virtual Value<std::string> &addVarSymbol() = 0;
    virtual Value<std::string> &addAccountName() = 0;
    virtual Value<std::string> &addIdentifier() = 0;
    // TODO maybe other items from bank_ebanka_list

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class OnlineStatement

class OnlineStatementImpl: virtual public OnlineStatement {
public:
    OnlineStatementImpl();
    virtual ~OnlineStatementImpl();

    virtual Table &joinOnlineStatementTable();
    virtual Value<Database::ID> &addId();
    virtual Value<Database::ID> &addAccountId();
    virtual Interval<Database::DateTimeInterval> &addCreateTime();
    virtual Value<std::string> &addAccountNumber();
    virtual Value<std::string> &addBankCode();
    virtual Value<std::string> &addConstSymbol();
    virtual Value<std::string> &addVarSymbol();
    virtual Value<std::string> &addAccountName();
    virtual Value<std::string> &addIdentifier();

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OnlineStatement);
    }
}; // class OnlineStatementImpl

} // namespace Filters
} // namespace Database

#endif // _ONLINE_STATATEMENT_FILTER_H_

