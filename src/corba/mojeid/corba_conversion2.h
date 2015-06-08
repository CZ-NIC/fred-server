#ifndef CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965

#include "src/corba/MojeID2.hh"
#include "src/corba/mojeid/corba_common_conversion2.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

template < >
struct corba_type_conversion< Registry::MojeID::Date, boost::gregorian::date >
: corba_type_conversion_base< Registry::MojeID::Date, boost::gregorian::date >
{
    static corba_type& set(corba_type &dst, const convertible_type &src)
    {
        return dst = CORBA::string_dup(boost::gregorian::to_iso_extended_string(src).c_str());
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        return dst = boost::gregorian::from_simple_string(src);
    }
};

template < >
struct corba_type_conversion< Registry::MojeID::Date, std::string >
: corba_type_conversion_base< Registry::MojeID::Date, std::string >
{
    static corba_type& set(corba_type &dst, const convertible_type &src)
    {
        enum { GREGORIAN_DATE_LENGTH = 10 };
        const std::string str_date = src.substr(0, GREGORIAN_DATE_LENGTH);
        return corba_value(dst).set(boost::gregorian::from_simple_string(str_date));
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        return dst = boost::gregorian::to_iso_extended_string(corba_value(src).get< boost::gregorian::date >());
    }
};

template < >
struct corba_type_conversion< Registry::MojeID::DateTime, boost::posix_time::ptime >
: corba_type_conversion_base< Registry::MojeID::DateTime, boost::posix_time::ptime >
{
    static corba_type& set(corba_type &dst, const convertible_type &src)
    {
        return dst = CORBA::string_dup(boost::posix_time::to_iso_extended_string(src).c_str());
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        std::string iso_extended = src;//2015-06-24T13:05:03.000123
        enum { T_POS = 10 };
        if ((T_POS <= iso_extended.length()) && (iso_extended[T_POS] == 'T')) {
            iso_extended[T_POS] = ' ';
        }
        return dst = boost::posix_time::time_from_string(iso_extended);
    }
};

#endif//CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965
