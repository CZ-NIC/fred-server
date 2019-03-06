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
#include "src/util/tz/local_timestamp.hh"

namespace Tz {

LocalTimestamp::LocalTimestamp(
        const boost::posix_time::ptime& _local_time,
        ::int16_t _timezone_offset_in_minutes)
    : local_time_(_local_time),
      timezone_offset_in_minutes_(_timezone_offset_in_minutes)
{ }

namespace {

class Rfc3339
{
public:
    static int get_time_zone_offset(const std::string& _src);
    static void check_local_time_part(const std::string& _src);
private:
    static bool is_zulu_zone_indicator(char _c);
    static bool is_start_of_local_offset(char _c);
    static int get_value(char _c);
    static int get_value(const char* _str, int _length);
};

} // namespace Tz::{anonymous}

//YYYY-MM-DDThh:mm:ss[.f*][Z|+oo:oo|-oo:oo]
LocalTimestamp LocalTimestamp::from_rfc3339_formated_string(const std::string& _src)
{
    static const int length_of_the_shortest_value = std::strlen("YYYY-MM-DDThh:mm:ssZ");
    if (_src.length() < length_of_the_shortest_value)
    {
        throw NotRfc3339Compliant("string too short to represent datetime in RFC 3339 compliant format");
    }
    static const int date_time_delimiter_position = std::strlen("YYYY-MM-DD");
    switch (_src[date_time_delimiter_position])
    {
        case 'T'://preferred by RFC 3339
        case 't'://allowed by RFC 3339
        case ' '://allowed by RFC 3339
            break;
        default:
            throw NotRfc3339Compliant("unexpected date-time delimiter");
    }
    try
    {
        Rfc3339::check_local_time_part(_src);
        const std::string date_part = _src.substr(0, std::strlen("YYYY-MM-DD"));
        const std::string time_part_truncated_to_seconds = _src.substr(std::strlen("YYYY-MM-DDT"), std::strlen("hh:mm:ss"));
        const std::string date_time = date_part + " " + time_part_truncated_to_seconds;
        const boost::posix_time::ptime local_time = boost::posix_time::time_from_string(date_time);
        const ::int16_t timezone_offset_in_minutes = Rfc3339::get_time_zone_offset(_src);
        return LocalTimestamp(local_time, timezone_offset_in_minutes);
    }
    catch (const NotRfc3339Compliant&)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        throw NotRfc3339Compliant(e.what());
    }
    catch (...)
    {
        throw NotRfc3339Compliant("unexpected exception");
    }
}

LocalTimestamp LocalTimestamp::within_utc(const boost::posix_time::ptime& _src_utc_time)
{
    return LocalTimestamp(_src_utc_time, 0);
}

std::string LocalTimestamp::get_rfc3339_formated_string()const
{
    const std::string date_time_part = boost::posix_time::to_iso_extended_string(local_time_);
    std::ostringstream offset_part;
    if (timezone_offset_in_minutes_ == 0)
    {
        offset_part << "Z";
    }
    else if (timezone_offset_in_minutes_ < 0)
    {
        offset_part << "-"
                    << std::setw(2) << std::setfill('0') << (-timezone_offset_in_minutes_ / 60) << ":"
                    << std::setw(2) << std::setfill('0') << (-timezone_offset_in_minutes_ % 60);
    }
    else
    {
        offset_part << "+"
                    << std::setw(2) << std::setfill('0') << (timezone_offset_in_minutes_ / 60) << ":"
                    << std::setw(2) << std::setfill('0') << (timezone_offset_in_minutes_ % 60);
    }
    return date_time_part + offset_part.str();
}

const boost::posix_time::ptime& LocalTimestamp::get_local_time()const
{
    return local_time_;
}

::int16_t LocalTimestamp::get_timezone_offset_in_minutes()const
{
    return timezone_offset_in_minutes_;
}

