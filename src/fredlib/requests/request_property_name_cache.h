/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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

#include <map>
#include <boost/thread/mutex.hpp>

#include "db_settings.h"
#include "model_request_property_name.h"

// FRED logging
#include "log/logger.h"
#include "log/context.h"

namespace Fred {
namespace Logger {


// TODO this should be in some other header
inline void logger_notice(const char *str);
inline void logger_error(const char *str);
inline void logger_notice(boost::format &fmt);
inline void logger_error(boost::format &fmt);

inline void logger_notice(const char *str)
{
        boost::format fmt(str);
        logger_notice(fmt);
}

inline void logger_error(const char *str)
{
        boost::format fmt(str);
    logger_error(fmt);
}

inline void logger_notice(boost::format &fmt)
{
#ifdef HAVE_LOGGER
    LOGGER("fred-server").notice(fmt);
#endif
}

inline void logger_error(boost::format &fmt)
{
#ifdef HAVE_LOGGER
    LOGGER("fred-server").error(fmt);
#endif
}

using namespace Database;

class RequestPropertyNameCache {
public:
    RequestPropertyNameCache(Connection &conn);

    ID find_property_name_id(const std::string &name, Connection &conn);

    /** Limit the number of entries read from request_property_name table
     * (which is supposed to contain limited number of distinct property names )
     */

    static const unsigned int PROP_NAMES_SIZE_LIMIT;
    static const int MAX_NAME_LENGTH;

private:
    boost::mutex properties_mutex;
    std::map<std::string, Database::ID> property_names;

};

}
}
