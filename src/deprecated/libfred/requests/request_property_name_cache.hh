/*
 * Copyright (C) 2011-2020  CZ.NIC, z. s. p. o.
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
#include <map>
#include <boost/thread/mutex.hpp>

#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/requests/model_request_property_name.hh"

// FRED logging
#include "util/log/logger.hh"
#include "util/log/context.hh"

namespace LibFred {
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
    LOGGER.notice(fmt);
#endif
}

inline void logger_error(boost::format &fmt)
{
#ifdef HAVE_LOGGER
    LOGGER.error(fmt);
#endif
}

}
}
