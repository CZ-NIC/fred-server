/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef LOCAL_TIMESTAMP_HH_2B3D1BC865F96692A6178061F9D356C0//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define LOCAL_TIMESTAMP_HH_2B3D1BC865F96692A6178061F9D356C0

#include <boost/date_time/posix_time/posix_time.hpp>

#include <stdint.h>
#include <exception>
#include <string>

namespace Tz {

class LocalTimestamp
{
public:
    LocalTimestamp(
            const boost::posix_time::ptime& _local_time,
            ::int16_t _timezone_offset_in_minutes);

    class NotRfc3339Compliant:public std::exception
    {
    public:
        NotRfc3339Compliant(const std::string& _msg):msg_(_msg) { }
        ~NotRfc3339Compliant() { }
        const char* what()const noexcept { return msg_.c_str(); }
    private:
        const std::string msg_;
    };

    static LocalTimestamp from_rfc3339_formated_string(const std::string& _src);

    static LocalTimestamp within_utc(const boost::posix_time::ptime& _src_utc_time);

    template <typename SRC_TIMEZONE, class CONVERTOR>
    static LocalTimestamp within(const CONVERTOR& _convertor, const boost::posix_time::ptime& _src_local_time)
    {
        return _convertor.template within<SRC_TIMEZONE>(_src_local_time);
    }

    template <typename DST_TIMEZONE, class CONVERTOR>
    static LocalTimestamp into(const CONVERTOR& _convertor, const LocalTimestamp& _src)
    {
        return _convertor.template into<DST_TIMEZONE>(
                _src.local_time_,
                _src.timezone_offset_in_minutes_);
    }

    std::string get_rfc3339_formated_string()const;

    const boost::posix_time::ptime& get_local_time()const;

    ::int16_t get_timezone_offset_in_minutes()const;
private:
    boost::posix_time::ptime local_time_;
    ::int16_t timezone_offset_in_minutes_;
};

} // namespace Tz

#endif//LOCAL_TIMESTAMP_HH_2B3D1BC865F96692A6178061F9D356C0
