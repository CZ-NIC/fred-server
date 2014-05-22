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
 *  registrar zone access
 */

#ifndef REGISTRAR_ZONE_ACCESS_H_
#define REGISTRAR_ZONE_ACCESS_H_

#include <string>
#include <vector>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"

namespace Fred
{

    /**
     * Registrar zone access interval container.
     */
    class RegistrarZoneAccess : public Util::Printable
    {
        std::string zone_name_;/**< fully qualified name of the zone*/
        boost::gregorian::date from_date_;/**< first day of access interval to the zone*/
        Nullable<boost::gregorian::date> to_date_;/**< last day of access interval to the zone or null if not ended*/
    public:

        /**
         * Empty destructor.
         */
        virtual ~RegistrarZoneAccess(){}

        /**
         * Constructor initializing all attributes.
         * @param zone_name sets name of the zone into @ref zoner_name_ attribute
         * @param from_date sets first day of access into @ref from_date_ attribute
         * @param to_date sets last day of access into @ref to_date_ attribute
         */
        RegistrarZoneAccess(const std::string& zone_name,
            const boost::gregorian::date& from_date,
            const Nullable<boost::gregorian::date>& to_date)
        : zone_name_(zone_name)
        , from_date_(from_date)
        , to_date_(to_date)
        {}

        /**
         * Zone name getter.
         * @return fully qualified name of the zone viz @ref zone_name_
         */
        std::string get_zone_name() const
        {
            return zone_name_;
        }

        /**
         * First day of access interval to the zone getter.
         * @return first day of access to the zone interval viz @ref from_date_
         */
        boost::gregorian::date get_from_date() const
        {
            return from_date_;
        }

        /**
         * Last day of access interval to the zone getter.
         * @return last day of access to the zone interval or null if not ended viz @ref to_date_
         */
        boost::gregorian::date get_to_date() const
        {
            return to_date_;
        }

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const
        {
            return Util::format_data_structure("RegistrarZoneAccess",
            Util::vector_of<std::pair<std::string,std::string> >
            (std::make_pair("zone_name",zone_name_))
            (std::make_pair("from_date",boost::lexical_cast<std::string>(from_date_)))
            (std::make_pair("to_date",to_date_.print_quoted()))
            );
        }
    };
}//namespace Fred

#endif//REGISTRAR_ZONE_ACCESS_H_
