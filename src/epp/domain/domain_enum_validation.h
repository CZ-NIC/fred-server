/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 */

#ifndef EPP_DOMAIN_ENUM_VALIDATION_H_1bd0bdf53cf64258a3ac2f212d7b593e
#define EPP_DOMAIN_ENUM_VALIDATION_H_1bd0bdf53cf64258a3ac2f212d7b593e

#include <boost/date_time/gregorian/gregorian.hpp>
#include <stdexcept>

namespace Epp {

    /**
     *
     */
    class ENUMValidationExtension
    {
        boost::gregorian::date valexdate;
        bool publish;

    public:
        ENUMValidationExtension()
        : publish(false)
        {}
        ENUMValidationExtension(const boost::gregorian::date& _valexdate,
                bool _publish)
        : valexdate(_valexdate)
        , publish(_publish)
        {}

        boost::gregorian::date get_valexdate() const
        {
            return valexdate;
        }
        bool get_publish() const
        {
            return publish;
        }
    };
};

#endif
