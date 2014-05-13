/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  @file
 *  UUID type
 */

#ifndef UTIIL_UUID_H_2323468522
#define UTIIL_UUID_H_2323468522

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

class uuid {
    public:
        struct ExceptionInvalidUuid {
            const char* what() const throw() {return "invalid input UUID";}
        };
    private:
        boost::uuids::uuid value_;

        // the weird syntax is Function Try Block
        explicit uuid(const std::string& _in)
        try
            : value_(boost::uuids::string_generator()(_in))
        {
            // cannonical form e. g. 550e8400-e29b-41d4-a716-446655440000
            if(_in.length() != 36) {
                throw ExceptionInvalidUuid();
            }
        }
        catch(const std::runtime_error&) {
            throw ExceptionInvalidUuid();
        }
    public:
        // intentionally not providing default constructor - empty uuid is not valid

        /**
         * named constructor
         *
         * @throws ExceptionInvalidUuid
         */
        static uuid from_string(const std::string& _in) {
            return uuid(_in);
        }

        explicit uuid(const boost::uuids::uuid& _in)
        : value_(_in)
        { }

        uuid& operator=(const uuid& _rhs) {
            value_ = _rhs.value_;

            return *this;
        }

        uuid& operator=(const boost::uuids::uuid& _rhs) {
            value_ = _rhs;

            return *this;
        }

        operator std::string() const {
            return boost::lexical_cast<std::string>(value_);
        }

        std::string to_string() const {
            return static_cast<std::string>(*this);
        }
};

#endif