namespace {

int Rfc3339::get_time_zone_offset(const std::string& _src)
{
    const int zulu_zone_position = _src.length() - std::strlen("Z");
    if (is_zulu_zone_indicator(_src[zulu_zone_position]))
    {
        static const int zulu_zone_offset = 0;
        return zulu_zone_offset;
    }
    const int local_zone_position = _src.length() - std::strlen("+00:00");
    if (!is_start_of_local_offset(_src[local_zone_position]))
    {
        throw std::invalid_argument("string does not contain any time zone information");
    }
    if (_src[local_zone_position + 3] != ':')
    {
        throw std::invalid_argument("invalid time zone hours-minutes delimiter");
    }
    const int sign = _src[local_zone_position] == '+' ? 1 : -1;
    const int offset_hours = get_value(_src.c_str() + local_zone_position + 1, 2);
    const int offset_minutes = get_value(_src.c_str() + local_zone_position + 4, 2);
    if (13 <= offset_hours)
    {
        throw std::invalid_argument("too many hours in offset");
    }
    if (60 <= offset_minutes)
    {
        throw std::invalid_argument("too many minutes in offset");
    }
    const int offset = 60 * offset_hours + offset_minutes;
    const bool the_offset_is_unknown = (sign == -1) && (offset == 0);//offset `-00:00` means "the offset to local time is unknown"
    if (the_offset_is_unknown)
    {
        throw std::invalid_argument("time zone offset is unknown");
    }
    return sign * offset;
}

void Rfc3339::check_local_time_part(const std::string& _src)
{
    //hh:mm:ss[.f*]
    static const int local_time_part_begin = std::strlen("YYYY-MM-DDT");
    if ((_src[local_time_part_begin + std::strlen("hh")] != ':') ||
        (_src[local_time_part_begin + std::strlen("hh:mm")] != ':'))
    {
        throw std::invalid_argument("invalid hours-minutes-seconds delimiter");
    }
    const int hours = get_value(_src.c_str() + local_time_part_begin + 0, 2);
    const int minutes = get_value(_src.c_str() + local_time_part_begin + 3, 2);
    const int seconds = get_value(_src.c_str() + local_time_part_begin + 6, 2);
    if (24 <= hours)
    {
        throw std::invalid_argument("too many hours");
    }
    if (60 <= minutes)
    {
        throw std::invalid_argument("too many minutes");
    }
    if (60 <= seconds)
    {
        throw std::invalid_argument("too many seconds");
    }
    const int fraction_begin = local_time_part_begin + std::strlen("hh:mm:ss");
    int fraction_end = fraction_begin;
    if (_src[fraction_begin] == '.')
    {
        ++fraction_end;
        while (('0' <= _src[fraction_end]) && (_src[fraction_end] <= '9'))
        {
            ++fraction_end;
        }
        if (fraction_end <= static_cast<int>(fraction_begin + std::strlen(".")))
        {
            throw std::invalid_argument("fraction of seconds has to contain at least one cipher");
        }
    }
    const int zone_length = _src.length() - fraction_end;
    if (zone_length == std::strlen("Z"))
    {
        return;
    }
    if ((zone_length == std::strlen("+00:00")) && !is_zulu_zone_indicator(_src[_src.length() - 1]))
    {
        return;
    }
    throw std::invalid_argument("invalid time zone offset");
}

bool Rfc3339::is_zulu_zone_indicator(char _c)
{
    switch (_c)
    {
        case 'Z':
        case 'z':
            return true;
    }
    return false;
}

bool Rfc3339::is_start_of_local_offset(char _c)
{
    switch (_c)
    {
        case '+':
        case '-':
            return true;
    }
    return false;
}

int Rfc3339::get_value(char _c)
{
    if (('0' <= _c) && (_c <= '9'))
    {
        return _c - '0';
    }
    throw std::invalid_argument("character out of range");
}

int Rfc3339::get_value(const char* _str, int _length)
{
    int value = 0;
    for (const char* const end = _str + _length; _str != end; ++_str)
    {
        value = 10 * value + get_value(*_str);
    }
    return value;
}

} // namespace Tz::{anonymous}
} // namespace Tz
